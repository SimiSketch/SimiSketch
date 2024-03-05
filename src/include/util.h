#pragma once
#ifndef __UTIL_H__

#define __UTIL_H__
#include "defs.h"

inline TP now() { return std::chrono::high_resolution_clock::now(); }

inline int Open(const char* file, int flag, int perm = NEW_FILE_PERM)
{
    int fd;
    if ((fd=open(file,flag,perm))<0)
    {
        LOG_ERROR("Open file error!");
        LOG_ERROR("Can not open file: %s", file);
        exit(-1);
    }
    return fd;
}

inline void Write(int fd, const void* buf, size_t len)
{
    if (write(fd,buf,len)<0)
    {
        LOG_ERROR("Write error!");
        exit(-1);
    }
}

#endif