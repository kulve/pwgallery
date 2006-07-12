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
#include "vfs.h"

#include <glib.h>
#include <libgnomevfs/gnome-vfs.h>
#include <stdlib.h>                 /* EXIT_FAILURE */

gboolean
vfs_is_file(struct data *data, const gchar *uri)
{
    GnomeVFSURI *guri;
    gboolean found;

    g_assert(data != NULL);
    g_assert(uri != NULL);

    guri = gnome_vfs_uri_new(uri);
    /* FIXME: should check that the uri is file */
    found = gnome_vfs_uri_exists(guri);
    gnome_vfs_uri_unref(guri);

    return found;
}



gboolean
vfs_is_dir(struct data *data, const gchar *uri)
{
    GnomeVFSURI *guri;
    gboolean found;

    g_assert(data != NULL);
    g_assert(uri != NULL);

    guri = gnome_vfs_uri_new(uri);
    /* FIXME: should check that the uri is directory */
    found = gnome_vfs_uri_exists(guri);
    gnome_vfs_uri_unref(guri);

    return found;
}



void
vfs_mkdir(struct data *data, const gchar *uri)
{
    GnomeVFSResult result;
    
    g_assert(data != NULL);
    g_assert(uri != NULL);


    result = gnome_vfs_make_directory(uri,
                                      GNOME_VFS_PERM_USER_ALL |
                                      GNOME_VFS_PERM_GROUP_ALL);
    if (result != GNOME_VFS_OK)
    {
        /* FIXME: show popup */
        g_warning("Exiting because failed to make directory '%s': %s", 
                  uri, gnome_vfs_result_to_string(result));
        exit(EXIT_FAILURE);
    }

}



void
vfs_read_file(struct data *data, const gchar *uri, gchar **content,
              gsize *content_len)
{
    GnomeVFSResult result;
    int file_size;

    g_assert(data != NULL);
    g_assert(uri != NULL);
    g_assert(content != NULL);
    g_assert(content_len != NULL);

    result = gnome_vfs_read_entire_file(uri, &file_size, content);
    if (result != GNOME_VFS_OK)
    {
        /* FIXME: show popup */
        g_warning("Exiting because failed to read entire file '%s': %s", 
                  uri, gnome_vfs_result_to_string(result));
        exit(EXIT_FAILURE);
    }

    /* -1 because of NULL termination */
    *content_len = file_size - 1;
}



void
vfs_write_file(struct data *data, const gchar *uri, const gchar *content,
               gsize content_len)
{
    gchar *dir;
    GnomeVFSURI *vfsuri;
    GnomeVFSResult result;
    GnomeVFSHandle *handle;
    GnomeVFSFileSize bytes_written;
    GnomeVFSFileSize bytes_written_total = 0;

    g_assert(data != NULL);
    g_assert(uri != NULL);
    g_assert(content != NULL);
    g_assert(content_len != 0);

    vfsuri = gnome_vfs_uri_new(uri);

    /* make dir if needed */
    dir = gnome_vfs_uri_extract_dirname(vfsuri);
    if (vfs_is_dir(data, dir) == FALSE)
    {
        vfs_mkdir(data, dir);
    }       

    /* open uri */
    result = gnome_vfs_create_uri(&handle, vfsuri,
                                  GNOME_VFS_OPEN_WRITE | 
                                  GNOME_VFS_OPEN_TRUNCATE,
                                  FALSE,
                                  GNOME_VFS_PERM_USER_READ |
                                  GNOME_VFS_PERM_USER_WRITE |
                                  GNOME_VFS_PERM_GROUP_READ |
                                  GNOME_VFS_PERM_GROUP_WRITE);
    if (result != GNOME_VFS_OK)
    {
        /* FIXME: show popup */
        g_warning("Exiting because failed to create uri '%s': %s", 
                  uri, gnome_vfs_result_to_string(result));
        exit(EXIT_FAILURE);
    }

    /* write file */
    while (bytes_written_total < content_len)
    {
        result = gnome_vfs_write(handle, content, 
                                 content_len - bytes_written_total,
                                 &bytes_written);
        if (result != GNOME_VFS_OK &&
            result != GNOME_VFS_ERROR_INTERRUPTED)
        {
            /* FIXME: show popup */
            g_warning("Exiting because failed to write file '%s': %s", 
                      uri, gnome_vfs_result_to_string(result));
            exit(EXIT_FAILURE);
        }
     
        bytes_written_total += bytes_written;
    }

    gnome_vfs_close(handle);
    gnome_vfs_uri_unref(vfsuri);
}


/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
