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
#include "html.h"

#include <glib.h>
#include <gdk/gdkkeysyms.h>          /* GDK key symbols */
#include <gtk/gtk.h>
#include <string.h>                  /* memset, strcmp */
#include <strings.h>                 /* rindex */



static gboolean _make_thumbnails(struct data *data);
static gboolean _make_webimages(struct data *data);
static gint sort_exif_timestamp(gconstpointer a, gconstpointer b);
static gboolean ss_key(GtkWidget *widget,
                       GdkEventKey *event,
                       gpointer user_data);
static gboolean ss_motion(GtkWidget *widget,
                          GdkEventMotion *event,
                          gpointer user_data);
static gboolean ss_load_next(gpointer user_data);
static void ss_stop(struct data *data);
static void ss_restart_timer(struct data *data);
static void ss_skip_forward(struct data *data);
static void ss_skip_backward(struct data *data);



void
gallery_init(struct data *data)
{

	g_assert(data != NULL);

    g_debug("in gallery_init");

	if (data->gal != NULL)
		gallery_free(data);

	data->gal = g_new0(struct gallery, 1);

    data->current_img = NULL; /* no currently selected image */
    data->current_ss_img = NULL; /* no currently selected image for slideshow */

	data->gal->edited = FALSE;
    data->gal->images = NULL;

	/* Set default values */
	data->gal->uri            = g_strdup("");
    data->gal->name           = g_strdup("");
    data->gal->desc           = g_strdup("");
	data->gal->base_dir       = g_strdup(data->output_dir);
	data->gal->dir_name       = g_strdup("pwgallery");
	data->gal->output_dir     = g_strdup_printf("%s/%s", data->gal->base_dir,
                                                data->gal->dir_name);
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

	g_free(data->gal->base_dir);
    data->gal->base_dir = NULL;

	g_free(data->gal->dir_name);
    data->gal->dir_name = NULL;

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
    if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_ACCEPT)
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

    /* Make sure that dir_name is non-empty (to avoid e.g. trying to
       rename file://tmp */
    if (data->gal->dir_name[0] == '\0') {
        GtkWidget *label;
        dialog = gtk_dialog_new_with_buttons(_("Specify directory name!"),
                                             GTK_WINDOW(data->top_window),
                                             GTK_DIALOG_MODAL | 
                                             GTK_DIALOG_DESTROY_WITH_PARENT,
                                             GTK_STOCK_OK,
                                             GTK_RESPONSE_OK,
                                             NULL);


        label = gtk_label_new(_("You must specify directory name for the "
                                "gallery.\n"
                                "Check from menu: Gallery -> Settings.\n"));
   
        gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
                           label);
        gtk_widget_show(label);

        result = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy (dialog);
        
        return;
    }

    /* if gallery is modified, ask if it should be saved before making
     * it or cancel the making. */
    if (data->gal->edited == TRUE) {
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

        switch(result) {
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
        gchar *dir;
        int i = 1;
        do {
            dir = g_strdup_printf("%s.%d", data->gal->output_dir, i);
            if(vfs_is_dir(data, dir) == FALSE) {
                break;
            }
            g_free(dir);
            ++i;
        } while(1);

        vfs_rename(data, data->gal->output_dir, dir);
        vfs_mkdir(data, data->gal->output_dir);
        vfs_copy_htaccess(data, dir, data->gal->output_dir);
        g_free(dir);
    } else {
        vfs_mkdir(data, data->gal->output_dir);
    }

    widgets_set_progress(data, 0, _("Creating gallery"));

    /* make thumbnails */
    if (!_make_thumbnails(data)) {
        widgets_set_progress(data, 0, _("Failed!"));
        return;
    }

    /* make webimages */
    if (!_make_webimages(data)) {
        widgets_set_progress(data, 0, _("Failed!"));
        return;
    }

    /* make index page */
    if (!html_make_index_page(data)) {
        widgets_set_progress(data, 0, _("Failed!"));
        return;
    }

    /* make image pages */
    if (!html_make_image_pages(data)) {
        widgets_set_progress(data, 0, _("Failed!"));
        return;
    }

    widgets_set_progress(data, 0, _("Idle"));
}



void
gallery_sort_by_time(struct data *data)
{
    g_debug("in %s", __FUNCTION__);

    if (data && data->gal && data->gal->images) {

        /* Sort the gallery based on exif time stamps */
        data->gal->images =
            g_slist_sort(data->gal->images, sort_exif_timestamp);

        widgets_update_table(data);

    }
}



