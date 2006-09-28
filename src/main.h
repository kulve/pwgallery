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

#ifndef PWGALLERY_MAIN_H
#define PWGALLERY_MAIN_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


/* FIXME: PACKAGE_DATA_DIR */
/* Glade XML file */
#define PWGALLERY_GLADE_FILE               "src/glade/pwgallery.glade"   

/* Buf size while reading images */
#define PWGALLERY_IMG_READ_BUF_SIZE        4096
/* Thumbnail width in the list */
#define PWGALLERY_THUMB_W                  250
/* Border width for the thumbnail images in buttons */
#define PWGALLERY_THUMBNAIL_BORDER_WIDTH   10

/* Page generators (template, script) */
#define PWGALLERY_PAGE_GEN_TEMPL           1
#define PWGALLERY_PAGE_GEN_PROG            2




/* configrc directory under $HOME */
#define PWGALLERY_CONFIGRC_DIR             ".pwgallery"
/* configrc file */
#define PWGALLERY_CONFIGRC_FILE            "configrc"

/* RC key for image directory */
#define PWGALLERY_RCKEY_IMAGE_DIR          "image_dir"
/* RC key for gallery directory */
#define PWGALLERY_RCKEY_GAL_DIR            "gal_dir"
/* RC key for template direcotry */
#define PWGALLERY_RCKEY_TEMPL_DIR          "template_dir"

/* RC key for page generator style */
#define PWGALLERY_RCKEY_PAGE_GEN           "page_generator"
/* RC key for page generator program */
#define PWGALLERY_RCKEY_PAGE_GEN_PROG      "page_generator_program"

/* RC key for output dir */
#define PWGALLERY_RCKEY_OUTPUT_DIR         "output_dir"
/* RC key for thumb width */
#define PWGALLERY_RCKEY_THUMB_W            "thumb_width"
/* RC key for image height */
#define PWGALLERY_RCKEY_IMAGE_H            "image_height"
/* RC key for image height2 */
#define PWGALLERY_RCKEY_IMAGE_H2           "image_heigh2t"
/* RC key for image height3 */
#define PWGALLERY_RCKEY_IMAGE_H3           "image_height3"
/* RC key for image height4 */
#define PWGALLERY_RCKEY_IMAGE_H4           "image_height4"
/* RC key for index page template */
#define PWGALLERY_RCKEY_TEMPL_INDEX        "template_index"
/* RC key for index page per image template */
#define PWGALLERY_RCKEY_TEMPL_INDEXIMG     "template_index_per_image"
/* RC key for index page per generic template */
#define PWGALLERY_RCKEY_TEMPL_INDEXGEN     "template_index_per_gen"
/* RC key for image page template */
#define PWGALLERY_RCKEY_TEMPL_IMAGE        "template_image"
/* RC key for gen page template */
#define PWGALLERY_RCKEY_TEMPL_GEN          "template_gen"
/* RC key for exif removal */
#define PWGALLERY_RCKEY_REMOVE_EXIF        "remove_exif"
/* RC key for image renameing */
#define PWGALLERY_RCKEY_RENAME             "rename_images"

/* Default image directory */
#define PWGALLERY_DEFAULT_IMAGE_DIR        "file:///tmp"
/* Default gallery directory */
#define PWGALLERY_DEFAULT_GAL_DIR          "galleries"
/* Default template direcotry */
#define PWGALLERY_DEFAULT_TEMPL_DIR        "templates"


/* Default page generator style (must match to PWGALLERY_PAGE_GEN_TEMPL) */
#define PWGALLERY_DEFAULT_PAGE_GEN         "1"
/* Default page generator programe */
#define PWGALLERY_DEFAULT_PAGE_GEN_PROG    "file:///tmp/unimplemented.sh"

