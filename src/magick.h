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

#ifndef PWGALLERY_MAGICK_H
#define PWGALLERY_MAGICK_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


/*
 * Make a thumbnail for the given image and save it to a file.
 */
gboolean magick_make_thumbnail(struct data *data, 
                               struct image *image,
                               const gchar *uri);

/*
 * Make a webimage for the given image and save it to a file.
 */
gboolean magick_make_webimage(struct data *data, 
                              struct image *image,
                              const gchar *uri,
                              gint image_h);

/*
 * Show webimage as a preview
 */
gboolean magick_show_preview(struct data *data, 
                             struct image *image,
                             gint image_h);


#endif

/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
