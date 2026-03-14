#include "pch.h"
#include "util/alloc.h"
#include <stddef.h>
#ifdef _WIN32
#include <Windows.h>
#else
#define _GNU_SOURCE
#include <sys/mman.h>
#include <unistd.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


namespace shmy::core {

size_t system_page_size(void) {
    static size_t size = SIZE_MAX;
    if (size == SIZE_MAX) {
#ifdef _WIN32
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        size = info.dwPageSize;
#else
        size = (size_t)sysconf(_SC_PAGESIZE);
#endif
    }
    return size;
}

#define ROUND(x, n) ( ((x)+(n)-1) / (n) * (n) )


ArenaAllocator::ArenaAllocator(ArenaAllocator&& other) : base_ptr(other.base_ptr), curr_ptr(other.curr_ptr), last_ptr(other.last_ptr) {
    other.base_ptr = nullptr;
}
ArenaAllocator::ArenaAllocator(size_t size) {
    init(size);
}
ArenaAllocator::~ArenaAllocator() {
    free(base_ptr);
}

ArenaAllocator& ArenaAllocator::operator=(ArenaAllocator&& other) {
    free(base_ptr);
    base_ptr = other.base_ptr;
    curr_ptr = other.curr_ptr;
    last_ptr = other.last_ptr;
    other.base_ptr = nullptr;
    return *this;
}

void ArenaAllocator::init(size_t nbytes) {
    if (base_ptr) return;
    size = ROUND(nbytes, system_page_size());
    base_ptr = (uint8_t*)malloc(size);
    curr_ptr = base_ptr;
    last_ptr = nullptr;
}

void ArenaAllocator::deinit() {
    free(base_ptr);
    base_ptr = nullptr;
}

void ArenaAllocator::reset() {
    curr_ptr = base_ptr;
    last_ptr = nullptr;
}


void* ArenaAllocator::allocate(size_t nbytes) {
    if (!base_ptr || nbytes == 0) return nullptr;
    const size_t block = ROUND(nbytes, 16);
    const size_t used = (size_t)(curr_ptr - base_ptr);
    if (used + block > size) {
        return nullptr;
    }
    uint8_t* ptr = curr_ptr;
    curr_ptr += block;
    last_ptr = ptr;
    return ptr;
}

}
