#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#AC_PREREQ(2.59)
AC_INIT(tgcd, 1.0.2, faraz@fzv.ca)
AC_CONFIG_SRCDIR([includes.h])
#AC_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE(tgcd,1.0.2)
AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

AC_PROG_INSTALL
# Checks for libraries.
# FIXME: Replace `main' with a function in `-lwrap':
AC_CHECK_LIB([wrap], [main])
AC_CHECK_LIB([nsl], [main])

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h netdb.h arpa/inet.h netinet/in.h stdlib.h string.h sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h syslog.h unistd.h bits/sockaddr.h asm/types.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_FORK
# The following line is to fix another stupid bug of autoconf
# mis-handling cross-compiling
export ac_cv_func_malloc_0_nonnull=yes
AC_FUNC_MALLOC
AC_PROG_GCC_TRADITIONAL
AC_FUNC_SELECT_ARGTYPES

# AC_FUNC_SETPGRP unfortunately is an executed test, with no valid reason to be so
#AC_FUNC_SETPGRP
dnl AC_FUNC_SETPGRP does not work if cross compiling
dnl Instead, assume we will have a prototype for setpgrp if cross compiling.
if test "$cross_compiling" = no; then
  AC_FUNC_SETPGRP
else
  AC_CACHE_CHECK([whether setpgrp takes no argument], ac_cv_func_setpgrp_void,
    [AC_TRY_COMPILE([
#include <unistd.h>
], [
  if (setpgrp(1,1) == -1)
    exit (0);
  else
    exit (1);
], ac_cv_func_setpgrp_void=no, ac_cv_func_setpgrp_void=yes)])
if test $ac_cv_func_setpgrp_void = yes; then
  AC_DEFINE(SETPGRP_VOID, 1)
fi
fi 


AC_TYPE_SIGNAL
AC_FUNC_STRFTIME
AC_FUNC_WAIT3
AC_CHECK_FUNCS([gethostbyname inet_ntoa gettimeofday memset select socket strcasecmp strdup strerror])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
