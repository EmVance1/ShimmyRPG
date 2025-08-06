#define _CRT_SECURE_NO_WARNINGS
#include <cstdint>
#include <cstdlib>
#include <cstdio>

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <queue>
#include <memory>

#include <iostream>
#include <fstream>
#include <sstream>

#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <stdexcept>
#include <chrono>
#include <random>
#include <filesystem>
namespace std { namespace fs = filesystem; }
#ifdef SHMY_WINDOWS
#define PATH_NORM(p) p
#else
#define PATH_NORM(p) p.u8string()
#endif

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>

#include <lua/lua.hpp>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/filereadstream.h>

#include <thread_pool/thread_pool.h>

#define UTF_CPP_CPLUSPLUS 202002L // C++20
#include <utf8.h>

#include <sfutil/sfutil.h>

namespace std { namespace fs = filesystem; }

