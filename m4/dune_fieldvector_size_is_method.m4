dnl This macro introduces a configure flag --enable-fieldvector-size-is method
dnl that is used to orchestrate the transition from FieldVector::size as an enum
dnl and FieldVector::size() as a method (see FS 819 for details).
dnl If this flag is not set (the default), then FieldVector::size is an enum,
dnl but a compiler warning appears.  If it is set then the warning disappears,
dnl but FieldVector::size() becomes a method.
dnl
dnl This flag and the corresponding preprocessor directives should be removed
dnl not earlier than after dune-2.2 has been released.

AC_DEFUN([DUNE_FIELDVECTOR_SIZE_IS_METHOD],[
  AC_ARG_ENABLE(fieldvector-size-is-method,
    AS_HELP_STRING([--enable-fieldvector-size-is-method],[If this is set, the member 'size' of FieldVector is a method rather than an enum]))

  AS_IF([test "x$enable_fieldvector_size_is_method" = "xyes"],
    AC_DEFINE(DUNE_COMMON_FIELDVECTOR_SIZE_IS_METHOD, 1, [If this is set, the member 'size' of FieldVector is a method rather than an enum]))
])
