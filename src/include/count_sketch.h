#pragma once

#include <algorithm>
#include <vector>
#include "defs.h"
#include "hash.h"
#include <fstream>
#include <vector>

class COUNT_SKETCH
{
public:
    int N_ARRAY, LEN;
    count_t **nt;
    seed_t *slot_seed;
    seed_t sign_seed;

    COUNT_SKETCH(int n_array, int len) : N_ARRAY(n_array), LEN(len)
    {
        srand(clock());
        slot_seed = new seed_t[N_ARRAY];
        sign_seed = rand();

        nt = new count_t *[N_ARRAY];
        for (int i = 0; i < N_ARRAY; i++)
        {
            slot_seed[i] = rand();
            nt[i] = new count_t[len];
            memset(nt[i], 0, sizeof(count_t) * len);
        }
    }

    ~COUNT_SKETCH()
    {
        for (int i = 0; i < N_ARRAY; i++)
        {
            delete[] nt[i];
        }
        delete[] nt;
        delete[] slot_seed;
    }

    void insert(data_t item)
    {
        for (int i = 0; i < N_ARRAY; i++)
        {
            int pos = HASH::hash(item, slot_seed[i]) % LEN;
            int sign = (HASH::hash(item, sign_seed) % 2) * 2 - 1;
            nt[i][pos] += sign;
        }
    }
};

class COUNT_SKETCH_WEIGHTED_BY_CM
{
public:
    int N_ARRAY, LEN;
    count_t **nt;
    seed_t *slot_seed;
    seed_t sign_seed;
    count_t **cm_weighter;

    COUNT_SKETCH_WEIGHTED_BY_CM(int n_array, int len) : N_ARRAY(n_array), LEN(len)
    {
        srand(clock());
        slot_seed = new seed_t[N_ARRAY];
        sign_seed = rand();

        nt = new count_t *[N_ARRAY];
        cm_weighter = new count_t *[N_ARRAY];
        for (int i = 0; i < N_ARRAY; i++)
        {
            slot_seed[i] = rand();

            nt[i] = new count_t[len];
            memset(nt[i], 0, sizeof(count_t) * len);

            cm_weighter[i] = new count_t[len];
            memset(cm_weighter[i], 0, sizeof(count_t) * len);
        }
    }

    ~COUNT_SKETCH_WEIGHTED_BY_CM()
    {
        for (int i = 0; i < N_ARRAY; i++)
        {
            delete[] nt[i];
        }
        delete[] nt;
        delete[] slot_seed;

        for (int i = 0; i < N_ARRAY; i++)
        {
            delete[] cm_weighter[i];
        }
        delete[] cm_weighter;
    }

    void insert(data_t item)
    {
        for (int i = 0; i < N_ARRAY; i++)
        {
            int pos = HASH::hash(item, slot_seed[i]) % LEN;
            int sign = (HASH::hash(item, sign_seed) % 2) * 2 - 1;
            nt[i][pos] += sign;
            cm_weighter[i][pos]++;
        }
    }
};

class COUNT_SKETCH_MERGE
{
public:
    int N, LEN;
    count_t **counter_8bits;
    count_t **counter_16bits;
    seed_t *hash_seeds;
    seed_t sign_seed;

    std::vector<bool> *merge_bit_8_to_16;

    int overflow_127;
    int overflow_32767;

    COUNT_SKETCH_MERGE(int n_array, int len) : N(n_array), LEN(len)
    {
        srand(clock());
        hash_seeds = new seed_t[N];
        sign_seed = rand();

        counter_8bits = new count_t *[N];
        for (int i = 0; i < N; i++)
        {
            hash_seeds[i] = rand();
            counter_8bits[i] = new count_t[len];
            memset(counter_8bits[i], 0, sizeof(count_t) * len);
        }

        counter_16bits = new count_t *[N];
        for (int i = 0; i < N; i++)
        {
            counter_16bits[i] = new count_t[len / 2];
            memset(counter_16bits[i], 0, sizeof(count_t) * len / 2);
        }

        merge_bit_8_to_16 = new std::vector<bool>[N];
        for (int i = 0; i < N; i++)
            merge_bit_8_to_16[i].resize(LEN / 2, false);

        overflow_127 = 0;
        overflow_32767 = 0;
    }

