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

#define PWGALLERY_IMAGE_MOVE_TOP      1
#define PWGALLERY_IMAGE_MOVE_UP       2
#define PWGALLERY_IMAGE_MOVE_DOWN     3
#define PWGALLERY_IMAGE_MOVE_BOTTOM   4

static void action_quit(gpointer user_data);
static void action_show_preferences(gpointer user_data);
static void action_gal_show_settings(gpointer user_data);
static void action_gal_new(gpointer user_data);
static void action_gal_open(gpointer user_data);
static void action_gal_save(gpointer user_data);
static void action_gal_save_as(gpointer user_data);
static void action_gal_make(gpointer user_data);
static void action_image_add(gpointer user_data);
static void action_image_remove(gpointer user_data);
static void action_about_show(gpointer user_data);
static void action_image_move(gpointer user_data, gint place);

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
on_menu_new_activate(GtkMenuItem *menuitem,
                     gpointer user_data)
{
    g_debug("in on_menu_new_activate");

    action_gal_new(user_data);
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
    g_debug("in on_menu_save_activate");

    action_gal_save(user_data);
}



void
on_menu_save_as_activate(GtkMenuItem *menuitem,
                      gpointer user_data)
{
    g_debug("in on_menu_save_as_activate");

    action_gal_save_as(user_data);
}


void
on_menu_make_gal_activate(GtkMenuItem *menuitem,
                          gpointer user_data)
{
    g_debug("in on_menu_make_activate");

    action_gal_make(user_data);
}



/*
 * Image menu actions 
 */

void 
on_menu_add_image_activate(GtkMenuItem *menuitem,
                           gpointer user_data)
{
    g_debug("on_menu_add_image_activate");
    
    action_image_add(user_data);
}



void 
on_menu_remove_image_activate(GtkMenuItem *menuitem,
                              gpointer user_data)
{
    g_debug("on_menu_remove_image_activate");
    
    action_image_remove(user_data);
}



/*
 * Help menu actions 
 */

void on_menu_about_activate(GtkMenuItem *menuitem,
                            gpointer user_data)
{
    struct data *data;

    g_debug("in on_menu_about_activate");
    
    g_assert(user_data != NULL);

    data = user_data;

    action_about_show(data);
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



void
on_check_settings_nomodify_toggled(GtkToggleButton *togglebutton,
                                   gpointer user_data)
{
    struct data *data;

    g_debug("in on_check_settings_nomodify_toggled");
    
    g_assert(user_data != NULL);

    data = user_data;

    /* do nothing if no images */
    if (data->current_img == NULL)
        return;

    data->current_img->nomodify = gtk_toggle_button_get_active(togglebutton);
    
}



void
on_radiobutton_rotate_0_toggled(GtkToggleButton *togglebutton,
                                gpointer user_data)
{
    struct data *data;

    g_debug("in on_radiobutton_rotate_0_toggled");
    
    g_assert(user_data != NULL);

    data = user_data;

    /* do nothing if no images */
    if (data->current_img == NULL)
        return;

    data->current_img->rotate = 0;
}



void
on_radiobutton_rotate_90_toggled(GtkToggleButton *togglebutton,
                                 gpointer user_data)
{
    struct data *data;

    g_debug("in on_radiobutton_rotate_90_toggled");
    
    g_assert(user_data != NULL);

    data = user_data;

    /* do nothing if no images */
    if (data->current_img == NULL)
        return;

    data->current_img->rotate = 90;
}



void
on_radiobutton_rotate_180_toggled(GtkToggleButton *togglebutton,
                                  gpointer user_data)
{
    struct data *data;

    g_debug("in on_radiobutton_rotate_180_toggled");
    
    g_assert(user_data != NULL);

    data = user_data;

    /* do nothing if no images */
    if (data->current_img == NULL)
        return;

    data->current_img->rotate = 180;
}



void
on_radiobutton_rotate_270_toggled(GtkToggleButton *togglebutton,
                                  gpointer user_data)
{
    struct data *data;

    g_debug("in on_radiobutton_rotate_270_toggled");
    
    g_assert(user_data != NULL);

    data = user_data;

    /* do nothing if no images */
    if (data->current_img == NULL)
        return;

    data->current_img->rotate = 270;
}



void
on_button_move_top_clicked(GtkButton *button, gpointer user_data)
{
    g_debug("in on_button_move_top_clicked");

    action_image_move(user_data, PWGALLERY_IMAGE_MOVE_TOP);
}


void
on_button_move_up_clicked(GtkButton *button, gpointer user_data)
{
    g_debug("in on_button_move_up_clicked");

    action_image_move(user_data, PWGALLERY_IMAGE_MOVE_UP);
}


void
on_button_move_down_clicked(GtkButton *button, gpointer user_data)
{
    g_debug("in on_button_move_down_clicked");

    action_image_move(user_data, PWGALLERY_IMAGE_MOVE_DOWN);
}


void
on_button_move_bottom_clicked(GtkButton *button, gpointer user_data)
{
    g_debug("in on_button_move_bottom_clicked");

    action_image_move(user_data, PWGALLERY_IMAGE_MOVE_BOTTOM);
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
    struct data *data;

    g_assert(user_data != NULL );

    g_debug("in action_quit");

    data = user_data;

    /* save selected image's (if any) text */
    gallery_image_save_text(data);

    /* FIXME: check for data->gal->edited */

    gtk_main_quit();
}



/*
 * Show preferences window
 */
static void 
action_show_preferences(gpointer user_data)
{
    struct data *data;

    g_assert(user_data != NULL );

    g_debug("in action_show_preferences");

    data = user_data;

    widgets_prefs_show(user_data);
}



/*
 * Show gallery settings window
 */
static void 
action_gal_show_settings(gpointer user_data)
{
    struct data *data;

    g_assert(user_data != NULL );

    g_debug("in action_gal_show_settings");

    data = user_data;

    widgets_gal_settings_show(user_data);
}



/*
 * New gallery
 */
static void 
action_gal_new(gpointer user_data)
{
    struct data *data;
    
    g_assert(user_data != NULL );
    
    g_debug("in action_gal_new");

    data = user_data;
    
    /* save selected image's (if any) text */
    gallery_image_save_text(data);

    /* changes are checked in gallery_new */
    gallery_new(data);
}



/*
 * Open gallery
 */
static void 
action_gal_open(gpointer user_data)
{
    struct data *data;
    
    g_assert(user_data != NULL );
    
    g_debug("in action_gal_open");
    
    data = user_data;

    /* save selected image's (if any) text */
    gallery_image_save_text(data);

    /* changes are checked in gallery_new */
    gallery_open(data);
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

    /* save selected image's (if any) text */
    gallery_image_save_text(data);

    gallery_save(data);
}



/*
 * Save gallery as
 */
static void 
action_gal_save_as(gpointer user_data)
{
    struct data *data;

    g_assert(user_data != NULL );

    g_debug("in action_gal_save_as");

    data = user_data;

    /* save selected image's (if any) text */
    gallery_image_save_text(data);

    gallery_save_as(data);
}



/*
 * Make gallery
 */
static void 
action_gal_make(gpointer user_data)
{
    struct data *data;

    g_assert(user_data != NULL );

    g_debug("in action_gal_make");

    data = user_data;

    /* save selected image's (if any) text */
    gallery_image_save_text(data);

    /* changes are checked in gallery_make */
    gallery_make(data);
}



/*
 * Add images to gallery
 */
static void action_image_add(gpointer user_data)
{
    struct data *data;
    GtkWidget *dialog;
    int result;
    GSList *uris;

    g_assert(user_data != NULL);

    data = user_data;

    dialog = gtk_file_chooser_dialog_new("Select Images",
                                         GTK_WINDOW(data->top_window),
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), 
                                            data->img_dir);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);

    result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result != GTK_RESPONSE_ACCEPT)
    {
        gtk_widget_destroy (dialog);
        return;
    }

    uris = gtk_file_chooser_get_uris(GTK_FILE_CHOOSER(dialog));

    gtk_widget_destroy (dialog);

    gallery_add_new_images(data, uris);    
}



