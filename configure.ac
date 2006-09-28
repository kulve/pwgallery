AC_INIT(Makefile.am)
AM_INIT_AUTOMAKE(pwgallery, 0.1.0)
AM_CONFIG_HEADER(config.h)

dnl Set of available languages.
ALL_LINGUAS="en"

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

CFLAGS="-Wall -Werror -pedantic -Wmissing-prototypes -Wmissing-declarations -O0 -ggdb -std=gnu99"

dnl FIXME: NO HARDCODING!
AH_TEMPLATE([PACKAGE_DATA_DIR], [Application's data directory])
AC_DEFINE(PACKAGE_DATA_DIR, "/usr/share/pwgallery")

dnl Checks for libraries.
PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.2.4)
AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)

PKG_CHECK_MODULES(IMAGEMAGICK, ImageMagick >= 6.0.6)
AC_SUBST(IMAGEMAGICK_LIBS)
AC_SUBST(IMAGEMAGICK_CFLAGS)

PKG_CHECK_MODULES(WAND, Wand >= 6.0.6)
AC_SUBST(WAND_LIBS)
AC_SUBST(WAND_CFLAGS)

PKG_CHECK_MODULES(GLADE, libglade-2.0 >= 2.5.1)
AC_SUBST(GLADE_LIBS)
AC_SUBST(GLADE_CFLAGS)

PKG_CHECK_MODULES(GNOMEVFS, gnome-vfs-2.0 >= 2.14.2)
AC_SUBST(GNOMEVFS_LIBS)
AC_SUBST(GNOMEVFS_CFLAGS)

PKG_CHECK_MODULES(GMODULE, gmodule-2.0 >= 2.10.2)
AC_SUBST(GMODULE_LIBS)
AC_SUBST(GMODULE_CFLAGS)

PKG_CHECK_MODULES(XML, libxml-2.0 >= 2.6.16)
AC_SUBST(XML_LIBS)
AC_SUBST(XML_CFLAGS)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h libintl.h limits.h locale.h stdlib.h string.h unistd.h])

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for library functions.
AC_CHECK_FUNCS([strerror])
AC_OUTPUT(Makefile
        src/Makefile)