    ~COUNT_SKETCH_MERGE()
    {
        for (int i = 0; i < N; i++)
        {
            delete[] counter_8bits[i];
        }
        delete[] counter_8bits;

        for (int i = 0; i < N; i++)
        {
            delete[] counter_16bits[i];
        }
        delete[] counter_16bits;

        delete[] hash_seeds;
        delete[] merge_bit_8_to_16;
    }

    void merge(int line, int pos_8)
    {
        // LOG_DEBUG("merged");

        overflow_127++;

        int pair_merge_bit = (pos_8 % 2 == 0 ? pos_8 + 1 : pos_8 - 1);
        int pos_16 = pos_8 / 2;

        merge_bit_8_to_16[line][pos_16] = true;

        counter_16bits[line][pos_16] = counter_8bits[line][pos_8] + counter_8bits[line][pair_merge_bit];
    }

    void insert(data_t item)
    {
        // LOG_DEBUG("item: %llu", item);
        for (int i = 0; i < N; i++)
        {
            int pos_8 = HASH::hash(item, hash_seeds[i]) % LEN;
            int sign = (HASH::hash(item, sign_seed) % 2) * 2 - 1;

            counter_8bits[i][pos_8] += sign;

            bool merged = merge_bit_8_to_16[i][pos_8 / 2];
            if (merged)
            {
                int pos_16 = pos_8 / 2;
                counter_16bits[i][pos_16] += sign;
            }
            else
            {
                if (counter_8bits[i][pos_8] > 127 || counter_8bits[i][pos_8] < -128)
                    merge(i, pos_8);
            }
        }
    }
};

class COUNT_SKETCH_MERGE_WEIGHTED_BY_CM
{
public:
    int N, LEN;
    count_t **counter_8bits;
    count_t **counter_16bits;
    seed_t *hash_seeds;
    seed_t sign_seed;

    std::vector<bool> *merge_bit_8_to_16;

    count_t **cm_weighter_8bits;
    count_t **cm_weighter_16bits;

    int overflow_127;
    int overflow_32767;

    COUNT_SKETCH_MERGE_WEIGHTED_BY_CM(int n_array, int len) : N(n_array), LEN(len)
    {
        srand(clock());
        hash_seeds = new seed_t[N];
        sign_seed = rand();

        counter_8bits = new count_t *[N];
        for (int i = 0; i < N; i++)
        {
            hash_seeds[i] = rand();
            counter_8bits[i] = new count_t[len];
            memset(counter_8bits[i], 0, sizeof(count_t) * len);
        }

        counter_16bits = new count_t *[N];
        for (int i = 0; i < N; i++)
        {
            counter_16bits[i] = new count_t[len / 2];
            memset(counter_16bits[i], 0, sizeof(count_t) * len / 2);
        }

        merge_bit_8_to_16 = new std::vector<bool>[N];
        for (int i = 0; i < N; i++)
            merge_bit_8_to_16[i].resize(LEN / 2, false);

        cm_weighter_8bits = new count_t *[N];
        for (int i = 0; i < N; i++)
        {
            cm_weighter_8bits[i] = new count_t[len];
            memset(cm_weighter_8bits[i], 0, sizeof(count_t) * len);
        }

        cm_weighter_16bits = new count_t *[N];
        for (int i = 0; i < N; i++)
        {
            cm_weighter_16bits[i] = new count_t[len / 2];
            memset(cm_weighter_16bits[i], 0, sizeof(count_t) * len / 2);
        }

        overflow_127 = 0;
        overflow_32767 = 0;
    }

    ~COUNT_SKETCH_MERGE_WEIGHTED_BY_CM()
    {
        for (int i = 0; i < N; i++)
        {
            delete[] counter_8bits[i];
        }
        delete[] counter_8bits;

        for (int i = 0; i < N; i++)
        {
            delete[] counter_16bits[i];
        }
        delete[] counter_16bits;

        delete[] hash_seeds;
        delete[] merge_bit_8_to_16;

        for (int i = 0; i < N; i++)
        {
            delete[] cm_weighter_8bits[i];
        }
        delete[] cm_weighter_8bits;

        for (int i = 0; i < N; i++)
        {
            delete[] cm_weighter_16bits[i];
        }
        delete[] cm_weighter_16bits;
    }

