INCLUDES = $(GTK_CFLAGS) $(IMAGEMAGICK_CFLAGS) $(WAND_CLFAGS) $(GLADE_CFLAGS)

bin_PROGRAMS = pwgallery

pwgallery_LDADD = $(GTK_LIBS) $(IMAGEMAGICK_LIBS) $(WAND_LIBS) $(GLADE_LIBS)
pwgallery_LDFLAGS = -export-dynamic
pwgallery_SOURCES = main.c main.h \
	callbacks.c callbacks.h
