
bin_PROGRAMS = pwgallery

pwgallery_LDADD = $(GTK_LIBS) $(IMAGEMAGICK_LIBS) $(WAND_LIBS) $(GLADE_LIBS) \
	$(GNOMEVFS_LIBS) $(GMODULE_LIBS) $(XML_LIBS) $(EXIF_LIBS)
pwgallery_LDFLAGS = -export-dynamic
pwgallery_CPPFLAGS = $(GTK_CFLAGS) $(IMAGEMAGICK_CFLAGS) $(WAND_CLFAGS) $(GLADE_CFLAGS) \
	$(GNOMEVFS_CFLAGS) $(GMODULE_CFLAGS) $(XML_CFLAGS) $(EXIT_CFLAGS)
pwgallery_SOURCES = main.c main.h \
	callbacks.c callbacks.h \
	image.c image.h \
	gallery.c gallery.h \
	widgets.c widgets.h \
	magick.c magick.h \
	vfs.c vfs.h \
	xml.c xml.h \
	html.c html.h \
	exif.c exif.h \
	configrc.c configrc.h



