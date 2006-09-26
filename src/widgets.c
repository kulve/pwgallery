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
#include "widgets.h"
#include "configrc.h"

#include <glib.h>
#include <gtk/gtk.h>

void
widgets_update_table(struct data *data) 
{
    gint             listlen;
    gint             i;
    GSList           *list;
	static GtkWidget *table = NULL;

    g_assert(data);
    
    if (table == NULL)
		table = glade_xml_get_widget( data->glade, "table_thumb");
	g_assert(table != NULL);

    listlen = g_slist_length( data->gal->images );
    g_debug("in widgets_update_table, listlen: %d", listlen);
    
    if (listlen == 0)
        return; /* nothing to todo */

    gtk_widget_hide(GTK_WIDGET(table));
    gtk_table_resize(GTK_TABLE(table), listlen, 1);

    list = data->gal->images;

    for( i = 0; i < listlen; ++i )
    {
        struct image *img;

        img = list->data;

        /* remove button from gtktable before adding it again */
        if( gtk_widget_get_parent(img->button) != NULL)
            gtk_container_remove(GTK_CONTAINER(table), img->button);
        
        gtk_table_attach_defaults(GTK_TABLE(table), img->button,
                                  0, 1, i, i + 1);
        
        list = list->next;
    }

    gtk_widget_show(table);
}



void
widgets_set_progress(struct data *data, gfloat fraction, const gchar *text)
{
	static GtkWidget *pbar = NULL;

	g_assert(data != NULL);
	g_assert(text != NULL);

	if (pbar == NULL)
		pbar = glade_xml_get_widget( data->glade, "progressbar_status");
	g_assert(pbar != NULL);

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbar), fraction);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbar), text);
}



void
widgets_set_status(struct data *data, const gchar *text)
{
	static GtkWidget *label = NULL;

	g_assert(data != NULL);
	g_assert(text != NULL);

	if (label == NULL)
		label = glade_xml_get_widget( data->glade, "label_pwg_status");
	g_assert(label != NULL);

	gtk_label_set_text(GTK_LABEL(label), text);
}



