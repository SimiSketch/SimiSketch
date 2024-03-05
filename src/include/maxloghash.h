#pragma once

#include <algorithm>
#include <vector>
#include "defs.h"
#include "hash.h"
#include <fstream>
#include <vector>

uint64_t totalShingles = (1ull << 32) - 1;

class MAXLOGHASH_CM
{
public:
    int N_ARRAY, LEN, HASH_CNT;
    count_t **cm;
    seed_t *slot_seed;
    seed_t *hash_seed;
    uint64_t *max_hash_value;
    uint8_t *is_the_only_max;

    MAXLOGHASH_CM(int n_array, int len, int hash_cnt) : N_ARRAY(n_array), LEN(len), HASH_CNT(hash_cnt)
    {
        srand(clock());
        slot_seed = new seed_t[N_ARRAY];

        cm = new count_t *[N_ARRAY];
        for (int i = 0; i < N_ARRAY; i++)
        {
            slot_seed[i] = rand();
            cm[i] = new count_t[len];
            memset(cm[i], 0, sizeof(count_t) * len);
        }

        hash_seed = new seed_t[HASH_CNT];
        for (int i = 0; i < HASH_CNT; i++)
        {
            hash_seed[i] = rand();
        }

        max_hash_value = new uint64_t[HASH_CNT];
        for (int i = 0; i < HASH_CNT; i++)
        {
            max_hash_value[i] = 0;
        }

        is_the_only_max = new uint8_t[HASH_CNT];
        for (int i = 0; i < HASH_CNT; i++)
        {
            is_the_only_max[i] = 1;
        }
    }

    ~MAXLOGHASH_CM()
    {
        for (int i = 0; i < N_ARRAY; i++)
        {
            delete[] cm[i];
        }
        delete[] cm;
        delete[] slot_seed;
        delete[] hash_seed;
        delete[] max_hash_value;
        delete[] is_the_only_max;
    }

    int get_value_from_cm(data_t item){
        int min_cm_value = 0x7fffffff;
        for (int i = 0; i < N_ARRAY; i++)
        {
            int pos = HASH::hash(item, slot_seed[i]) % LEN;
            cm[i][pos] += 1;
            min_cm_value = std::min(min_cm_value, cm[i][pos]);
        }
        return min_cm_value;
    }

    void insert(data_t item)
    {
        int min_cm_value =get_value_from_cm(item);
        for (int i = 0; i < HASH_CNT; i++)
        {
            uint64_t hash_value = HASH::hash(HASH::hash(item, hash_seed[i]), min_cm_value);
            LOG_DEBUG("hash_value: %llu", hash_value);
            double normalized_hash_value = 1. * (hash_value % totalShingles) / totalShingles;
            LOG_DEBUG("normalized_hash_value: %lf", normalized_hash_value);
            double hash_value_log = -(log(normalized_hash_value) / log(2)); 
            LOG_DEBUG("hash_value_log: %lf", hash_value_log);
            assert(hash_value_log >= 0);
            uint64_t hash_value_log_int = (uint64_t)hash_value_log;
            if (hash_value_log_int > max_hash_value[i])
            {
                max_hash_value[i] = hash_value_log_int;
                is_the_only_max[i] = 1;
            }
            else if (hash_value_log_int == max_hash_value[i])
            {
                is_the_only_max[i] = 0;
            }
        }
    }
};

double similarity_maxloghash_cm(MAXLOGHASH_CM *maxloghash1, MAXLOGHASH_CM *maxloghash2)
{
    LOG_DEBUG("into similarity_maxloghash()");
    double similarity = 0;
    int same_hash = 0, all_hash = maxloghash1->HASH_CNT;
    for (int i = 0; i < all_hash; i++)
    {
        LOG_DEBUG("maxloghash1->max[%d]: %lld, maxloghash1->is_only[%d]: %d, maxloghash2->max[%d]: %lld, maxloghash2->is_only[%d]: %d", i, maxloghash1->max_hash_value[i], i, maxloghash1->is_the_only_max[i], i, maxloghash2->max_hash_value[i], i, maxloghash2->is_the_only_max[i]);

        if (maxloghash1->max_hash_value[i] > maxloghash2->max_hash_value[i] && maxloghash1->is_the_only_max[i])
            same_hash++;
        else if (maxloghash1->max_hash_value[i] < maxloghash2->max_hash_value[i] && maxloghash2->is_the_only_max[i])
            same_hash++;
    }
    similarity = 1. - 1.0 * same_hash / all_hash / 0.7213;
    LOG_DEBUG("same_hash: %d, all_hash: %d", same_hash, all_hash);
    // LOG_RESULT("similarity: %lf", similarity);
    LOG_DEBUG("exit similarity_maxloghash()");
    return similarity;
}