    void merge(int line, int pos_8)
    {
        // LOG_DEBUG("merged");

        overflow_127++;

        int pair_merge_bit = (pos_8 % 2 == 0 ? pos_8 + 1 : pos_8 - 1);
        int pos_16 = pos_8 / 2;

        merge_bit_8_to_16[line][pos_16] = true;

        counter_16bits[line][pos_16] = counter_8bits[line][pos_8] + counter_8bits[line][pair_merge_bit];
        cm_weighter_16bits[line][pos_16] = cm_weighter_8bits[line][pos_8] + cm_weighter_8bits[line][pair_merge_bit];
    }

    void insert(data_t item)
    {
        // LOG_DEBUG("item: %llu", item);
        for (int i = 0; i < N; i++)
        {
            int pos_8 = HASH::hash(item, hash_seeds[i]) % LEN;
            int sign = (HASH::hash(item, sign_seed) % 2) * 2 - 1;

            counter_8bits[i][pos_8] += sign;
            cm_weighter_8bits[i][pos_8]++;

            bool merged = merge_bit_8_to_16[i][pos_8 / 2];
            if (merged)
            {
                int pos_16 = pos_8 / 2;
                counter_16bits[i][pos_16] += sign;
                cm_weighter_16bits[i][pos_16]++;
            }
            else
            {
                // if (counter_8bits[i][pos_8] > 127 || counter_8bits[i][pos_8] < -128 || cm_weighter_8bits[i][pos_8] > 127 || cm_weighter_8bits[i][pos_8] < -128)
                //     merge(i, pos_8);
                if (counter_8bits[i][pos_8] > 127 || counter_8bits[i][pos_8] < -128)
                    merge(i, pos_8);
            }
        }
    }
};

double similarity_cs(COUNT_SKETCH *cs1, COUNT_SKETCH *cs2)
{
    int N_ARRAY = cs1->N_ARRAY;
    int LEN = cs1->LEN;
    count_t **nt1 = cs1->nt;
    count_t **nt2 = cs2->nt;

    double similarity = 0;
    int valid_counter_num = 0;
    for (int i = 0; i < N_ARRAY; i++)
    {
        for (int j = 0; j < LEN; j++)
        {
            if ((nt1[i][j] > 0 && nt2[i][j] > 0) || (nt1[i][j] < 0 && nt2[i][j] < 0))
            {
                double s1 = abs(nt1[i][j]);
                double s2 = abs(nt2[i][j]);
                similarity += std::min(s1, s2) / std::max(s1, s2);
            }
            if (nt1[i][j] != 0 || nt2[i][j] != 0)
                valid_counter_num++;
        }
    }
    similarity /= valid_counter_num;
    return similarity;
}

double cosine_similarity_cs(COUNT_SKETCH *cs1, COUNT_SKETCH *cs2)
{
    uint64_t numerator = 0, denominator_1 = 0, denominator_2 = 0;
    int N_ARRAY = cs1->N_ARRAY;
    int LEN = cs1->LEN;
    count_t **nt1 = cs1->nt;
    count_t **nt2 = cs2->nt;

    for (int i = 0; i < N_ARRAY; i++)
    {
        for (int j = 0; j < LEN; j++)
        {
            if (uint64_t(nt1[i][j]) * nt2[i][j] > 0)
                numerator += uint64_t(nt1[i][j]) * nt2[i][j];
            denominator_1 += uint64_t(nt1[i][j]) * nt1[i][j];
            denominator_2 += uint64_t(nt2[i][j]) * nt2[i][j];
        }
    }
    return double(numerator) / sqrt(denominator_1) / sqrt(denominator_2);
}

