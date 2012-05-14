# -*-m4-*- Process this file with autoconf to produce a configure script.


######################################################################
#
# General system tests required for building platform independent
# stuff.
#
#
AC_INIT([Bubbling Load Monitor], 3.0, johan.walles@gmail.com, bubblemon)
AC_CONFIG_SRCDIR(src/bubblemon.h)
AC_CONFIG_MACRO_DIR([m4])

# Install in /usr by default since many GNOME installations aren't
# aware of the existence of /usr/local.
AC_PREFIX_DEFAULT(/usr)

AM_INIT_AUTOMAKE([1.5])
AC_PREREQ(2.61)

AM_CONFIG_HEADER(config.h)

AM_PROG_LIBTOOL
AC_PROG_INTLTOOL([0.35])

# Compiler setup
AC_PROG_CC
if test "x$GCC" = "xyes"; then
  # -Wno-missing-field-initializers is because of
  # http://bugs.debian.org/509663
  #
  # -Wno-unused-parameter is because in gnome2-ui.c & gnome3-ui.c our function
  # prototypes are decided by the GNOME API, and we have to accept
  # some parameters we never use.
  CFLAGS="-g -O2 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers"
  AC_SUBST(CFLAGS)
fi
AM_PROG_CC_STDC
AC_ISC_POSIX
AC_HEADER_STDC

# Find some more or less non-standard data types
AC_CHECK_TYPE(u_int32_t, unsigned int)
AC_CHECK_TYPE(u_int16_t, unsigned short int)
AC_CHECK_TYPE(u_int8_t, unsigned char)

# Find out whether we are on a big-endian system
AC_C_BIGENDIAN

# For i18n and l10n
ALL_LINGUAS="sv fr pl de ko fi da pt ro es no hu it is ru ca pt_BR nl"

# Check if the user wants all languages to be compiled
AC_ARG_WITH(
  all-linguas,
  [  --with-all-linguas      Compile support for all avaliable languages],
  all_linguas=yes)

if test "x$all_linguas" = "xyes"; then
  LINGUAS=$ALL_LINGUAS
fi

AC_MSG_CHECKING(what languages to include)
AC_MSG_RESULT($LINGUAS)

GETTEXT_PACKAGE=bubblemon
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext wants to know...])
AC_SUBST(GETTEXT_PACKAGE)

# Where do we put locale information?
if test "x$prefix" = "xNONE"; then
  LOCALEDIR=$ac_default_prefix/share/locale
else
  LOCALEDIR=$prefix/share/locale
fi
AC_DEFINE_UNQUOTED(LOCALEDIR, "$LOCALEDIR", [Where language files get installed])

# We need the exp2() function for generating the tool tip
AC_SEARCH_LIBS([exp2], [m], , AC_MSG_ERROR([exp2() function not found]))


######################################################################
#
# Bubblemon specific tests
#
#

# Check if the user wants to enable profiling
AC_ARG_ENABLE(
  profiling,
  [  --enable-profiling      Pass profiling options to the compiler],
  profiling=yes)

