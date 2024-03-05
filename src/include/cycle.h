#pragma once
#ifndef __CYCLE_H__

#define __CYCLE_H__
#include <algorithm>
#include "defs.h"
#include "hash.h"

class cycle_ours
{
public:
    int *buckets1;
    int *buckets2;
    int *mask1;
    int *mask2;
    int *cm1;
    int *cm2;
    seed_t id;
    seed_t fuhao;
    int LEN;
    cycle_ours(int len) : LEN(len)
    {
        id = rand();
        fuhao = rand();
        mask1 = new int[LEN];
        mask2 = new int[LEN];
        buckets1 = new int[LEN];
        buckets2 = new int[LEN];
        cm1 = new int[LEN];
        cm2 = new int[LEN];
        memset(mask1, 0, sizeof(int) * LEN);
        memset(mask2, 0, sizeof(int) * LEN);
        memset(buckets1, 0, sizeof(int) * LEN);
        memset(buckets2, 0, sizeof(int) * LEN);
        memset(cm1, 0, sizeof(int) * LEN);
        memset(cm2, 0, sizeof(int) * LEN);
    }
    ~cycle_ours()
    {
        delete mask1;
        delete mask2;
        delete buckets1;
        delete buckets2;
        delete cm1;
        delete cm2;
    }

    int get_thres(int merge_time)
    {
        if (merge_time == 1)
        {
            return INT8_MAX;
        }
        if (merge_time == 2)
        {
            return INT16_MAX;
        }
        if (merge_time == 3)
        {
            return (1 << 23) - 1;
        }
        if (merge_time == 4)
        {
            return INT32_MAX;
        }
        return INT32_MAX; 
    }

    void merge1(int index)
    {
        // printf("1");
        int s_index1 = index;
        int e_index1 = index;
        while (mask1[e_index1] != 0)
        {
            e_index1++;
            if (e_index1 == LEN)
                e_index1 = 0;
        }
        int s_index2 = e_index1 + 1;
        if (s_index2 == LEN)
            s_index2 = 0;
        int e_index2 = s_index2;
        while (mask1[e_index2] != 0)
        {
            e_index2++;
            if (e_index2 == LEN)
                e_index2 = 0;
        }
        int merge_num = buckets1[s_index1] + buckets1[s_index2];
        int cm_num = cm1[s_index1] + cm1[s_index2];
        mask1[e_index1] = 1;
        buckets1[s_index1] = merge_num;
        buckets1[s_index2] = merge_num;
        cm1[s_index1] = cm_num;
        cm1[s_index2] = cm_num;
        while (s_index1 != e_index1)
        {
            s_index1++;
            if (s_index1 == LEN)
                s_index1 = 0;
            buckets1[s_index1] = merge_num;
            cm1[s_index1] = cm_num;
        }
        while (s_index2 != e_index2)
        {
            s_index2++;
            if (s_index2 == LEN)
                s_index2 = 0;
            buckets1[s_index2] = merge_num;
            cm1[s_index2] = cm_num;
        }
    }

    void insert1(data_t item)
    {
        int pos = HASH::hash(item, id) % LEN;
        int f = (HASH::hash(item, fuhao) % 2) * 2 - 1;
        int s_pos;
        int e_pos;
        e_pos = pos;
        s_pos = pos - 1;
        if (s_pos < 0)
            s_pos += LEN;
        while (mask1[s_pos] != 0)
        {
            s_pos--;
            if (s_pos < 0)
                s_pos += LEN;
        }
        s_pos++;
        if (s_pos == LEN)
            s_pos = 0;
        while (mask1[e_pos] != 0)
        {
            e_pos++;
            if (e_pos == LEN)
            {
                e_pos = 0;
            }
        }
        buckets1[s_pos] += f;
        cm1[s_pos]++;
        int num = cm1[s_pos];
        int it = s_pos;
        int merge_time = 1;
        while (it != e_pos)
        {
            it++;
            if (it == LEN)
                it = 0;
            buckets1[it] += f;
            cm1[it]++;
            merge_time++;
        }
        // printf("%d,%d,%d,%d\n",s_pos,e_pos,merge_time,num);
        int thres = get_thres(merge_time);
        // printf("%d,%d\n",merge_time,thres);
        if (num > thres)
        {
            // printf("%d,%d,%d\n",num,thres,merge_time);
            // printf("1");
            merge1(s_pos);
        }
    }

    void merge2(int index)
    {
        // printf("1");
        int s_index1 = index;
        int e_index1 = index;
        while (mask2[e_index1] != 0)
        {
            e_index1++;
            if (e_index1 == LEN)
                e_index1 = 0;
        }
        int s_index2 = e_index1 + 1;
        if (s_index2 == LEN)
            s_index2 = 0;
        int e_index2 = s_index2;
        while (mask2[e_index2] != 0)
        {
            e_index2++;
            if (e_index2 == LEN)
                e_index2 = 0;
        }
        int merge_num = buckets2[s_index1] + buckets2[s_index2];
        int cm_num = cm2[s_index1] + cm2[s_index2];
        mask2[e_index1] = 1;
        buckets2[s_index1] = merge_num;
        buckets2[s_index2] = merge_num;
        cm2[s_index1] = cm_num;
        cm2[s_index2] = cm_num;
        while (s_index1 != e_index1)
        {
            s_index1++;
            if (s_index1 == LEN)
                s_index1 = 0;
            buckets2[s_index1] = merge_num;
            cm2[s_index1] = cm_num;
        }
        while (s_index2 != e_index2)
        {
            s_index2++;
            if (s_index2 == LEN)
                s_index2 = 0;
            buckets2[s_index2] = merge_num;
            cm2[s_index2] = cm_num;
        }
    }

