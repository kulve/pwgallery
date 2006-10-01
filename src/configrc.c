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
#include "configrc.h"

#include <glib.h>


static void set_defaults(GKeyFile *keyfile);
static void set_rc_values(struct data *data, GKeyFile *keyfile);
static void get_rc_values(struct data *data, GKeyFile *keyfile);

void
configrc_load(struct data *data)
{
    gchar       *dir_uri;
    gchar       *configrc_file;
    gchar       *configrc_data;
    gsize       configrc_data_len;
    GKeyFile    *keyfile;

    g_assert(data != NULL);

    /* check that the proper directories exist */
    dir_uri = g_strdup_printf("file://%s/%s", 
                          g_get_home_dir(), PWGALLERY_CONFIGRC_DIR);
    if (vfs_is_dir(data, dir_uri) == FALSE)
        vfs_mkdir(data, dir_uri);
    g_free(dir_uri);

    dir_uri = g_strdup_printf("file://%s/%s/%s", 
                              g_get_home_dir(), PWGALLERY_CONFIGRC_DIR,
                              PWGALLERY_DEFAULT_GAL_DIR);
    if (vfs_is_dir(data, dir_uri) == FALSE)
        vfs_mkdir(data, dir_uri);
    g_free(dir_uri);

    dir_uri = g_strdup_printf("file://%s/%s/%s", 
                              g_get_home_dir(), PWGALLERY_CONFIGRC_DIR,
                              PWGALLERY_DEFAULT_TEMPL_DIR);
    if (vfs_is_dir(data, dir_uri) == FALSE)
        vfs_mkdir(data, dir_uri);
    g_free(dir_uri);


    /* construct path for configrc file */
    configrc_file = g_strdup_printf("file://%s/%s/%s", g_get_home_dir(), 
                                    PWGALLERY_CONFIGRC_DIR,
                                    PWGALLERY_CONFIGRC_FILE);

    keyfile = g_key_file_new();

    /* Try to load configrc values if the file exists */
    if (vfs_is_file(data, configrc_file) == TRUE)
    {
        gboolean ok;

        /* read values from configrc file */
        vfs_read_file(data, configrc_file, &configrc_data, &configrc_data_len);

        ok = g_key_file_load_from_data(keyfile,
                                       configrc_data, configrc_data_len,
                                       G_KEY_FILE_KEEP_COMMENTS | 
                                       G_KEY_FILE_KEEP_TRANSLATIONS ,
                                       NULL);
        g_assert(ok == TRUE);
        g_free(configrc_data);
        configrc_data = NULL;
    }

    /* set defaults for those keys that are missing */
    set_defaults(keyfile);
    
    /* save configrc file to disk (maybe if didn't exists or maybe new
     * keys are introduced in a new version. ie. just be sure) */
    configrc_data = g_key_file_to_data(keyfile, &configrc_data_len, NULL);
    vfs_write_file(data, configrc_file, configrc_data, configrc_data_len);

    set_rc_values(data, keyfile);

    g_free(configrc_data);
    g_free(configrc_file);
    g_key_file_free(keyfile);


}



void
configrc_save(struct data *data)
{
    gchar       *configrc_file;
    gchar       *configrc_data;
    gsize       configrc_data_len;
    GKeyFile    *keyfile;
    gboolean    ok;

    g_assert(data != NULL);

    configrc_file = g_strdup_printf("file://%s/%s/%s", g_get_home_dir(), 
                                    PWGALLERY_CONFIGRC_DIR,
                                    PWGALLERY_CONFIGRC_FILE);
    keyfile = g_key_file_new();

    /* FIXME: this is a bit ugly, but we'll read all from the file
     * because of the comments. We should probably keep the *keyfile
     * loaded in configrc_load */

    /* read values from configrc file */
    vfs_read_file(data, configrc_file, &configrc_data, &configrc_data_len);
    
    ok = g_key_file_load_from_data(keyfile,
                                   configrc_data, configrc_data_len,
                                   G_KEY_FILE_KEEP_COMMENTS | 
                                   G_KEY_FILE_KEEP_TRANSLATIONS ,
                                   NULL);
    g_assert(ok == TRUE);
    
    /* set rc values from struct data to keyfile */
    get_rc_values(data, keyfile);

    /* save configrc file to disk*/
    configrc_data = g_key_file_to_data(keyfile, &configrc_data_len, NULL);
    
    vfs_write_file(data, configrc_file, configrc_data, configrc_data_len);
}


