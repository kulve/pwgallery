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

#include "xml.h"
#include "main.h"
#include "image.h"
#include "gallery.h"

#include <glib.h>

#include <libxml/parser.h>


static void parse_gal_settings(struct data *data, xmlNodePtr node);
static struct image *parse_image_settings(struct data *data, xmlNodePtr node);
static struct image *parse_gen_settings(struct data *data, xmlNodePtr node);

/*
 * Create xml content for the gallery ready to be written to disk.
 */
guchar *
xml_gal_write(struct data *data, gsize *len)
{
    xmlDocPtr doc;
    xmlNodePtr gallery, settings, pages, page, page_settings;
    xmlChar *xmlbuff;
    gchar tmp_setting[256];
    int bufsize;
    GSList *list;

    g_assert(data != NULL);
    g_assert(len != NULL);

    g_debug("in xml_gal_write");

    /* CHECKME: extra block to help emacs with indenting.. */
    { LIBXML_TEST_VERSION }

    /* create new xml with pwgallery as the root element */
    doc = xmlNewDoc(BAD_CAST "1.0");
    gallery = xmlNewNode(NULL, BAD_CAST "pwgallery");
    xmlDocSetRootElement(doc, gallery);

    /* add settings node under pwgallery node */
    settings = xmlNewNode(NULL, BAD_CAST "settings");
    xmlAddChild(gallery, settings);

    /* add settings under the settings node */
    xmlNewChild(settings, NULL, BAD_CAST "version",
                BAD_CAST VERSION);
    xmlNewTextChild(settings, NULL, BAD_CAST "name",
                BAD_CAST data->gal->name);
    xmlNewTextChild(settings, NULL, BAD_CAST "dir_name",
                BAD_CAST data->gal->dir_name);
    xmlNewTextChild(settings, NULL, BAD_CAST "desc",
                BAD_CAST data->gal->desc);
    xmlNewChild(settings, NULL, BAD_CAST "output_dir",
                BAD_CAST data->gal->base_dir);
    xmlNewChild(settings, NULL, BAD_CAST "page_gen_prog",
                BAD_CAST data->gal->page_gen_prog);
    xmlNewChild(settings, NULL, BAD_CAST "templ_index",
                BAD_CAST data->gal->templ_index);
    xmlNewChild(settings, NULL, BAD_CAST "templ_indeximg",
                BAD_CAST data->gal->templ_indeximg);
    xmlNewChild(settings, NULL, BAD_CAST "templ_indexgen",
                BAD_CAST data->gal->templ_indexgen);
    xmlNewChild(settings, NULL, BAD_CAST "templ_image",
                BAD_CAST data->gal->templ_image);
    xmlNewChild(settings, NULL, BAD_CAST "templ_gen",
                BAD_CAST data->gal->templ_gen);
    
    g_snprintf(tmp_setting, 256, "%d", data->gal->page_gen);
    xmlNewChild(settings, NULL, BAD_CAST "page_gen", BAD_CAST tmp_setting);

    g_snprintf(tmp_setting, 256, "%d", data->gal->thumb_w);
    xmlNewChild(settings, NULL, BAD_CAST "thumb_w", BAD_CAST tmp_setting);

    g_snprintf(tmp_setting, 256, "%d", data->gal->image_h);
    xmlNewChild(settings, NULL, BAD_CAST "image_h", BAD_CAST tmp_setting);

    g_snprintf(tmp_setting, 256, "%d", data->gal->image_h2);
    xmlNewChild(settings, NULL, BAD_CAST "image_h2", BAD_CAST tmp_setting);

    g_snprintf(tmp_setting, 256, "%d", data->gal->image_h3);
    xmlNewChild(settings, NULL, BAD_CAST "image_h3", BAD_CAST tmp_setting);

    g_snprintf(tmp_setting, 256, "%d", data->gal->image_h4);
    xmlNewChild(settings, NULL, BAD_CAST "image_h4", BAD_CAST tmp_setting);

    xmlNewChild(settings, NULL, BAD_CAST "edited", 
                BAD_CAST (data->gal->edited ? "true" : "false"));

    xmlNewChild(settings, NULL, BAD_CAST "remove_exif", 
                BAD_CAST (data->gal->remove_exif ? "true" : "false"));

    xmlNewChild(settings, NULL, BAD_CAST "rename", 
                BAD_CAST (data->gal->rename ? "true" : "false"));


    
    /* add pages node under pwgallery node */
    pages = xmlNewNode(NULL, BAD_CAST "pages");
    xmlAddChild(gallery, pages);

    /* add pages (images/generic) under the pages node */
    list = data->gal->images;
    while(list != NULL)
    {
        struct image *img = list->data;

        /* FIXME: how to support generic pages here? */

        page = xmlNewNode(NULL, BAD_CAST "image");
        xmlAddChild(pages, page);
        
        /* add page settings node under page node */
        page_settings = xmlNewNode(NULL, BAD_CAST "settings");
        xmlAddChild(page, page_settings);
        
        /* add page settings under the page settings node */
        xmlNewTextChild(page_settings, NULL, BAD_CAST "text",
                        BAD_CAST img->text);

        xmlNewTextChild(page_settings, NULL, BAD_CAST "uri",
                        BAD_CAST img->uri);

        g_snprintf(tmp_setting, 256, "%f", img->gamma);
        xmlNewChild(page_settings, NULL, BAD_CAST "gamma",
                    BAD_CAST tmp_setting);

        g_snprintf(tmp_setting, 256, "%d", img->rotate);
        xmlNewChild(page_settings, NULL, BAD_CAST "rotate",
                    BAD_CAST tmp_setting);

        g_snprintf(tmp_setting, 256, "%d", img->image_h);
        xmlNewChild(page_settings, NULL, BAD_CAST "image_h",
                    BAD_CAST tmp_setting);

        g_snprintf(tmp_setting, 256, "%s", 
                   img->nomodify == TRUE ? "true" : "false");
        xmlNewChild(page_settings, NULL, BAD_CAST "nomodify",
                    BAD_CAST tmp_setting);

        list = list->next;
    }

    /* dump xml to a buffer */
    xmlDocDumpFormatMemory(doc, &xmlbuff, &bufsize, 1);
    *len = (gsize)bufsize;

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return (guchar *)xmlbuff;
}