double similarity_cs_weighted_by_cm(COUNT_SKETCH_WEIGHTED_BY_CM *cs1, COUNT_SKETCH_WEIGHTED_BY_CM *cs2)
{
    int N_ARRAY = cs1->N_ARRAY;
    int LEN = cs1->LEN;
    count_t **nt1 = cs1->nt;
    count_t **nt2 = cs2->nt;

    double similarity = 0;
    int total_weight = 0;
    for (int i = 0; i < N_ARRAY; i++)
    {
        for (int j = 0; j < LEN; j++)
        {
            int s1 = nt1[i][j]; 
            int s2 = nt2[i][j];

            int weight = std::max(cs1->cm_weighter[i][j], cs2->cm_weighter[i][j]);
            if ((s1 > 0 && s2 > 0) || (s1 < 0 && s2 < 0))
            {
                similarity += (double)std::min(abs(s1), abs(s2)) / std::max(abs(s1), abs(s2)) * weight;
                total_weight += weight;
            }
            else if (s1 * s2 <= 0 && (s1 != 0 || s2 != 0))
            {
                total_weight += weight;
            }
        }
    }
    return similarity / total_weight;
}

double similarity_cs_merge(COUNT_SKETCH_MERGE *cs1, COUNT_SKETCH_MERGE *cs2)
{
    int N = cs1->N;
    int LEN = cs1->LEN;
    int valid_counter_num = 0;

    double similarity = 0;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < LEN; j++)
        {
            bool merged = cs1->merge_bit_8_to_16[i][j / 2];
            // merged = false; 

            if (merged)
            {
                assert(j % 2 == 0);

                int s1 = (cs1->merge_bit_8_to_16[i][j / 2] ? cs1->counter_16bits[i][j / 2] : cs1->counter_8bits[i][j] + cs1->counter_8bits[i][j + 1]);
                int s2 = (cs2->merge_bit_8_to_16[i][j / 2] ? cs2->counter_16bits[i][j / 2] : cs2->counter_8bits[i][j] + cs2->counter_8bits[i][j + 1]);
                if (s1 > 32767 || s1 < -32768)
                    cs1->overflow_32767++;
                if (s2 > 32767 || s2 < -32768)
                    cs2->overflow_32767++;
                // s1 = std::max(s1, -32768);
                // s1 = std::min(s1, 32767);
                // s2 = std::max(s2, -32768);
                // s2 = std::min(s2, 32767);

                if ((s1 > 0 && s2 > 0) || (s1 < 0 && s2 < 0))
                {
                    similarity += (double)std::min(abs(s1), abs(s2)) / std::max(abs(s1), abs(s2));
                    valid_counter_num++; 
                }
                else if (s1 * s2 <= 0 && (s1 != 0 || s2 != 0))
                {
                    valid_counter_num++; 
                }

                j++; 
            }
            else
            {
                int s1 = cs1->counter_8bits[i][j];
                int s2 = cs2->counter_8bits[i][j];
                s1 = std::max(s1, -128);
                s1 = std::min(s1, 127);
                s2 = std::max(s2, -128);
                s2 = std::min(s2, 127);

                if ((s1 > 0 && s2 > 0) || (s1 < 0 && s2 < 0))
                {
                    similarity += (double)std::min(abs(s1), abs(s2)) / std::max(abs(s1), abs(s2));
                    valid_counter_num++; // 
                }
                else if (s1 * s2 <= 0 && (s1 != 0 || s2 != 0))
                {
                    valid_counter_num++; // 
                }
            }
        }
    }
    similarity /= valid_counter_num;
    // LOG_DEBUG("cs1 overflow_127: %d, cs2 overflow_127: %d", cs1->overflow_127, cs2->overflow_127);
    // LOG_DEBUG("cs1 overflow_32767: %d, cs2 overflow_32767: %d", cs1->overflow_32767, cs2->overflow_32767);
    return similarity;
}

