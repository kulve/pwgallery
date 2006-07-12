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
    GtkWidget *filechooserbutton_pref_templ_dir;
    GtkWidget *filechooserbutton_pref_templ_image;
    GtkWidget *filechooserbutton_pref_templ_indeximg;
    GtkWidget *filechooserbutton_pref_templ_index;
    GtkWidget *spinbutton_pref_thumb_w;
    GtkWidget *spinbutton_pref_image_h;
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
    filechooserbutton_pref_templ_index =
        glade_xml_get_widget( data->glade,
                              "filechooserbutton_pref_templ_index");
    spinbutton_pref_thumb_w = 
        glade_xml_get_widget( data->glade, "spinbutton_pref_thumb_w");
    spinbutton_pref_image_h = 
        glade_xml_get_widget( data->glade, "spinbutton_pref_image_h");
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
        GTK_FILE_CHOOSER(filechooserbutton_pref_templ_image),
        data->templ_image);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_pref_thumb_w),
                              (gdouble)data->thumb_w);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_pref_image_h),
                              (gdouble)data->image_h);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(togglebutton_pref_hideexif),
                                 data->remove_exif);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(togglebutton_pref_rename),
                                 data->rename);

    /* show dialog */
    result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    /* cancel button pressed, do nothing */
    if (result == GTK_RESPONSE_CANCEL)
        return;
    
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

    g_free(data->templ_image);
    data->templ_image = gtk_file_chooser_get_uri(
        GTK_FILE_CHOOSER(filechooserbutton_pref_templ_image));
    g_assert(data->templ_image != NULL);

    data->thumb_w = (gint)gtk_spin_button_get_value(
        GTK_SPIN_BUTTON(spinbutton_pref_thumb_w));

    data->image_h = gtk_spin_button_get_value(
        GTK_SPIN_BUTTON(spinbutton_pref_image_h));

    data->remove_exif = gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(togglebutton_pref_hideexif));

    data->rename = gtk_toggle_button_get_active(
        GTK_TOGGLE_BUTTON(togglebutton_pref_rename));

    gtk_widget_hide(dialog);

    /* save to disk */
    configrc_save(data);

}

/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
