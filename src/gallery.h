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

#ifndef PWGALLERY_GALLERY_H
#define PWGALLERY_GALLERY_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "main.h"

#include <glib.h>
#include <gtk/gtk.h>

/* 
 * Initialize gallery
 */
void gallery_init(struct data *data);

/* 
 * New gallery
 */
void gallery_new(struct data *data);

/* 
 * Open gallery
 */
void gallery_open(struct data *data);

/* 
 * Save gallery
 */
void gallery_save(struct data *data);

/* 
 * Save gallery as
 */
void gallery_save_as(struct data *data);

/* 
 * Free gallery
 */
void gallery_free(struct data *data);

/*
 * Add new images to gallery
 */
void gallery_add_new_images(struct data *data, GSList *uris);

/*
 * Open existing images to gallery
 */
void gallery_open_images(struct data *data, GSList *imgs);

#endif

/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
