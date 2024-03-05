#pragma once

#include <algorithm>
#include <vector>
#include "defs.h"
#include "hash.h"
#include <fstream>
#include <vector>

class MINHASH_CM
{
public:
    int N_ARRAY, LEN, HASH_CNT;
    count_t **cm;
    seed_t *slot_seed;
    seed_t *hash_seed;
    uint64_t *min_hash_value;

    MINHASH_CM(int n_array, int len, int hash_cnt) : N_ARRAY(n_array), LEN(len), HASH_CNT(hash_cnt)
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

        min_hash_value = new uint64_t[HASH_CNT];
        for (int i = 0; i < HASH_CNT; i++)
        {
            min_hash_value[i] = 0xffffffffffffffff;
        }
    }

    ~MINHASH_CM()
    {
        for (int i = 0; i < N_ARRAY; i++)
        {
            delete[] cm[i];
        }
        delete[] cm;
        delete[] slot_seed;
        delete[] hash_seed;
        delete[] min_hash_value;
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
        int min_cm_value = get_value_from_cm(item);
        for (int i = 0; i < HASH_CNT; i++)
        {
            min_hash_value[i] = std::min(min_hash_value[i], HASH::hash(HASH::hash(item, hash_seed[i]), min_cm_value));
        }
    }
};

double similarity_minhash_cm(MINHASH_CM *minhash1, MINHASH_CM *minhash2)
{
    LOG_DEBUG("into similarity_minhash()");
    double similarity = 0;
    int same_hash = 0, all_hash = minhash1->HASH_CNT;
    for (int i = 0; i < all_hash; i++)
        if (minhash1->min_hash_value[i] == minhash2->min_hash_value[i])
            same_hash++;
    similarity = 1.0 * same_hash / all_hash;
    LOG_DEBUG("same_hash: %d, all_hash: %d", same_hash, all_hash);
    LOG_RESULT("similarity: %lf", similarity);
    LOG_DEBUG("exit similarity_minhash()");
    return similarity;
}


class MINHASH_HASH
{
public:
    int HASH_LEN, HASH_CNT;
    seed_t *hash_seed;
    uint64_t *min_hash_value;
    seed_t index_s;
    struct u *hash;
    int left_pos;

    MINHASH_HASH( int len, int hash_cnt) : HASH_LEN(len), HASH_CNT(hash_cnt)
    {
        srand(clock());
        index_s = rand();
        left_pos=HASH_LEN;
        hash = new struct u[HASH_LEN];
        hash_seed = new seed_t[HASH_CNT];
        memset(hash, 0, sizeof(struct u) * HASH_LEN);
        hash_seed = new seed_t[HASH_CNT];
        for (int i = 0; i < HASH_CNT; i++)
        {
            hash_seed[i] = rand();
        }

        min_hash_value = new uint64_t[HASH_CNT];
        for (int i = 0; i < HASH_CNT; i++)
        {
            min_hash_value[i] = 0xffffffffffffffff;
        }
    }

    ~MINHASH_HASH()
    {
        delete[] hash;
        delete[] hash_seed;
        delete[] min_hash_value;
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
        int min_cm_value = get_index_from_hash(item);
        for (int i = 0; i < HASH_CNT; i++)
        {
            min_hash_value[i] = std::min(min_hash_value[i], HASH::hash(HASH::hash(item, hash_seed[i]), min_cm_value));
        }
    }
};

double similarity_minhash_hash(MINHASH_HASH *minhash1, MINHASH_HASH *minhash2)
{
    LOG_DEBUG("into similarity_minhash()");
    double similarity = 0;
    int same_hash = 0, all_hash = minhash1->HASH_CNT;
    for (int i = 0; i < all_hash; i++)
        if (minhash1->min_hash_value[i] == minhash2->min_hash_value[i])
            same_hash++;
    similarity = 1.0 * same_hash / all_hash;
    LOG_DEBUG("same_hash: %d, all_hash: %d", same_hash, all_hash);
    LOG_RESULT("similarity: %lf", similarity);
    LOG_DEBUG("exit similarity_minhash()");
    return similarity;
}
