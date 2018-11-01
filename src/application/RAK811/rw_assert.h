#ifndef _RW_ASSERT_H_
#define _RW_ASSERT_H_


#ifdef SYS_ASSERT
#define ASSERT(_cond_) if(!(_cond_)) \
		_assert(__FILE__, __LINE__, #_cond_)
#else /* ! SYS_ASSERT */
#define ASSERT(_cond_)
#endif /* SYS_ASSERT */

void _assert(const char *filename, int lineno, 
	    const char* fail_cond);

#endif // _RW_ASSERT_H
