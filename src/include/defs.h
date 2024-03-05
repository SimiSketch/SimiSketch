#pragma once
#ifndef __DEFS_H__

#define __DEFS_H__
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <deque>
#include <cstdint>
#include <string>
#include <chrono>
#include <cmath>
#include "logger.h"

#define NEW_FILE_PERM (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)

// typedef uint64_t data_t;
typedef uint32_t data_t;
typedef int32_t count_t;
typedef uint64_t seed_t;

struct u
{
    data_t item;
    int index;
};


typedef std::chrono::high_resolution_clock::time_point TP;

#endif