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
#include "vfs.h"

#include <stdlib.h>		/* exit, EXIT_SUCCESS/FAILURE */
#include <getopt.h>		/* getopt */
#include <limits.h>     /* PATH_MAX */

#include <glib.h>
#include <gtk/gtk.h>
#include <libgnomevfs/gnome-vfs.h>

static struct data *init_data(void);
static void init_pwgallery(struct data *data);
static void free_data(struct data *data);

static int parse_args(struct data *data, int argc, char *argv[]);
static void print_usage(const char *self);
static void print_version(const char *self);

static void new_gallery(struct data *data);

static void generate_img_gslist(struct data *data, int argc, char *argv[]);

int
main(int argc, char *argv[])
{
    struct data *data;
    int r;
 
    g_type_init(); /* Initialize glib type system before using anything */

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

    if (data->use_gui) {
        gtk_init(&argc, &argv);
        if (!g_thread_supported ()){
            g_thread_init (NULL);
        }
    }

    /* set critical to be always fatal */
    g_log_set_always_fatal(G_LOG_LEVEL_CRITICAL);
        
    init_pwgallery(data);

    configrc_load(data);

    gallery_init(data);

    if (data->arg_new != NULL) {
        /* Create new gallery without GUI */
        new_gallery(data);
        gallery_save(data);
    } else {
        /* Start GUI */

        /* connect signals */
        gtk_builder_connect_signals(data->builder, data);
        
        /* show main window */
        gtk_widget_show_all(data->top_window);

        gtk_main();
    }

    free_data(data);

    exit(EXIT_SUCCESS);
}



/*
 * Initialize struct data fields
 */
static struct data *
init_data(void)
{
    struct data *data;

    data = g_new0(struct data, 1);

    data->use_gui = TRUE; /* Use GUI unless state otherwise */
    data->ss_timer_interval = 2000; /* Default to 2sec slide show interval */

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

    if (data->use_gui) { 
        gchar gladefile[PATH_MAX];
        gchar *dir;
        gchar *filep;
        GError* error = NULL;

        dir = g_get_current_dir();

        g_snprintf(gladefile, PATH_MAX, "%s/%s", 
                   dir, "src/glade/pwgallery.ui");
        g_free(dir);

        if (vfs_is_file(data, gladefile)) {
            g_debug("Loading glade file from source tree.");
            filep = gladefile;
        } else {
            filep = PWGALLERY_BUILDER_FILE;
        }

        data->builder = gtk_builder_new();
        if (!gtk_builder_add_from_file(data->builder, filep, &error))  {
            g_warning ("Couldn't load builder file: %s", error->message);
            g_error_free(error);
            exit(1);
        }

        /* find top level window */
        data->top_window = GTK_WIDGET(gtk_builder_get_object(data->builder, "mainwindow"));
        g_assert(data->top_window);
    }
}



/*
 * Free stuff
 */
static void
free_data(struct data *data)
{
    g_assert(data);


    /* Free image list for create new gallery cmdline argument */
    if (data->arg_new_imgs != NULL) {
        GSList *list;
        list = data->arg_new_imgs;
        while(list) {
            g_free(list->data);
            list->data = NULL;
            list = list->next;
        }
        g_slist_free(data->arg_new_imgs);
    }


    if (data->gal != NULL) {
        gallery_free(data);
    }

    if (data->builder) {
        g_object_unref(data->builder);
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
			{"new",	1, 0, 'n'},
			{0,		0, 0, 0}
		};
		c = getopt_long(argc, argv, "hvn:", 
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
		case 'n':
            data->arg_new = g_strdup(optarg);
            data->use_gui = FALSE;
            break;
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

        /* Add the rest of the arguments as images to be added to gallery */
        if (data->arg_new != NULL) {
            generate_img_gslist(data, argc, argv);
        } else {
            g_warning("Invalid parameters: ");

            while (optind < argc) {
                g_print("%s ", argv[optind++]);
            }
            g_print("\n");

            print_usage(argv[0]);
            return -1;
        }
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
Usage: %s [options] [image ...]\n\
\n\
Options\n\
  -h  --help               Show this usage\n\
  -v  --version            Show version\n\
  -n  --new gallery_name   Create new gallery\n\
",
            self);
}



/*
 * Print version number
 */
static void
print_version(const char *self)
{
    g_print(PACKAGE_STRING "\n" "$LastChangedDate: 2009-03-15 20:10:59 +0200 (Sun, 15 Mar 2009) $" "\n" "$Rev: 77 $" "\n" );
}



/*
 * Add arguments to images list 
 */
static void
generate_img_gslist(struct data *data, int argc, char *argv[])
{
    gchar *pwd;

    /* get our current dir to form proper uris for images */
    pwd = g_strdup_printf("file://%s", g_get_current_dir());

    while (optind < argc) {
        gchar *img;
        
        if (g_path_is_absolute(argv[optind])) {
            img = g_strdup_printf("file://%s", argv[optind]);
        } else {
            img = g_strdup_printf("%s/%s", pwd, argv[optind]);
        }
        optind++;

        data->arg_new_imgs = g_slist_append(data->arg_new_imgs, img);
    }
}



/*
 * Create new gallery without GUI. Use default values.
 */
static void
new_gallery(struct data *data)
{
    g_assert(data != NULL);

    data->gal->uri = g_strdup_printf("%s/%s.xml", data->gal_dir, data->arg_new);
    g_free(data->arg_new);

    gallery_add_new_images(data, data->arg_new_imgs);
}
        



/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
