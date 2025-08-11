#define _CRT_SECURE_NO_WARNINGS
#include <cstdlib>
#include <cstdio>
#include <cstdint>

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <queue>
#include <memory>
#include <string_view>

#include <iostream>
#include <fstream>
#include <sstream>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <random>
#include <filesystem>
namespace std { namespace fs = filesystem; }
#ifdef _WIN32
#define PATH_NORM(p) p
#else
#define PATH_NORM(p) p.string()
#endif

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>

#include <luajit-2.1/lua.hpp>

#include <rapidjson/rapidjson.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/document.h>

#define UTF_CPP_CPLUSPLUS 202002L // C++20
#include <utf8.h>

#include <thread_pool/thread_pool.h>
