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
#include <string.h>                 /* strncmp */

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
vfs_is_image(struct data *data, const gchar *uri)
{
    GnomeVFSFileInfo info;
    GnomeVFSResult result;

    g_assert(data != NULL);
    g_assert(uri != NULL);

    /* get mime type */
    result = gnome_vfs_get_file_info(uri, &info, 
                                     GNOME_VFS_FILE_INFO_DEFAULT | 
                                     GNOME_VFS_FILE_INFO_GET_MIME_TYPE |
                                     GNOME_VFS_FILE_INFO_FORCE_SLOW_MIME_TYPE |
                                     GNOME_VFS_FILE_INFO_FOLLOW_LINKS);
    if (result == GNOME_VFS_OK) {
        const char *mime = gnome_vfs_file_info_get_mime_type(&info);
        if (strncmp(mime, "image/", 6) == 0) {
            return TRUE;
        }
    } 

    return FALSE;
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
    if (result != GNOME_VFS_OK) {
        /* FIXME: show popup */
        g_warning("Exiting because failed to make directory '%s': %s", 
                  uri, gnome_vfs_result_to_string(result));
        exit(EXIT_FAILURE);
    }

}



void
vfs_copy(struct data *data, const gchar *src, const gchar *dst)
{
    GnomeVFSResult result;
    GnomeVFSURI *src_uri, *dst_uri;

    g_assert(data != NULL);
    g_assert(src != NULL);
    g_assert(dst != NULL);

    src_uri = gnome_vfs_uri_new(src);
    dst_uri = gnome_vfs_uri_new(dst);

    g_assert(src_uri != NULL);
    g_assert(dst_uri != NULL);
    

    result = gnome_vfs_xfer_uri(src_uri,
                                dst_uri,
                                GNOME_VFS_XFER_DEFAULT,
                                GNOME_VFS_XFER_ERROR_MODE_ABORT,
                                GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE,
                                NULL,
                                NULL);

    gnome_vfs_uri_unref(src_uri);
    gnome_vfs_uri_unref(dst_uri);

    if (result != GNOME_VFS_OK) {
        /* FIXME: show popup */
        g_warning("Exiting because failed to xfer %s -> %s: %s", 
                  src, dst, gnome_vfs_result_to_string(result));
        exit(EXIT_FAILURE);
    }

}



void
vfs_rename(struct data *data, const gchar *from, const gchar *to)
{
    GnomeVFSResult result;
    
    g_assert(data != NULL);
    g_assert(from != NULL);
    g_assert(to != NULL);

    result = gnome_vfs_move(from, to, FALSE);

    if (result != GNOME_VFS_OK) {
        /* FIXME: show popup */
        g_warning("Exiting because failed to rename uri '%s' ->'%s': %s", 
                  from, to, gnome_vfs_result_to_string(result));
        exit(EXIT_FAILURE);
    }


}



void
vfs_read_file(struct data *data, const gchar *uri, guchar **content,
              gsize *content_len)
{
    GnomeVFSResult result;
    int file_size;

    g_assert(data != NULL);
    g_assert(uri != NULL);
    g_assert(content != NULL);
    g_assert(content_len != NULL);

    result = gnome_vfs_read_entire_file(uri, &file_size, (gchar **)content);
    if (result != GNOME_VFS_OK) {
        /* FIXME: show popup */
        g_warning("Exiting because failed to read entire file '%s': %s", 
                  uri, gnome_vfs_result_to_string(result));
        exit(EXIT_FAILURE);
    }

    /* -1 because of NULL termination */
    *content_len = file_size - 1;
}



void
vfs_write_file(struct data *data, const gchar *uri, const guchar *content,
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
    if (vfs_is_dir(data, dir) == FALSE) {
        vfs_mkdir(data, dir);
    }       
    g_free(dir);

    /* open uri */
    result = gnome_vfs_create_uri(&handle, vfsuri,
                                  GNOME_VFS_OPEN_WRITE | 
                                  GNOME_VFS_OPEN_TRUNCATE,
                                  FALSE,
                                  GNOME_VFS_PERM_USER_READ |
                                  GNOME_VFS_PERM_USER_WRITE |
                                  GNOME_VFS_PERM_GROUP_READ |
                                  GNOME_VFS_PERM_GROUP_WRITE);
    if (result != GNOME_VFS_OK) {
        /* FIXME: show popup */
        g_warning("Exiting because failed to create uri '%s': %s", 
                  uri, gnome_vfs_result_to_string(result));
        exit(EXIT_FAILURE);
    }

    /* write file */
    while (bytes_written_total < content_len) {
        result = gnome_vfs_write(handle, content, 
                                 content_len - bytes_written_total,
                                 &bytes_written);
        if (result != GNOME_VFS_OK &&
            result != GNOME_VFS_ERROR_INTERRUPTED) {
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
