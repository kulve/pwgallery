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
#include "image.h"
#include "gallery.h"
#include "html.h"
#include "vfs.h"

#include <glib.h>
#include <strings.h>              /* rindex */
#include <string.h>               /* strstr */

struct pwg_tag {
    gchar *tag;
    gsize len;
    gsize pos;
};

#define TAG_INDEX_TITLE       "<<TITLE>>"
#define TAG_INDEX_INDEX_IMG   "<<INDEX_IMG>>"

#define TAG_INDEX_IMAGE_PAGE  "<<IMAGE_PAGE>>"
#define TAG_INDEX_DESC        "<<DESC>>"
#define TAG_INDEX_THUMB_IMG   "<<THUMB_IMG>>"
#define TAG_INDEX_THUMB_W     "<<THUMB_W>>"
#define TAG_INDEX_THUMB_H     "<<THUMB_H>>"
#define TAG_INDEX_THUMB_ALT   "<<THUMB_ALT>>"

#define TAG_IMAGE_TITLE       "<<TITLE>>"
#define TAG_IMAGE_LINK        "<<IMAGE>>"
#define TAG_IMAGE_PREV        "<<PREV>>"
#define TAG_IMAGE_NEXT        "<<NEXT>>"
#define TAG_IMAGE_INDEX       "<<INDEX>>"
#define TAG_IMAGE_SIZE_1      "<<SIZE_1>>"
#define TAG_IMAGE_SIZE_2      "<<SIZE_2>>"
#define TAG_IMAGE_SIZE_3      "<<SIZE_3>>"
#define TAG_IMAGE_SIZE_4      "<<SIZE_4>>"
#define TAG_IMAGE_W           "<<IMAGE_W>>"
#define TAG_IMAGE_H           "<<IMAGE_H>>"
#define TAG_IMAGE_ALT         "<<IMAGE_ALT>>"
#define TAG_IMAGE_DESC        "<<DESC>>"



void _tag_replace(GString **templ, const gchar *tag, const gchar *value);

/*
 * Make index page html
 */
gboolean
html_make_index_page(struct data *data)
{
    guchar      *templ_index_data;
    gsize       templ_index_len;
    guchar      *templ_index_img_data;
    gsize       templ_index_img_len;
    gchar       *ext;
    gchar       *image_tmpl_ext;
    GSList      *images;  
    GString     *index_img;
    GString     *index_img_templ;
    GString     *index_templ;
    GString     *tmp;
    gchar       *index_page_uri;

    g_assert(data != NULL);

    g_debug("in html_make_index_page");


    /* read index page template to memory */
    vfs_read_file(data, data->gal->templ_index, &templ_index_data,
                  &templ_index_len);

    /* init g_string for index page template. Let's hope 100k is
       usually enough */
    index_templ = g_string_sized_new(100*1024);
    index_templ = g_string_append(index_templ, (gchar*)templ_index_data);
    g_free(templ_index_data);


    /* read index page template to memory */
    vfs_read_file(data, data->gal->templ_indeximg, &templ_index_img_data,
                  &templ_index_img_len);

    /* init g_string for index_img template. */
    index_img_templ = g_string_new((gchar *)templ_index_img_data);
    g_free(templ_index_img_data);


    /* init g_string for all index imgs. Let's hope 100k is usually enough */
    index_img = g_string_sized_new(100*1024);


    /* init g_string for single index imgs. Let's hope 4096 bytes is
       usually enough */
    tmp = g_string_sized_new(4096);


    /* get template image page extension to be used for links */
    image_tmpl_ext = rindex(data->gal->templ_image, '.');
    if (image_tmpl_ext == NULL) {
        /* no extension found, assume html */
        image_tmpl_ext = g_strdup("html");
    } else {
        /* copy extension */
        image_tmpl_ext = g_strdup(image_tmpl_ext + 1);
    }


    /* make the index_img for all images in gallery */
    images = data->gal->images;
    while(images != NULL) {
        struct image       *image = images->data;
        struct image_size  *size;
        gchar              tmpbuf[1024];

        if (image->sizes == NULL) {
            /* FIXME: popup */
            g_error("html_make_index_page: no sizes!");
        }

        size = image->sizes->data;

        /* copy empty index_img template */
        tmp = g_string_assign(tmp, index_img_templ->str);

        /* image_page */
        g_snprintf(tmpbuf, 1024, "%s.%s", 
                   image->basefilename, image_tmpl_ext);
        _tag_replace(&tmp, TAG_INDEX_IMAGE_PAGE, tmpbuf);

        /* description */
        _tag_replace(&tmp, TAG_INDEX_DESC, image->text);

        /* thumb_img */
        g_snprintf(tmpbuf, 1024, "thumbnails/%s.%s", 
                   image->basefilename, image->ext);
        _tag_replace(&tmp, TAG_INDEX_THUMB_IMG, tmpbuf);
        
        /* thumb_w */
        g_snprintf(tmpbuf, 1024, "%d", image->thumb_w);
        _tag_replace(&tmp, TAG_INDEX_THUMB_W, tmpbuf);

        /* thumb_h */
        g_snprintf(tmpbuf, 1024, "%d", image->thumb_h);
        _tag_replace(&tmp, TAG_INDEX_THUMB_H, tmpbuf);
        
        /* thumb_alt */
        g_snprintf(tmpbuf, 1024, "%dKb", size->size);
        _tag_replace(&tmp, TAG_INDEX_THUMB_ALT, tmpbuf);
        
        /* append data to index_img for later addition to index page */
        index_img = g_string_append(index_img, tmp->str);

        images = images->next;
    }
    g_string_free(tmp, TRUE);
    g_free(image_tmpl_ext);

    /* replace tags in index page template */
    _tag_replace(&index_templ, TAG_INDEX_TITLE, data->gal->name);
    _tag_replace(&index_templ, TAG_INDEX_INDEX_IMG, index_img->str);


    /* save index page to file */

    /* get template index extension */
    ext = rindex(data->gal->templ_index, '.');
    if (ext == NULL) {
        /* no extension found, assume html */
        ext = g_strdup("html");
    } else {
        /* copy extension */
        ext = g_strdup(ext + 1);
    }

    index_page_uri = g_strdup_printf("%s/index.%s", data->gal->output_dir, ext);
    g_free(ext);

    vfs_write_file(data, index_page_uri, 
                   (guchar*)index_templ->str, index_templ->len);
    g_string_free(index_templ, TRUE);
    g_free(index_page_uri);

    return TRUE;
}



