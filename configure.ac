# Process this file with autoconf to produce a configure script.

#######################################################################
# Initial configuration                                               #
#######################################################################

AC_PREREQ([2.57])

define([WESNOTH_VERSION],[1.0.2])

define([DEFAULT_SERVER],[server.wesnoth.org])
dnl define([DEFAULT_SERVER],[devsrv.wesnoth.org:14998])

AC_INIT([Battle for Wesnoth], WESNOTH_VERSION, [davidnwhite@verizon.net], [wesnoth])
AC_DEFINE([WESNOTH_DEFAULT_SERVER], ["]DEFAULT_SERVER["], [The default server for this version])

dnl
dnl Generate wesconfig.h from the information above
dnl
AC_CONFIG_COMMANDS([src/wesconfig.h],
		   [cat > src/wesconfig.h <<EOF
#ifndef WESCONFIG_H_INCLUDED
#define WESCONFIG_H_INCLUDED

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define VERSION "]WESNOTH_VERSION["
# define WESNOTH_DEFAULT_SERVER "]DEFAULT_SERVER["
# define PACKAGE "wesnoth"
# ifndef LOCALEDIR
#  define LOCALEDIR "translations"
# endif
#endif

#endif
EOF
])

AC_REVISION([$Revision$])

AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([src/actions.cpp])
AC_CONFIG_HEADER([config.h])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([1.9 tar-ustar foreign])
AM_GNU_GETTEXT([external])
AC_PROG_RANLIB

#######################################################################
# Configuration options                                               #
#######################################################################

AC_ARG_ENABLE([debug],
	      AS_HELP_STRING([--enable-debug], [enable debug in wesnoth]),
	      [debug=$enableval],
	      [debug=no])

if test "x$debug" = "xyes"
then
	CXXFLAGS="$CXXFLAGS -DDEBUG -O0 -ggdb3 -W -Wall -ansi " 
else
	CXXFLAGS="$CXXFLAGS  -W -Wall -ansi " 
fi

AC_ARG_ENABLE([static],
              AS_HELP_STRING([--enable-static], [enable static building of wesnoth]),
	      [static=$enableval],
	      [static=no])

AC_ARG_ENABLE([lite],
              AS_HELP_STRING([--enable-lite], [enable lite version of wesnoth (without music or large images)]),
	      [lite=$enableval],
	      [lite=no])

AC_ARG_ENABLE([tinygui],
              AS_HELP_STRING([--enable-tinygui], [enable GUI reductions for resolutions down to 320x240 (PDAs)]),
	      [tinygui=$enableval],
	      [tinygui=no])

if test "x$tinygui" = "xyes"
then
	CPPFLAGS="$CPPFLAGS -DUSE_TINY_GUI"
fi
AM_CONDITIONAL([TINYGUI], [test "x$tinygui" = "xyes"])

DATADIR=$PACKAGE
AC_ARG_WITH([datadir-name],
            AS_HELP_STRING([--with-datadir-name@<:@=DIR@:>@], [change name of data directory @<:@wesnoth@:>@]),
	      [case "${withval}" in
	       yes)
			DATADIR="wesnoth"
			;;
		no)
			;;
		*)
			DATADIR="${withval}"
			;;
		esac])
AC_SUBST([DATADIR])

#LOCALEDIR="$datadir/locale"
LOCALEDIR=translations
AC_ARG_WITH([localedir],
            AS_HELP_STRING([--with-localedir@<:@=DIR@:>@], [install locale data under dir @<:@translations@:>@]),
	      [case "${withval}" in
	       yes)
			LOCALEDIR="translations"
			;;
		no)
			;;
		*)
			LOCALEDIR="${withval}"
			;;
		esac])
AC_SUBST([LOCALEDIR])

