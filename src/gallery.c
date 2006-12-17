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
#include "magick.h"
#include "widgets.h"
#include "vfs.h"
#include "xml.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <string.h>        /* memset, strcmp */
#include <strings.h>       /* rindex */



static gboolean _make_thumbnails(struct data *data);



void
gallery_init(struct data *data)
{

	g_assert(data != NULL);

    g_debug("in gallery_init");

	if (data->gal != NULL)
		gallery_free(data);

	data->gal = g_new0(struct gallery, 1);

    data->current_img = NULL; /* no currently selected image */

	data->gal->edited = FALSE;
    data->gal->images = NULL;

	/* Set default values */
	data->gal->uri            = g_strdup("");
    data->gal->name           = g_strdup("");
    data->gal->desc           = g_strdup("");
	data->gal->output_dir     = g_strdup(data->output_dir);
    data->gal->page_gen       = data->page_gen;
    data->gal->page_gen_prog  = g_strdup(data->page_gen_prog);
	data->gal->templ_index    = g_strdup(data->templ_index);
	data->gal->templ_indeximg = g_strdup(data->templ_indeximg);
	data->gal->templ_indexgen = g_strdup(data->templ_indexgen);
	data->gal->templ_image    = g_strdup(data->templ_image);
	data->gal->templ_gen      = g_strdup(data->templ_gen);
	data->gal->thumb_w        = data->thumb_w;
	data->gal->image_h        = data->image_h;
	data->gal->image_h2       = data->image_h2;
	data->gal->image_h3       = data->image_h3;
	data->gal->image_h4       = data->image_h4;
	data->gal->remove_exif    = data->remove_exif;
	data->gal->rename         = data->rename;
}



void
gallery_free(struct data *data)
{
	GSList *list;
	struct image *img;

	g_assert(data != NULL);

	if (data->gal == NULL)
		return;

    g_debug("in gallery_free");

	/* free images */
    if (data->gal->images != NULL)
    {
        list = data->gal->images;
        while (list) 
        {
            img = list->data;
            image_free(img);
            list->data = NULL;
            list = list->next;
        }
        g_slist_free(data->gal->images);
        data->gal->images = NULL;
    }

	/* free other fields */
	g_free(data->gal->uri);
    data->gal->uri = NULL;

	g_free(data->gal->name);
    data->gal->name = NULL;

	g_free(data->gal->desc);
    data->gal->desc = NULL;

	g_free(data->gal->output_dir);
    data->gal->output_dir = NULL;

	g_free(data->gal->page_gen_prog);
    data->gal->page_gen_prog = NULL;

	g_free(data->gal->templ_index);
    data->gal->templ_index = NULL;

	g_free(data->gal->templ_image);
    data->gal->templ_image = NULL;

	g_free(data->gal->templ_indeximg);
    data->gal->templ_indeximg = NULL;

	g_free(data->gal->templ_indexgen);
    data->gal->templ_indexgen = NULL;

	g_free(data->gal->templ_gen);
    data->gal->templ_gen = NULL;

	g_free(data->gal);
	data->gal = NULL;
}



void
gallery_new(struct data *data)
{
    int result;
    GtkWidget *dialog, *label;

    g_assert(data != NULL );

    g_debug("in gallery_new");

    /* if gallery is modified, ask if it should be saved before
     * creating a new one */
    if (data->gal->edited == TRUE)
    {
        dialog = gtk_dialog_new_with_buttons(_("Save changes?"),
                                             GTK_WINDOW(data->top_window),
                                             GTK_DIALOG_MODAL | 
                                             GTK_DIALOG_DESTROY_WITH_PARENT,
                                             GTK_STOCK_CANCEL,
                                             GTK_RESPONSE_CANCEL,
                                             GTK_STOCK_NO,
                                             GTK_RESPONSE_NO,
                                             GTK_STOCK_YES,
                                             GTK_RESPONSE_YES,
                                             NULL);


        label = gtk_label_new(_("Gallery has been modified.\n"
                                "Save changes before creating "
                                "the new gallery?"));
   
        gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
                           label);
        gtk_widget_show(label);

        result = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy (dialog);

        switch(result)
        {
        case GTK_RESPONSE_CANCEL:
            return;
        case GTK_RESPONSE_YES:
            gallery_save(data);
            break;
        default: /* do nothing on NO response */
            break;
        }
    }

    gallery_free(data);
    gallery_init(data);
}