/*
 * Make html for image pages
 */
gboolean
html_make_image_pages(struct data *data)
{
    guchar       *templ_page_data;
    gsize        templ_page_len;
    gchar        *index_ext;
    gchar        *page_ext;
    GSList       *images;
    GSList       *sizes;
    GString      *page_templ;
    GString      *page;
    struct image *prev_img = NULL;

    g_assert(data != NULL);

    g_debug("in html_make_image_pages");

    /* read index page template to memory */
    vfs_read_file(data, data->gal->templ_image, &templ_page_data,
                  &templ_page_len);

    /* init g_string for page template. */
    page_templ = g_string_new((gchar*)templ_page_data);
    g_free(templ_page_data);

    /* init g_string for page html. Let's hope 10k is usually enough */
    page = g_string_sized_new(10*1024);
   
    /* get index template extension */
    index_ext = rindex(data->gal->templ_index, '.');
    if (index_ext == NULL) {
        /* no extension found, assume html */
        index_ext = g_strdup("html");
    } else {
        /* copy extension */
        index_ext = g_strdup(index_ext + 1);
    }

    /* get image template extension */
    page_ext = rindex(data->gal->templ_image, '.');
    if (page_ext == NULL) {
        /* no extension found, assume html */
        page_ext = g_strdup("html");
    } else {
        /* copy extension */
        page_ext = g_strdup(page_ext + 1);
    }

    /* make the html pages for all images in gallery */
    images = data->gal->images;
    while(images != NULL) {
        struct image   *image = images->data;
        gchar          tmpbuf[1024];
        gboolean       first_size = TRUE;

        /* go through all image sizes */
        sizes = image->sizes;
        while(sizes) {
            struct image_size *size = sizes->data;

            /* copy empty image template */
            page = g_string_assign(page, page_templ->str);

            /* replace tags in index page template */
            
            /* title */
            _tag_replace(&page, TAG_IMAGE_TITLE, data->gal->name);
            
            /* prev link to previous image or, if null, to index */
            if (prev_img == NULL) {
                g_snprintf(tmpbuf, 1024, "%sindex.%s", 
                           (first_size ? "" : "../"), index_ext);
            } else {
                g_snprintf(tmpbuf, 1024, "%s.%s", 
                           prev_img->basefilename, page_ext);
            }
            _tag_replace(&page, TAG_IMAGE_PREV, tmpbuf);
            
            /* next link to next image or, if null, to index */
            if (images->next == NULL) {
                g_snprintf(tmpbuf, 1024, "%sindex.%s",
                           (first_size ? "" : "../"), index_ext);
            } else {
                g_snprintf(tmpbuf, 1024, "%s.%s", 
                           ((struct image *)(images->next->data))->basefilename,
                           page_ext);
            }
            _tag_replace(&page, TAG_IMAGE_NEXT, tmpbuf);
            
            /* link to index */
            g_snprintf(tmpbuf, 1024, "%sindex.%s", 
                       (first_size ? "" : "../"),
                       index_ext);
            _tag_replace(&page, TAG_IMAGE_INDEX, tmpbuf);
            
            /* image link */
            g_snprintf(tmpbuf, 1024, "%s%s.%s", 
                       (first_size ? "images/" : ""),
                       image->basefilename, image->ext);
            _tag_replace(&page, TAG_IMAGE_LINK, tmpbuf);
            
            /* link to size 1 (default size) image */
            g_snprintf(tmpbuf, 1024, "%s%s.%s", 
                       (first_size ? "" : "../"),
                       image->basefilename, page_ext);
            _tag_replace(&page, TAG_IMAGE_SIZE_1, tmpbuf);

            /* link to size 2 image */
            g_snprintf(tmpbuf, 1024, "%simages_%d/%s.%s", 
                       (first_size ? "" : "../"),
                       data->gal->image_h2,
                       image->basefilename, page_ext);
            _tag_replace(&page, TAG_IMAGE_SIZE_2, tmpbuf);

            /* link to size 3 image */
            g_snprintf(tmpbuf, 1024, "%simages_%d/%s.%s", 
                       (first_size ? "" : "../"),
                       data->gal->image_h3,
                       image->basefilename, page_ext);
            _tag_replace(&page, TAG_IMAGE_SIZE_3, tmpbuf);

            /* link to size 4 image */
            g_snprintf(tmpbuf, 1024, "%simages_%d/%s.%s", 
                       (first_size ? "" : "../"),
                       data->gal->image_h4,
                       image->basefilename, page_ext);
            _tag_replace(&page, TAG_IMAGE_SIZE_4, tmpbuf);

            /* image width */
            g_snprintf(tmpbuf, 1024, "%d", size->width);
            _tag_replace(&page, TAG_IMAGE_W, tmpbuf);
            
            /* image height */
            g_snprintf(tmpbuf, 1024, "%d", size->height);
            _tag_replace(&page, TAG_IMAGE_H, tmpbuf);
            
            /* image alt desc */
            _tag_replace(&page, TAG_IMAGE_ALT, "");
            
            /* image desc */
            _tag_replace(&page, TAG_IMAGE_DESC, image->text);
            
            /* save page to file */
            if (first_size) {
                g_snprintf(tmpbuf, 1024, "%s/%s.%s", 
                           data->gal->output_dir, 
                           image->basefilename, page_ext);
            } else {
                g_snprintf(tmpbuf, 1024, "%s/images_%d/%s.%s", 
                           data->gal->output_dir,
                           size->height, 
                           image->basefilename, page_ext);
            }
            vfs_write_file(data, tmpbuf, (guchar*)page->str, page->len);
            
            first_size = FALSE;
            sizes = sizes->next;
        }
        prev_img = image;
        images = images->next;
    }

    g_free(index_ext);
    g_string_free(page, TRUE);
    g_string_free(page_templ, TRUE);

    return TRUE;
}



/*
 *
 * Static functions
 *
 */


/*
 * Replace "tag" with "value" in "templ
 */
void _tag_replace(GString **templ, const gchar *tag, const gchar *value)
{
    gchar *pos;
    gssize intpos;

    g_assert(templ != NULL);
    g_assert(tag != NULL);
    g_assert(value != NULL);

    while((pos = strstr((*templ)->str, tag)) != NULL) {
        
        /* calculate position from the start */
        intpos = pos - (*templ)->str;
        
        /* replace tag with value */
        *templ = g_string_erase(*templ, intpos, strlen(tag));
        *templ = g_string_insert(*templ, intpos, value);
    }
}





/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
