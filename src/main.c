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

#include <stdlib.h>           /* exit */

#include <glib.h>
#include <gtk/gtk.h>
#include <glade/glade-xml.h>

int
main (int argc, char *argv[])
{
    GladeXML  *glade;

#ifdef ENABLE_NLS
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
#endif
    
    gtk_set_locale ();
    gtk_init (&argc, &argv);

    glade = glade_xml_new(PWGALLERY_GLADE_FILE, "mainwindow", NULL);

    if (glade == NULL) {
        g_warning("Error reading glade file: %s\n", PWGALLERY_GLADE_FILE);
        exit(1);
    }

    glade_xml_signal_autoconnect(glade);

    gtk_main ();

    return 0;
}

/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
