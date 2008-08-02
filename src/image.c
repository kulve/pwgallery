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
#include "exif.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <libgnomevfs/gnome-vfs.h>
#include <string.h>       /* memset */


static void set_size( GdkPixbufLoader *gdkpixbufloader, 
					  gint arg1, gint arg2, gpointer data);


struct image *
image_init(struct data *data)
{
	struct image *img;

	g_assert(data != NULL);

	img = g_new0(struct image, 1);

    /* initialize values */
    img->image        = NULL;
    img->button       = NULL;
    img->sizes        = NULL;
    img->width        = 0;
    img->height       = 0;
    img->thumb_w      = 0;
    img->thumb_h      = 0;
    img->rotate       = 0;
    img->gamma        = 1.0;
    img->text         = g_strdup("");
    img->uri          = g_strdup("");
    img->basefilename = g_strdup("");
    img->ext          = g_strdup("");
    img->nomodify     = FALSE;
    img->exif         = exif_new();

	return img;
}



void
image_free(struct image *img)
{
    GSList *list;

	g_assert(img != NULL);
	
	/* destroy widgets */
	if (img->button)
	{
		gtk_widget_destroy(img->button);
        /* No need to destroy img->image, since its contained in the button */
	}

    /* free list of image sizes */
    list = img->sizes;
    while(list) {
        g_free(list->data);
        list = g_slist_delete_link(list, list);
    }

	/* free other fields */
	g_free(img->text);
	g_free(img->uri);
    g_free(img->basefilename);
    g_free(img->ext);
    exif_free(img->exif);
	g_free(img);
}



struct image *
image_open(struct data *data, gchar *uri, gint rotate)
{

    GdkPixbufLoader  *loader;
    guchar           buf[PWGALLERY_IMG_READ_BUF_SIZE];
	struct image     *img;
    GdkColor         color, prelight;
	GnomeVFSResult   result;
	GnomeVFSHandle   *handle;
	GnomeVFSFileSize bytes;
	GError           *error = NULL;
    gchar            *tmpp;

	g_assert(data != NULL);
	g_assert(uri != NULL);

	g_debug("in image_open");

    /* check if the file is image */
    if (vfs_is_image(data, uri) == FALSE) {
        return NULL;
    }

	/* open image */
	result = gnome_vfs_open_uri(&handle, gnome_vfs_uri_new(uri),
								GNOME_VFS_OPEN_READ);
	if (result != GNOME_VFS_OK)
	{
		/* FIXME: popup */
		g_warning("Skipping image because of error opening '%s': %s", uri, 
				  gnome_vfs_result_to_string(result));
		/* FIXME: show invalid image? */
		return NULL;
	}

	img = image_init(data);

    g_free(img->uri);
    img->uri = uri;

    /* load exif data and set rotation */
    exif_data_get(data, img);
    img->rotate = img->exif->orientation;
    if (img->rotate == 0 && rotate != 0)
    {
        img->rotate = rotate;
    }

    if (data->use_gui) {
        loader = gdk_pixbuf_loader_new();
        g_signal_connect(loader, "size-prepared", G_CALLBACK(set_size), img);
    }

	/* read image from the file */
	while (TRUE)
	{
		result = gnome_vfs_read(handle, buf,
								PWGALLERY_IMG_READ_BUF_SIZE, &bytes);

		/* all read */
		if (result == GNOME_VFS_ERROR_EOF)
			break;
		
		/* error reading */
		if (result != GNOME_VFS_OK)
		{
			/* FIXME: popup */
			g_warning("Skipping image because of read error '%s': %s", uri, 
					  gnome_vfs_result_to_string(result));
			image_free(img);
            if (data->use_gui) {
                gdk_pixbuf_loader_close (loader, NULL);
            }
			return NULL;
		}

		/* error parsing image data */
		if (data->use_gui && 
            gdk_pixbuf_loader_write(loader, buf, bytes, &error) == FALSE)
		{
			gdk_pixbuf_loader_close (loader, NULL);
			/* FIXME: popup */
			g_warning("Skipping image because of parse error '%s': %s", uri,
					  error->message);
			g_error_free(error);
			image_free(img);
			return NULL;
		}
		
	}
    
	gnome_vfs_close(handle); /* ignore result */

    if (data->use_gui) {
        gdk_pixbuf_loader_close(loader, NULL); /* no more writes */	

        /* create button and set colors */
        img->button = gtk_button_new();
        g_object_ref(img->button);

        color.pixel = 0;
        color.red   = 10000;
        color.green = 20000;
        color.blue  = 50000;
        prelight.pixel = 0;
        prelight.red   = 15000;
        prelight.green = 15000;
        prelight.blue  = 37500;
        
        gtk_widget_modify_bg( img->button, GTK_STATE_NORMAL, &color);
        gtk_widget_modify_bg( img->button, GTK_STATE_SELECTED, &color);
        gtk_widget_modify_bg( img->button, GTK_STATE_PRELIGHT, &prelight );
        
        g_signal_connect( img->button, "button_press_event", 
                          G_CALLBACK( gallery_image_selected ), data );
        
        if (img->rotate == 90 || img->rotate == 270)
            {
                GdkPixbuf *pix, *pix_rotated;
                GdkPixbufRotation rot;
                
                if (img->rotate == 90)
                    rot = GDK_PIXBUF_ROTATE_CLOCKWISE;
                else
                    rot = GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE;
                
                pix = gdk_pixbuf_loader_get_pixbuf(loader);
                pix_rotated = gdk_pixbuf_rotate_simple(pix, rot);
                
                img->image = gtk_image_new_from_pixbuf(pix_rotated);
            } else {
                img->image = gtk_image_new_from_pixbuf
                    (gdk_pixbuf_loader_get_pixbuf(loader));
            }
        
        gtk_container_add( GTK_CONTAINER( img->button ), img->image);
        gtk_container_set_border_width( GTK_CONTAINER( img->button ), 
                                        PWGALLERY_THUMBNAIL_BORDER_WIDTH );
    }

    /* Set default values for a new image */
	img->nomodify = FALSE;
    img->gamma = 1.0;

    g_free(img->text);
    img->text = g_strdup( _("") );

    /* get basename of the file without extension and just the extension  */
    g_free(img->basefilename);
    g_free(img->ext);
    img->basefilename = g_path_get_basename(img->uri);
    tmpp = rindex(img->basefilename, '.');
    if (tmpp != NULL) {
        img->ext = g_strdup(tmpp + 1);
        *tmpp = '\0';
    } else {
        /* let's do JPGs if failed to check to real extension */
        img->ext = g_strdup("jpg");
    }

    return img;	
}


