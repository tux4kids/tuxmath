/* Required headers */
#cmakedefine HAVE_INTTYPES_H 1
#cmakedefine HAVE_UNISTD_H 1
#cmakedefine HAVE_INTTYPES_H_WITH_UINTMAX 1
#cmakedefine HAVE_STDINT_H_WITH_UINTMAX 1

/* Required functions */
#cmakedefine HAVE_STPCPY 1
#cmakedefine HAVE_MEMPCPY 1
#cmakedefine HAVE_GETCWD 1

/* Required definitions */
#cmakedefine ICONV_SECOND_ARGUMENT_IS_CONST 1
#ifdef ICONV_SECOND_ARGUMENT_IS_CONST
#define ICONV_CONST const
#else
#define ICONV_CONST
#endif
#define ENABLE_NLS 1

/* Stuff that may be optional */
#cmakedefine HAVE_ALLOCA_H 1
#cmakedefine HAVE_ALLOCA 1

#define __libc_lock_t                   gl_lock_t
#define __libc_lock_define              gl_lock_define
#define __libc_lock_define_initialized  gl_lock_define_initialized
#define __libc_lock_init                gl_lock_init
#define __libc_lock_lock                gl_lock_lock
#define __libc_lock_unlock              gl_lock_unlock
#define __libc_lock_recursive_t                   gl_recursive_lock_t
#define __libc_lock_define_recursive              gl_recursive_lock_define
#define __libc_lock_define_initialized_recursive  gl_recursive_lock_define_initialized
#define __libc_lock_init_recursive                gl_recursive_lock_init
#define __libc_lock_lock_recursive                gl_recursive_lock_lock
#define __libc_lock_unlock_recursive              gl_recursive_lock_unlock
#define glthread_in_use  libintl_thread_in_use
#define glthread_lock_init     libintl_lock_init
#define glthread_lock_lock     libintl_lock_lock
#define glthread_lock_unlock   libintl_lock_unlock
#define glthread_lock_destroy  libintl_lock_destroy
#define glthread_rwlock_init     libintl_rwlock_init
#define glthread_rwlock_rdlock   libintl_rwlock_rdlock
#define glthread_rwlock_wrlock   libintl_rwlock_wrlock
#define glthread_rwlock_unlock   libintl_rwlock_unlock
#define glthread_rwlock_destroy  libintl_rwlock_destroy
#define glthread_recursive_lock_init     libintl_recursive_lock_init
#define glthread_recursive_lock_lock     libintl_recursive_lock_lock
#define glthread_recursive_lock_unlock   libintl_recursive_lock_unlock
#define glthread_recursive_lock_destroy  libintl_recursive_lock_destroy
#define glthread_once                 libintl_once
#define glthread_once_call            libintl_once_call
#define glthread_once_singlethreaded  libintl_once_singlethreaded
