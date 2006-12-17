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

static gboolean _apply_modifications(struct data *data, 
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
        scale = (double)image->height / (double)image->width;
        break;
    case 90:
    case 270:
        scale = (double)image->width / (double)image->height;
        break;
        /* FIXME: just to get some values.. */
    default:
        scale = (double)image->width / (double)image->height;
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



/*
 *
 * Static functions
 *
 */

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
    guchar *img_data;
    gsize img_len;

    g_debug("in _apply_modifications");

    g_assert(data != NULL);
    g_assert(wand != NULL);
    g_assert(image != NULL);

    /* Read image from file to memory */
    vfs_read_file(data, image->uri, &img_data, &img_len);
    
    /* Read image to image magick */
    if (!MagickReadImageBlob(wand, img_data, img_len)) {
        desc = MagickGetException(wand, &severity) ;
     
        /* FIXME: popup */
        g_warning("_apply_modifications: error reading image: %s\n", desc);
        g_free(img_data);
        return FALSE;
    }
    g_free(img_data);
    
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
    guchar *img_data;
    gsize img_len;

    g_debug("in _save");

    g_assert(data != NULL);
    g_assert(wand != NULL);
    g_assert(uri != NULL);
 
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