/*
 * This parses a gallery xml file content and replaces current gallery
 * with it. The current gallery should be freshly initialized before
 * calling this function.
 */
void
xml_gal_parse(struct data *data, guchar *content, gsize len)
{
    GSList *list = NULL;
    xmlDocPtr doc;
    xmlNodePtr gallery = NULL, pages = NULL, current = NULL;

    g_assert(data != NULL);
    g_assert(content != NULL);

    g_debug("in xml_gal_parse");

    /* CHECKME: extra block to help emacs with indenting.. */
    { LIBXML_TEST_VERSION }
    
    /* FIXME: XML_PARSE_NONET? XML_PARSE_NOENT? */
    doc = xmlReadMemory((gchar*)content, len, "gallery.xml", NULL, 0);
    if (doc == NULL)
    {
        /* FIXME: popup */
        g_warning("xml_gal_parse: Failed to parse document\n");
        return;
    }
    
    /* get root and check that it matches <pwgallery> */
    gallery = xmlDocGetRootElement(doc);

    if (xmlStrcmp(gallery->name, (const xmlChar *) "pwgallery")) {
        /* FIXME: popup */
		g_warning("xml_gal_parse: Document of the wrong type");
		xmlFreeDoc(doc);
        xmlCleanupParser();
		return;
	}
    
    /* find <settings> */
    current = gallery->xmlChildrenNode;
    while (current != NULL)
    {
        if ((!xmlStrcmp(current->name, (const xmlChar *) "settings")))
        {
            parse_gal_settings(data, current->xmlChildrenNode);
            break;
        }
        current = current->next;
    }
    
    /* FIXME: invalid xml. Better error handling? */
    g_assert(current != NULL);

    /* find subelements of <pages> */
    while (current != NULL)
    {
        if ((!xmlStrcmp(current->name, (const xmlChar *) "pages")))
        {
            pages = current->xmlChildrenNode;
            break;
        }
        current = current->next;
    }

    /* find <image> or <generic> page under <pages> */
    while (pages != NULL)
    {
        struct image *img = NULL;
        if ((!xmlStrcmp(pages->name, (const xmlChar *) "image")))
            img = parse_image_settings(data, pages->xmlChildrenNode);
        else if ((!xmlStrcmp(pages->name, (const xmlChar *) "generic")))
            img = parse_gen_settings(data, pages->xmlChildrenNode);

        /* if page found, load it to the gallery */
        if (img != NULL)
            list = g_slist_append(list, img);

        pages = pages->next;
    }
    
    xmlFreeDoc(doc);
    xmlCleanupParser();

    gallery_open_images(data, list);
    
}