double cosine_similarity_cs_merge(COUNT_SKETCH_MERGE *cs1, COUNT_SKETCH_MERGE *cs2)
{
    uint64_t numerator = 0, denominator_1 = 0, denominator_2 = 0;

    int N = cs1->N;
    int LEN = cs1->LEN;

    double similarity = 0;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < LEN; j++)
        {
            bool merged = cs1->merge_bit_8_to_16[i][j / 2];
            // merged = false; // 

            if (merged)
            {
                assert(j % 2 == 0);

                int s1 = (cs1->merge_bit_8_to_16[i][j / 2] ? cs1->counter_16bits[i][j / 2] : cs1->counter_8bits[i][j] + cs1->counter_8bits[i][j + 1]);
                int s2 = (cs2->merge_bit_8_to_16[i][j / 2] ? cs2->counter_16bits[i][j / 2] : cs2->counter_8bits[i][j] + cs2->counter_8bits[i][j + 1]);
                if (s1 > 32767 || s1 < -32768)
                    cs1->overflow_32767++;
                if (s2 > 32767 || s2 < -32768)
                    cs2->overflow_32767++;
                s1 = std::max(s1, -32768);
                s1 = std::min(s1, 32767);
                s2 = std::max(s2, -32768);
                s2 = std::min(s2, 32767);

                if (uint64_t(s1) * s2 > 0)
                {
                    numerator += uint64_t(s1) * s2;
                }
                denominator_1 += uint64_t(s1) * s1;
                denominator_2 += uint64_t(s2) * s2;
                j++; 
            }
            else
            {
                int s1 = cs1->counter_8bits[i][j];
                int s2 = cs2->counter_8bits[i][j];
                s1 = std::max(s1, -128);
                s1 = std::min(s1, 127);
                s2 = std::max(s2, -128);
                s2 = std::min(s2, 127);

                if (uint64_t(s1) * s2 > 0)
                {
                    numerator += uint64_t(s1) * s2;
                }
                denominator_1 += uint64_t(s1) * s1;
                denominator_2 += uint64_t(s2) * s2;
            }
        }
    }
    similarity = double(numerator) / sqrt(denominator_1) / sqrt(denominator_2);
    return similarity;
}

double similarity_cs_merge_weighted_by_cm(COUNT_SKETCH_MERGE_WEIGHTED_BY_CM *cs1, COUNT_SKETCH_MERGE_WEIGHTED_BY_CM *cs2)
{
    int N = cs1->N;
    int LEN = cs1->LEN;
    int TOTAL_WEIGHT = 0;

    double similarity = 0;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < LEN; j++)
        {
            bool merged = cs1->merge_bit_8_to_16[i][j / 2];
            // merged = false; 

            if (merged)
            {
                assert(j % 2 == 0);

                int s1 = (cs1->merge_bit_8_to_16[i][j / 2] ? cs1->counter_16bits[i][j / 2] : cs1->counter_8bits[i][j] + cs1->counter_8bits[i][j + 1]);
                int s2 = (cs2->merge_bit_8_to_16[i][j / 2] ? cs2->counter_16bits[i][j / 2] : cs2->counter_8bits[i][j] + cs2->counter_8bits[i][j + 1]);
                if (s1 > 32767 || s1 < -32768)
                    cs1->overflow_32767++;
                if (s2 > 32767 || s2 < -32768)
                    cs2->overflow_32767++;
                // s1 = std::max(s1, -32768);
                // s1 = std::min(s1, 32767);
                // s2 = std::max(s2, -32768);
                // s2 = std::min(s2, 32767);

                int weight1 = (cs1->merge_bit_8_to_16[i][j / 2] ? cs1->cm_weighter_16bits[i][j / 2] : cs1->cm_weighter_8bits[i][j] + cs1->cm_weighter_8bits[i][j + 1]);
                int weight2 = (cs2->merge_bit_8_to_16[i][j / 2] ? cs2->cm_weighter_16bits[i][j / 2] : cs2->cm_weighter_8bits[i][j] + cs2->cm_weighter_8bits[i][j + 1]);
                int weight = std::max(weight1, weight2);
                // weight = std::min(weight, 32767);
                // weight = std::max(weight, -32768);

                if ((s1 > 0 && s2 > 0) || (s1 < 0 && s2 < 0))
                {
                    similarity += (double)std::min(abs(s1), abs(s2)) / std::max(abs(s1), abs(s2)) * weight;
                    TOTAL_WEIGHT += weight; 
                }
                else if (s1 * s2 <= 0 && (s1 != 0 || s2 != 0))
                {
                    TOTAL_WEIGHT += weight; 
                }

                j++; 
            }
            else
            {
                int s1 = cs1->counter_8bits[i][j];
                int s2 = cs2->counter_8bits[i][j];
                s1 = std::max(s1, -128);
                s1 = std::min(s1, 127);
                s2 = std::max(s2, -128);
                s2 = std::min(s2, 127);

                int weight = std::max(cs1->cm_weighter_8bits[i][j], cs2->cm_weighter_8bits[i][j]);

                if ((s1 > 0 && s2 > 0) || (s1 < 0 && s2 < 0))
                {
                    similarity += (double)std::min(abs(s1), abs(s2)) / std::max(abs(s1), abs(s2)) * weight;
                    TOTAL_WEIGHT += weight; 
                }
                else if (s1 * s2 <= 0 && (s1 != 0 || s2 != 0))
                {
                    TOTAL_WEIGHT += weight; 
                }
            }
        }
    }
    similarity /= TOTAL_WEIGHT;
    // LOG_DEBUG("cs1 overflow_127: %d, cs2 overflow_127: %d", cs1->overflow_127, cs2->overflow_127);
    // LOG_DEBUG("cs1 overflow_32767: %d, cs2 overflow_32767: %d", cs1->overflow_32767, cs2->overflow_32767);
    return similarity;
}