void
widgets_prefs_show(struct data *data)
{
	GtkWidget *dialog;
    GtkWidget *filechooserbutton_pref_img_dir;
    GtkWidget *filechooserbutton_pref_output_dir;
    GtkWidget *filechooserbutton_pref_gal_dir;
    GtkWidget *radiobutton_pref_gen_templ;
    GtkWidget *radiobutton_pref_gen_prog;
    GtkWidget *filechooserbutton_pref_page_gen_prog;
    GtkWidget *filechooserbutton_pref_templ_dir;
    GtkWidget *filechooserbutton_pref_templ_index;
    GtkWidget *filechooserbutton_pref_templ_indeximg;
    GtkWidget *filechooserbutton_pref_templ_indexgen;
    GtkWidget *filechooserbutton_pref_templ_image;
    GtkWidget *filechooserbutton_pref_templ_gen;
    GtkWidget *spinbutton_pref_thumb_w;
    GtkWidget *spinbutton_pref_image_h;
    GtkWidget *spinbutton_pref_image_h2;
    GtkWidget *spinbutton_pref_image_h3;
    GtkWidget *spinbutton_pref_image_h4;
    GtkWidget *togglebutton_pref_hideexif;
    GtkWidget *togglebutton_pref_rename;
    gint result;

	g_assert(data != NULL);

    /* Get widgets */
    dialog = glade_xml_get_widget( data->glade, "dialog_pref");
    filechooserbutton_pref_img_dir = 
        glade_xml_get_widget( data->glade, "filechooserbutton_pref_img_dir");
    filechooserbutton_pref_output_dir = 
        glade_xml_get_widget( data->glade, "filechooserbutton_pref_output_dir");
    filechooserbutton_pref_gal_dir = 
        glade_xml_get_widget( data->glade,
                              "filechooserbutton_pref_gal_dir");
    filechooserbutton_pref_templ_dir =
        glade_xml_get_widget( data->glade,
                              "filechooserbutton_pref_templ_dir");
    filechooserbutton_pref_templ_image =
        glade_xml_get_widget( data->glade,
                              "filechooserbutton_pref_templ_image");
    filechooserbutton_pref_templ_indeximg = 
        glade_xml_get_widget( data->glade,
                              "filechooserbutton_pref_templ_indeximg");
    filechooserbutton_pref_templ_indexgen = 
        glade_xml_get_widget( data->glade,
                              "filechooserbutton_pref_templ_indexgen");
    filechooserbutton_pref_templ_index =
        glade_xml_get_widget( data->glade,
                              "filechooserbutton_pref_templ_index");
    filechooserbutton_pref_templ_gen =
        glade_xml_get_widget( data->glade,
                              "filechooserbutton_pref_templ_gen");
    spinbutton_pref_thumb_w = 
        glade_xml_get_widget( data->glade, "spinbutton_pref_thumb_w");
    spinbutton_pref_image_h = 
        glade_xml_get_widget( data->glade, "spinbutton_pref_image_h");
    spinbutton_pref_image_h2 = 
        glade_xml_get_widget(data->glade, "spinbutton_pref_image_h2");
    spinbutton_pref_image_h3 = 
        glade_xml_get_widget(data->glade, "spinbutton_pref_image_h3");
    spinbutton_pref_image_h4 = 
        glade_xml_get_widget(data->glade, "spinbutton_pref_image_h4");
    radiobutton_pref_gen_templ = 
        glade_xml_get_widget(data->glade, "radiobutton_pref_gen_templ");
    radiobutton_pref_gen_prog = 
        glade_xml_get_widget(data->glade, "radiobutton_pref_gen_prog");
    togglebutton_pref_hideexif = 
        glade_xml_get_widget( data->glade, "togglebutton_pref_hideexif");
    togglebutton_pref_rename = 
        glade_xml_get_widget( data->glade, "togglebutton_pref_rename");

    /* Set values */
    gtk_file_chooser_set_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_img_dir),
        data->img_dir);
    gtk_file_chooser_set_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_output_dir),
        data->output_dir);
    gtk_file_chooser_set_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_gal_dir),
        data->gal_dir);
    gtk_file_chooser_set_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_templ_dir),
        data->templ_dir);
    gtk_file_chooser_set_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_templ_index), 
        data->templ_index);
    gtk_file_chooser_set_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_templ_indeximg),
        data->templ_indeximg);
    gtk_file_chooser_set_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_templ_indexgen),
        data->templ_indexgen);
    gtk_file_chooser_set_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_templ_image),
        data->templ_image);
    gtk_file_chooser_set_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_templ_gen),
        data->templ_gen);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_pref_thumb_w),
                              (gdouble)data->thumb_w);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_pref_image_h),
                              (gdouble)data->image_h);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_pref_image_h2),
                              (gdouble)data->image_h2);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_pref_image_h3),
                              (gdouble)data->image_h3);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_pref_image_h4),
                              (gdouble)data->image_h4);
     radiobutton_pref_gen_templ = 
        glade_xml_get_widget(data->glade, "radiobutton_pref_gen_templ");
    radiobutton_pref_gen_prog = 
        glade_xml_get_widget(data->glade, "radiobutton_pref_gen_prog");

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(togglebutton_pref_hideexif),
                                 data->remove_exif);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(togglebutton_pref_rename),
                                 data->rename);

    /* show dialog (in a loop because of help dialog) */
    do
    {
        result = gtk_dialog_run(GTK_DIALOG(dialog));    
        switch(result)
        {
        case GTK_RESPONSE_DELETE_EVENT:
        case GTK_RESPONSE_CANCEL: /* hide dialog, save nothing, return */
            gtk_widget_hide(dialog);
            return;
        case GTK_RESPONSE_HELP:   /* show help */
            widgets_help_show(data, "Help for preferences window.");
            break;
        default:                  /* ok pressed, hide dialog */
            gtk_widget_hide(dialog);
        }
    } while(result != GTK_RESPONSE_OK);
    
    /* save button pressed */

    /* get values */
    g_free(data->img_dir);
    data->img_dir = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_img_dir));
    g_assert(data->img_dir != NULL);

    g_free(data->output_dir);
    data->output_dir = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_output_dir));
    g_assert(data->output_dir != NULL);

    g_free(data->gal_dir);
    data->gal_dir = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_gal_dir));
    g_assert(data->gal_dir != NULL);

    g_free(data->templ_dir);
    data->templ_dir = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_templ_dir));
    g_assert(data->templ_dir != NULL);

    g_free(data->templ_index);
    data->templ_index = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_templ_index));
    g_assert(data->templ_index != NULL);
    
    g_free(data->templ_indeximg);
    data->templ_indeximg = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_templ_indeximg));
    g_assert(data->templ_indeximg != NULL);

    g_free(data->templ_indexgen);
    data->templ_indexgen = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_templ_indexgen));
    g_assert(data->templ_indexgen != NULL);

    g_free(data->templ_image);
    data->templ_image = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_templ_image));
    g_assert(data->templ_image != NULL);

    g_free(data->templ_gen);
    data->templ_gen = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_templ_gen));
    g_assert(data->templ_gen != NULL);

    data->thumb_w = (gint)gtk_spin_button_get_value(
        GTK_SPIN_BUTTON(spinbutton_pref_thumb_w));

    data->image_h = gtk_spin_button_get_value(
        GTK_SPIN_BUTTON(spinbutton_pref_image_h));
    data->image_h2 = gtk_spin_button_get_value(
        GTK_SPIN_BUTTON(spinbutton_pref_image_h2));
    data->image_h3 = gtk_spin_button_get_value(
        GTK_SPIN_BUTTON(spinbutton_pref_image_h3));
    data->image_h4 = gtk_spin_button_get_value(
        GTK_SPIN_BUTTON(spinbutton_pref_image_h4));

    if (gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(radiobutton_pref_gen_templ)) == TRUE)
        data->page_gen = PWGALLERY_PAGE_GEN_TEMPL;
    else
        data->page_gen = PWGALLERY_PAGE_GEN_PROG;


    g_free(data->page_gen_prog);
    data->page_gen_prog = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_page_gen_prog));
    g_assert(data->page_gen_prog != NULL);


    data->remove_exif = gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(togglebutton_pref_hideexif));

    data->rename = gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(togglebutton_pref_rename));

    /* save to disk */
    configrc_save(data);

}