void
gallery_slide_show(struct data *data)
{
    GtkWidget *ss_image;
    struct image *img;
    GdkColor color = {0, 0, 0, 0};

    g_debug("in %s", __FUNCTION__);

    if (!data || !data->gal || !data->gal->images) {
        /* No images, notify the user */
        GtkWidget *label;
        GtkWidget *dialog;
        int result;
        dialog = gtk_dialog_new_with_buttons(_("No images to show!"),
                                             GTK_WINDOW(data->top_window),
                                             GTK_DIALOG_MODAL | 
                                             GTK_DIALOG_DESTROY_WITH_PARENT,
                                             GTK_STOCK_OK,
                                             GTK_RESPONSE_OK,
                                             NULL);


        label = gtk_label_new(_("Load a gallery before starting a slide show"));

        
        gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox),
                           label);
        gtk_widget_show(label);
        
        result = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy (dialog);
        return;
    }

    if (data->ss_window != NULL) {
        gtk_widget_destroy(data->ss_window);
        data->ss_window = NULL;
    }

    /* Create a new full screen window for the slide show */
    data->ss_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_modify_bg(data->ss_window, GTK_STATE_NORMAL, &color);
    gtk_window_fullscreen(GTK_WINDOW(data->ss_window));
    gtk_widget_add_events(data->ss_window, GDK_POINTER_MOTION_MASK); 

    /* Set fullscreen window key handler */
    g_signal_connect(data->ss_window, "key-press-event",
                     (GCallback)&ss_key, (gpointer)data);

    /* Set fullscreen window motion handler */
    g_signal_connect(data->ss_window, "motion-notify-event",
                     (GCallback)&ss_motion, (gpointer)data);

    img = data->gal->images->data;
    if (img->ss_pixbuf == NULL) {
        data->current_ss_img = img;
        if (!image_load_ss_pixbuf(data, img)) {
            return;
        }
    }

    ss_image = gtk_image_new_from_pixbuf(img->ss_pixbuf);
    
    gtk_container_add(GTK_CONTAINER(data->ss_window), ss_image);

    gtk_widget_show_all(data->ss_window);

    data->ss_timer = g_timeout_add(data->ss_timer_interval,
                                   ss_load_next, (gpointer)data);
}