/* Default output dir */
#define PWGALLERY_DEFAULT_OUTPUT_DIR       "file:///tmp/pwgallery_test"
/* Default thumb width */
#define PWGALLERY_DEFAULT_THUMB_W          "150"
/* Default image height */
#define PWGALLERY_DEFAULT_IMAGE_H          "500"
/* Default image height2 */
#define PWGALLERY_DEFAULT_IMAGE_H2         "0"
/* Default image height3 */
#define PWGALLERY_DEFAULT_IMAGE_H3         "0"
/* Default image height4 */
#define PWGALLERY_DEFAULT_IMAGE_H4         "0"
/* Default index page template */
#define PWGALLERY_DEFAULT_TEMPL_INDEX      "templ_index.shtml"
/* Default index page per image template */
#define PWGALLERY_DEFAULT_TEMPL_INDEXIMG   "templ_indeximg.shtml"
/* Default index page per generic template */
#define PWGALLERY_DEFAULT_TEMPL_INDEXGEN   "templ_indexgen.shtml"
/* Default image page template */
#define PWGALLERY_DEFAULT_TEMPL_IMAGE      "templ_image.shtml"
/* Default generic page template */
#define PWGALLERY_DEFAULT_TEMPL_GEN        "templ_gen.shtml"
/* Default exif removal value */
#define PWGALLERY_DEFAULT_REMOVE_EXIF      "true"
/* Default image renameing value */
#define PWGALLERY_DEFAULT_RENAME           "false"



#define _(String) gettext (String)
#include <libintl.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <glade/glade-xml.h>


struct data
{
    GladeXML       *glade;             /* Glade XML */
    GtkWidget      *top_window;        /* pointer to top level window */
    struct gallery *gal;               /* pointer to current gallery */

    gchar          *img_dir;           /* image directory */
    gchar          *output_dir;        /* Default gallery's output dir */
    gchar          *gal_dir;           /* directory for gallery files */
    gchar          *templ_dir;         /* template directory */
    /* Default values */
    gint           page_gen;           /* Default page generator */
    gchar          *page_gen_prog;     /* Default page generator program */
    gchar          *templ_index;       /* Default template for the index page */
    gchar          *templ_indeximg;    /* Default templ for the index img page*/
    gchar          *templ_indexgen;    /* Default templ for the index gen page*/
    gchar          *templ_image;       /* Default template for the image page */
    gchar          *templ_gen;         /* Default template for the gen page */
    gint           thumb_w;            /* Default width of thumbs */
    gint           image_h;            /* Default height of web images */
    gint           image_h2;           /* Default height of web images2 */
    gint           image_h3;           /* Default height of web images3 */
    gint           image_h4;           /* Default height of web images4 */
    gboolean       remove_exif;        /* Default value for remove exif info */
    gboolean       rename;             /* Default value for rename images */

};

struct gallery
{
    GSList         *images;            /* list of images in the gallery */
    gchar          *uri;               /* uri to the gallery file */
    gchar          *name;              /* name of the current gal. */
    gchar          *desc;              /* description of the current gal. */
    gchar          *output_dir;        /* gallery's output dir */
    gint           page_gen;           /* page generator */
    gchar          *page_gen_prog;     /* page generator program */
    gchar          *templ_index;       /* template for the index page */
    gchar          *templ_indeximg;    /* templ for the index img page*/
    gchar          *templ_indexgen;    /* templ for the index gen page*/
    gchar          *templ_image;       /* template for the image page */
    gchar          *templ_gen;         /* template for the gen page */
    gint           thumb_w;            /* width of thumbs */
    gint           image_h;            /* height of web images */
    gint           image_h2;           /* height of web images2 */
    gint           image_h3;           /* height of web images3 */
    gint           image_h4;           /* height of web images4 */
    gboolean       edited;             /* is the gallery edited */
    gboolean       remove_exif;        /* remove exif info */
    gboolean       rename;             /* rename images */

};

struct image
{
    /* FIXME: is image need? Would child-of-button do? */
    GtkWidget       *image;            /* pointer to image widget */
    GtkWidget       *button;           /* pointer to button widget */
    gint            width;             /* modified width of the image */
    gint            height;            /* modified height of the image */
    gint            rotate;            /* rotation of the image */
    gfloat          gamma;             /* gamma of the image */
    gchar           *text;             /* image description */
    gchar           *uri;              /* URI to the original image */
    /* FIXME: add edited uri */
    gboolean        nomodify;          /* do not modify flag */
};

#endif

/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
