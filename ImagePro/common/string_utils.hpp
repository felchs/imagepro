#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <windows.h>
#include <iostream>

namespace imagepro
{
	LPWSTR CharToLPWSTR(const char* str);
	char* LPWSTRToChar(const LPWSTR wideStr);
}

#endif