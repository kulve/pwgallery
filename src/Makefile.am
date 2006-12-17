INCLUDES = $(GTK_CFLAGS) $(IMAGEMAGICK_CFLAGS) $(WAND_CLFAGS) $(GLADE_CFLAGS) \
	$(GNOMEVFS_CFLAGS) $(GMODULE_CFLAGS) $(XML_CFLAGS)

bin_PROGRAMS = pwgallery

pwgallery_LDADD = $(GTK_LIBS) $(IMAGEMAGICK_LIBS) $(WAND_LIBS) $(GLADE_LIBS) \
	$(GNOMEVFS_LIBS) $(GMODULE_LIBS) $(XML_LIBS)
pwgallery_LDFLAGS = -export-dynamic
pwgallery_SOURCES = main.c main.h \
	callbacks.c callbacks.h \
	debug.h \
	image.c image.h \
	gallery.c gallery.h \
	widgets.c widgets.h \
	magick.c magick.h \
	vfs.c vfs.h \
	xml.c xml.h \
	configrc.c configrc.h