gboolean
image_is_edited(struct data *data, struct image *img)
{
    gchar *urimatch;
    gboolean is_edited;

	g_assert(data != NULL);
	g_assert(img != NULL);

	g_debug("in image_is_edited");

    urimatch = g_strdup_printf("/edited/%s.%s", img->basefilename, img->ext);

    is_edited = g_str_has_suffix(img->uri, urimatch);
    g_free(urimatch);

    return is_edited;
}



/**********************
 *                    *
 * Static functions   *
 *                    *
 **********************/


static void 
set_size( GdkPixbufLoader *gdkpixbufloader, gint arg1, gint arg2, gpointer data)
{
    gint         w, h;
    gdouble      scale;
    struct image *img;

	g_assert( data != NULL );

    g_debug( "in set_size" );

    img = data;
    
    if (img->rotate == 90 || img->rotate == 270)
    {
        scale = (gdouble)arg2 / (gdouble)arg1;
        
        /* FIXME: get this from the width of the gtk_table? */
        h = PWGALLERY_THUMB_W;
        w = (gint)(h / scale);

        g_debug("in set_size: %dx%d -> %dx%d",  arg1, arg2, w, h);
        
        img->width = arg1;
        img->height = arg2;
    } else {
        scale = (gdouble)arg1 / (gdouble)arg2;
        
        /* FIXME: get this from the width of the gtk_table? */
        w = PWGALLERY_THUMB_W;
        h = (gint)(w / scale);

        g_debug("in set_size: %dx%d -> %dx%d",  arg1, arg2, w, h);
        
        img->width = arg1;
        img->height = arg2;
    }

    gdk_pixbuf_loader_set_size(gdkpixbufloader, w, h);

}







/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