case "`eval echo \"$LOCALEDIR\"`" in
/*) FULLLOCALEDIR="$LOCALEDIR"; HAS_RELATIVE_LOCALEDIR=0 ;;
*)  FULLLOCALEDIR='${datadir}/${DATADIR}/${LOCALEDIR}'; HAS_RELATIVE_LOCALEDIR=1 ;;
esac
AC_SUBST([FULLLOCALEDIR])
AC_SUBST([HAS_RELATIVE_LOCALEDIR])


# icondir and desktopdir may differ from datadir (suggested by Gentoo)
AC_ARG_WITH([icondir],
            AS_HELP_STRING([--with-icondir@<:@=DIR@:>@], [change icon directory under gnome]),
		  [case "${withval}" in
		   yes)
				ICONDIR="${datadir}/icons"
				;;
		   no)
				;;
		   *)
				ICONDIR="${withval}"
				;;
		   esac])
AC_SUBST([ICONDIR])

AC_ARG_WITH([desktopdir],
            AS_HELP_STRING([--with-desktopdir@<:@=DIR@:>@], [change desktop directory under gnome 2]),
		  [case "${withval}" in
		   yes)	
				DESKTOPDIR="${datadir}/applications"
				;;
		   no)
				;;
		   *)
				DESKTOPDIR="${withval}"
				;;
		   esac])
AC_SUBST([DESKTOPDIR])


AC_ARG_ENABLE([game],
              AS_HELP_STRING([--disable-game], [disable compilation of game]),
	      [game=$enableval],
	      [game=yes])

AC_ARG_ENABLE([server],
              AS_HELP_STRING([--enable-server], [enable compilation of server]),
	      [server=$enableval],
	      [server=no])

AC_ARG_WITH([fifodir],
	         AS_HELP_STRING([--with-fifodir], [directory for the wesnothd fifo socket file]),
	      [fifodir=$withval],
	      [fifodir=$localstatedir/run/wesnothd])
AC_SUBST([fifodir])

AC_ARG_WITH([server-uid],
	          AS_HELP_STRING([--with-server-uid], [user id of the user who runs wesnothd]),
	      [serveruid=$withval],
	      [serveruid=0])
AC_SUBST([serveruid])

AC_ARG_WITH([server-gid],
	          AS_HELP_STRING([--with-server-gid], [group id of the user who runs wesnothd]),
	      [servergid=$withval],
	      [servergid=0])
AC_SUBST([servergid])

AC_ARG_ENABLE([campaign_server],
              AS_HELP_STRING([--enable-campaign-server], [enable compilation of campaign server]),
	      [campaignserver=$enableval],
	      [campaignserver=no])

AC_ARG_ENABLE([editor],
              AS_HELP_STRING([--enable-editor], [enable compilation of map editor]),
	      [editor=$enableval],
	      [editor=no])

AC_ARG_ENABLE([tools],
              AS_HELP_STRING([--enable-tools], [enable compilation of tools for translators and artists]),
	      [tools=$enableval],
	      [tools=no])

AC_ARG_WITH([kde],
            AS_HELP_STRING([--with-kde], [enable installation of icon and KDE menu entry]),
	      [kde=$withval],
	      [kde=no])

AC_ARG_WITH([gnome],
            AS_HELP_STRING([--with-gnome], [enable installation of icon and GNOME menu entry]),
	      [gnome=$withval],
	      [gnome=no])

AC_ARG_ENABLE([gnome1],
              AS_HELP_STRING([--disable-gnome1], [disable installation of icon and menu entry in GNOME1]),
              [gnome1=$enableval],
              [gnome1=yes])

AC_ARG_ENABLE([gnome2],
              AS_HELP_STRING([--disable-gnome2], [disable installation of icon and menu entry in GNOME2]),
		  [gnome2=$enableval],
		  [gnome2=yes])

AC_ARG_ENABLE([dummy-locales],
              AS_HELP_STRING([--enable-dummy-locales], [enable installation of Wesnoth own private locales]),
              [dummylocales=$enableval],
              [dummylocales=no])

AC_ARG_ENABLE([internal-data],
		AS_HELP_STRING([--enable-internal-data],
			[put data inside application: Mac OS X only]),
		[internaldata=$enableval],
		[internaldata=no])

AM_CONDITIONAL([STATIC], [test x$static = xyes])
AM_CONDITIONAL([GAME], [test x$game = xyes])
AM_CONDITIONAL([SERVER], [test x$server = xyes])
AM_CONDITIONAL([CAMPAIGNSERVER], [test x$campaignserver = xyes])
AM_CONDITIONAL([EDITOR], [test x$editor = xyes])
AM_CONDITIONAL([TOOLS], [test x$tools = xyes])
AM_CONDITIONAL([GCC], [test x$GXX = xyes])
AM_CONDITIONAL([INCLUDEDINTL], [test x$nls_cv_use_gnu_gettext = xyes])
AM_CONDITIONAL([INSTALLDATA], [test x$game = xyes || x$editor = xyes])
AM_CONDITIONAL([DUMMYLOCALES], [test x$dummylocales = xyes])

if test x$dummylocales = xyes; then
  AC_DEFINE([USE_DUMMYLOCALES],,[Define if the game should not use system locales])
fi

if test x$internaldata = xyes; then
  AC_DEFINE([USE_INTERNAL_DATA],[],[Define if translations should be placed inside app, for Mac OS X])
fi

#######################################################################
# Checks for programs.                                                #
#######################################################################

AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O

have_libx11='no'
if test "$with_x" != 'no'; then

dnl Locate X include files and libraries
   AC_PATH_XTRA
   NEW_LIBS="$X_LIBS -lX11"

   AC_CHECK_LIB(X11, XOpenDisplay, have_libx11='yes',have_libx11='no',$X_LIBS)
   if test "$have_libx11" != 'no'; then
     AC_DEFINE([HAVE_LIBX11],,[Define if you have X11 libraries])
     X_LIBS="$NEW_LIBS"
     CPPFLAGS="$X_CFLAGS $CPPFLAGS"
     LIBS="$X_LIBS $LIBS"
   fi
fi

AM_CONDITIONAL([X11], [test "$have_libx11" = 'yes'])

# SDL_CONFIG

AC_PATH_PROGS([SDL_CONFIG], [sdl-config sdl11-config], [none])

if test "x$SDL_CONFIG" = "xnone"; then

	AC_MSG_ERROR([*** SDL not found! Get SDL from www.libsdl.org.
If you already installed it, check it's in the path. If problem remains,
please send a mail to the address that appears in ./configure --version
indicating your platform, the version of configure script and the problem.])

fi

# libpng-config

AC_PATH_PROGS([PNG_CONFIG], [libpng-config libpng12-config], [none])

if test "x$PNG_CONFIG" = "xnone"; then
    AC_PATH_PROG([PNG_CONFIG], [pkg-config], [none])
    if test "x$PNG_CONFIG" = "xnone"; then
	pngfound=no
	AC_MSG_WARN([*** LIBPNG not found.])
    else
	pngfound=yes
	PNG_CFLAGS=`$PNG_CONFIG --cflags libpng12`
	PNG_LIBS=`$PNG_CONFIG --libs libpng12`
    fi
else
    pngfound=yes
    PNG_CFLAGS=`$PNG_CONFIG --cflags`
    PNG_LIBS=`$PNG_CONFIG --libs`
fi

AC_SUBST([PNG_CFLAGS])
AC_SUBST([PNG_LIBS])
AM_CONDITIONAL([LIBPNG], [test x$pngfound = xyes])

# Check for SDL version. Taken from sdl.m4

AC_ARG_ENABLE([sdltest],
              AS_HELP_STRING([--disable-sdltest], [do not try to compile and run a test SDL program]),
	      ,
              [enable_sdltest=yes])

min_sdl_version=1.2.7
AC_MSG_CHECKING(for SDL - version >= $min_sdl_version)

SDL_CFLAGS=`$SDL_CONFIG --cflags`
SDL_LIBS=`$SDL_CONFIG --libs`
sdl_major_version=`$SDL_CONFIG --version | \
    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
sdl_minor_version=`$SDL_CONFIG --version | \
    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
sdl_micro_version=`$SDL_CONFIG --version | \
    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

if test "x$enable_sdltest" = "xyes" ; then
    ac_save_CFLAGS="$CFLAGS"
    ac_save_LIBS="$LIBS"
    CFLAGS="$CFLAGS $SDL_CFLAGS"
    LIBS="$LIBS $SDL_LIBS"

    # Now check if the installed SDL is sufficiently new. (Also sanity
    # checks the results of sdl-config to some extent

    rm -f conf.sdltest
    AC_RUN_IFELSE([AC_LANG_SOURCE([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"

char*
my_strdup (char *str)
{
  char *new_str;

  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;

  return new_str;
}

int main (int argc, char **argv)
{
  int major, minor, micro;
  char *tmp_version;

  /* This hangs on some systems (?)
  system ("touch conf.sdltest");
  */
  { FILE *fp = fopen("conf.sdltest", "a"); if ( fp ) fclose(fp); }

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_sdl_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_sdl_version");
     exit(1);
   }

   if (($sdl_major_version > major) ||
      (($sdl_major_version == major) && ($sdl_minor_version > minor)) ||
      (($sdl_major_version == major) && ($sdl_minor_version == minor) && ($sdl_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'sdl-config --version' returned %d.%d.%d, but the minimum version\n", $sdl_major_version, $sdl_minor_version, $sdl_micro_version);
      printf("*** of SDL required is %d.%d.%d. If sdl-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If sdl-config was wrong, set the environment variable SDL_CONFIG\n");
      printf("*** to point to the correct copy of sdl-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}
    ])],
    [AC_MSG_RESULT(yes)],
    [AC_MSG_RESULT(no)]
    [AC_MSG_ERROR([*** Please upgrade your SDL version])],
    [AC_MSG_RESULT([not tested in cross-compiling])])
    rm -f conf.sdltest

    CFLAGS="$ac_save_CFLAGS"
    LIBS="$ac_save_LIBS"