if test "x$profiling" = "xyes"; then
  AC_MSG_CHECKING(what profiling flags to pass to the compiler)

  profilingFLAGS="-pg"

  CFLAGS="$CFLAGS $profilingFLAGS"

  AC_MSG_RESULT($profilingFLAGS)

  AC_DEFINE([ENABLE_PROFILING], [], [Build a bubblemon that bubbles a lot for a minute and then exits])

  AC_MSG_WARN()
  AC_MSG_WARN(You have enabled profiling by specifying the --enable-profiling switch.)
  AC_MSG_WARN(Please don't install the applet the usual way; read the file PROFILING)
  AC_MSG_WARN(for instructions.)
  AC_MSG_WARN()
fi


#######################################
# Desktop environment to build for

# first platform given becomes the default
m4_define([bubblemon_supported_platforms], [[gnome3], [gnome2], [xfce4]])
BUBBLEMON_PLATFORMS_DEFAULT=m4_argn(1, bubblemon_supported_platforms)
BUBBLEMON_PLATFORMS_SUPPORTED=m4_join("|", bubblemon_supported_platforms);
AC_SUBST(BUBBLEMON_PLATFORMS_DEFAULT)
AC_SUBST(BUBBLEMON_PLATFORMS_SUPPORTED)
AC_ARG_WITH(
  [desktop-environment],
  [AS_HELP_STRING([--with-desktop-environment],
    [desktop environment to build for @<:@$BUBBLEMON_PLATFORMS_SUPPORTED; default is $BUBBLEMON_PLATFORMS_DEFAULT@:>@])],
  [AS_CASE([$with_desktop_environment],
    [$BUBBLEMON_PLATFORMS_SUPPORTED], [],
    [*], [AC_MSG_WARN(bubblemon does not support the \"$with_desktop_environment\" desktop environment)
          AC_MSG_WARN(defaulting to \"$BUBBLEMON_PLATFORMS_DEFAULT\")
          with_desktop_environment=$BUBBLEMON_PLATFORMS_DEFAULT])],
  [with_desktop_environment=$BUBBLEMON_PLATFORMS_DEFAULT])
BUBBLEMON_PLATFORM=$with_desktop_environment

AC_SUBST(BUBBLEMON_PLATFORM)


##############################
# Platform specific tests

AS_CASE([$BUBBLEMON_PLATFORM],
  [gnome3], [PKG_CHECK_MODULES(PLATFORM,
              [libpanelapplet-4.0 >= 3.0.0
               libgtop-2.0 >= 2
               gtk+-3.0 >= 3],
	      ,
	      AC_MSG_ERROR([GNOME3 panel applet libraries not found.]))],
  [gnome2], [PKG_CHECK_MODULES(PLATFORM,
              [libpanelapplet-2.0 >= 2.0.0
               libgtop-2.0 >= 2],
              ,
              AC_MSG_ERROR([GNOME2 panel applet library not found.]))],
  [xfce4], [PKG_CHECK_MODULES(PLATFORM,
             [libxfcegui4-1.0 >= 4.3.20
              libxfce4panel-1.0 >= 4.3.20
              libgtop-2.0 >= 2],
             ,
             AC_MSG_ERROR([XFCE4 panel plugin libraries not found.]))])


# all platforms need these (the prefix MUST match 1st arg to PKG_CHECK_MODULES)
AC_SUBST(PLATFORM_CFLAGS)
AC_SUBST(PLATFORM_LIBS)

# allow setting extra sources based on platform in src/Makefile.am
AM_CONDITIONAL([GNOME3_BUILD], [test x$BUBBLEMON_PLATFORM = xgnome3])
AM_CONDITIONAL([GNOME2_BUILD], [test x$BUBBLEMON_PLATFORM = xgnome2])
AM_CONDITIONAL([XFCE4_BUILD], [test x$BUBBLEMON_PLATFORM = xxfce4])

# gnome3 & gnome2 both use these vars
DBUS_SERVICE_DIR=$($PKG_CONFIG --variable=session_bus_services_dir dbus-1)
AC_SUBST(DBUS_SERVICE_DIR)

PANEL_APPLET_DIR=$($PKG_CONFIG --variable=libpanel_applet_dir libpanelapplet-4.0)
AC_SUBST(PANEL_APPLET_DIR)


######################################################################
#
# Output our test results
#
#
if test "x$GCC" = "xyes"; then
  # this causes some autoconf tests to fail if done earlier
  CFLAGS="$CFLAGS -Werror"
fi

AC_OUTPUT([
Makefile
m4/Makefile
po/Makefile.in
po/Makefile
src/Makefile
platform/Makefile
platform/gnome2/Makefile
platform/gnome3/Makefile
packaging/gnome-applet-bubblemon.spec
pixmaps/Makefile
man/Makefile
man/bubblemon-program.1
man/hu/Makefile
man/hu/bubblemon-program.1
man/sv/Makefile
man/sv/bubblemon-program.1
])