    void insert2(data_t item)
    {
        int pos = HASH::hash(item, id) % LEN;
        int f = (HASH::hash(item, fuhao) % 2) * 2 - 1;
        int s_pos;
        int e_pos;
        e_pos = pos;
        s_pos = pos - 1;
        if (s_pos < 0)
            s_pos += LEN;
        while (mask2[s_pos] != 0)
        {
            s_pos--;
            if (s_pos < 0)
                s_pos += LEN;
        }
        s_pos++;
        if (s_pos == LEN)
            s_pos = 0;
        while (mask2[e_pos] != 0)
        {
            e_pos++;
            if (e_pos == LEN)
            {
                e_pos = 0;
            }
        }
        buckets2[s_pos] += f;
        cm2[s_pos]++;
        int num = cm2[s_pos];
        int it = s_pos;
        int merge_time = 1;
        while (it != e_pos)
        {
            it++;
            if (it == LEN)
                it = 0;
            buckets2[it] += f;
            cm2[it]++;
            merge_time++;
        }
        // printf("%d,%d,%d,%d\n",s_pos,e_pos,merge_time,num);
        int thres = get_thres(merge_time);
        // printf("%d,%d\n",merge_time,thres);
        if (num > thres)
        {
            // printf("%d,%d,%d\n",num,thres,merge_time);
            // printf("1");
            merge2(s_pos);
        }
    }

    void merge_all(int *mask)
    {
        // for(int i=0;i<LEN;i++){
        //     printf("%d",mask[i]);
        // }
        for (int i = 0; i < LEN; i++)
        {
            if (mask1[i] == 0 && mask[i] == 1)
            {
                int j = i - 1;
                if (j < 0)
                    j = LEN - 1;
                while (mask1[j] != 0)
                {
                    j--;
                    if (j < 0)
                        j = LEN - 1;
                }
                j++;
                if (j == LEN)
                    j = 0;
                merge1(j);
            }
            if (mask2[i] == 0 && mask[i] == 1)
            {
                int j = i - 1;
                if (j < 0)
                    j = LEN - 1;
                while (mask2[j] != 0)
                {
                    j--;
                    if (j < 0)
                        j = LEN - 1;
                }
                j++;
                if (j == LEN)
                    j = 0;
                merge2(j);
            }
        }
        for (int i = 0; i < LEN; i++)
        {
            if (mask1[i] != mask[i])
            {
                printf("mask1 error!\n");
            }
            if (mask2[i] != mask[i])
            {
                printf("mask2 error!\n");
            }
        }
    }

    void get_block_distrubution()
    {
        int cm[100];
        for (int i = 0; i < 100; i++)
        {
            cm[i] = 0;
        }
        printf("#########The stream 1 distrubution:\n");
        for (int i = 0; i < LEN; i++)
        {
            if (mask1[i] == 0)
            {
                int j = i - 1;
                if (j < 0)
                    j = LEN - 1;
                int merge_time = 1;
                while (mask1[j] != 0)
                {
                    merge_time++;
                    j--;
                    if (j < 0)
                        j = LEN - 1;
                }
                cm[merge_time]++;
            }
        }
        for (int i = 0; i < 100; i++)
        {
            if (cm[i] == 0)
                continue;
            printf("The merge time:%d,num:%d\n", i, cm[i]);
        }
        for (int i = 0; i < 100; i++)
        {
            cm[i] = 0;
        }
        printf("#########The stream 2 distrubution:\n");
        for (int i = 0; i < LEN; i++)
        {
            if (mask2[i] == 0)
            {
                int j = i - 1;
                if (j < 0)
                    j = LEN - 1;
                int merge_time = 1;
                while (mask2[j] != 0)
                {
                    merge_time++;
                    j--;
                    if (j < 0)
                        j = LEN - 1;
                }
                cm[merge_time]++;
            }
        }
        for (int i = 0; i < 100; i++)
        {
            if (cm[i] == 0)
                continue;
            printf("The merge time:%d,num:%d\n", i, cm[i]);
        }
    }

    double similarity()
    {
        int *mask = new int[LEN];
        for (int i = 0; i < LEN; i++)
        {
            mask[i] = mask1[i] || mask2[i];
        }
        merge_all(mask);
        double similarity = 0;
        int total_w = 0;
        for (int i = 0; i < LEN; i++)
        {
            int weight = std::max(cm1[i], cm2[i]);
            if (mask[i] == 0)
            {
                if ((buckets1[i] > 0 && buckets2[i] > 0) || (buckets1[i] < 0 && buckets2[i] < 0))
                {
                    similarity += (double)std::min(abs(buckets1[i]), abs(buckets2[i])) / std::max(abs(buckets1[i]), abs(buckets2[i])) * weight;
                    total_w += weight;
                }
                else
                {
                    total_w += weight;
                }
            }
        }
        return similarity / total_w;
    }
};
#endif