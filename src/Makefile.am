INCLUDES = $(GTK_CFLAGS) $(IMAGEMAGICK_CFLAGS) $(WAND_CLFAGS) $(GLADE_CLFAGS)

bin_PROGRAMS = pwgallery

pwgallery_LDADD = $(GTK_LIBS) $(IMAGEMAGICK_LIBS) $(WAND_LIBS) $(GLADE_LIBS)

pwgallery_SOURCES = main.c \
	callbacks.c glade/callbacks.h \
	glade/interface.c glade/interface.h \
	glade/support.c glade/support.h