fi


# GNOME_CONFIG

AC_PATH_PROGS([GNOME_CONFIG], [gnome-config], [none])

if test "x$GNOME_CONFIG" = "xnone"; then
	gnome1found=no
else
	GNOME1_DESKTOP=`$GNOME_CONFIG --datadir`/gnome/apps/Games
	gnome1found=yes
fi

AC_SUBST([GNOME1_DESKTOP])


# KDE_CONFIG

AC_PATH_PROGS([KDE_CONFIG], [kde-config], [none])

if test "x$KDE_CONFIG" = "xnone"; then
	kdefound=no
else
	KDE_DESKTOP=`$KDE_CONFIG --install apps`/Games/TacticStrategy
	KDE_ICON=`$KDE_CONFIG --install icon`
	kdefound=yes
fi

AC_SUBST([KDE_DESKTOP])
AC_SUBST([KDE_ICON])

AM_CONDITIONAL([KDE], [test x$kde = xyes && test x$kdefound = xyes])
AM_CONDITIONAL([GNOME1], [test x$gnome = xyes && test x$gnome1found = xyes && test x$gnome1 = xyes])
AM_CONDITIONAL([GNOME2], [test x$gnome = xyes && test x$gnome2 = xyes])

#######################################################################
# Checks for libraries.                                               #
#######################################################################

