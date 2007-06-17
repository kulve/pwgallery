/*
 * Copyright (C) 2006 Tuomas Kulve <tuomas@kulve.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "main.h"
#include "image.h"
#include "gallery.h"
#include "vfs.h"
#include "magick.h"

#include <glib.h>                 /* glib */
#include <wand/magick-wand.h>     /* ImageMagick */
#include <wand/pixel-wand.h>      /* ImageMagick */
#include <libgnomevfs/gnome-vfs.h>/* gnome_vfs_get_file_info */

static gboolean _apply_modifications(struct data *data, 
                                     MagickWand *wand, 
                                     struct image *image);
static gboolean _load_image(struct data *data, 
                            MagickWand *wand, 
                            struct image *image);
static gboolean _resize(struct data *data,
                        MagickWand *wand, 
                        struct image *image,
                        gint width,
                        gint height);
static gboolean _save(struct data *data, 
                      MagickWand *wand, 
                      const gchar *uri);
static MagickWand *_generate_webimage(struct data *data, 
                                      struct image *image,
                                      gint image_h,
                                      struct image_size **img_size);


gboolean magick_make_thumbnail(struct data *data, 
                               struct image *image,
                               const gchar *uri)
{
    MagickWand *wand;
    gdouble scale;

    g_assert(data != NULL);
    g_assert(image != NULL);
    
    g_debug("in magick_make_thumbnail");

    wand = NewMagickWand();
    g_return_val_if_fail( wand, FALSE );

    /* load image from file */
    if (!_load_image(data, wand, image)) {
        DestroyMagickWand(wand);
        return FALSE;
    }

    /* apply modifications, if nomodify is not checked */
    if (!image->nomodify) {
        if (!_apply_modifications(data, wand, image)) {
            DestroyMagickWand(wand);
            return FALSE;
        }
    }
    
    /* calculate width and height for the thumbnail */
    image->thumb_w = data->gal->thumb_w;
    switch( image->rotate ) 
    {
    case 0:
    case 180:
        scale = (gdouble)image->height / (gdouble)image->width;
        break;
    case 90:
    case 270:
        scale = (gdouble)image->width / (gdouble)image->height;
        break;
        /* FIXME: just to get some values.. */
    default:
        scale = (gdouble)image->width / (gdouble)image->height;
        break;
    }
    image->thumb_h = (gint)(image->thumb_w * scale);

    /* resize the thumbnail */
    if (!_resize(data, wand, image, image->thumb_w, image->thumb_h)) {
        DestroyMagickWand(wand);
        return FALSE;
    }

    
    /* save the thumbnail to a file */
    if (!_save(data, wand, uri)) {
        DestroyMagickWand(wand);
        return FALSE;
    }
    
    DestroyMagickWand(wand);

    return TRUE;
}


gboolean magick_make_webimage(struct data *data, 
                              struct image *image,
                              const gchar *uri,
                              gint image_h)
{
    MagickWand *wand;
    struct image_size *img_size = NULL;
    GnomeVFSResult result;
    GnomeVFSFileInfo info;

    g_debug("in magick_make_webimage");

    wand = _generate_webimage(data, image, image_h, &img_size);
    if (wand == NULL) 
        return FALSE;

    /* save the image to a file */
    if (!_save(data, wand, uri)) {
        DestroyMagickWand(wand);
        g_free(img_size);
        return FALSE;
    }
    
    DestroyMagickWand(wand);

    /* get file size */
    result = gnome_vfs_get_file_info(uri, &info,
                                     GNOME_VFS_FILE_INFO_DEFAULT | 
                                     GNOME_VFS_FILE_INFO_FOLLOW_LINKS);
    if (result == GNOME_VFS_OK) {
        img_size->size = info.size / 1024;
    } else {
        img_size->size = 0;
    }
    
    image->sizes = g_slist_append(image->sizes, img_size);

    return TRUE;
}



gboolean magick_show_preview(struct data *data, 
                             struct image *image,
                             gint image_h)
{
    MagickWand *wand;
    struct image_size *img_size = NULL;
    const gchar *display;

    g_debug("in magick_show_preview");

    wand = _generate_webimage(data, image, image_h, &img_size);
    g_free(img_size);

    if (wand == NULL)
        return FALSE;

    gtk_window_iconify( GTK_WINDOW( data->top_window ) );
    while (g_main_context_iteration(NULL, FALSE));

    display = g_getenv("DISPLAY");
    MagickDisplayImage(wand, display ? display : ":0.0");

    DestroyMagickWand(wand);

    gtk_window_deiconify( GTK_WINDOW( data->top_window ) );
    while (g_main_context_iteration(NULL, FALSE));

    return TRUE;
}



/*
 *
 * Static functions
 *
 */


/*
 * Generate a webimage for preview or saving to a file.
 */
static MagickWand *_generate_webimage(struct data *data, 
                                      struct image *image,
                                      gint image_h,
                                      struct image_size **img_size)
{

    MagickWand *wand;
    gdouble scale;

    g_assert(data != NULL);
    g_assert(image != NULL);
    
    g_debug("in _generate_webimage");

    *img_size = g_new0(struct image_size, 1);