void
gallery_open(struct data *data)
{
    GtkWidget *dialog;
    int result;
    gchar *uri;
    guchar *xml_content;
    gsize xml_content_len;

    g_assert(data != NULL );

    g_debug("in gallery_open");

    /* if gallery is modified, ask if it should be saved before
     * opening a different one */
    if (data->gal->edited == TRUE)
    {
        GtkWidget *label;
        dialog = gtk_dialog_new_with_buttons(_("Save changes?"),
                                             GTK_WINDOW(data->top_window),
                                             GTK_DIALOG_MODAL | 
                                             GTK_DIALOG_DESTROY_WITH_PARENT,
                                             GTK_STOCK_CANCEL,
                                             GTK_RESPONSE_CANCEL,
                                             GTK_STOCK_NO,
                                             GTK_RESPONSE_NO,
                                             GTK_STOCK_YES,
                                             GTK_RESPONSE_YES,
                                             NULL);


        label = gtk_label_new(_("Gallery has been modified.\n"
                                "Save changes before opening "
                                "another gallery?"));
   
        gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
                           label);
        gtk_widget_show(label);

        result = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy (dialog);

        switch(result)
        {
        case GTK_RESPONSE_CANCEL:
            return;
        case GTK_RESPONSE_YES:
            gallery_save(data);
            break;
        default: /* do nothing on no-response */
            break;
        }
    }

    gallery_free(data);
    gallery_init(data);

    dialog = gtk_file_chooser_dialog_new(_("Open Gallery"),
                                         GTK_WINDOW(data->top_window),
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                         NULL);

    gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), 
                                            data->gal_dir);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), FALSE);

    /* cancel pressed, destroy the dialog and return */
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_CANCEL)
    {
        gtk_widget_destroy (dialog);
        return;
    }

    uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));
    g_assert(uri != NULL); /* FIXME: better error handling? */

    gtk_widget_destroy (dialog);

    g_free(data->gal->uri);
    data->gal->uri = uri;

    /* read the gallery to memory */
    vfs_read_file(data, data->gal->uri, &xml_content, &xml_content_len);

    /* create a new gallery structure based on xml file */
    xml_gal_parse(data, xml_content, xml_content_len);
    g_free(xml_content);

    /* update the image text etc shown in the main window */
    widgets_set_image_information(data, data->current_img);

    /* no need to save an gallery that is just opened */
	data->gal->edited = FALSE;

}



void
gallery_save(struct data *data)
{
    guchar *xml_content;
    gsize xml_content_size;

    g_assert(data != NULL );

    g_debug("in gallery_save");

    /* if uri not set yet, call "save as", and it will call this function
     * again with the uri set.. */
    if (strlen(data->gal->uri) == 0)
    {
        gallery_save_as(data);
        return;
    }

    xml_content = xml_gal_write(data, &xml_content_size);

    vfs_write_file(data, data->gal->uri, xml_content, xml_content_size);

    /* not edited anymore */
    data->gal->edited = FALSE;
}



void
gallery_save_as(struct data *data)
{
    GtkWidget *dialog;
    gchar *uri;

    g_assert(data != NULL );

    g_debug("in gallery_save_as");

    dialog = gtk_file_chooser_dialog_new(_("Save Gallery As"),
                                         GTK_WINDOW(data->top_window),
                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                         NULL);

    /* FIXME: gtk_file_chooser_set_do_overwrite_confirmation ()? */
    gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), 
                                            data->gal_dir);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), FALSE);

    /* cancel, destroy dialog and return */
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_CANCEL)
    {
        gtk_widget_destroy (dialog);
        return;
    }

    uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));
    g_assert(uri != NULL); /* CHECKME: better error handling? */

    gtk_widget_destroy (dialog);

    g_free(data->gal->uri);


    /* add .xml if it is not written there by the user */
    if (g_str_has_suffix(uri, ".xml") == FALSE)
        data->gal->uri = g_strdup_printf("%s.xml", uri);
    else
        data->gal->uri = g_strdup(uri);

    g_free(uri);

    /* FIXME: check for existing file */

    /* now we have the new uri, save the gallery */
    gallery_save(data);
}