double kl_divergence_of_distribution_cs(COUNT_SKETCH *s1, COUNT_SKETCH *s2)
{
    std::vector<int> A, B;
    int num = 0, num1 = 0, num2 = 0;
    for (int i = 0; i < s1->N_ARRAY; i++)
    {
        for (int j = 0; j < s1->LEN; j++)
        {
            if (s1->nt[i][j] != 0)
            {
                num1++;
                A.push_back(abs(s1->nt[i][j]));
            }
            if (s2->nt[i][j] != 0)
            {
                num2++;
                B.push_back(abs(s2->nt[i][j]));
            }
        }
    }
    num = std::min(num1, num2);

    sort(A.begin(), A.end(), [](int a, int b)
         { return a > b; });
    sort(B.begin(), B.end(), [](int a, int b)
         { return a > b; });
    double kl = 0;
    double all1 = 0, all2 = 0;
    for (int i = 0; i < num; i++)
    {
        all1 += A[i];
        all2 += B[i];
    }
    for (int i = 0; i < num; i++)
    {
        kl += double(A[i]) / all1 * log((double(A[i]) / all1) / (double(B[i]) / all2)) / log(2);
        kl += double(B[i]) / all2 * log((double(B[i]) / all2) / (double(A[i]) / all1)) / log(2);
    }
    return kl / 2;
}

double kl_divergence_distribution_cs_weighted_by_cm(COUNT_SKETCH_WEIGHTED_BY_CM *s1, COUNT_SKETCH_WEIGHTED_BY_CM *s2)
{
    std::vector<int> A, B;
    int num = 0, num1 = 0, num2 = 0;
    for (int i = 0; i < s1->N_ARRAY; i++)
    {
        for (int j = 0; j < s1->LEN; j++)
        {
            if (s1->cm_weighter[i][j] != 0)
            {
                num1++;
                A.push_back(abs(s1->cm_weighter[i][j]));
            }
            if (s2->cm_weighter[i][j] != 0)
            {
                num2++;
                B.push_back(abs(s2->cm_weighter[i][j]));
            }
        }
    }
    num = std::min(num1, num2);

    sort(A.begin(), A.end(), [](int a, int b)
         { return a > b; });
    sort(B.begin(), B.end(), [](int a, int b)
         { return a > b; });

    printf("A max: %d, B max: %d\n", A[0], B[0]); 

    double kl = 0;
    double all1 = 0, all2 = 0;
    for (int i = 0; i < num; i++)
    {
        all1 += A[i];
        all2 += B[i];
    }
    for (int i = 0; i < num; i++)
    {
        kl += double(A[i]) / all1 * log((double(A[i]) / all1) / (double(B[i]) / all2)) / log(2);
        kl += double(B[i]) / all2 * log((double(B[i]) / all2) / (double(A[i]) / all1)) / log(2);
    }
    return kl / 2;
}