/**********************
 *                    *
 * Static functions   *
 *                    *
 **********************/


static void
set_defaults(GKeyFile *keyfile)
{
    gchar *dir;
    GError *error = NULL;

    g_assert(keyfile != NULL);

    /* Image dir */
    if (g_key_file_has_key(keyfile, "Default", PWGALLERY_RCKEY_IMAGE_DIR,
                           NULL ) == FALSE)
    {
        g_key_file_set_value(keyfile, "Default", PWGALLERY_RCKEY_IMAGE_DIR,
                             PWGALLERY_DEFAULT_IMAGE_DIR);

        g_key_file_set_comment(keyfile, "Default", PWGALLERY_RCKEY_IMAGE_DIR, 
                               _("Image directory"),
                               NULL);        
    }

    /* Galleries dir */
    if (g_key_file_has_key(keyfile, "Default", PWGALLERY_RCKEY_GAL_DIR,
                           NULL ) == FALSE)
    {
        dir = g_strdup_printf("file://%s/%s/%s", g_get_home_dir(), 
                              PWGALLERY_CONFIGRC_DIR,
                              PWGALLERY_DEFAULT_GAL_DIR);
        g_key_file_set_value(keyfile, "Default", PWGALLERY_RCKEY_GAL_DIR,
                             dir);
        g_free(dir);
        g_key_file_set_comment(keyfile, "Default", PWGALLERY_RCKEY_GAL_DIR, 
                               _("Galleries directory"),
                               NULL);        
    }

    /* Templates dir */
    if (g_key_file_has_key(keyfile, "Default", PWGALLERY_RCKEY_TEMPL_DIR,
                           NULL ) == FALSE)
    {
        dir = g_strdup_printf("file://%s/%s/%s", g_get_home_dir(), 
                              PWGALLERY_CONFIGRC_DIR,
                              PWGALLERY_DEFAULT_TEMPL_DIR);
        g_key_file_set_value(keyfile, "Default", PWGALLERY_RCKEY_TEMPL_DIR,
                             dir);
        g_free(dir);
        g_key_file_set_comment(keyfile, "Default", PWGALLERY_RCKEY_TEMPL_DIR, 
                               _("Template directory"),
                               NULL);        
    }

    /* Output dir */
    if (g_key_file_has_key(keyfile, "Default", PWGALLERY_RCKEY_OUTPUT_DIR,
                           NULL ) == FALSE)
    {
        g_key_file_set_value(keyfile, "Default", PWGALLERY_RCKEY_OUTPUT_DIR,
                             PWGALLERY_DEFAULT_OUTPUT_DIR);

        g_key_file_set_comment(keyfile, "Default", PWGALLERY_RCKEY_OUTPUT_DIR, 
                               _("Default output directory for galleries"),
                               NULL);        
    }

    /* Page generator style */
    if (g_key_file_has_key(keyfile, "Default", PWGALLERY_RCKEY_PAGE_GEN,
                           NULL ) == FALSE)
    {        
        g_key_file_set_value(keyfile, "Default", PWGALLERY_RCKEY_PAGE_GEN,
                             PWGALLERY_DEFAULT_PAGE_GEN);

        g_key_file_set_comment(keyfile, "Default", PWGALLERY_RCKEY_PAGE_GEN, 
                               _("Default page generator style"),
                               NULL);
    }

    /* Page generator program */
    if (g_key_file_has_key(keyfile, "Default", PWGALLERY_RCKEY_PAGE_GEN_PROG,
                           NULL ) == FALSE)
    {        
        g_key_file_set_value(keyfile, "Default", PWGALLERY_RCKEY_PAGE_GEN_PROG,
                             PWGALLERY_DEFAULT_PAGE_GEN_PROG);

        g_key_file_set_comment(keyfile, "Default", 
                               PWGALLERY_RCKEY_PAGE_GEN_PROG, 
                               _("Default page generator program"),
                               NULL);
    }

    /* Thumbnail width */
    if (g_key_file_has_key(keyfile, "Default", PWGALLERY_RCKEY_THUMB_W,
                           NULL ) == FALSE)
    {
        g_key_file_set_value(keyfile, "Default", PWGALLERY_RCKEY_THUMB_W,
                             PWGALLERY_DEFAULT_THUMB_W);

        g_key_file_set_comment(keyfile, "Default", PWGALLERY_RCKEY_THUMB_W, 
                               _("Default width of the thumbnails"),
                               NULL);    
    }


    /* Image height */
    if (g_key_file_has_key(keyfile, "Default", PWGALLERY_RCKEY_IMAGE_H,
                           NULL ) == FALSE)
    {        
        g_key_file_set_value(keyfile, "Default", PWGALLERY_RCKEY_IMAGE_H,
                             PWGALLERY_DEFAULT_IMAGE_H);

        g_key_file_set_comment(keyfile, "Default", PWGALLERY_RCKEY_IMAGE_H, 
                               _("Default height of the images"),
                               NULL);
    }

    /* Image height2 */
    if (g_key_file_has_key(keyfile, "Default", PWGALLERY_RCKEY_IMAGE_H2,
                           NULL ) == FALSE)
    {        
        g_key_file_set_value(keyfile, "Default", PWGALLERY_RCKEY_IMAGE_H2,
                             PWGALLERY_DEFAULT_IMAGE_H2);

        g_key_file_set_comment(keyfile, "Default", PWGALLERY_RCKEY_IMAGE_H2, 
                               _("Default height of the second set of images"),
                               NULL);
    }

    /* Image height3 */
    if (g_key_file_has_key(keyfile, "Default", PWGALLERY_RCKEY_IMAGE_H3,
                           NULL ) == FALSE)
    {        
        g_key_file_set_value(keyfile, "Default", PWGALLERY_RCKEY_IMAGE_H3,
                             PWGALLERY_DEFAULT_IMAGE_H3);

        g_key_file_set_comment(keyfile, "Default", PWGALLERY_RCKEY_IMAGE_H3, 
                               _("Default height of the third set of images"),
                               NULL);
    }

    /* Image height4 */
    if (g_key_file_has_key(keyfile, "Default", PWGALLERY_RCKEY_IMAGE_H4,
                           NULL ) == FALSE)
    {        
        g_key_file_set_value(keyfile, "Default", PWGALLERY_RCKEY_IMAGE_H4,
                             PWGALLERY_DEFAULT_IMAGE_H4);

        g_key_file_set_comment(keyfile, "Default", PWGALLERY_RCKEY_IMAGE_H4, 
                               _("Default height of the fourth set of images"),
                               NULL);
    }


    /* Index page template */
    if (g_key_file_has_key(keyfile, "Default", PWGALLERY_RCKEY_TEMPL_INDEX,
                           NULL ) == FALSE)
    {    
        dir = g_strdup_printf("file://%s/%s/%s/%s", g_get_home_dir(), 
                              PWGALLERY_CONFIGRC_DIR,
                              PWGALLERY_DEFAULT_TEMPL_DIR,
                              PWGALLERY_DEFAULT_TEMPL_INDEX);
        g_key_file_set_value(keyfile, "Default", PWGALLERY_RCKEY_TEMPL_INDEX,
                             dir);
        g_free(dir);

        g_key_file_set_comment(keyfile, "Default", PWGALLERY_RCKEY_TEMPL_INDEX, 
                               _("Default template for index page"),
                               NULL);
    }

    /* Index page per image template */
    if (g_key_file_has_key(keyfile, "Default", PWGALLERY_RCKEY_TEMPL_INDEXIMG,
                           NULL ) == FALSE)
    {        
        dir = g_strdup_printf("file://%s/%s/%s/%s", g_get_home_dir(), 
                              PWGALLERY_CONFIGRC_DIR,
                              PWGALLERY_DEFAULT_TEMPL_DIR,
                              PWGALLERY_DEFAULT_TEMPL_INDEXIMG);
        g_key_file_set_value(keyfile, "Default", PWGALLERY_RCKEY_TEMPL_INDEXIMG,
                             dir);
        g_free(dir);

        g_key_file_set_comment(keyfile, "Default",
                               PWGALLERY_RCKEY_TEMPL_INDEXIMG, 
                               _("Default template for index page per image"),
                               NULL);
    }

    /* Index page per generic template */
    if (g_key_file_has_key(keyfile, "Default", PWGALLERY_RCKEY_TEMPL_INDEXGEN,
                           NULL ) == FALSE)
    {        
        dir = g_strdup_printf("file://%s/%s/%s/%s", g_get_home_dir(), 
                              PWGALLERY_CONFIGRC_DIR,
                              PWGALLERY_DEFAULT_TEMPL_DIR,
                              PWGALLERY_DEFAULT_TEMPL_INDEXGEN);
        g_key_file_set_value(keyfile, "Default", PWGALLERY_RCKEY_TEMPL_INDEXGEN,
                             dir);
        g_free(dir);

        g_key_file_set_comment(keyfile, "Default",
                               PWGALLERY_RCKEY_TEMPL_INDEXGEN, 
                               _("Default template for index page per generic"),
                               NULL);
    }

    /* Image page template */
    if (g_key_file_has_key(keyfile, "Default", PWGALLERY_RCKEY_TEMPL_IMAGE,
                           NULL ) == FALSE)
    {        
        dir = g_strdup_printf("file://%s/%s/%s/%s", g_get_home_dir(), 
                              PWGALLERY_CONFIGRC_DIR,
                              PWGALLERY_DEFAULT_TEMPL_DIR,
                              PWGALLERY_DEFAULT_TEMPL_IMAGE);
        g_key_file_set_value(keyfile, "Default", PWGALLERY_RCKEY_TEMPL_IMAGE,
                             dir);
        g_free(dir);

        g_key_file_set_comment(keyfile, "Default", PWGALLERY_RCKEY_TEMPL_IMAGE, 
                               _("Default template for image page"),
                               NULL);
    }

    /* Generic page template */
    if (g_key_file_has_key(keyfile, "Default", PWGALLERY_RCKEY_TEMPL_GEN,
                           NULL ) == FALSE)
    {        
        dir = g_strdup_printf("file://%s/%s/%s/%s", g_get_home_dir(), 
                              PWGALLERY_CONFIGRC_DIR,
                              PWGALLERY_DEFAULT_TEMPL_DIR,
                              PWGALLERY_DEFAULT_TEMPL_GEN);
        g_key_file_set_value(keyfile, "Default", PWGALLERY_RCKEY_TEMPL_GEN,
                             dir);
        g_free(dir);

        g_key_file_set_comment(keyfile, "Default", PWGALLERY_RCKEY_TEMPL_GEN, 
                               _("Default template for generic page"),
                               NULL);
    }


    /* Remove exif info */
    if (g_key_file_has_key(keyfile, "Default", PWGALLERY_RCKEY_REMOVE_EXIF,
                           NULL ) == FALSE)
    {        
        g_key_file_set_value(keyfile, "Default", PWGALLERY_RCKEY_REMOVE_EXIF,
                             PWGALLERY_DEFAULT_REMOVE_EXIF);

        g_key_file_set_comment(keyfile, "Default", PWGALLERY_RCKEY_REMOVE_EXIF, 
                               _("Default value for exif information "
                                 "removal (0 for keep, 1 for remove)"),
                               NULL);
    }

    /* Rename images */
    if (g_key_file_has_key(keyfile, "Default", PWGALLERY_RCKEY_RENAME,
                           NULL ) == FALSE)
    {
        g_key_file_set_value(keyfile, "Default", PWGALLERY_RCKEY_RENAME,
                             PWGALLERY_DEFAULT_RENAME);

        g_key_file_set_comment(keyfile, "Default", PWGALLERY_RCKEY_RENAME, 
                               _("Default value for renameing images "
                                 "automatically"),
                               NULL);
    }


    /* Global comment */
    g_key_file_set_comment(keyfile, "Default", NULL, 
                           _("This is automatically created configrc file "
                             "for Penguin's Web Gallery."),
                           &error);
    if (error != NULL)
    {
        g_warning("Failed to set comment: %s",
                  error->message);
        g_error_free(error);
        error = NULL;
    }

}