# Use a modified version of ac_link so that libtool gets called
# this seems pretty broken on most systems
AC_PATH_PROG([LTOOL], [libtool], [])
if test "$static" = "yes" -a -n "$LTOOL"
then
	LDPREFIX="$LTOOL --mode=link --tag=CXX"
else
	LDPREFIX=""
fi
AC_SUBST([LDPREFIX])

#

if test -n "$LDPREFIX" -a -r `$SDL_CONFIG --prefix`/lib/libSDL.la
then SDL_LIBS=`$SDL_CONFIG --prefix`/lib/libSDL.la
else SDL_LIBS=`$SDL_CONFIG --libs`
fi
case $host_os in
	darwin*)
		SDL_LIBS="-framework Carbon $SDL_LIBS"
esac
case $host_os in
	mingw32*)
		SDL_LIBS="-lunicows $SDL_LIBS"
esac
OLD_LIBS=$LIBS
LIBS="$LIBS $SDL_LIBS"

# There's no need for this, $SDL_CONFIG comes with libsdl and
# it doesn't find it in FreeBSD
# AC_CHECK_LIB([SDL], [SDL_Init])
# unfortunately, sdl_config is not shipped with the Mac OS X packages...
# so recommend using fink sdl packages as a workaround

ac_link="$LDPREFIX $ac_link"
AC_CHECK_LIB([SDL_image],
	     [IMG_Load],
	     [if test -n "$LDPREFIX" -a -r `$SDL_CONFIG --prefix`/lib/libSDL_image.la
then SDL_IMAGE_LIBS=`$SDL_CONFIG --prefix`/lib/libSDL_image.la
else SDL_IMAGE_LIBS=-lSDL_image
fi],
	     [AC_MSG_ERROR([*** SDL_image lib not found! Get SDL_image from
http://www.libsdl.org/projects/SDL_image/index.html])])

AC_CHECK_LIB([SDL_mixer],
	     [Mix_OpenAudio],
	     [if test -n "$LDPREFIX" -a -r `$SDL_CONFIG --prefix`/lib/libSDL_mixer.la
then SDL_MIXER_LIBS=`$SDL_CONFIG --prefix`/lib/libSDL_mixer.la
else SDL_MIXER_LIBS=-lSDL_mixer
fi],
	     [AC_MSG_ERROR([*** SDL_mixer lib not found! Get SDL_mixer from
http://www.libsdl.org/projects/SDL_mixer/index.html])])

AC_CHECK_LIB([SDL_net],
	     [SDLNet_Init],
	     [if test -n "$LDPREFIX" -a -r `$SDL_CONFIG --prefix`/lib/libSDL_net.la
then SDL_NET_LIBS=`$SDL_CONFIG --prefix`/lib/libSDL_net.la
else SDL_NET_LIBS=-lSDL_net
fi],
	     [AC_MSG_ERROR([*** SDL_net lib not found! Get SDL_net from
http://www.libsdl.org/projects/SDL_net/index.html])])

LIBS=$OLD_LIBS

AC_SUBST([SDL_LIBS])
AC_SUBST([SDL_IMAGE_LIBS])
AC_SUBST([SDL_MIXER_LIBS])
AC_SUBST([SDL_NET_LIBS])

# Checking for the freetype library.

AC_PATH_PROG([PKG_CONFIG], [pkg-config], [none])
if test "x$PKG_CONFIG" = "xnone"; then
    AC_MSG_ERROR([*** Please install pkg-config or pkgconf])
fi

AC_MSG_CHECKING(for freetype2)
$PKG_CONFIG --exists freetype2
if test $? != 0 ; then
    AC_MSG_RESULT(no)
    AC_MSG_ERROR([*** Unable to find FreeType2 library (https://www.freetype.org/)])
else
    AC_MSG_RESULT(yes)
    CPPFLAGS="$CPPFLAGS `$PKG_CONFIG --cflags freetype2`"
    FREETYPE_LIBS="`$PKG_CONFIG --libs freetype2`"
fi

AC_SUBST([FREETYPE_LIBS])

AC_ARG_WITH([zipios],
	AS_HELP_STRING([--without-zipios], [Do not use zipios++ even if installed]),
	[checkzipios="$withval"],
	[checkzipios="yes"])

AC_LANG(C++)
LIBZIPIOS=
if test $checkzipios = yes; then
	AC_MSG_CHECKING([for libzipios++])
	OLD_LIBS="$LIBS"
	LIBS="-lzipios -lz"
	AC_LINK_IFELSE([AC_LANG_SOURCE([
	#include <zipios++/dircoll.h>
	int main(int argc, char **argv)
	{
		zipios::DirectoryCollection z("src");
	}
		])],
		[AC_MSG_RESULT(yes)
		LIBZIPIOS="-lzipios -lz"
		AC_DEFINE([USE_ZIPIOS],[1],[Make use of the zipios++ library to read data in zip files])],
		[AC_MSG_RESULT(no)
		AC_MSG_WARN([*** libzipios++ not found - support for ZIP files is disabled])])
	LIBS="$OLD_LIBS"
else
	AC_MSG_NOTICE([skipping check for libzipios++])
fi
AC_SUBST([LIBZIPIOS])

#######################################################################
# Checks for header files.                                            #
#######################################################################

AC_HEADER_DIRENT
AC_HEADER_STDC

OLD_CPPFLAGS=$CPPFLAGS
OLD_CXXFLAGS=$CXXFLAGS

SDL_CFLAGS=`$SDL_CONFIG --cflags`
SDL_CFLAGS="$SDL_CFLAGS"
CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"


AC_CHECK_HEADER([SDL.h],
		[],
		[AC_MSG_ERROR([*** SDL include files not found!
You should install SDL development package.])])

AC_CHECK_HEADER([SDL_image.h],
		[],
		[AC_MSG_ERROR([*** SDL_image include files not found!
You should install development package.])])

AC_CHECK_HEADER([SDL_mixer.h],
		[],
		[AC_MSG_ERROR([*** SDL_mixer include files not found!
You should install development package.])])

AC_CHECK_HEADER([SDL_net.h],
		[],
		[AC_MSG_ERROR([*** SDL_net include files not found!
You should install development package.])])

CPPFLAGS=$OLD_CPPFLAGS
CXXFLAGS=$OLD_CXXFLAGS

AC_SUBST([SDL_CFLAGS])

AC_CHECK_HEADERS([stdlib.h unistd.h poll.h sys/poll.h sys/select.h])


#######################################################################
# Checks for typedefs, structures, and compiler characteristics.      #
#######################################################################

AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([floor socket strtoul])


#######################################################################
# Check for PNG support in SDL_image                                  #
#######################################################################

AC_LANG([C])
AC_MSG_CHECKING([for PNG support in SDL_image])

OLD_CPPFLAGS=$CPPFLAGS
OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS

CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS $SDL_IMAGE_LIBS"

ac_link="$LDPREFIX $ac_link"
AC_RUN_IFELSE([AC_LANG_SOURCE([
#include <SDL_image.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	SDL_RWops *src;

	src = SDL_RWFromFile("images/mage.png", "rb");
	if (src == NULL)
		exit(0);
	exit(!IMG_isPNG(src));
}
	])],
	[AC_MSG_RESULT(yes)],
	[AC_MSG_RESULT(no)] 
	[AC_MSG_ERROR([*** SDL_image has no PNG support! You need SDL_image with PNG support])],
	[AC_MSG_RESULT([not tested in cross-compiling])])

CPPFLAGS=$OLD_CPPFLAGS
CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS


#######################################################################
# Check for OGG support in SDL_mixer                                  #
#######################################################################

if test "x$lite" = "xno"; then
    if test -e "music/main_menu.ogg" ; then
        AC_LANG([C])
	AC_MSG_CHECKING([for OGG support in SDL_mixer])

        OLD_CPPFLAGS=$CPPFLAGS
	OLD_CFLAGS=$CFLAGS
        OLD_LIBS=$LIBS


	CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
        CFLAGS="$CFLAGS $SDL_CFLAGS"
	LIBS="$LIBS $SDL_LIBS $SDL_MIXER_LIBS"

	ac_link="$LDPREFIX $ac_link"
        AC_RUN_IFELSE([AC_LANG_SOURCE([
	#include <SDL_mixer.h>
        #include <stdlib.h>

	int main(int argc, char **argv)
        {
	    Mix_Music* music = Mix_LoadMUS("music/main_menu.ogg");
	    if (music == NULL)
		exit(1);
	    exit(0);
        }
	])],
	[AC_MSG_RESULT(yes)],
	[AC_MSG_RESULT(no)]
	[AC_MSG_ERROR([*** SDL_mixer has no OGG support! You need SDL_mixer with OGG support])],
	[AC_MSG_RESULT([not tested in cross-compiling])])


        CPPFLAGS=$OLD_CPPFLAGS
	CFLAGS=$OLD_CFLAGS
        LIBS=$OLD_LIBS
    fi
fi



#######################################################################
# Tune gettext stuff for our needs                                    #
#######################################################################

case $srcdir in
/*) topdir=$srcdir ;;
*)  topdir=`pwd`/$srcdir ;;
esac
for domain in `grep ^SUBDIRS $srcdir/po/Makefile.am | cut -d= -f2`
do
    # Symlinks to the single copy of Makefile.in.in
    echo "creating po/$domain/Makefile.in.in"
    mkdir -p po/$domain
    rm -f po/$domain/Makefile.in.in
    ln -s "$topdir/po/Makefile.in.in" "po/$domain/Makefile.in.in"
done

AC_CONFIG_COMMANDS([translations],
[rm -rf translations
case $srcdir in
/*) topdir=$srcdir ;;
*)  topdir=`pwd`/$srcdir ;;
esac
for domain in `grep ^SUBDIRS $srcdir/po/Makefile.am | cut -d= -f2`
do
    # Symlinks that allow message catalogs to be used from build tree
    if test -w $srcdir; then
	for lang in `cat $srcdir/po/$domain/LINGUAS`
	do
	    mkdir -p $srcdir/translations/$lang/LC_MESSAGES
	    rm -f $srcdir/translations/$lang/LC_MESSAGES/$domain.mo
	    ln -s $topdir/po/$domain/$lang.gmo $srcdir/translations/$lang/LC_MESSAGES/$domain.mo
	done
    fi
done])

#######################################################################
# Data file substitution.                                             #
#######################################################################

AM_CONDITIONAL([LITE], [test "x$lite" = "xyes"])
if test "x$lite" = "xyes"; then
    PACKAGE=$PACKAGE-lite
fi
AC_SUBST([DATA_FILES])
AC_SUBST([FONT_FILES])
AC_SUBST([IMAGE_FILES])
AC_SUBST([MUSIC_FILES])
AC_SUBST([SOUND_FILES])


AC_CONFIG_FILES([Makefile
		 po/Makefile
		 po/wesnoth/Makefile.in
		 po/wesnoth-editor/Makefile.in
		 po/wesnoth-lib/Makefile.in
		 po/wesnoth-httt/Makefile.in
		 po/wesnoth-tdh/Makefile.in
		 po/wesnoth-sotbe/Makefile.in
		 po/wesnoth-ei/Makefile.in
		 po/wesnoth-trow/Makefile.in
		 m4/Makefile
		 icons/Makefile
		 src/Makefile
		 doc/Makefile
		 doc/man/Makefile])

AC_OUTPUT