void
widgets_gal_settings_show(struct data *data)
{
	GtkWidget *dialog;
    GtkWidget *entry_gal_name;
    GtkWidget *textview_gal_desc;
    GtkWidget *filechooserbutton_gal_dest_dir;
    GtkWidget *spinbutton_gal_thumb_w;
    GtkWidget *spinbutton_gal_image_h;
    GtkWidget *spinbutton_gal_image_h2;
    GtkWidget *spinbutton_gal_image_h3;
    GtkWidget *spinbutton_gal_image_h4;
    GtkWidget *radiobutton_gal_gen_templ;
    GtkWidget *radiobutton_gal_gen_prog;
    GtkWidget *filechooserbutton_gal_page_gen_prog;
    GtkWidget *filechooserbutton_gal_templ_index;
    GtkWidget *filechooserbutton_gal_templ_indeximg;
    GtkWidget *filechooserbutton_gal_templ_indexgen;
    GtkWidget *filechooserbutton_gal_templ_image;
    GtkWidget *filechooserbutton_gal_templ_gen;
    GtkWidget *togglebutton_gal_hideexif;
    GtkWidget *togglebutton_gal_rename;

    GtkTextIter end_iter;
    GtkTextIter start_iter;
    GtkTextBuffer *textview_buffer;
    gint result;

	g_assert(data != NULL);

    /* Get widgets */
    dialog = glade_xml_get_widget(data->glade, "dialog_gal");
    entry_gal_name = 
        glade_xml_get_widget(data->glade, "entry_gal_name");
    textview_gal_desc = 
        glade_xml_get_widget(data->glade, "textview_gal_desc");
    filechooserbutton_gal_dest_dir = 
        glade_xml_get_widget(data->glade, "filechooserbutton_gal_dest_dir");
    spinbutton_gal_thumb_w = 
        glade_xml_get_widget(data->glade, "spinbutton_gal_thumb_w");
    spinbutton_gal_image_h = 
        glade_xml_get_widget(data->glade, "spinbutton_gal_image_h");
    spinbutton_gal_image_h2 = 
        glade_xml_get_widget(data->glade, "spinbutton_gal_image_h2");
    spinbutton_gal_image_h3 = 
        glade_xml_get_widget(data->glade, "spinbutton_gal_image_h3");
    spinbutton_gal_image_h4 = 
        glade_xml_get_widget(data->glade, "spinbutton_gal_image_h4");
    radiobutton_gal_gen_templ = 
        glade_xml_get_widget(data->glade, "radiobutton_gal_gen_templ");
    radiobutton_gal_gen_prog = 
        glade_xml_get_widget(data->glade, "radiobutton_gal_gen_prog");
    filechooserbutton_gal_page_gen_prog = 
        glade_xml_get_widget( 
            data->glade, "filechooserbutton_gal_page_gen_prog");
    filechooserbutton_gal_templ_index = 
        glade_xml_get_widget(data->glade, "filechooserbutton_gal_templ_index");
    filechooserbutton_gal_templ_indeximg = 
        glade_xml_get_widget( 
            data->glade, "filechooserbutton_gal_templ_indeximg");
    filechooserbutton_gal_templ_indexgen = 
        glade_xml_get_widget( 
            data->glade, "filechooserbutton_gal_templ_indexgen");
    filechooserbutton_gal_templ_image = 
        glade_xml_get_widget(data->glade, "filechooserbutton_gal_templ_image");
    filechooserbutton_gal_templ_gen = 
        glade_xml_get_widget(data->glade, "filechooserbutton_gal_templ_gen");
    togglebutton_gal_hideexif = 
        glade_xml_get_widget(data->glade, "togglebutton_gal_hideexif");
    togglebutton_gal_rename = 
        glade_xml_get_widget(data->glade, "togglebutton_gal_rename");


    /* Set values */
    gtk_entry_set_text(GTK_ENTRY(entry_gal_name), data->gal->name);
    textview_buffer = 
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_gal_desc));
    gtk_text_buffer_set_text(textview_buffer, data->gal->desc, -1);
    gtk_file_chooser_set_uri(
        GTK_FILE_CHOOSER(filechooserbutton_gal_dest_dir),
        data->gal->output_dir);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_gal_thumb_w),
                              (gdouble)data->gal->thumb_w);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_gal_image_h),
                              (gdouble)data->gal->image_h);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_gal_image_h2),
                              (gdouble)data->gal->image_h2);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_gal_image_h3),
                              (gdouble)data->gal->image_h3);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_gal_image_h4),
                              (gdouble)data->gal->image_h4);
    if (data->gal->page_gen == PWGALLERY_PAGE_GEN_TEMPL)
        gtk_toggle_button_set_active(
            GTK_TOGGLE_BUTTON(radiobutton_gal_gen_templ), TRUE);
    else
        gtk_toggle_button_set_active(
            GTK_TOGGLE_BUTTON(radiobutton_gal_gen_prog), TRUE);
    gtk_file_chooser_set_uri(
        GTK_FILE_CHOOSER(filechooserbutton_gal_page_gen_prog),
        data->gal->page_gen_prog);
    gtk_file_chooser_set_uri(
        GTK_FILE_CHOOSER(filechooserbutton_gal_templ_index), 
        data->gal->templ_index);
    gtk_file_chooser_set_uri(
        GTK_FILE_CHOOSER(filechooserbutton_gal_templ_indeximg),
        data->gal->templ_indeximg);
    gtk_file_chooser_set_uri(
        GTK_FILE_CHOOSER(filechooserbutton_gal_templ_indexgen),
        data->gal->templ_indexgen);
    gtk_file_chooser_set_uri(
        GTK_FILE_CHOOSER(filechooserbutton_gal_templ_image),
        data->gal->templ_image);
    gtk_file_chooser_set_uri(
        GTK_FILE_CHOOSER(filechooserbutton_gal_templ_gen),
        data->gal->templ_gen);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(togglebutton_gal_hideexif),
                                 data->gal->remove_exif);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(togglebutton_gal_rename),
                                 data->gal->rename);

    /* show dialog (in a loop because of help dialog) */
    do
    {
        result = gtk_dialog_run(GTK_DIALOG(dialog));    
        switch(result)
        {
        case GTK_RESPONSE_DELETE_EVENT:
        case GTK_RESPONSE_CANCEL: /* hide dialog, save nothing, return */
            gtk_widget_hide(dialog);
            return;
        case GTK_RESPONSE_HELP:   /* show help */
            widgets_help_show(data, "Help for settings window.");
            break;
        default:                  /* ok pressed, hide dialog */
            gtk_widget_hide(dialog);
        }
    } while(result != GTK_RESPONSE_OK);
    
    /* save button pressed */

    /* get values */
    g_free(data->gal->name);
    data->gal->name = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_gal_name)));

    g_free(data->gal->desc);
    gtk_text_buffer_get_end_iter(textview_buffer, &end_iter);
    gtk_text_buffer_get_start_iter(textview_buffer, &start_iter);
    data->gal->desc = gtk_text_buffer_get_text(textview_buffer,
                                               &end_iter, &start_iter, TRUE);
    g_warning(data->gal->desc);
    g_free(data->gal->output_dir);
    data->gal->output_dir = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_gal_dest_dir));
    g_assert(data->gal->output_dir != NULL);

    g_free(data->gal->output_dir);
    data->gal->output_dir = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_gal_dest_dir));
    g_assert(data->gal->output_dir != NULL);

    data->gal->thumb_w = (gint)gtk_spin_button_get_value(
        GTK_SPIN_BUTTON(spinbutton_gal_thumb_w));

    data->gal->image_h = gtk_spin_button_get_value(
        GTK_SPIN_BUTTON(spinbutton_gal_image_h));
    data->gal->image_h2 = gtk_spin_button_get_value(
        GTK_SPIN_BUTTON(spinbutton_gal_image_h2));
    data->gal->image_h3 = gtk_spin_button_get_value(
        GTK_SPIN_BUTTON(spinbutton_gal_image_h3));
    data->gal->image_h4 = gtk_spin_button_get_value(
        GTK_SPIN_BUTTON(spinbutton_gal_image_h4));

    if (gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(radiobutton_gal_gen_templ)) == TRUE)
        data->gal->page_gen = PWGALLERY_PAGE_GEN_TEMPL;
    else
        data->gal->page_gen = PWGALLERY_PAGE_GEN_PROG;


    g_free(data->gal->page_gen_prog);
    data->gal->page_gen_prog = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_gal_page_gen_prog));
    /* FIXME: this is not implemented yet, so it can be NULL.. */
    if (data->gal->page_gen_prog == NULL)
        data->gal->page_gen_prog = g_strdup("file:///tmp/unimplemented.sh");


    g_free(data->gal->templ_index);
    data->gal->templ_index = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_gal_templ_index));
    g_assert(data->gal->templ_index != NULL);

    g_free(data->gal->templ_indeximg);
    data->gal->templ_indeximg = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_gal_templ_indeximg));
    g_assert(data->gal->templ_indeximg != NULL);

    g_free(data->gal->templ_indexgen);
    data->gal->templ_indexgen = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_gal_templ_indexgen));
    g_assert(data->gal->templ_indexgen != NULL);

    g_free(data->gal->templ_image);
    data->gal->templ_image = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_gal_templ_image));
    g_assert(data->templ_image != NULL);

    g_free(data->gal->templ_gen);
    data->gal->templ_gen = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_gal_templ_gen));
    g_assert(data->templ_gen != NULL);

    data->gal->remove_exif = gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(togglebutton_gal_hideexif));

    data->gal->rename = gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(togglebutton_gal_rename));

    /* gallery (settings) has been now edited */
    data->gal->edited = TRUE;
}



void
widgets_help_show(struct data *data, const gchar *helptext)
{
	GtkWidget     *dialog;
    GtkWidget     *textview;
    GtkTextBuffer *buffer;
    gint          result;

	g_assert(data != NULL);
	g_assert(helptext != NULL);

    /* Get widgets */
    dialog = glade_xml_get_widget( data->glade, "dialog_help");
    textview = glade_xml_get_widget( data->glade, "textview_help");

    /* Set values */
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_set_text(buffer, helptext, -1);

    /* show help dialog */
    result = gtk_dialog_run(GTK_DIALOG(dialog));    
    gtk_widget_hide(dialog);
}



/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
