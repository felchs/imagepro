#include "string_utils.hpp"

namespace imagepro
{
	LPWSTR CharToLPWSTR(const char* str)
	{
		// Get the required buffer size for the wide character string
		int bufferSize = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);

		if (bufferSize == 0)
		{
			std::cerr << "Error in conversion!" << std::endl;
			return NULL;
		}

		// Allocate memory for the wide string (LPWSTR)
		LPWSTR wideStr = new wchar_t[bufferSize];

		// Perform the conversion
		MultiByteToWideChar(CP_UTF8, 0, str, -1, wideStr, bufferSize);

		return wideStr;
	}

	char* LPWSTRToChar(const LPWSTR wideStr)
	{
		// Get the required buffer size for the multi-byte string
		int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, NULL, 0, NULL, NULL);

		if (bufferSize == 0)
		{
			std::cerr << "Error in conversion!" << std::endl;
			return nullptr;
		}

		// Allocate memory for the char string (multi-byte)
		char* charStr = new char[bufferSize];

		// Perform the conversion
		WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, charStr, bufferSize, NULL, NULL);

		return charStr;
	}
}