void
gallery_make(struct data *data)
{
    GtkWidget *dialog;
    gint result;

    g_assert(data != NULL );

    g_debug("in gallery_make");

    /* if gallery is modified, ask if it should be saved before making
     * it or cancel the making. */
    if (data->gal->edited == TRUE)
    {
        GtkWidget *label;
        dialog = gtk_dialog_new_with_buttons(_("Save changes?"),
                                             GTK_WINDOW(data->top_window),
                                             GTK_DIALOG_MODAL | 
                                             GTK_DIALOG_DESTROY_WITH_PARENT,
                                             GTK_STOCK_CANCEL,
                                             GTK_RESPONSE_CANCEL,
                                             GTK_STOCK_SAVE,
                                             GTK_RESPONSE_YES,
                                             NULL);


        label = gtk_label_new(_("Gallery has been modified.\n"
                                "Changes must be saved before continuing.\n"));
   
        gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
                           label);
        gtk_widget_show(label);

        result = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy (dialog);

        switch(result)
        {
        case GTK_RESPONSE_CANCEL:
            return;
        case GTK_RESPONSE_YES:
            gallery_save(data);
            break;
        default: /* There are only cancel and yes responses.. */
            g_assert(421 == 0);
        }
    }

    /* make top level directory for gallery */
    if (vfs_is_dir(data, data->gal->output_dir) == TRUE) {
        /* FIXME: move */
        g_error("move not implemented");
        g_assert(435 == 0);
    }
    vfs_mkdir(data, data->gal->output_dir);

    /* make thumbnails */
    _make_thumbnails(data);

    /* make webimages */

    /* make index page */
    
    /* make image pages */
}



void
gallery_add_new_images(struct data *data, GSList *uris)
{
	GtkWidget    *pbar;
	struct image *img;
	GSList       *first;
	gint         tot_files, file_counter;
	gchar        p_text[128];

	g_assert(data != NULL);
	g_assert(uris != NULL);

    g_debug("in gallery_add_new_images");

	first = uris;
	tot_files = g_slist_length(uris); /* number of images to add */
	file_counter = 0;
	
	pbar = glade_xml_get_widget( data->glade, "progressbar_status");
	g_assert(pbar != NULL);

	widgets_set_status(data, _("Adding images"));

	/* Add images */
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

            while (g_main_context_iteration(NULL, FALSE));
		}
		uris = uris->next;
	}

	g_slist_free(first);

    /* select first image, if there was no images before this addition */
    if (data->current_img == NULL)
        data->current_img = (struct image *)(data->gal->images->data);

    widgets_update_table(data);
    
    /* total files now in the gallery */
	tot_files = g_slist_length(data->gal->images);

	/* set progress and status */
	widgets_set_progress(data, 0, _("Idle"));
	g_snprintf(p_text, 128, "%d %s", tot_files, 
               tot_files == 1 ? _("Image") : _("Images"));
	widgets_set_status(data, p_text);

    /* gallery edited */
    data->gal->edited = TRUE;

}



void
gallery_open_images(struct data *data, GSList *imgs)
{
	GtkWidget    *pbar;
	struct image *img, *tmpimg;
	GSList       *first;
	gint         tot_files, file_counter;
	gchar        p_text[128];

	g_assert(data != NULL);
    /* imgs can be null */

    g_debug("in gallery_open_images");

	first = imgs;
	tot_files = g_slist_length(imgs); /* number of images to open */
	file_counter = 0;
	
	pbar = glade_xml_get_widget( data->glade, "progressbar_status");
	g_assert(pbar != NULL);

	widgets_set_status(data, _("Opening images"));

	/* Open images */
	while (imgs)
	{
        tmpimg = imgs->data;
		img = image_open(data, g_strdup(tmpimg->uri));
		if (img != NULL)
		{
			/* update progress */
			g_snprintf(p_text, 128, "%d/%d", file_counter++, tot_files);
			widgets_set_progress(data, (gfloat)file_counter/(gfloat)tot_files,
								 p_text);
			data->gal->images = g_slist_append(data->gal->images, img);

			gtk_widget_show( img->image );
			gtk_widget_show( img->button );

            /* set image values from */
            g_free(img->text);
            img->text     = g_strdup(tmpimg->text);
            img->gamma    = tmpimg->gamma;
            img->rotate   = tmpimg->rotate;
            img->nomodify = tmpimg->nomodify;
            
            while (g_main_context_iteration(NULL, FALSE));
		}
        image_free(tmpimg);
		imgs = imgs->next;
	}

	g_slist_free(first);

    /* select first image of the gallery */
    data->current_img = (struct image *)(data->gal->images->data);

	widgets_update_table(data);
    
    /* total files now in the gallery */
	tot_files = g_slist_length(data->gal->images);

	/* set progress and status */
	widgets_set_progress(data, 0, _("Idle"));
	g_snprintf(p_text, 128, "%d %s", tot_files, 
               tot_files == 1 ? _("Image") : _("Images"));
	widgets_set_status(data, p_text);

    /* data->gal->edited is set in gallery_open() */
}