class MAXLOGHASH_HASH
{
public:
    int HASH_LEN, HASH_CNT;
    seed_t *hash_seed;
    uint64_t *max_hash_value;
    uint8_t *is_the_only_max;
    seed_t index_s;
    struct u *hash;
    int left_pos;

    MAXLOGHASH_HASH(int len, int hash_cnt) : HASH_LEN(len), HASH_CNT(hash_cnt)
    {
        srand(clock());
        index_s = rand();
        left_pos=HASH_LEN;
        hash = new struct u[HASH_LEN];
        hash_seed = new seed_t[HASH_CNT];
        memset(hash, 0, sizeof(struct u) * HASH_LEN);
        for (int i = 0; i < HASH_CNT; i++)
        {
            hash_seed[i] = rand();
        }

        max_hash_value = new uint64_t[HASH_CNT];
        for (int i = 0; i < HASH_CNT; i++)
        {
            max_hash_value[i] = 0;
        }

        is_the_only_max = new uint8_t[HASH_CNT];
        for (int i = 0; i < HASH_CNT; i++)
        {
            is_the_only_max[i] = 1;
        }
    }

    ~MAXLOGHASH_HASH()
    {
        delete[] hash;
        delete[] hash_seed;
        delete[] max_hash_value;
        delete[] is_the_only_max;
    }

    int get_index_from_hash(data_t item)
    {
        // printf("left_pos 1:%d\n",left_pos1);
        int index = HASH::hash(item, index_s) % HASH_LEN;
        if (hash[index].item == 0 && hash[index].index == 0)
        {
            hash[index].item = item;
            hash[index].index = 1;
            left_pos--;
            return 1;
        }
        if (hash[index].item == item)
        {
            hash[index].index++;
            return hash[index].index;
        }
        if (left_pos == 0)
            return 0;
        int dst_id = (index + 1) % HASH_LEN;
        while (hash[dst_id].item != item)
        {
            if (hash[dst_id].item == 0 && hash[dst_id].index == 0)
            {
                break;
            }
            dst_id = (dst_id + 1) % HASH_LEN;
            if (dst_id == index)
                return 0;
        }
        if (hash[dst_id].item == 0 && hash[dst_id].index == 0)
        {
            hash[dst_id].item = item;
            hash[dst_id].index = 1;
            left_pos--;
            return 1;
        }
        hash[dst_id].index++;
        return hash[dst_id].index;
    }

    void insert(data_t item)
    {
        int min_cm_value =get_index_from_hash(item);
        for (int i = 0; i < HASH_CNT; i++)
        {
            uint64_t hash_value = HASH::hash(HASH::hash(item, hash_seed[i]), min_cm_value);
            LOG_DEBUG("hash_value: %llu", hash_value);
            double normalized_hash_value = 1. * (hash_value % totalShingles) / totalShingles;
            LOG_DEBUG("normalized_hash_value: %lf", normalized_hash_value);
            double hash_value_log = -(log(normalized_hash_value) / log(2)); // 放缩到0-1然后取log
            LOG_DEBUG("hash_value_log: %lf", hash_value_log);
            assert(hash_value_log >= 0);
            uint64_t hash_value_log_int = (uint64_t)hash_value_log;
            if (hash_value_log_int > max_hash_value[i])
            {
                max_hash_value[i] = hash_value_log_int;
                is_the_only_max[i] = 1;
            }
            else if (hash_value_log_int == max_hash_value[i])
            {
                is_the_only_max[i] = 0;
            }
        }
    }
};

double similarity_maxloghash_hash(MAXLOGHASH_HASH *maxloghash1, MAXLOGHASH_HASH *maxloghash2)
{
    LOG_DEBUG("into similarity_maxloghash()");
    double similarity = 0;
    int same_hash = 0, all_hash = maxloghash1->HASH_CNT;
    for (int i = 0; i < all_hash; i++)
    {
        LOG_DEBUG("maxloghash1->max[%d]: %lld, maxloghash1->is_only[%d]: %d, maxloghash2->max[%d]: %lld, maxloghash2->is_only[%d]: %d", i, maxloghash1->max_hash_value[i], i, maxloghash1->is_the_only_max[i], i, maxloghash2->max_hash_value[i], i, maxloghash2->is_the_only_max[i]);

        if (maxloghash1->max_hash_value[i] > maxloghash2->max_hash_value[i] && maxloghash1->is_the_only_max[i])
            same_hash++;
        else if (maxloghash1->max_hash_value[i] < maxloghash2->max_hash_value[i] && maxloghash2->is_the_only_max[i])
            same_hash++;
    }
    similarity = 1. - 1.0 * same_hash / all_hash / 0.7213;
    LOG_DEBUG("same_hash: %d, all_hash: %d", same_hash, all_hash);
    // LOG_RESULT("similarity: %lf", similarity);
    LOG_DEBUG("exit similarity_maxloghash()");
    return similarity;
}
