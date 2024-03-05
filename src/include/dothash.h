#pragma once
#ifndef __DOTHASH_H__

#define __DOTHASH_H__

#include "defs.h"
#include "hash.h"
#include <algorithm>
#include <map>
#include <unordered_map>
using namespace std;

struct u
{
    data_t item;
    int index;
};

class dothash_cm
{
public:
    double *sum1;
    double *sum2;
    uint64_t size1,size2;
    seed_t *seed;
    seed_t cm_s;
    int D;
    int *cm1;
    int *cm2;
    int CM_LEN;
    dothash_cm(int dimension, int cm_len)
    {
        size1=0;
        size2=0;
        D=dimension;
        sum1=new double[D];
        sum2=new double[D];
        seed=new seed_t[D];
        cm_s=rand();
        for(int i=0;i<D;i++){
            seed[i]=rand();
        }
        cm1 = new int[cm_len];
        cm2 = new int[cm_len];
        CM_LEN = cm_len;
        memset(cm1, 0, sizeof(int) * cm_len);
        memset(cm2, 0, sizeof(int) * cm_len);
        memset(sum1, 0, sizeof(double) * D);
        memset(sum2, 0, sizeof(double) * D);
    }
    ~dothash_cm()
    {
        delete[] cm1;
        delete[] cm2;
        delete[] sum1;
        delete[] sum2;
        delete[] seed;
    }
    uint64_t _hash(const char *s, size_t len, uint64_t seed)
    {
        return NAMESPACE_FOR_HASH_FUNCTIONS::Hash64WithSeed(s, len, seed);
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
    void insert1(data_t item)
    {
        size1++;
        struct u tmp;
        tmp.item = item;
        tmp.index = get_index_from_cm1(item);
        double vec[D];
        double l=0;
        for(int i=0;i<D;i++){
            uint64_t finger =_hash(reinterpret_cast<const char *>(&tmp), 12, seed[i]);
            vec[i]= double(finger%10000)/double(10000);
            l=l+vec[i]*vec[i];
        }
        l=sqrt(l);
        for(int i=0;i<D;i++){
            vec[i]=vec[i]/l;
            sum1[i]+=vec[i];
        }
    }
    void insert2(data_t item)
    {
        size2++;
        struct u tmp;
        tmp.item = item;
        tmp.index = get_index_from_cm2(item);
        double vec[D];
        double l=0;
        for(int i=0;i<D;i++){
            uint64_t finger =_hash(reinterpret_cast<const char *>(&tmp), 12, seed[i]);
            vec[i]= double(finger%10000)/double(10000);
            l=l+vec[i]*vec[i];
        }
        l=sqrt(l);
        for(int i=0;i<D;i++){
            vec[i]=vec[i]/l;
            sum2[i]+=vec[i];
        }
    }
    double simiarity(){
        double A_B=0;
        for(int i=0;i<D;i++){
            A_B+=(sum1[i]*sum2[i]);
        }
        return A_B/double(size1+size2)-A_B;
    }
};

#endif