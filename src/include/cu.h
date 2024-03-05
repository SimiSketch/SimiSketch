#pragma once
#ifndef __CU_H__

#define __CU_H__
#include <algorithm>
#include "defs.h"
#include "hash.h"
#include <vector>

class CU
{
public:
    int N_ARRAY, LEN;
    count_t **nt1;
    count_t **nt2;
    seed_t *slot_seed;

    CU(int n_array, int len) : N_ARRAY(n_array), LEN(len)
    {
        srand(clock());
        slot_seed = new seed_t[N_ARRAY];

        nt1 = new count_t *[N_ARRAY];
        nt2 = new count_t *[N_ARRAY];
        for (int i = 0; i < N_ARRAY; i++)
        {
            slot_seed[i] = rand();
            nt1[i] = new count_t[len];
            nt2[i] = new count_t[len];
            memset(nt1[i], 0, sizeof(count_t) * len);
            memset(nt2[i], 0, sizeof(count_t) * len);
        }
    }

    ~CU()
    {
        for (int i = 0; i < N_ARRAY; i++)
        {
            delete[] nt1[i];
            delete[] nt2[i];
        }
        delete[] nt1;
        delete[] nt2;
    }

    void insert1(data_t item)
    {
        std::vector<std::pair<int, int>> id_chart;
        int min_cnt = INT32_MAX;
        for (int i = 0; i < N_ARRAY; i++)
        {
            int pos = HASH::hash(item, slot_seed[i]) % LEN;
            id_chart.push_back(std::make_pair(pos, nt1[i][pos]));
            if (nt1[i][pos] < min_cnt)
            {
                min_cnt = nt1[i][pos];
            }
        }
        for (int i = 0; i < N_ARRAY; i++)
        {
            auto t = id_chart[i];
            if (t.second == min_cnt)
            {
                nt1[i][t.first]++;
            }
        }
    }

    void insert2(data_t item)
    {
        std::vector<std::pair<int, int>> id_chart;
        int min_cnt = INT32_MAX;
        for (int i = 0; i < N_ARRAY; i++)
        {
            int pos = HASH::hash(item, slot_seed[i]) % LEN;
            id_chart.push_back(std::make_pair(pos, nt2[i][pos]));
            if (nt2[i][pos] < min_cnt)
            {
                min_cnt = nt2[i][pos];
            }
        }
        for (int i = 0; i < N_ARRAY; i++)
        {
            auto t = id_chart[i];
            if (t.second == min_cnt)
            {
                nt2[i][t.first]++;
            }
        }
    }

    double similarity()
    {
        int N_intersection = INT32_MAX;
        int N_union = 0;
        for (int j = 0; j < N_ARRAY; j++)
        {
            int cur_intersection = 0;
            int cur_union = 0;
            for (int k = 0; k < LEN; k++)
            {
                cur_intersection += std::min(nt1[j][k], nt2[j][k]);
                cur_union += std::max(nt1[j][k], nt2[j][k]);
            }
            N_intersection = std::min(N_intersection, cur_intersection);
            N_union = std::max(N_union, cur_union);
        }
        // LOG_INFO("#Intersection: %d, #Union: %d", N_intersection, N_union);
        // LOG_RESULT("Similarity: %lf", double(N_intersection) / N_union);
        return double(N_intersection) / N_union;
    }
};

#endif
