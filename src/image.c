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
    img->image    = NULL;
    img->button   = NULL;
    img->width    = 0;
    img->height   = 0;
    img->thumb_w  = 0;
    img->thumb_h  = 0;
    img->rotate   = 0;
    img->gamma    = 1.0;
    img->text     = g_strdup("");
    img->uri      = g_strdup("");
    img->nomodify = FALSE;
        
	return img;
}



void
image_free(struct image *img)
{
	g_assert(img != NULL);
	
	/* destroy widgets */
	if (img->button)
	{
		gtk_widget_destroy(img->button);
        /* No need to destroy img->image, since its contained in the button */
	}

	/* free other fields */
	g_free(img->text);
	g_free(img->uri);
	g_free(img);
}



struct image *
image_open(struct data *data, gchar *uri)
{

    GdkPixbufLoader  *loader;
    guchar           buf[PWGALLERY_IMG_READ_BUF_SIZE];
	struct image     *img;
    GdkColor         color, prelight;
	GnomeVFSResult   result;
	GnomeVFSHandle   *handle;
	GnomeVFSFileSize bytes;
	GError           *error = NULL;

	g_assert(data != NULL);
	g_assert(uri != NULL);

	g_debug("in image_open");

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

    loader = gdk_pixbuf_loader_new();
    g_signal_connect(loader, "size-prepared", G_CALLBACK(set_size), img);

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
			gdk_pixbuf_loader_close (loader, NULL);
			return NULL;
		}

		/* error parsing image data */
		if (gdk_pixbuf_loader_write(loader, buf, bytes, &error) == FALSE)
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

    gdk_pixbuf_loader_close(loader, NULL); /* no more writes */
	gnome_vfs_close(handle); /* ignore result */
	
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

    img->image = 
		gtk_image_new_from_pixbuf( gdk_pixbuf_loader_get_pixbuf( loader ) );

    gtk_container_add( GTK_CONTAINER( img->button ), img->image);
    gtk_container_set_border_width( GTK_CONTAINER( img->button ), 
				    PWGALLERY_THUMBNAIL_BORDER_WIDTH );

    /* Set default values for a new image */
	img->nomodify = FALSE;
    img->gamma    = 1.0;

    g_free(img->text);
    img->text     = g_strdup( _("Add text") );

    g_free(img->uri);
    img->uri      = uri;

    return img;	
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
    
    scale = (gdouble)arg1 / (gdouble)arg2;

	/* FIXME: get this from the width of the gtk_table? */
    w = PWGALLERY_THUMB_W;
    h = (gint)(w / scale);

    g_debug("in set_size: %dx%d -> %dx%d",  arg1, arg2, w, h);
    
    img->width = arg1;
    img->height = arg2;

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
