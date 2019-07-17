/*
 * hw_utils.h
 */

#ifndef HW_UTILS_H_
#define HW_UTILS_H_

#include "ini.h"
#if __WIN32 || __WIN64
#	include <windows.h>
#elif linux || __linux__ || __APPLE__
#	include <time.h>
#else
#	error "Unknown build platform."
#endif
//------------------------------------------------------------------------------
typedef const char * sz_ptr;
//------------------------------------------------------------------------------
bool CheckDependencies(void);
#if __WIN32 || __WIN64
double time_difference_s(LARGE_INTEGER start, LARGE_INTEGER end);
double time_difference_ms(LARGE_INTEGER start, LARGE_INTEGER end);
#elif linux || __linux__ || __APPLE__
timespec time_difference(timespec start, timespec end);
double time_difference_s(timespec start, timespec end);
double time_difference_ms(timespec start, timespec end);
#endif
//------------------------------------------------------------------------------

#endif /* HW_UTILS_H_ */
