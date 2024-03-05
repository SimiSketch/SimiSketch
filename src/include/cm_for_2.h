#pragma once
#include <algorithm>
#include "defs.h"
#include "hash.h"
#include "count_sketch.h"
#include <vector>
#include <fstream>

class CM_FOR_2
{
public:
    int N_ARRAY, LEN;
    count_t **nt1;
    count_t **nt2;
    seed_t *slot_seed;

    CM_FOR_2(int n_array, int len) : N_ARRAY(n_array), LEN(len)
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

    ~CM_FOR_2()
    {
        for (int i = 0; i < N_ARRAY; i++)
        {
            delete[] nt1[i];
            delete[] nt2[i];
        }
        delete[] nt1;
        delete[] nt2;
        delete[] slot_seed;
    }

    void insert1(data_t item)
    {
        for (int i = 0; i < N_ARRAY; i++)
        {
            int pos = HASH::hash(item, slot_seed[i]) % LEN;
            nt1[i][pos]++;
        }
    }

    void insert2(data_t item)
    {
        for (int i = 0; i < N_ARRAY; i++)
        {
            int pos = HASH::hash(item, slot_seed[i]) % LEN;
            nt2[i][pos]++;
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
        return double(N_intersection) / N_union;
    }

    double cosine_similarity()
    {
        uint64_t numerator = 0, denominator_1 = 0, denominator_2 = 0;

        for (int i = 0; i < N_ARRAY; i++)
        {
            for (int j = 0; j < LEN; j++)
            {
                numerator += uint64_t(nt1[i][j]) * nt2[i][j];
                denominator_1 += uint64_t(nt1[i][j]) * nt1[i][j];
                denominator_2 += uint64_t(nt2[i][j]) * nt2[i][j];
            }
        }

        return double(numerator) / (sqrt(denominator_1) * sqrt(denominator_2));
    }

    double cross_entropy()
    {
        double rst = 0;
        double all1 = 0, all2 = 0;
        for (int i = 0; i < N_ARRAY; i++)
        {
            for (int j = 0; j < LEN; j++)
            {
                all1 += nt1[i][j];
                all2 += nt2[i][j];
            }
        }
        for (int i = 0; i < N_ARRAY; i++)
        {
            for (int j = 0; j < LEN; j++)
            {
                double p = double(nt1[i][j]) / all1;
                double q = double(nt2[i][j]) / all2;
                if (nt2[i][j] == 0)
                    q = 1. / LEN;
                rst += p * log(q) / log(2);
            }
        }
        return -rst;
    }

    double kl_divergence()
    {
        double rst = 0;
        double all1 = 0, all2 = 0;
        for (int i = 0; i < N_ARRAY; i++)
        {
            for (int j = 0; j < LEN; j++)
            {
                all1 += nt1[i][j];
                all2 += nt2[i][j];
            }
        }
        for (int i = 0; i < N_ARRAY; i++)
        {
            for (int j = 0; j < LEN; j++)
            {
                double p = double(nt1[i][j]) / all1;
                double q = double(nt2[i][j]) / all2;
                if (nt2[i][j] == 0)
                    q = 1. / LEN;
                rst += p * log(p / q) / log(2);
            }
        }
        return rst;
    }

    double kl_divergence_of_distribution()
    {
        std::vector<int> A, B;
        for (int i = 0; i < N_ARRAY; i++)
        {
            for (int j = 0; j < LEN; j++)
            {
                A.push_back(nt1[i][j]);
                B.push_back(nt2[i][j]);
            }
        }
        int num = N_ARRAY * LEN;
        sort(A.begin(), A.end(), [](int a, int b)
             { return a > b; });
        sort(B.begin(), B.end(), [](int a, int b)
             { return a > b; });
        std::ofstream out1("./plot/cm1.txt");
        std::ofstream out2("./plot/cm2.txt");
        for (int it : A)
        {
            out1 << it << ' ';
        }
        for (int it : B)
        {
            out2 << it << ' ';
        }
        double kl = 0;
        double all1 = 0, all2 = 0;
        for (int i = 0; i < num; i++)
        {
            all1 += A[i];
            all2 += B[i];
        }
        for (int i = 0; i < num; i++)
        {
            kl += double(A[i]) / all1 * log((double(A[i]) / all1) / (double(B[i]) / all2));
            kl += double(B[i]) / all2 * log((double(B[i]) / all2) / (double(A[i]) / all1));
        }
        return kl / 2;
    }
};
