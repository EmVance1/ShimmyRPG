#pragma once

#ifdef _WIN32
#define PATH_NORM(p) p
#else
#define PATH_NORM(p) p.u8string()
#endif

#ifdef _WIN32
#define path_fopen(filename, mode) _wfopen(filename.wstring().c_str(), L##mode);
#else
#define path_fopen(filename, mode) fopen((const char*)filename.u8string().c_str(), mode);
#endif

