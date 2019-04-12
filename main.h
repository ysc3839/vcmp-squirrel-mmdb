#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
	#define EXPORT __declspec(dllexport)

	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>

	#define strncpy(dst, src, n) strcpy_s(dst, n, src)

	#define access _access
	#define R_OK 4
#else
	#define EXPORT
#endif

#include "console.h"
#include "VCMP.h"
#include "SqImports.h"

HSQAPI sq;
PluginFuncs* vcmp_functions;

#include "sqfuncs.h"
