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

#include <glib.h>
#include <gtk/gtk.h>
#include <glade/glade-xml.h>

void 
on_button_pwg_quit_clicked( GtkToolButton *toolbutton,
                            gpointer user_data)
{
    g_debug("in on_button_pwg_quit_clicked\n");

    gtk_main_quit();
}



void
on_button_gallery_configure_clicked(GtkToolButton *toolbutton,
                                    gpointer user_data)
{
    GladeXML  *glade;

    g_debug("in on_button_gallery_configure_clicked\n");

    /* FIXME: this leaks, right? */
    glade = glade_xml_new(PWGALLERY_GLADE_FILE, "gallerywindow", NULL);
}



void
on_button_pwg_pref_clicked(GtkToolButton *toolbutton,
                           gpointer user_data)
{
    GladeXML  *glade;

    g_debug("in on_button_pwg_pref_clicked\n");

    /* FIXME: this leaks, right? */
    glade = glade_xml_new(PWGALLERY_GLADE_FILE, "preferenceswindow", NULL);
}



gboolean
on_mainwindow_delete_event(GtkWidget *widget,
                           GdkEvent *event,
                           gpointer user_data)
{
    g_debug("in on_mainwindow_delete_event");

    gtk_main_quit();

    return FALSE;
}



/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
