/* Required headers */
#cmakedefine LINEWRAP 1
#cmakedefine HAVE_STDINT_H 1

/* Required definitions */
#cmakedefine ICONV_SECOND_ARGUMENT_IS_CONST 1
#ifdef ICONV_SECOND_ARGUMENT_IS_CONST
#define ICONV_CONST const
#else
#define ICONV_CONST
#endif