/*
 *
 * Static functions 
 *
 */



/* Parse gallery settings. The node should point to first node under
 * the settings node. */
static void
parse_gal_settings(struct data *data, xmlNodePtr node)
{
    g_assert(data != NULL);
    g_assert(node != NULL);

    g_debug("in parse_gal_settings");

    /* find separate settings */
    /* FIXME: isn't this long and ugly.. */
    while (node != NULL)
    {
        if (node->type != XML_ELEMENT_NODE)
        {
            node = node->next;
            continue;
        }
        if ((!xmlStrcmp(node->name, (const xmlChar *) "name")))
        {
            g_free(data->gal->name);
            data->gal->name = (gchar *)xmlNodeGetContent(node);
        }
        if ((!xmlStrcmp(node->name, (const xmlChar *) "dir_name")))
        {
            g_free(data->gal->dir_name);
            data->gal->dir_name = (gchar *)xmlNodeGetContent(node);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "desc")))
        {
            g_free(data->gal->desc);
            data->gal->desc = (gchar *)xmlNodeGetContent(node);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "output_dir")))
        {
            g_free(data->gal->base_dir);
            data->gal->base_dir = (gchar *)xmlNodeGetContent(node);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "page_gen_prog")))
        {
            g_free(data->gal->page_gen_prog);
            data->gal->page_gen_prog = (gchar *)xmlNodeGetContent(node);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "templ_index")))
        {
            g_free(data->gal->templ_index);
            data->gal->templ_index = (gchar *)xmlNodeGetContent(node);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "templ_indeximg")))
        {
            g_free(data->gal->templ_indeximg);
            data->gal->templ_indeximg = (gchar *)xmlNodeGetContent(node);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "templ_indexgen")))
        {
            g_free(data->gal->templ_indexgen);
            data->gal->templ_indexgen = (gchar *)xmlNodeGetContent(node);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "templ_image")))
        {
            g_free(data->gal->templ_image);
            data->gal->templ_image = (gchar *)xmlNodeGetContent(node);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "templ_gen")))
        {
            g_free(data->gal->templ_gen);
            data->gal->templ_gen = (gchar *)xmlNodeGetContent(node);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "page_gen")))
        {
            gchar *str = (gchar *)xmlNodeGetContent(node);
            data->gal->page_gen = (gint) g_ascii_strtoull(str, NULL, 0);
            xmlFree((xmlChar*)str);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "thumb_w")))
        {
            gchar *str = (gchar *)xmlNodeGetContent(node);
            data->gal->thumb_w = (gint) g_ascii_strtoull(str, NULL, 0);
            xmlFree((xmlChar*)str);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "image_h")))
        {
            gchar *str = (gchar *)xmlNodeGetContent(node);
            data->gal->image_h = (gint) g_ascii_strtoull(str, NULL, 0);
            xmlFree((xmlChar*)str);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "image_h2")))
        {
            gchar *str = (gchar *)xmlNodeGetContent(node);
            data->gal->image_h2 = (gint) g_ascii_strtoull(str, NULL, 0);
            xmlFree((xmlChar*)str);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "image_h3")))
        {
            gchar *str = (gchar *)xmlNodeGetContent(node);
            data->gal->image_h3 = (gint) g_ascii_strtoull(str, NULL, 0);
            xmlFree((xmlChar*)str);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "image_h4")))
        {
            gchar *str = (gchar *)xmlNodeGetContent(node);
            data->gal->image_h4 = (gint) g_ascii_strtoull(str, NULL, 0);
            xmlFree((xmlChar*)str);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "edited")))
        {
            xmlChar *str = xmlNodeGetContent(node);
            if ((!xmlStrcmp(str, (const xmlChar *) "true")))
                data->gal->edited = 1;
            else
                data->gal->edited = 0;
            xmlFree(str);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "remove_exif")))
        {
            xmlChar *str = xmlNodeGetContent(node);
            if ((!xmlStrcmp(str, (const xmlChar *) "true")))
                data->gal->remove_exif = 1;
            else
                data->gal->remove_exif = 0;
            xmlFree(str);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "rename")))
        {
            xmlChar *str = xmlNodeGetContent(node);
            if ((!xmlStrcmp(str, (const xmlChar *) "true")))
                data->gal->rename = 1;
            else
                data->gal->rename = 0;
            xmlFree(str);
        }
        node = node->next;
    }

    /* concatenate the actual output dir */
    g_free(data->gal->output_dir);
    data->gal->output_dir = g_strdup_printf("%s/%s", data->gal->base_dir, 
                                            data->gal->dir_name);
            
}