static void
set_rc_values(struct data *data, GKeyFile *keyfile)
{
    gchar *value;

    g_assert(data != NULL);
    g_assert(keyfile != NULL);

    /* Image dir */
    value = g_key_file_get_value(keyfile, "Default",
                                 PWGALLERY_RCKEY_IMAGE_DIR,  NULL);
    g_assert(value != NULL);
    g_free(data->img_dir);
    data->img_dir = value;

    /* Templates dir */
    value = g_key_file_get_value(keyfile, "Default",
                                 PWGALLERY_RCKEY_TEMPL_DIR,  NULL);
    g_assert(value != NULL);
    g_free(data->templ_dir);
    data->templ_dir = value;

    /* Galleries dir */
    value = g_key_file_get_value(keyfile, "Default",
                                 PWGALLERY_RCKEY_GAL_DIR,  NULL);
    g_assert(value != NULL);
    g_free(data->gal_dir);
    data->gal_dir = value;

    /* Output dir */
    value = g_key_file_get_value(keyfile, "Default",
                                 PWGALLERY_RCKEY_OUTPUT_DIR,  NULL);
    g_assert(value != NULL);
    g_free(data->output_dir);
    data->output_dir = value;

    /* Page generator style, no error checking.. */
    data->page_gen = g_key_file_get_integer(keyfile, "Default",
                                            PWGALLERY_RCKEY_PAGE_GEN,  NULL);

    /* Page generator program */
    value = g_key_file_get_value(keyfile, "Default",
                                 PWGALLERY_RCKEY_PAGE_GEN_PROG,  NULL);
    g_assert(value != NULL);
    g_free(data->page_gen_prog);
    data->page_gen_prog = value;

    /* Thumbnail width, no error checking.. */
    data->thumb_w = g_key_file_get_integer(keyfile, "Default",
                                           PWGALLERY_RCKEY_THUMB_W,  NULL);

    /* Image height, no error checking.. */
    data->image_h = g_key_file_get_integer(keyfile, "Default",
                                           PWGALLERY_RCKEY_IMAGE_H,  NULL);

    /* Image height2, no error checking.. */
    data->image_h2 = g_key_file_get_integer(keyfile, "Default",
                                            PWGALLERY_RCKEY_IMAGE_H2,  NULL);

    /* Image height3, no error checking.. */
    data->image_h3 = g_key_file_get_integer(keyfile, "Default",
                                            PWGALLERY_RCKEY_IMAGE_H3,  NULL);

    /* Image height4, no error checking.. */
    data->image_h4 = g_key_file_get_integer(keyfile, "Default",
                                            PWGALLERY_RCKEY_IMAGE_H4,  NULL);

    /* Index page template */
    value = g_key_file_get_value(keyfile, "Default",
                                 PWGALLERY_RCKEY_TEMPL_INDEX,  NULL);
    g_assert(value != NULL);
    g_free(data->templ_index);
    data->templ_index = value;

    /* Index page per image template */
    value = g_key_file_get_value(keyfile, "Default",
                                 PWGALLERY_RCKEY_TEMPL_INDEXIMG,  NULL);
    g_assert(value != NULL);
    g_free(data->templ_indeximg);
    data->templ_indeximg = value;

    /* Index page per generic template */
    value = g_key_file_get_value(keyfile, "Default",
                                 PWGALLERY_RCKEY_TEMPL_INDEXGEN,  NULL);
    g_assert(value != NULL);
    g_free(data->templ_indexgen);
    data->templ_indexgen = value;

    /* Image page template */
    value = g_key_file_get_value(keyfile, "Default",
                                 PWGALLERY_RCKEY_TEMPL_IMAGE,  NULL);
    g_assert(value != NULL);
    g_free(data->templ_image);
    data->templ_image = value;

    /* Generic page template */
    value = g_key_file_get_value(keyfile, "Default",
                                 PWGALLERY_RCKEY_TEMPL_GEN,  NULL);
    g_assert(value != NULL);
    g_free(data->templ_gen);
    data->templ_gen = value;

    /* Remove exif info, no error checking */
    data->remove_exif = g_key_file_get_boolean(keyfile, "Default",
                                               PWGALLERY_RCKEY_REMOVE_EXIF,
                                               NULL);

    /* Rename images */
    data->rename = g_key_file_get_boolean(keyfile, "Default",
                                          PWGALLERY_RCKEY_RENAME,  NULL);

}



