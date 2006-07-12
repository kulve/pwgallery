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
#include "gallery.h"
#include "image.h"
#include "widgets.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <string.h>       /* memset */


void
gallery_init(struct data *data)
{

	g_assert(data != NULL);

	if (data->gal)
		gallery_free(data);

	data->gal = g_new0(struct gallery, 1);

	data->gal->edited = TRUE;

	/* Set default values */
	data->gal->output_dir = data->output_dir;
	data->gal->thumb_w = data->thumb_w;
	data->gal->image_h = data->image_h;
	data->gal->templ_index = g_strdup(data->templ_index);
	data->gal->templ_indeximg = g_strdup(data->templ_indeximg);
	data->gal->templ_image = g_strdup(data->templ_image);
	data->gal->remove_exif = data->remove_exif;
	data->gal->rename = data->rename;

}



void
gallery_free(struct data *data)
{
	GSList *list;
	struct image *img;

	g_assert(data != NULL);

	if (data->gal == NULL)
		return;

	/* free images */
	list = data->gal->images;
	while (list) 
	{
		img = list->data;
		image_free(img);
		list->data = NULL;
		list = list->next;
	}
	g_slist_free(list);

	/* free other fields */
	g_free(data->gal->gal_name);
	g_free(data->gal->output_dir);
	g_free(data->gal->templ_index);
	g_free(data->gal->templ_image);

	g_free(data->gal);
	data->gal = NULL;
}



void
gallery_add_images(struct data *data, GSList *uris)
{
	GtkWidget    *pbar;
	struct image *img;
	GSList       *first;
	gint         tot_files, file_counter;
	gchar        p_text[128];

	g_assert(data != NULL);
	g_assert(uris != NULL);

	first = uris;
	tot_files = g_slist_length(uris);
	file_counter = 0;
	
	pbar = glade_xml_get_widget( data->glade, "progressbar_status");
	g_assert(pbar != NULL);

	widgets_set_status(data, _("Loading images"));

	/* Load images */
	while (uris)
	{
		img = image_open(data, uris->data);
		if (img != NULL)
		{
			/* update progress */
			g_snprintf(p_text, 128, "%d/%d", file_counter++, tot_files);
			widgets_set_progress(data, (gfloat)file_counter/(gfloat)tot_files,
								 p_text);
			data->gal->images = g_slist_append(data->gal->images, img);

			gtk_widget_show( img->image );
			gtk_widget_show( img->button );


		}
		uris = uris->next;
	}

	g_slist_free(first);

	widgets_update_table(data);

	/* set progress and status */
	widgets_set_progress(data, 0, _("Idle"));
	g_snprintf(p_text, 128, "%d %s", tot_files, _("Images"));
	widgets_set_status(data, p_text);
}


/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