void
gallery_add_new_images(struct data *data, GSList *uris)
{
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
	
    widgets_set_status(data, _("Adding images"));

	/* Add images */
	while (uris) {
		img = image_open(data, uris->data, 0);
		if (img != NULL) {
			/* update progress */
			g_snprintf(p_text, 128, "%d/%d", file_counter++, tot_files);
			widgets_set_progress(data, (gfloat)file_counter/(gfloat)tot_files,
								 p_text);
			data->gal->images = g_slist_append(data->gal->images, img);
            
            if (data->use_gui) {
                gtk_widget_show(img->image);
                gtk_widget_show(img->button);
            }
		}
		uris = uris->next;
	}
    
	g_slist_free(first);

    /* select first image, if there was no images before this addition */
    if (data->current_img == NULL) {
        if (data->gal->images != NULL) {
            data->current_img = (struct image *)(data->gal->images->data);
            /* update the image text etc shown in the main window */
            widgets_set_image_information(data, data->current_img);
        } else {
            /* nothing to do, still no images */
            return ;
        }
    }

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
		img = image_open(data, g_strdup(tmpimg->uri), tmpimg->rotate);
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

    /* on doubleclick we already have set everything on first
     * click. Now just show the web image */
    if (event->type == GDK_2BUTTON_PRESS)
    {
        widgets_set_status(data, "Showing preview..");        
        magick_show_preview(data, data->current_img,  data->gal->image_h);
        widgets_set_status(data, "Idle");
        return FALSE;
    }

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
 * Make thumbnails for the gallery.
 */
static gboolean
_make_thumbnails(struct data *data)
{
    gchar       *dir_uri;
    GSList      *images;  
    gint        tot, i;

    g_assert(data != NULL);

    g_debug("in _make_thumbnails");

    tot = g_slist_length(data->gal->images);
    i = 0;

    /* make the thumbnail directory */
    dir_uri = g_strdup_printf("%s/thumbnails", data->gal->output_dir);
    vfs_mkdir(data, dir_uri);

    /* make the thumbnails for all images in gallery */
    images = data->gal->images;
    while(images != NULL) {
        gchar *thumb_uri;
        struct image *image = images->data;
        gfloat frac;
        gchar progress[256];

        thumb_uri = g_strdup_printf("%s/%s.%s", dir_uri, 
                                    image->basefilename, image->ext);
        
        /* make the thumbnail and save it to a file */
        if (magick_make_thumbnail(data, image, thumb_uri) == FALSE) {
            g_free(thumb_uri);
            g_free(dir_uri);
            return FALSE;
        }
        g_free(thumb_uri);

        images = images->next;
        /* update status */
        ++i;
        snprintf(progress, 256, "%s: %d/%d", _("Creating thumbnails"), i, tot);
        frac = (gfloat)i/(gfloat)tot;
        g_debug("frac: %f", frac);
        widgets_set_progress(data, frac, progress);
    }
    g_free(dir_uri);

    return TRUE;
}



/*
 * Make webimages for the gallery.
 */
static gboolean
_make_webimages(struct data *data)
{
    gint image_index = 0;
    gint tot, i;

    g_assert(data != NULL);

    g_debug("in _make_webimages");

    tot = g_slist_length(data->gal->images);

    /* go through all sizes */
    while(++image_index < 5) {
        gchar       *dir_uri;
        GSList      *images;
        gint        image_h = -1;

        i = 0; /* zero image counter */

        /* FIXME: ugly. Sizes should be in a list */
        switch(image_index) {
        case 1:
            image_h = data->gal->image_h;
            break;
        case 2:
            image_h = data->gal->image_h2;
            break;
        case 3:
            image_h = data->gal->image_h3;
            break;
        case 4:
            image_h = data->gal->image_h4;
            break;
        default: 
            /* we shouldn't be here */
            g_error("_make_webimages: Too many image sizes");
        }
        
        /* make only images with specified size */
        if (image_h == 0) {
            continue;
        }

        /* make the webimage directory */
        if (image_index == 1) {
            /* default size images to "images" dir for backward compability */
            dir_uri = g_strdup_printf("%s/images", data->gal->output_dir);
        } else {
            dir_uri = g_strdup_printf("%s/images_%d", 
                                      data->gal->output_dir, image_h);
        }
        vfs_mkdir(data, dir_uri);

        /* make the webimages for all images in gallery */
        images = data->gal->images;
        while(images != NULL) {

            gchar *img_uri;
            struct image *image = images->data;
            gfloat frac;
            gchar progress[256];
            
            img_uri = g_strdup_printf("%s/%s.%s", dir_uri, image->basefilename, 
                                      image->ext);
            
            /* make the webimage and save it to a file */
            if (magick_make_webimage(data, image, img_uri, image_h) == FALSE) {
                g_free(img_uri);
                g_free(dir_uri);
                return FALSE;
            }
            g_free(img_uri);
            
            images = images->next;

            /* update status */
            ++i;
            snprintf(progress, 256, "%s %d: %d/%d", 
                     _("Creating images"), image_h, i, tot);
            frac = (gfloat)i/(gfloat)tot;
            g_debug("frac: %f", frac);
            widgets_set_progress(data, frac, progress);
        }
        g_free(dir_uri);
    }
    return TRUE;
}



/* Compare the exif timestamps */
static gint
sort_exif_timestamp(gconstpointer a, gconstpointer b)
{
    const struct image *aimg, *bimg;

    if (a == NULL || b == NULL)
    {
        return 0;
    }

    aimg = (struct image *)a;
    bimg = (struct image *)b;

    /* If either one is missing the exif timestamp, compare the file names */
    if (aimg->exif == NULL || aimg->exif->timestamp == NULL ||
        bimg->exif == NULL || bimg->exif->timestamp == NULL)
    {
        return g_str_equal(aimg->basefilename, bimg->basefilename);
    }

    /* String comparison equals to time comparison in case of exif
       timestamp format */
    return g_ascii_strcasecmp(aimg->exif->timestamp, bimg->exif->timestamp);
}



/*
 * Key press handler for the fullscreen slide show window
 */
static gboolean
ss_key(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    struct data *data;

    g_assert(user_data != NULL);
    data = user_data;

    g_debug("in %s, keyval: %d", __FUNCTION__, event->keyval);
  
    if (event->type != GDK_KEY_PRESS) {
        return FALSE;
    }

    switch (event->keyval) {

    case GDK_Escape:           /* Stop slide show on ESC or Q */
    case GDK_q:
        ss_stop(data);
        break;
    case GDK_plus:             /* Increase timer interval */
    case GDK_KP_Add:
        data->ss_timer_interval += 500;
        g_debug("in %s, new ss timer interval: %d",
                __FUNCTION__, data->ss_timer_interval);
        ss_restart_timer(data);
        break;
    case GDK_minus:             /* Decrease timer interval */
    case GDK_KP_Subtract:
        data->ss_timer_interval -= 500;
        g_debug("in %s, new ss timer interval: %d",
                __FUNCTION__, data->ss_timer_interval);
        ss_restart_timer(data);
        break;
    case GDK_Left:              /* Select previous image, restart timer */
        ss_skip_backward(data);
        ss_restart_timer(data);
        break;
    case GDK_Right:             /* Select next image, restart timer */
        ss_skip_forward(data);
        ss_restart_timer(data);
        break;
    }



    return FALSE;
}



/*
 * This is called everytime the mouse moves in the slide show window
 */
static gboolean
ss_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
    struct data *data;

    g_assert(user_data != NULL);
    data = user_data;

    g_debug("in %s", __FUNCTION__);

    if (event->type != GDK_MOTION_NOTIFY) {
        return FALSE;
    }

    ss_restart_timer(data);
    
    return FALSE;
}