    wand = NewMagickWand();
    g_return_val_if_fail( wand, FALSE );

    /* load image from file */
    if (!_load_image(data, wand, image)) {
        DestroyMagickWand(wand);
        return FALSE;
    }

    /* apply modifications, if nomodify is not checked */
    if (!image->nomodify) {
        if (!_apply_modifications(data, wand, image)) {
            DestroyMagickWand(wand);
            g_free(*img_size);
            return NULL;
        }
        
        /* calculate width and height for the webimage */
        (*img_size)->height = image_h;
        switch( image->rotate ) 
            {
            case 90:
            case 270:
                scale = (gdouble)image->height / (gdouble)image->width;
                break;
            case 0:
            case 180:
                scale = (gdouble)image->width / (gdouble)image->height;
                break;
                /* FIXME: just to get some values.. */
            default:
                scale = (gdouble)image->width / (gdouble)image->height;
                break;
            }
        (*img_size)->width = (gint)((*img_size)->height * scale);
        
        /* resize the webimage */
        if (!_resize(data, wand, image, 
                     (*img_size)->width, (*img_size)->height)) {
            DestroyMagickWand(wand);
            g_free((*img_size));
            return NULL;
        }
    } else {
        /* no modify, just return original size */
        (*img_size)->height = image->height;
        (*img_size)->width = image->width;
   }
    
    return wand;
}

/*
 * Load image to image magic
 */
static gboolean _load_image(struct data *data, 
                            MagickWand *wand, 
                            struct image *image)
{
    gchar *desc;
    ExceptionType severity;
    guchar *img_data;
    gsize img_len;

    g_debug("in _load_image");

    g_assert(data != NULL);
    g_assert(wand != NULL);
    g_assert(image != NULL);

    /* Read image from file to memory */
    vfs_read_file(data, image->uri, &img_data, &img_len);
    
    /* Read image to image magick */
    if (!MagickReadImageBlob(wand, img_data, img_len)) {
        desc = MagickGetException(wand, &severity) ;
     
        /* FIXME: popup */
        g_warning("_load_image: error reading image: %s\n", desc);
        g_free(img_data);
        return FALSE;
    }

    g_free(img_data);

    return TRUE;
}    

/*
 * Apply image modifications (rotate, gamma, etc) to the
 * image. Resizing to web image or thumbnail is done afterwards
 */
static gboolean _apply_modifications(struct data *data, 
                                     MagickWand *wand, 
                                     struct image *image)
{
    gchar *desc;
    ExceptionType severity;

    g_debug("in _apply_modifications");

    g_assert(data != NULL);
    g_assert(wand != NULL);
    g_assert(image != NULL);
    
    /* rotate image */
    if (image->rotate) {
        PixelWand *px;

        px = NewPixelWand();
        g_assert(px);

        PixelSetColor(px, "blue");

        if( !MagickRotateImage(wand, px, image->rotate)) {
            desc = MagickGetException(wand, &severity);
            /* FIXME: popup */
            g_warning("_apply_modifications: "
                      "error rotating image: %s\n", desc);
            ClearPixelWand(px);
            MagickRelinquishMemory(desc);
            return FALSE;
        }
        DestroyPixelWand( px );
    }

    /* Apply gamma, if over changed over 0.01 */
    if (image->gamma <= 0.99 || image->gamma >= 1.01) {
        if (!MagickGammaImage(wand, image->gamma)) {
            desc = MagickGetException(wand, &severity);
            
            /* FIXME: popup */
            g_warning("_apply_modifications: "
                      "error setting gamma (%.2f) of image: %s\n",
                      image->gamma, desc);
            return FALSE;
        }
    }

    return TRUE;
}



/*
 * Resize image. 
 */
static gboolean _resize(struct data *data,
                        MagickWand *wand, 
                        struct image *image,
                        gint width,
                        gint height)
{
    gchar *desc;
    ExceptionType severity;

    g_debug("in _resize");

    g_assert(data != NULL);
    g_assert(wand != NULL);
    g_assert(image != NULL);
    
    /* CHECKME: 1.0 ok? LanczosFilter ok? */
    if (!MagickResizeImage(wand, width, height, LanczosFilter, 1.0 ) )
    {
        desc = MagickGetException(wand, &severity);

        g_warning("_resize: error resizing image: %s\n", desc);
        return FALSE;
    }

    /* FIXME: MagickUnsharpMaskImage */

    return TRUE;
}


/* save image to file */
static gboolean _save(struct data *data, 
                      MagickWand *wand, 
                      const gchar *uri)
{
    gchar *desc;
    guchar *img_data;
    gsize img_len;
    ExceptionType severity;

    g_debug("in _save");

    g_assert(data != NULL);
    g_assert(wand != NULL);
    g_assert(uri != NULL);
 
    if (data->gal->remove_exif && !MagickStripImage(wand))
    {
        /* CHECKME: should desc be freed? */
        desc = MagickGetException(wand, &severity);
        g_warning("_save: error stripping image: %s\n", desc);
    }

    img_data = MagickGetImagesBlob(wand, &img_len);

    vfs_write_file(data, uri, img_data, img_len);

    MagickRelinquishMemory(img_data);

    return TRUE;
}

/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
