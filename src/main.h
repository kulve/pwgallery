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

/* RC key for output dir */
#define PWGALLERY_RCKEY_OUTPUT_DIR         "output_dir"
/* RC key for thumb width */
#define PWGALLERY_RCKEY_THUMB_W            "thumb_width"
/* RC key for image height */
#define PWGALLERY_RCKEY_IMAGE_H            "image_height"
/* RC key for index page template */
#define PWGALLERY_RCKEY_TEMPL_INDEX        "template_index"
/* RC key for index page per image template */
#define PWGALLERY_RCKEY_TEMPL_INDEXIMG     "template_index_per_image"
/* RC key for image page template */
#define PWGALLERY_RCKEY_TEMPL_IMAGE        "template_image"
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


/* Default output dir */
#define PWGALLERY_DEFAULT_OUTPUT_DIR       "file:///tmp"
/* Default thumb width */
#define PWGALLERY_DEFAULT_THUMB_W          "150"
/* Default image height */
#define PWGALLERY_DEFAULT_IMAGE_H          "500"
/* Default index page template */
#define PWGALLERY_DEFAULT_TEMPL_INDEX      "templ_index.shtml"
/* Default index page per image template */
#define PWGALLERY_DEFAULT_TEMPL_INDEXIMG   "templ_indeximg.shtml"
/* Default image page template */
#define PWGALLERY_DEFAULT_TEMPL_IMAGE      "templ_image.shtml"
/* Default exif removal value */
#define PWGALLERY_DEFAULT_REMOVE_EXIF      "true"
/* Default image renameing value */
#define PWGALLERY_DEFAULT_RENAME           "false"


/* Buf size while reading images */
#define PWGALLERY_IMG_READ_BUF_SIZE        4096
/* Thumbnail width in the list */
#define PWGALLERY_THUMB_W                  250
/* Border width for the thumbnail images in buttons */
#define PWGALLERY_THUMBNAIL_BORDER_WIDTH   10

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

    gchar         *gal_dir;            /* image directory */
    gchar         *img_dir;            /* image directory */
    gchar         *templ_dir;          /* template directory */
    
    /* Default values */
    gchar          *output_dir;        /* Default gallery's output dir */
    gint           thumb_w;            /* Default width of thumbs */
    gint           image_h;            /* Default height of web images */
    gchar          *templ_index;       /* Default template for the index page */
    gchar          *templ_indeximg;    /* Default templ for the index img page*/
    gchar          *templ_image;       /* Default template for the image page */
    gboolean       remove_exif;        /* Default value for remove exif info */
    gboolean       rename;             /* Default value for rename images */

};

struct gallery
{
    GSList         *images;
    gchar          *gal_name;          /* name of the current gal. */
    gchar          *output_dir;        /* gallery's output dir */
    gint           thumb_w;            /* width of thumbs */
    gint           image_h;            /* height of web images */
    gchar          *templ_index;       /* template for the index page */
    gchar          *templ_indeximg;    /* Default templ for the index img page*/
    gchar          *templ_image;       /* template for the image page */
    gboolean       edited;             /* is the gallery edited */
    gboolean       remove_exif;        /* remove exif info */
    gboolean       rename;             /* rename images */

};

struct image
{
    GtkWidget       *image;            /* pointer to image widget */
    GtkWidget       *button;           /* pointer to button widget */
    gint            width;             /* modified width of the image */
    gint            height;            /* modified height of the image */
    gint            rotate;            /* rotation of the image */
    gfloat          gamma;             /* gamma of the image */
    gchar           *text;             /* image description */
    gchar           *uri;              /* URI to the original/edited image */
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