/*
 * Called with certain interval to load next image in slide show
 */
static gboolean
ss_load_next(gpointer user_data)
{
    struct data *data;
    GSList *current_image;
    struct image *old_img;
    GtkWidget *ss_image;

    g_assert(user_data != NULL);
    data = user_data;

    g_debug("in %s", __FUNCTION__);
  
    current_image = g_slist_find(data->gal->images, data->current_ss_img);

    ss_image = gtk_bin_get_child(GTK_BIN(data->ss_window));

    if (current_image == NULL) {
        ss_stop(data);
        return FALSE;
    }

    /* Stop after the slide show keeping the last image showing */
    if (current_image->next == NULL) {
        return TRUE;
    }

    old_img = data->current_ss_img;
    data->current_ss_img = current_image->next->data;

    if (!image_load_ss_pixbuf(data, data->current_ss_img)) {
        ss_stop(data);
        return FALSE;
    }
    
    gtk_image_set_from_pixbuf(GTK_IMAGE(ss_image), 
                              data->current_ss_img->ss_pixbuf);

    g_object_unref(old_img->ss_pixbuf);
    old_img->ss_pixbuf = NULL;

    return TRUE;
}



/*
 * Stop slide show
 */
static void
ss_stop(struct data *data)
{
    g_assert(data != NULL);

    g_debug("in %s", __FUNCTION__);

    if (data->ss_window != NULL) {
        gtk_widget_destroy(data->ss_window);
        data->ss_window = NULL;
    }

    if (data->ss_timer != 0) {
        g_source_remove(data->ss_timer);
        data->ss_timer = 0;
    }
}



/*
 * Restart the slide show timer
 */
static void
ss_restart_timer(struct data *data)
{
    g_assert(data != NULL);

    g_debug("in %s", __FUNCTION__);

    if (data->ss_timer != 0) {
        g_source_remove(data->ss_timer);
    }
    
    data->ss_timer = g_timeout_add(data->ss_timer_interval,
                                   ss_load_next, (gpointer)data);
}



/*
 * Skip to next image in slide show
 */
static void
ss_skip_forward(struct data *data)
{
    GSList *current_image;
    struct image *old_img;
    GtkWidget *ss_image;

    g_assert(data != NULL);

    g_debug("in %s", __FUNCTION__);
  
    current_image = g_slist_find(data->gal->images, data->current_ss_img);

    ss_image = gtk_bin_get_child(GTK_BIN(data->ss_window));

    if (current_image == NULL) {
        ss_stop(data);
        return;
    }

    /* Stop after the slide show keeping the last image showing */
    if (current_image->next == NULL) {
        return;
    }

    old_img = data->current_ss_img;
    data->current_ss_img = current_image->next->data;

    if (data->current_ss_img->ss_pixbuf == NULL) {
        if (!image_load_ss_pixbuf(data, data->current_ss_img)) {
            ss_stop(data);
            return;
        }
    }
    
    gtk_image_set_from_pixbuf(GTK_IMAGE(ss_image), 
                              data->current_ss_img->ss_pixbuf);

    g_object_unref(old_img->ss_pixbuf);
    old_img->ss_pixbuf = NULL;

    return;

}



/*
 * Skip to previous image in slide show
 */
static void
ss_skip_backward(struct data *data)
{
    GSList *images, *prev_img = NULL;

    struct image *old_img;
    GtkWidget *ss_image;

    g_assert(data != NULL);

    g_debug("in %s", __FUNCTION__);
  
    /* find the previous image */
    images = data->gal->images->next;
    prev_img = data->gal->images;
    while (images) {
        if (images->data == data->current_ss_img) {
            break;
        }
        prev_img = images;
        images = images->next;
    }
    if (prev_img == NULL) {
        prev_img = data->gal->images;
    }

    ss_image = gtk_bin_get_child(GTK_BIN(data->ss_window));

    old_img = data->current_ss_img;
    data->current_ss_img = prev_img->data;

    if (data->current_ss_img->ss_pixbuf == NULL) {
        if (!image_load_ss_pixbuf(data, data->current_ss_img)) {
            ss_stop(data);
            return;
        }
    }
    
    gtk_image_set_from_pixbuf(GTK_IMAGE(ss_image), 
                              data->current_ss_img->ss_pixbuf);

    g_object_unref(old_img->ss_pixbuf);
    old_img->ss_pixbuf = NULL;

    return;

}


/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
