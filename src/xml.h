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

#ifndef PWGALLERY_XML_H
#define PWGALLERY_XML_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "main.h"

#include <glib.h>
#include <libxml/parser.h>

gchar *xml_gal_write(struct data *data, gsize *len);
void xml_gal_parse(struct data *data, gchar *content, gsize len);
void xml_gal_parse_settings(struct data *data, xmlNodePtr node);

#endif

/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