static void
get_rc_values(struct data *data, GKeyFile *keyfile)
{
    g_assert(data != NULL);
    g_assert(keyfile != NULL);

    /* Image dir */
    g_assert(data->img_dir != NULL);
    g_key_file_set_value(keyfile, "Default",
                         PWGALLERY_RCKEY_IMAGE_DIR, data->img_dir);

    /* Templates dir */
    g_assert(data->templ_dir != NULL);
    g_key_file_set_value(keyfile, "Default",
                         PWGALLERY_RCKEY_TEMPL_DIR, data->templ_dir);

    /* Galleries dir */
    g_assert(data->gal_dir != NULL);
    g_key_file_set_value(keyfile, "Default",
                         PWGALLERY_RCKEY_GAL_DIR, data->gal_dir);

    /* Output dir */
    g_assert(data->output_dir != NULL);
    g_key_file_set_value(keyfile, "Default",
                         PWGALLERY_RCKEY_OUTPUT_DIR, data->output_dir);

    /* Page generator style */
    g_key_file_set_integer(keyfile, "Default",
                           PWGALLERY_RCKEY_PAGE_GEN, data->page_gen);

    /* Page generator program */
    /* FIXME: this is not implemented yet, so it can be NULL.. */
    if (data->page_gen_prog == NULL)
        data->page_gen_prog = g_strdup("file:///tmp/unimplemented.sh");
    g_key_file_set_value(keyfile, "Default",
                         PWGALLERY_RCKEY_PAGE_GEN_PROG, data->page_gen_prog);

    /* Thumbnail width */
    g_key_file_set_integer(keyfile, "Default",
                           PWGALLERY_RCKEY_THUMB_W, data->thumb_w);

    /* Image height */
    g_key_file_set_integer(keyfile, "Default",
                           PWGALLERY_RCKEY_IMAGE_H, data->image_h);

    /* Image height2 */
    g_key_file_set_integer(keyfile, "Default",
                           PWGALLERY_RCKEY_IMAGE_H2, data->image_h2);

    /* Image height3 */
    g_key_file_set_integer(keyfile, "Default",
                           PWGALLERY_RCKEY_IMAGE_H3, data->image_h3);

    /* Image height4 */
    g_key_file_set_integer(keyfile, "Default",
                           PWGALLERY_RCKEY_IMAGE_H4, data->image_h4);

    /* Index page template */
    g_assert(data->templ_index != NULL);
    g_key_file_set_value(keyfile, "Default",
                         PWGALLERY_RCKEY_TEMPL_INDEX, data->templ_index);

    /* Index page per image template */
    g_assert(data->templ_indeximg != NULL);
    g_key_file_set_value(keyfile, "Default",
                         PWGALLERY_RCKEY_TEMPL_INDEXIMG, data->templ_indeximg);

    /* Index page per generic template */
    g_assert(data->templ_indexgen != NULL);
    g_key_file_set_value(keyfile, "Default",
                         PWGALLERY_RCKEY_TEMPL_INDEXGEN, data->templ_indexgen);

    /* Image page template */
    g_assert(data->templ_image != NULL);
    g_key_file_set_value(keyfile, "Default",
                         PWGALLERY_RCKEY_TEMPL_IMAGE, data->templ_image);

    /* Generic page template */
    g_assert(data->templ_gen != NULL);
    g_key_file_set_value(keyfile, "Default",
                         PWGALLERY_RCKEY_TEMPL_GEN, data->templ_gen);

    /* Remove exif info, no error checking */
    g_key_file_set_boolean(keyfile, "Default",
                           PWGALLERY_RCKEY_REMOVE_EXIF, data->remove_exif);

    /* Rename images */
    g_key_file_set_boolean(keyfile, "Default",
                           PWGALLERY_RCKEY_RENAME, data->rename);

}

/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
