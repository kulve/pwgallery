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
#include "exif.h"
#include "image.h"

#include <libexif/exif-data.h>

struct exif *
exif_new(void)
{
  	struct exif *exif;

	exif = g_new0(struct exif, 1);
	
	exif->orientation = 0;
    exif->timestamp = NULL;

	return exif;
}



void
exif_free(struct exif *exif)
{
    if (exif == NULL)
        return;
    
    if (exif->timestamp == NULL) {
        g_free(exif->timestamp);
    }

    g_free(exif);
}



gboolean
exif_data_get(struct data *data, struct image *img)
{
    ExifData        *edata;
    ExifEntry       *eentry;
    ExifContent     *eifd;
    ExifByteOrder   eorder;
    ExifShort       eorientation;
    gchar           *filename;
    GError          *error = NULL;

    g_assert(data != NULL);
    g_assert(img != NULL);
    
    g_debug("in exif_data_get");

    filename = g_filename_from_uri(img->uri, NULL, &error);
    if (filename == NULL)
    {
        g_warning("Failed to get filename from uri: %s", error->message);
        g_error_free(error);
        return FALSE;
    }

    edata = exif_data_new_from_file(filename);
    g_free(filename);

    eifd = edata->ifd[EXIF_IFD_0];
    eentry = exif_content_get_entry(eifd, EXIF_TAG_ORIENTATION);
    eorder = exif_data_get_byte_order(edata);

    if (eentry && eentry->format == EXIF_FORMAT_SHORT &&
        eentry->components == 1)
    {
    
        eorientation = exif_get_short(eentry->data, eorder);
        g_debug("EOrientation: %d", eorientation);

        /* http://www.exif.org/Exif2-2.PDF
         * page 24 
         */
        switch(eorientation)
        {
        case 1:
            img->exif->orientation = 0;
            break;
        case 3:
            img->exif->orientation = 180;
            break;
        case 6:
            img->exif->orientation = 90;
            break;
        case 8:
            img->exif->orientation = 270;
            break;
        default:
            img->exif->orientation = 0;
            break;
        }
    }

    eifd = edata->ifd[EXIF_IFD_EXIF];
    eentry = exif_content_get_entry(eifd, EXIF_TAG_DATE_TIME_ORIGINAL);
    if (eentry && eentry->format == EXIF_FORMAT_ASCII && 
       (eentry->data[0] >= '0' && eentry->data[0] <= '9'))
    {
        
        g_debug("EXIF_TAG_DATE_TIME: %s", eentry->data);
        
        /* DATETIME
         * http://www.exif.org/Exif2-2.PDF
         * page 28
         */
        img->exif->timestamp = g_strdup((gchar *)eentry->data);
    }

    g_debug("Orientation: %d", img->exif->orientation);
    exif_data_free(edata);

    return TRUE;
}


/* Emacs indentatation information
   Local Variables:
   indent-tabs-mode:nil
   tab-width:4
   c-set-offset:4
   c-basic-offset:4
   End: 
*/
