/*
 * hw_utils.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <uFCoder.h>

#include "hw_utils.h"
#include "ini.h"

/* includes:
 * stdio.h & stdlib.h are included by default (for printf and LARGE_INTEGER.QuadPart (long long) use %lld or %016llx).
 * inttypes.h, stdbool.h & string.h included for various type support and utilities.
 * windows.h is needed for various timer functions (GetTickCount(), QueryPerformanceFrequency(), QueryPerformanceCounter())
 */

#if __WIN32 || __WIN64
#	include <windows.h>
#elif linux || __linux__ || __APPLE__
#	include <time.h>
#else
#	error "Unknown build platform."
#endif

//------------------------------------------------------------------------------
bool CheckDependencies(void) {
#if defined(EXIT_ON_WRONG_FW_DEPENDENCY) || defined(EXIT_ON_WRONG_LIB_DEPENDENCY)
	uint8_t version_major, version_minor, build;
	bool wrong_version = false;
#endif
	UFR_STATUS status;

#ifdef EXIT_ON_WRONG_LIB_DEPENDENCY
	uint32_t dwDllVersion = 0;

	dwDllVersion = GetDllVersion();

	// "explode" the uFCoder library version:
	version_major = (uint8_t)dwDllVersion;
	version_minor = (uint8_t)(dwDllVersion >> 8);

	// Get the uFCoder library build number.
	build = (uint8_t)(dwDllVersion >> 16);

	if (version_major < MIN_DEPEND_LIB_VER_MAJOR) {
		wrong_version = true;
	} else if (version_major == MIN_DEPEND_LIB_VER_MAJOR && version_minor < MIN_DEPEND_LIB_VER_MINOR) {
		wrong_version = true;
	} else if (version_major == MIN_DEPEND_LIB_VER_MAJOR && version_minor == MIN_DEPEND_LIB_VER_MINOR && build < MIN_DEPEND_LIB_VER_BUILD) {
		wrong_version = true;
	}

	if (wrong_version) {
		printf("Wrong uFCoder library version (%d.%d.%d).\n"
			   "Please update uFCoder library to at last %d.%d.%d version.\n",
			   version_major, version_minor, build,
			   MIN_DEPEND_LIB_VER_MAJOR, MIN_DEPEND_LIB_VER_MINOR, MIN_DEPEND_LIB_VER_BUILD);
		return false;
	}
#endif
#ifdef EXIT_ON_WRONG_FW_DEPENDENCY
	wrong_version = false;
	status = GetReaderFirmwareVersion(&version_major, &version_minor);
	if (status != UFR_OK) {
		printf("Error while checking firmware version, status is: 0x%08X\n", status);
	}
	status = GetBuildNumber(&build);

	if (status != UFR_OK) {
		printf("Error while firmware version, status is: 0x%08X\n", status);
	}
	if (version_major < MIN_DEPEND_FW_VER_MAJOR) {
		wrong_version = true;
	} else if (version_major == MIN_DEPEND_FW_VER_MAJOR && version_minor < MIN_DEPEND_FW_VER_MINOR) {
		wrong_version = true;
	} else if (version_major == MIN_DEPEND_FW_VER_MAJOR && version_minor == MIN_DEPEND_FW_VER_MINOR && build < MIN_DEPEND_FW_VER_BUILD) {
		wrong_version = true;
	}

	if (wrong_version) {
		printf("Wrong uFR NFC reader firmware version (%d.%d.%d).\n"
			   "Please update uFR firmware to at last %d.%d.%d version.\n",
			   version_major, version_minor, build,
			   MIN_DEPEND_FW_VER_MAJOR, MIN_DEPEND_FW_VER_MINOR, MIN_DEPEND_FW_VER_BUILD);
		return false;
	}
#endif
	return true;
}
//------------------------------------------------------------------------------
#if __WIN32 || __WIN64

double time_difference_s(LARGE_INTEGER start, LARGE_INTEGER end) {
	static LARGE_INTEGER frequency;
	double res;

	if (frequency.QuadPart == 0) {
		QueryPerformanceFrequency(&frequency);
	}

	res = (double) (end.QuadPart - start.QuadPart) / frequency.QuadPart;
	return res;
}

double time_difference_ms(LARGE_INTEGER start, LARGE_INTEGER end) {
	double res = time_difference_s(start, end) * 1000.0;

    return res;
}

#elif linux || __linux__ || __APPLE__

timespec time_difference(timespec start, timespec end)
{
	timespec res;

    if ((end.tv_nsec - start.tv_nsec) < 0) {
    	res.tv_sec = end.tv_sec - start.tv_sec - 1;
    	res.tv_nsec = 1000000000L + end.tv_nsec - start.tv_nsec;
    } else {
    	res.tv_sec = end.tv_sec - start.tv_sec;
    	res.tv_nsec = end.tv_nsec - start.tv_nsec;
    }

    return res;
}

double time_difference_s(timespec start, timespec end)
{
	timespec diff = time_difference();
	double res = diff.tv_sec + (0.000000001 * diff.tv_nsec);

    return res;
}

double time_difference_ms(timespec start, timespec end)
{
	timespec diff = time_difference();
	double res = diff.tv_sec + (0.000001 * diff.tv_nsec);

    return res;
}

#endif
