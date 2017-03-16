dnl $Id$
dnl config.m4 for extension buffer

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(buffer, for buffer support,
dnl Make sure that the comment is aligned:
dnl [  --with-buffer             Include buffer support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(buffer, whether to enable buffer support,
Make sure that the comment is aligned:
[  --enable-buffer           Enable buffer support])

if test "$PHP_BUFFER" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-buffer -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/buffer.h"  # you most likely want to change this
  dnl if test -r $PHP_BUFFER/$SEARCH_FOR; then # path given as parameter
  dnl   BUFFER_DIR=$PHP_BUFFER
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for buffer files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       BUFFER_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$BUFFER_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the buffer distribution])
  dnl fi

  dnl # --with-buffer -> add include path
  dnl PHP_ADD_INCLUDE($BUFFER_DIR/include)

  dnl # --with-buffer -> check for lib and symbol presence
  dnl LIBNAME=buffer # you may want to change this
  dnl LIBSYMBOL=buffer # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $BUFFER_DIR/$PHP_LIBDIR, BUFFER_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_BUFFERLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong buffer lib version or lib not found])
  dnl ],[
  dnl   -L$BUFFER_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(BUFFER_SHARED_LIBADD)

  PHP_NEW_EXTENSION(buffer, buffer.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
