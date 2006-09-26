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
#include "callbacks.h"
#include "widgets.h"
#include "gallery.h"

#include <glib.h>
#include <gtk/gtk.h>
#include <glade/glade-xml.h>

static void action_quit(gpointer user_data);
static void action_show_preferences(gpointer user_data);
static void action_gal_show_settings(gpointer user_data);
static void action_gal_open(gpointer user_data);
static void action_gal_save(gpointer user_data);

/**********************************************************************
 * Widget action functions. These just calls the actual function to do
 * the things. This is trying to be an abstraction layer so that
 * e.g. quit or open functions can be called easier when button is
 * clicked or keyboard short cut is used.
 **********************************************************************/

/*
 * File menu actions 
 */

void 
on_menu_quit_activate( GtkMenuItem *menuitem,
                       gpointer user_data)
{
    g_debug("in on_menu_quit_activate");

    action_quit(user_data);
}



void 
on_menu_preferences_activate(GtkMenuItem *menuitem,
                             gpointer user_data)
{
    g_debug("in on_button_gallery_open_clicked");   

    action_show_preferences(user_data);
}


/*
 * Gallery menu actions 
 */

void
on_menu_gal_settings_activate(GtkMenuItem *menuitem,
                              gpointer user_data)
{
    g_debug("in on_menu_gal_settings_activate");

    action_gal_show_settings(user_data);
}

void
on_menu_open_activate(GtkMenuItem *menuitem,
                      gpointer user_data)
{
    g_debug("in on_menu_open_activate");

    action_gal_open(user_data);
}



void
on_menu_save_activate(GtkMenuItem *menuitem,
                      gpointer user_data)
{
    g_debug("in on_menu_open_activate");

    action_gal_save(user_data);
}


/*
 * Image menu actions 
 */

void 
on_button_image_add_clicked(GtkToolButton *toolbutton,
                            gpointer user_data)
{
    struct data *data;
    GtkWidget *dialog;

    g_debug("in on_button_gallery_open_clicked");
    
    g_assert(user_data != NULL);

    data = user_data;

    dialog = gtk_file_chooser_dialog_new("Select Images",
                                         GTK_WINDOW(data->top_window),
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    /* FIXME: gtk_file_chooser_set_current_folder () */
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        gallery_add_images(data, 
                           gtk_file_chooser_get_uris(GTK_FILE_CHOOSER(dialog)));
    }

    gtk_widget_destroy (dialog);
}


/*
 * Other callbacks
 */

gboolean
on_mainwindow_delete_event(GtkWidget *widget,
                           GdkEvent *event,
                           gpointer user_data)
{
    g_debug("in on_mainwindow_delete_event");

    action_quit(user_data);

    return FALSE;
}

/**********************************************************************
 * The actions functions. These are called from above wrappers.
 * CHECKME: the one-liners could be called directly..
 **********************************************************************/

/*
 * Quit
 */
static void 
action_quit(gpointer user_data)
{
    g_assert(user_data != NULL );

    g_debug("in action_quit");

    gtk_main_quit();
}



/*
 * Show preferences window
 */
static void 
action_show_preferences(gpointer user_data)
{
    g_assert(user_data != NULL );

    g_debug("in action_show_preferences");

    widgets_prefs_show(user_data);
}



/*
 * Show gallery settings window
 */
static void 
action_gal_show_settings(gpointer user_data)
{
    g_assert(user_data != NULL );

    g_debug("in action_gal_show_settings");

    widgets_gal_settings_show(user_data);
}

/*
 * Open gallery
 */
static void 
action_gal_open(gpointer user_data)
{
    struct data *data;
    GtkWidget *dialog;
    gchar *uri;

    g_assert(user_data != NULL );

    g_debug("in action_gal_open");

    data = user_data;

    dialog = gtk_file_chooser_dialog_new(_("Open Gallery"),
                                         GTK_WINDOW(data->top_window),
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                         NULL);

    gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), 
                                            data->gal_dir);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), FALSE);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));
    } 
    else
    {
        gtk_widget_destroy (dialog);
        return;
    }
    gtk_widget_destroy (dialog);

    g_assert(uri != NULL); /* CHECKME: better error handling? */
    gallery_open(data, uri);
    g_free(uri);
}

/*
 * Save gallery
 */
static void 
action_gal_save(gpointer user_data)
{
    struct data *data;

    g_assert(user_data != NULL );

    g_debug("in action_gal_save");

    data = user_data;

    gallery_save(data);
}

/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
