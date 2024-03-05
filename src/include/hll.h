#pragma once
#ifndef __HLL_H__

#define __HLL_H__

#include "defs.h"
#include "hash.h"
#include <algorithm>
#include <map>
#include <unordered_map>
using namespace std;
#define GET_BUCKET_INDEX(x) (x >> 50) & 0x3fff // get the bucket index with the highest 14 bits
#define GET_COUNT_KEY(x) (x & 0x3ffffffffffff) // get the count key with the lowest 50 bits


class hyperloglog_cm
{
public:
    uint8_t *bucket1;
    uint8_t *bucket2;
    seed_t seed;
    seed_t cm_s;
    int bucket_num;
    int *cm1;
    int *cm2;
    int CM_LEN;
    hyperloglog_cm(int bucketnum, int cm_len)
    {
        bucket1 = new uint8_t[bucketnum];
        bucket2 = new uint8_t[bucketnum];
        bucket_num = bucketnum;
        seed = rand();
        cm_s = rand();
        cm1 = new int[cm_len];
        cm2 = new int[cm_len];
        CM_LEN = cm_len;
        memset(cm1, 0, sizeof(int) * cm_len);
        memset(cm2, 0, sizeof(int) * cm_len);
        memset(bucket1, 0, sizeof(uint8_t) * bucketnum);
        memset(bucket2, 0, sizeof(uint8_t) * bucketnum);
    }
    ~hyperloglog_cm()
    {
        delete[] cm1;
        delete[] cm2;
        delete[] bucket1;
        delete[] bucket2;
    }
    int get_index_from_cm1(data_t item)
    {
        int index = HASH::hash(item, cm_s) % CM_LEN;
        cm1[index]++;
        return cm1[index];
    }
    int get_index_from_cm2(data_t item)
    {
        int index = HASH::hash(item, cm_s) % CM_LEN;
        cm2[index]++;
        return cm2[index];
    }
    uint64_t hll_hash(const char *s, size_t len, uint64_t seed)
    {
        return NAMESPACE_FOR_HASH_FUNCTIONS::Hash64WithSeed(s, len, seed);
    }
    uint8_t getcount(uint64_t key)
    {
        for (uint8_t i = 49; i >= 0; i--)
        {
            if (((1 << i) & key) != 0)
            {
                uint8_t ans = 50 - i;
                // printf("key:%llx,i:%d\n",key,i);
                return ans;
            }
        }
        return 0;
    }
    double get_constant(int bucketnum)
    {
        int i = 0;
        while (1)
        {
            i++;
            if ((1 << i) == bucketnum)
                break;
        }
        switch (i)
        {
        case 4:
            return 0.673;
        case 5:
            return 0.697;
        case 6:
            return 0.709;
        default:
            return 0.7213 / (1 + 1.079 / bucketnum);
        }
    }
    void insert1(data_t item)
    {
        struct u tmp;
        tmp.item = item;
        tmp.index = get_index_from_cm1(item);
        uint64_t finger = hll_hash(reinterpret_cast<const char *>(&tmp), sizeof(struct u), seed);
        uint8_t new_cnt = getcount(GET_COUNT_KEY(finger));
        int index = GET_BUCKET_INDEX(finger) % bucket_num;
        if (bucket1[index] < new_cnt)
        {
            bucket1[index] = new_cnt;
        }
    }
    void insert2(data_t item)
    {
        struct u tmp;
        tmp.item = item;
        tmp.index = get_index_from_cm2(item);
        uint64_t finger = hll_hash(reinterpret_cast<const char *>(&tmp), sizeof(struct u), seed);
        uint8_t new_cnt = getcount(GET_COUNT_KEY(finger));
        int index = GET_BUCKET_INDEX(finger) % bucket_num;
        if (bucket2[index] < new_cnt)
        {
            bucket2[index] = new_cnt;
        }
    }
    uint64_t get_estimated_size1()
    {
        double all = 0;
        for (int i = 0; i < bucket_num; i++)
        {
            all += (double)1 / (double)(1 << bucket1[i]);
        }
        // printf("all:%f",all);
        double constant = get_constant(bucket_num);
        uint64_t num = (uint64_t)(constant * (1 / all) * (bucket_num) * (bucket_num));
        return num;
    }
    uint64_t get_estimated_size2()
    {
        double all = 0;
        for (int i = 0; i < bucket_num; i++)
        {
            all += (double)1 / (double)(1 << bucket2[i]);
        }
        // printf("all:%f",all);
        double constant = get_constant(bucket_num);
        uint64_t num = (uint64_t)(constant * (1 / all) * (bucket_num) * (bucket_num));
        return num;
    }
    uint64_t get_estimated_size_1and2()
    {
        double all = 0;
        for (int i = 0; i < bucket_num; i++)
        {
            uint8_t m=std::max(bucket1[i],bucket2[i]);
            all += (double)1 / (double)(1 << m);
        }
        // printf("all:%f",all);
        double constant = get_constant(bucket_num);
        uint64_t num = (uint64_t)(constant * (1 / all) * (bucket_num) * (bucket_num));
        return num;
    }
    double similarity(){
        uint64_t size1=get_estimated_size1();
        uint64_t size2=get_estimated_size2();
        uint64_t size1_2=get_estimated_size_1and2();
        return double(size1+size2-size1_2)/double(size1_2);
    }
};

