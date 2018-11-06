#ifndef __INC_COMMON_C_H__
#define __INC_COMMON_C_H__

// For bool

#if defined(_MSC_VER) && _MSC_VER < 1900
#	ifndef __cplusplus
#		define bool char
#		define true 1
#		define false 0
#	endif
#else
#	ifndef __cplusplus
#		include <stdbool.h>
#	endif
#endif

#endif