void
gallery_remove_image(struct data *data, struct image *img)
{
    gint current_no;
    gint tot_files;
 	gchar p_text[128];
    GSList *tmplist;

	g_assert(data != NULL);
	g_assert(img != NULL);

    g_debug("in gallery_remove_image");

    /* get the index number of the currently selected image */
    current_no = g_slist_index(data->gal->images, img);
    g_assert(current_no > -1);

    /* free image and remove it from the list */
    data->gal->images = g_slist_remove(data->gal->images, img);
    image_free(img);

    /* set currently selected image to the next one of the deleted image */
    tmplist = g_slist_nth(data->gal->images, current_no);

    /* select the last image if deleted one was the last one */
    if (tmplist != NULL)
    {
        data->current_img = tmplist->data;
    }
    else
    {
        tmplist = g_slist_last(data->gal->images);
        if (tmplist != NULL)
            data->current_img = tmplist->data;
        else
            data->current_img = NULL;
    }

    /* update the thumbnail list */
	widgets_update_table(data);

   /* total files now in the gallery */
	tot_files = g_slist_length(data->gal->images);

    /* update the image text etc shown in the main window */
    widgets_set_image_information(data, data->current_img);

	/* set progress and status */
	g_snprintf(p_text, 128, "%d %s", tot_files, 
               tot_files == 1 ? _("Image") : _("Images"));
	widgets_set_status(data, p_text);

    data->gal->edited = TRUE;
}



gboolean
gallery_image_selected(GtkWidget *widget,
                       GdkEventButton *event,
                       gpointer user_data)
{
    GSList *imgs;
    struct data *data;
    struct image *img = NULL;

	g_assert(widget != NULL);
	g_assert(event != NULL);
	g_assert(user_data != NULL);

    g_debug("in gallery_image_selected");

    data = user_data;

    /* Find out which button was pressed by going through all image
     * buttons and matching pointer address.
     * CHECKME: This should be probably implemented in some more sane way 
     */
    imgs = data->gal->images;
    while (imgs)
    {
        img = imgs->data;
        if ((GtkWidget*)(img->button) == widget)
            break;
        imgs = imgs->next;
    }

    /* the image must be found since it was clicked.. */
    g_assert(img != NULL);

    /* save previously selected image's (if any) text */
    gallery_image_save_text(data);

    data->current_img = img;
    
    /* update the image text etc shown in the main window */
    widgets_set_image_information(data, data->current_img);

    return FALSE; /* let others handle the click too */
}



void
gallery_image_save_text(struct data *data)
{
    gchar *new_text;
    g_assert(data != NULL);

    g_debug("in gallery_image_save_text");

    /* nothing to save if no images */
    if (data->current_img == NULL)
        return;

    new_text = widgets_image_get_text(data);

    /* if the text is not changed */
    if (strcmp(new_text, data->current_img->text) == 0)
    {
        g_free(new_text);
        return;
    }

    /* replace old text with the new one and mark gallery as edited */
    g_free(data->current_img->text);
    data->current_img->text = new_text;
    data->gal->edited = TRUE;
}


/*
 *
 * Static functions
 *
 */

/*
 * Make thumbnail files for the gallery.
 */
static gboolean
_make_thumbnails(struct data *data)
{
    gchar       *dir_uri;
    GSList      *images;  

    g_assert(data != NULL);

    g_debug("in _make_thumbnails");

    /* make the thumbnail directory */
    dir_uri = g_strdup_printf("%s/thumbnails", data->gal->output_dir);
    vfs_mkdir(data, dir_uri);
    g_free(dir_uri);

    /* make the thumbnails for all images in gallery */
    images = data->gal->images;
    while(images != NULL) {
        gchar *thumb_uri, *filename, *tmpp, *basefilename;
        struct image *image = images->data;

        /* get filename without externsion */
        filename = g_strdup(image->uri);
        tmpp = rindex(filename, '.'); /* last dot to \0 */
        if (tmpp != NULL) {
            *tmpp = '\0';
        }
        tmpp = rindex(filename, '/'); /* last / to starting point */
        if (tmpp != NULL) {
            basefilename = tmpp;
        }
        
        thumb_uri = g_strdup_printf("%s/thumbnails/%s.jpg", 
                                    data->gal->output_dir, basefilename);
        
        /* make the thumbnail and save it to a file */
        if (magick_make_thumbnail(data, image, thumb_uri) == FALSE) {
            g_free(thumb_uri);
            g_free(filename);
            return FALSE;
        }
        g_free(thumb_uri);
        g_free(filename);

        images = images->next;
    }

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