class hyperloglog_excat
{
public:
    uint8_t *bucket1;
    uint8_t *bucket2;
    seed_t seed;
    int bucket_num;
    unordered_map<data_t, int> record_chart1;
    unordered_map<data_t, int> record_chart2;
    hyperloglog_excat(int bucketnum)
    {
        bucket1 = new uint8_t[bucketnum];
        bucket2 = new uint8_t[bucketnum];
        bucket_num = bucketnum;
        seed = rand();
        memset(bucket1, 0, sizeof(uint8_t) * bucketnum);
        memset(bucket2, 0, sizeof(uint8_t) * bucketnum);
    }
    ~hyperloglog_excat()
    {
        delete[] bucket1;
        delete[] bucket2;
    }
    int get_index_from_chart1(data_t item)
    {
        auto it = record_chart1.find(item);
        if (it == record_chart1.end())
        {
            record_chart1.insert(make_pair(item, 1));
            return 1;
        }
        it->second++;
        return it->second;
    }
    int get_index_from_chart2(data_t item)
    {
        auto it = record_chart2.find(item);
        if (it == record_chart2.end())
        {
            record_chart2.insert(make_pair(item, 1));
            return 1;
        }
        it->second++;
        return it->second;
    }
    uint64_t hll_hash(const char *s, size_t len, uint64_t seed)
    {
        return NAMESPACE_FOR_HASH_FUNCTIONS::Hash64WithSeed(s, len, seed);
    }
    uint8_t getcount(uint64_t key)
    {
        for (uint8_t i = 49; i >= 0; i--)
        {
            if (((1 << i) & key) != 0)
            {
                uint8_t ans = 50 - i;
                // printf("key:%llx,i:%d\n",key,i);
                return ans;
            }
        }
        return 0;
    }
    double get_constant(int bucketnum)
    {
        int i = 0;
        while (1)
        {
            i++;
            if ((1 << i) == bucketnum)
                break;
        }
        switch (i)
        {
        case 4:
            return 0.673;
        case 5:
            return 0.697;
        case 6:
            return 0.709;
        default:
            return 0.7213 / (1 + 1.079 / bucketnum);
        }
    }
    void insert1(data_t item)
    {
        struct u tmp;
        tmp.item = item;
        tmp.index = get_index_from_chart1(item);
        uint64_t finger = hll_hash(reinterpret_cast<const char *>(&tmp), sizeof(struct u), seed);
        uint8_t new_cnt = getcount(GET_COUNT_KEY(finger));
        int index = GET_BUCKET_INDEX(finger) % bucket_num;
        if (bucket1[index] < new_cnt)
        {
            bucket1[index] = new_cnt;
        }
    }
    void insert2(data_t item)
    {
        struct u tmp;
        tmp.item = item;
        tmp.index = get_index_from_chart2(item);
        uint64_t finger = hll_hash(reinterpret_cast<const char *>(&tmp), sizeof(struct u), seed);
        uint8_t new_cnt = getcount(GET_COUNT_KEY(finger));
        int index = GET_BUCKET_INDEX(finger) % bucket_num;
        if (bucket2[index] < new_cnt)
        {
            bucket2[index] = new_cnt;
        }
    }
    uint64_t get_estimated_size1()
    {
        double all = 0;
        for (int i = 0; i < bucket_num; i++)
        {
            all += (double)1 / (double)(1 << bucket1[i]);
        }
        // printf("all:%f",all);
        double constant = get_constant(bucket_num);
        uint64_t num = (uint64_t)(constant * (1 / all) * (bucket_num) * (bucket_num));
        return num;
    }
    uint64_t get_estimated_size2()
    {
        double all = 0;
        for (int i = 0; i < bucket_num; i++)
        {
            all += (double)1 / (double)(1 << bucket2[i]);
        }
        // printf("all:%f",all);
        double constant = get_constant(bucket_num);
        uint64_t num = (uint64_t)(constant * (1 / all) * (bucket_num) * (bucket_num));
        return num;
    }
    uint64_t get_estimated_size_1and2()
    {
        double all = 0;
        for (int i = 0; i < bucket_num; i++)
        {
            uint8_t m=std::max(bucket1[i],bucket2[i]);
            all += (double)1 / (double)(1 << m);
        }
        // printf("all:%f",all);
        double constant = get_constant(bucket_num);
        uint64_t num = (uint64_t)(constant * (1 / all) * (bucket_num) * (bucket_num));
        return num;
    }
    double similarity(){
        uint64_t size1=get_estimated_size1();
        uint64_t size2=get_estimated_size2();
        uint64_t size1_2=get_estimated_size_1and2();
        return double(size1+size2-size1_2)/double(size1_2);
    }
};

