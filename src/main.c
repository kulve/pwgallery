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
#include "gallery.h"
#include "configrc.h"

#include <stdlib.h>		/* exit, EXIT_SUCCESS/FAILURE */
#include <getopt.h>		/* getopt */

#include <glib.h>
#include <gtk/gtk.h>
#include <glade/glade-xml.h>
#include <libgnomevfs/gnome-vfs.h>

static void glade_xml_connect_func(const gchar *handler_name,
                                   GObject *object,
                                   const gchar *signal_name,
                                   const gchar *signal_data,
                                   GObject *connect_object,
                                   gboolean after,
                                   gpointer user_data);

static struct data *init_data(void);
static void init_pwgallery(struct data *data);
static void free_data(struct data *data);

static int parse_args(struct data *data, int argc, char *argv[]);
static void print_usage(const char *self);
static void print_version(const char *self);

int
main(int argc, char *argv[])
{
    struct data *data;
    int r;
 
#ifdef ENABLE_NLS
    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
#endif

    data = init_data();

    /* Parse arguments and exit if needed */
    r = parse_args(data, argc, argv);
    if (r != 0) {
        free_data(data);
        
        if (r == -1) {
            exit(EXIT_FAILURE);
        } else {
            exit(EXIT_SUCCESS);
        }
    }

    gtk_init(&argc, &argv);

    init_pwgallery(data);

    configrc_load(data);
    gallery_init(data);

    /* connect signals */
    glade_xml_signal_autoconnect_full(data->glade, glade_xml_connect_func,
                                      data);

    /* show main window */
    gtk_widget_show_all(data->top_window);

    /* set critical to be always fatal */
    g_log_set_always_fatal(G_LOG_LEVEL_CRITICAL);

    gtk_main();

    free_data(data);

    exit(EXIT_SUCCESS);
}

/*
 * Connect signals. 
 *
 * This is mainly copy paste from GnuCash.
 */
static void
glade_xml_connect_func(const gchar *handler_name,
                       GObject *object,
                       const gchar *signal_name,
                       const gchar *signal_data,
                       GObject *connect_object,
                       gboolean after,
                       gpointer user_data)
{
    static GModule *allsymbols = NULL;
    GtkSignalFunc func;
    GtkSignalFunc *p_func = &func;

    if (allsymbols == NULL) {
        /* get a handle on the main executable -- use this to find symbols */
        allsymbols = g_module_open(NULL, 0);
    }
    
    if (!g_module_symbol(allsymbols, handler_name, (gpointer *)p_func)) {
        g_warning("could not find signal handler '%s'.", handler_name);
        return;
    }
    
    if (connect_object) {
        if (after)
            g_signal_connect_after(object, signal_name, func, connect_object);
        else
            g_signal_connect(object, signal_name, func, connect_object);
    } else {

        if (after)
            g_signal_connect_after(object, signal_name, func, user_data);
        else
            g_signal_connect(object, signal_name, func, user_data);
    }

}



/*
 * Initialize struct data fields
 */
static struct data *
init_data(void)
{
    struct data *data;

    data = g_new0(struct data, 1);

    return data;
}



/*
 * Initialize struct data fields and other stuff
 */
static void
init_pwgallery(struct data *data)
{
    g_assert(data != NULL);

    if (gnome_vfs_init() == FALSE)
        g_error("Failed to initialize GnomeVFS");

    data->glade = glade_xml_new(PWGALLERY_GLADE_FILE, NULL, NULL);
    if (data->glade == NULL) {
        g_warning("Error reading glade file: %s\n", PWGALLERY_GLADE_FILE);
        exit(1);
    }

    /* find top level window */
    data->top_window = glade_xml_get_widget(data->glade, "mainwindow");
    g_assert(data->top_window);
}



/*
 * Free stuff
 */
static void
free_data(struct data *data)
{

    g_assert(data);

    if (data->gal != NULL) {
        gallery_free(data);
    }

    if (data->glade) {
        g_object_unref(data->glade);
    }
    g_free(data->img_dir);
    g_free(data->output_dir);
    g_free(data->gal_dir);
    g_free(data->templ_dir);
    g_free(data->templ_index);
    g_free(data->templ_indeximg);
    g_free(data->templ_indexgen);
    g_free(data->templ_image);
    g_free(data->templ_gen);
    g_free(data->page_gen_prog);
    g_free(data);
}



/*
 * Parse arguments
 */
static int
parse_args(struct data *data, int argc, char *argv[])
{
    int c;

	if (argv == NULL || argc == 0) {
		g_warning("parse_arguments: Invalid arguments\n");
		return -1;
	}

	while (1) {
		int option_index = 0;
		static struct option long_options[] =  {
			{"help",	0, 0, 'h'},
			{"version",	0, 0, 'v'},
			{0,		0, 0, 0}
		};
		c = getopt_long(argc, argv, "hv", 
				long_options, &option_index);
		if (c == -1) {
			break;
		}

		switch (c) {
		case 'h':
			print_usage(argv[0]);
			return 1;
		case 'v':
			print_version(argv[0]);
			return 1;
		case '?':
			g_warning("Unknown option");
			print_usage(argv[0]);
			return -1;

		default:
			g_print("Unknown option..\n");
			print_usage(argv[0]);
			return -1;
		}
	}

	if (optind < argc) {
		g_warning("Invalid parameters: ");
		while (optind < argc) {
			g_print("%s ", argv[optind++]);
		}
		g_print("\n");

		print_usage(argv[0]);
		return -1;
	}
	return 0;
}



/*
 * Print usage
 */
static void
print_usage(const char *self)
{
    g_print("\
Usage: %s [options]\n\
\n\
Options\n\
  -h  --help               Show this usage\n\
  -v  --version            Show version\n\
",
            self);
}



/*
 * Print version number
 */
static void
print_version(const char *self)
{
    g_print(PACKAGE_STRING "\n" "$LastChangedDate$" "\n" "$Rev$" "\n" );
}



/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