/*
 * Remove image from gallery
 */
static void action_image_remove(gpointer user_data)
{
    struct data *data;

    g_assert(user_data != NULL);

    data = user_data;

    if (data->current_img != NULL)
        gallery_remove_image(data, data->current_img);    
}



/*
 * Show About dialog
 */
static void action_about_show(gpointer user_data)
{
    struct data *data;

    g_assert(user_data != NULL );

    g_debug("in action_gal_save_as");

    data = user_data;

    widgets_about_show(data);
}



/*
 * Move current image to top,up,down or bottom
 */
static void
action_image_move(gpointer user_data, gint place)
{
    struct data *data;
    int current_no;

    g_assert(user_data != NULL );
    g_assert(place >= PWGALLERY_IMAGE_MOVE_TOP &&
             place <= PWGALLERY_IMAGE_MOVE_BOTTOM);

    data = user_data;

    g_debug("in action_image_move");

    /* return if no images */
    if (data->current_img == NULL)
        return;

    /* get the index number of the currently selected image */
    current_no = g_slist_index(data->gal->images, data->current_img);
    g_assert(current_no > -1);

    /* if moving up and  image is already on top, just return */
    if (current_no == 0 && place == PWGALLERY_IMAGE_MOVE_UP)
        return;

    /* delete element from the list */
    data->gal->images = g_slist_remove(data->gal->images, data->current_img);

    switch(place)
    {
    case PWGALLERY_IMAGE_MOVE_TOP:
        data->gal->images = g_slist_prepend(data->gal->images, 
                                            data->current_img);
        break;
    case PWGALLERY_IMAGE_MOVE_UP:
        data->gal->images = g_slist_insert(data->gal->images, 
                                           data->current_img,
                                           current_no - 1);
        break;
    case PWGALLERY_IMAGE_MOVE_DOWN:
        /* if current_no + 1 is too big, it's appended anyways */
        data->gal->images = g_slist_insert(data->gal->images, 
                                           data->current_img,
                                           current_no + 1);
        break;
    case PWGALLERY_IMAGE_MOVE_BOTTOM:
        data->gal->images = g_slist_append(data->gal->images, 
                                           data->current_img);
        break;
    }

    /* update the thumbnail list */
	widgets_update_table(data);

    data->gal->edited = TRUE;
}



/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