class hyperloglog_hash
{
public:
    uint8_t *bucket1;
    uint8_t *bucket2;
    seed_t seed;
    seed_t index_s;
    int bucket_num;
    struct u *hash1;
    struct u *hash2;
    int HASH_LEN;
    int left_pos1;
    int left_pos2;
    hyperloglog_hash(int bucketnum, int hash_len)
    {
        bucket1 = new uint8_t[bucketnum];
        bucket2 = new uint8_t[bucketnum];
        bucket_num = bucketnum;
        seed = rand();
        index_s = rand();
        hash1 = new struct u[hash_len];
        hash2 = new struct u[hash_len];
        HASH_LEN = hash_len;
        left_pos1 = hash_len;
        left_pos2 = hash_len;
        memset(hash1, 0, sizeof(struct u) * hash_len);
        memset(hash2, 0, sizeof(struct u) * hash_len);
        memset(bucket1, 0, sizeof(uint8_t) * bucketnum);
        memset(bucket2, 0, sizeof(uint8_t) * bucketnum);
    }
    ~hyperloglog_hash()
    {
        delete[] hash1;
        delete[] hash2;
        delete[] bucket1;
        delete[] bucket2;
    }
    int get_index_from_hash1(data_t item)
    { 
        // printf("left_pos 1:%d\n",left_pos1);
        int index = HASH::hash(item, index_s) % HASH_LEN;
        if (hash1[index].item == 0 && hash1[index].index == 0)
        {
            hash1[index].item = item;
            hash1[index].index = 1;
            left_pos1--;
            return 1;
        }
        if (hash1[index].item == item)
        {
            hash1[index].index++;
            return hash1[index].index;
        }
        if (left_pos1 == 0)
            return 0; 
        int dst_id = (index + 1) % HASH_LEN;
        while (hash1[dst_id].item != item)
        {
            if (hash1[dst_id].item == 0 && hash1[dst_id].index == 0)
            {
                break;
            }
            dst_id = (dst_id + 1) % HASH_LEN;
            if (dst_id == index)
                return 0;
        }
        if (hash1[dst_id].item == 0 && hash1[dst_id].index == 0)
        {
            hash1[dst_id].item = item;
            hash1[dst_id].index = 1;
            left_pos1--;
            return 1;
        }
        hash1[dst_id].index++;
        return hash1[dst_id].index;
    }
    int get_index_from_hash2(data_t item)
    { 
        int index = HASH::hash(item, index_s) % HASH_LEN;
        if (hash2[index].item == 0 && hash2[index].index == 0)
        {
            hash2[index].item = item;
            hash2[index].index = 1;
            left_pos2--;
            return 1;
        }
        if (hash2[index].item == item)
        {
            hash2[index].index++;
            return hash2[index].index;
        }
        if (left_pos2 == 0)
            return 0;
        int dst_id = (index + 1) % HASH_LEN;
        while (hash2[dst_id].item != item)
        {
            if (hash2[dst_id].item == 0 && hash2[dst_id].index == 0)
            {
                break;
            }
            dst_id = (dst_id + 1) % HASH_LEN;
            if (dst_id == index)
                return 0;
        }
        if (hash2[dst_id].item == 0 && hash2[dst_id].index == 0)
        {
            hash2[dst_id].item = item;
            hash2[dst_id].index = 1;
            left_pos2--;
            return 1;
        }
        hash2[dst_id].index++;
        return hash2[dst_id].index;
    }
    uint64_t hll_hash(const char *s, size_t len, uint64_t seed)
    {
        return NAMESPACE_FOR_HASH_FUNCTIONS::Hash64WithSeed(s, len, seed);
    }
    uint8_t getcount(uint64_t key)
    {
        for (uint8_t i = 49; i >= 0; i--)
        {
            if (((1 << i) & key) != 0)
            {
                uint8_t ans = 50 - i;
                // printf("key:%llx,i:%d\n",key,i);
                return ans;
            }
        }
        return 0;
    }
    double get_constant(int bucketnum)
    {
        int i = 0;
        while (1)
        {
            i++;
            if ((1 << i) == bucketnum)
                break;
        }
        switch (i)
        {
        case 4:
            return 0.673;
        case 5:
            return 0.697;
        case 6:
            return 0.709;
        default:
            return 0.7213 / (1 + 1.079 / bucketnum);
        }
    }
    void insert1(data_t item)
    {
        struct u tmp;
        tmp.item = item;
        tmp.index = get_index_from_hash1(item);
        if (tmp.index == 0)
            return;
        uint64_t finger = hll_hash(reinterpret_cast<const char *>(&tmp), sizeof(struct u), seed);
        uint8_t new_cnt = getcount(GET_COUNT_KEY(finger));
        int index = GET_BUCKET_INDEX(finger) % bucket_num;
        if (bucket1[index] < new_cnt)
        {
            bucket1[index] = new_cnt;
        }
    }
    void insert2(data_t item)
    {
        struct u tmp;
        tmp.item = item;
        tmp.index = get_index_from_hash2(item);
        if (tmp.index == 0)
            return;
        uint64_t finger = hll_hash(reinterpret_cast<const char *>(&tmp), sizeof(struct u), seed);
        uint8_t new_cnt = getcount(GET_COUNT_KEY(finger));
        int index = GET_BUCKET_INDEX(finger) % bucket_num;
        if (bucket2[index] < new_cnt)
        {
            bucket2[index] = new_cnt;
        }
    }
    uint64_t get_estimated_size1()
    {
        double all = 0;
        for (int i = 0; i < bucket_num; i++)
        {
            all += (double)1 / (double)(1 << bucket1[i]);
        }
        // printf("all:%f",all);
        double constant = get_constant(bucket_num);
        uint64_t num = (uint64_t)(constant * (1 / all) * (bucket_num) * (bucket_num));
        return num;
    }
    uint64_t get_estimated_size2()
    {
        double all = 0;
        for (int i = 0; i < bucket_num; i++)
        {
            all += (double)1 / (double)(1 << bucket2[i]);
        }
        // printf("all:%f",all);
        double constant = get_constant(bucket_num);
        uint64_t num = (uint64_t)(constant * (1 / all) * (bucket_num) * (bucket_num));
        return num;
    }
    uint64_t get_estimated_size_1and2()
    {
        double all = 0;
        for (int i = 0; i < bucket_num; i++)
        {
            uint8_t m=std::max(bucket1[i],bucket2[i]);
            all += (double)1 / (double)(1 << m);
        }
        // printf("all:%f",all);
        double constant = get_constant(bucket_num);
        uint64_t num = (uint64_t)(constant * (1 / all) * (bucket_num) * (bucket_num));
        return num;
    }
    double similarity(){
        uint64_t size1=get_estimated_size1();
        uint64_t size2=get_estimated_size2();
        uint64_t size1_2=get_estimated_size_1and2();
        return double(size1+size2-size1_2)/double(size1_2);
    }
};
#endif