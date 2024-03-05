#pragma once
#ifndef __HASH_H__

#define __HASH_H__
#include "farm.h"
#include "defs.h"

namespace HASH
{
    inline uint64_t hash(const data_t data, seed_t seed = 0U)
    {
        return NAMESPACE_FOR_HASH_FUNCTIONS::Hash64WithSeed(reinterpret_cast<const char *>(&data), sizeof(data_t), seed);
    }
}

#endif