/* Parse image settings. */
static struct image *
parse_image_settings(struct data *data, xmlNodePtr node)
{
    struct image *img;

    g_assert(data != NULL);
    g_assert(node != NULL);

    g_debug("in parse_image_settings");

    /* find <settings> element's children node */
    while (node != NULL)
    {
        if ((!xmlStrcmp(node->name, (const xmlChar *) "settings")))
        {
            node = node->xmlChildrenNode;
            break;
        }
        node = node->next;
    }

    /* FIXME: invalid xml. Better error handling? */
    g_assert(node != NULL);

    img = image_init(data);

    /* find values */
    while (node != NULL) /* uri, text, gamma, rotate, nomod */
    {
        if (node->type != XML_ELEMENT_NODE)
        {
            node = node->next;
            continue;
        }
        if ((!xmlStrcmp(node->name, (const xmlChar *) "uri")))
        {   
            g_free(img->uri);
            img->uri = (gchar *)xmlNodeGetContent(node);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "text")))
        {
            g_free(img->text);
            img->text = (gchar *)xmlNodeGetContent(node);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "gamma")))
        {
            gchar *tmpstr = (gchar *)xmlNodeGetContent(node);
            img->gamma = (gfloat)g_ascii_strtod(tmpstr, NULL);
            xmlFree((xmlChar*)tmpstr);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "rotate")))
        {
            gchar *tmpstr = (gchar *)xmlNodeGetContent(node);
            img->rotate = (gint)g_ascii_strtoull(tmpstr, NULL, 0);
            xmlFree((xmlChar*)tmpstr);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "image_h")))
        {
            gchar *tmpstr = (gchar *)xmlNodeGetContent(node);
            img->image_h = (gint)g_ascii_strtoull(tmpstr, NULL, 0);
            xmlFree((xmlChar*)tmpstr);
        }
        else if ((!xmlStrcmp(node->name, (const xmlChar *) "nomodify")))
        {
            xmlChar *tmpstr = xmlNodeGetContent(node);
            if ((!xmlStrcmp(tmpstr, (const xmlChar *) "true")))
                img->nomodify = TRUE;
            else
                img->nomodify = FALSE;
            xmlFree((xmlChar*)tmpstr);
        }
        node = node->next;
    }

    return img;
}


/* FIXME: will this be the same as the above actually..? or does this
 * create struct image too but with default values except for the
 * text? */
/* Parse generic page settings. */
static struct image *
parse_gen_settings(struct data *data, xmlNodePtr node)
{
    g_assert(data != NULL);
    g_assert(node != NULL);

    g_debug("in parse_gen_settings");

    /* find <settings> element's children node */
    while (node != NULL)
    {
        if (node->type != XML_ELEMENT_NODE)
        {
            node = node->next;
            continue;
        }
        if ((!xmlStrcmp(node->name, (const xmlChar *) "settings")))
        {
            node = node->xmlChildrenNode;
            break;
        }
        node = node->next;
    }

    /* FIXME: invalid xml. Better error handling? */
    g_assert(node != NULL);

    /* find settings */
    while (node != NULL)
    {
        if (node->type != XML_ELEMENT_NODE)
        {
            node = node->next;
            continue;
        }
        node = node->next;
    }
    
    return NULL;
}



/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
