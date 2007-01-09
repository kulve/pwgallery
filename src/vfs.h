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

#ifndef PWGALLERY_VFS_H
#define PWGALLERY_VFS_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "main.h"

#include <glib.h>

/*
 * Check if uri is found and is a file
 */
gboolean vfs_is_file(struct data *data, const gchar *uri);

/*
 * Check if uri is found and is an image according to the mimetype
 */
gboolean vfs_is_image(struct data *data, const gchar *uri);

/*
 * Check if uri is found and directory
 */
gboolean vfs_is_dir(struct data *data, const gchar *uri);

/*
 * Make directory
 */
void vfs_mkdir(struct data *data, const gchar *uri);

/*
 * Rename URI
 */
void vfs_rename(struct data *data, const gchar *from, const gchar *to);

/*
 * Read a whole uri to buffer
 */
void vfs_read_file(struct data *data, const gchar *uri, guchar **content,
                   gsize *content_len);

/*
 * Write data to given uri
 */
void vfs_write_file(struct data *data, const gchar *uri, const guchar *content,
                    gsize content_len);

#endif

/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
