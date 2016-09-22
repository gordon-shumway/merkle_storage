#include "utils.h"
#ifdef WIN32
#include <windows.h>
#endif


bool is_file_exists(const std::string& path)
{
#ifdef WIN32
	DWORD dwAttrib = GetFileAttributesA(path.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#endif
}

void delete_file(const std::string& path)
{
#ifdef WIN32
	::DeleteFileA(path.c_str());
#endif
}