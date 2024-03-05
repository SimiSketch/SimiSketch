#pragma once

#include "defs.h"
#include "hash.h"
#include <vector>
#include <map>
#include <fstream>

class Elastic
{
public:
    int WID;
    int NSLOT;
    int THRESHOLD;

    struct slot_t
    {
        data_t item;
        count_t cnt;
    };

    slot_t **slots;
    count_t *votes;
    seed_t seed;

    Elastic(int wid, int nslot, int threshold)
        : WID(wid), NSLOT(nslot), THRESHOLD(threshold)
    {
        slots = new slot_t *[WID];
        for (int i = 0; i < WID; i++)
        {
            slots[i] = new slot_t[NSLOT];
            memset(slots[i], 0, sizeof(slot_t) * NSLOT);
        }
        votes = new count_t[WID];
        memset(votes, 0, sizeof(count_t) * WID);
        srand(clock());
        seed = rand();
    }

    void insert(data_t item)
    {
        int pos = HASH::hash(item, seed) % WID;
        int ndx = -1, mn = INT32_MAX;
        for (int i = 0; i < NSLOT; i++)
        {
            if (slots[pos][i].item == item)
            {
                slots[pos][i].cnt++;
                return;
            }

            if (slots[pos][i].cnt < mn)
            {
                mn = slots[pos][i].cnt;
                ndx = i;
            }
        }

        // Not found
        if (mn == 0) // empty slot
        {
            slots[pos][ndx].item = item;
            slots[pos][ndx].cnt = 1;
            return;
        }

        votes[pos]++;
        if (votes[pos] > THRESHOLD * mn)
        {
            slots[pos][ndx].item = item;
            slots[pos][ndx].cnt = 1;
            votes[pos] = 0;
        }
    }

    void print_info()
    {
        LOG_INFO("Elastic total counters: %d w int", (WID * NSLOT * 3 + WID) / 10000);
    }

    std::map<data_t, count_t> GetItems()
    {
        std::map<data_t, count_t> rst;
        for (int i = 0; i < WID; i++)
        {
            for (int j = 0; j < NSLOT; j++)
            {
                if (slots[i][j].cnt > 0)
                    assert(rst.insert(std::make_pair(slots[i][j].item, slots[i][j].cnt)).second);
            }
        }

        return rst;
    }
};

double similarity(Elastic s1, Elastic s2)
{
    LOG_INFO("Calculate Jaccard similarity using Elastic:");

    auto giant1 = s1.GetItems();
    auto giant2 = s2.GetItems();
    int giant_intersection = 0;
    int giant_union = 0;

    for (const auto &t : giant1)
    {
        giant_union += t.second;
        auto it = giant2.find(t.first);
        if (it != giant2.end())
            giant_intersection += std::min(t.second, it->second);
    }
    for (const auto &t : giant2)
    {
        giant_union += t.second;
    }

    giant_union -= giant_intersection;

    // LOG_RESULT("Intersect: %d, union: %d, Jaccard Similarity: %lf", giant_intersection, giant_union, double(giant_intersection) / giant_union);
    return double(giant_intersection) / giant_union;
}

double Cosine_sim_2_streams(Elastic s1, Elastic s2)
{
    LOG_INFO("Calculate Cosine similarity using Elastic:");
    uint64_t l1 = 0, l2 = 0;
    uint64_t dot_2 = 0;
    auto giant1 = s1.GetItems();
    auto giant2 = s2.GetItems();
    for (const auto &t : giant1)
    {
        l1 = l1 + uint64_t(t.second) * uint64_t(t.second);
    }
    for (const auto &t : giant2)
    {
        l2 = l2 + uint64_t(t.second) * uint64_t(t.second);
    }
    for (const auto &t : giant1)
    {
        auto it = giant2.find(t.first);
        if (it != giant2.end())
        {
            dot_2 = dot_2 + uint64_t(t.second) * uint64_t(it->second);
        }
    }
    // printf("the giant part for stream1 l1:%llu,l2:%llu,dot_2:%llu\n",l1,l2,dot_2);
    double res = double(dot_2) / (sqrt(l1) * sqrt(l2));
    LOG_RESULT("Cosine Similarity: %lf", res);
    return res;
}

double KL_sim_2_streams(Elastic s1, Elastic s2)
{
    LOG_INFO("Calculate KL similarity using Elastic:");
    auto giant1 = s1.GetItems();
    auto giant2 = s2.GetItems();
    uint64_t sum1 = 0, sum2 = 0;
    std::vector<uint64_t> e1, e2;
    for (auto t : giant1)
    {
        sum1 = sum1 + t.second + 1;
        e1.push_back(t.second + 1);
        auto it = giant2.find(t.first);
        if (it != giant2.end())
        {
            e2.push_back(it->second + 1);
            sum2 = sum2 + it->second + 1;
        }
        else
        {
            e2.push_back(1);
            sum2++;
        }
    }
    for (auto t : giant2)
    {
        auto it = giant1.find(t.first);
        if (it == giant1.end())
        {
            e2.push_back(t.second + 1);
            e1.push_back(1);
            sum2 = sum2 + t.second + 1;
            sum1++;
        }
    }
    assert(e1.size() == e2.size());
    printf("the size of elastic kl is:%lu\n", e1.size());
    double kl = 0, kl12 = 0, kl21 = 0;
    for (int i = 0; i < e1.size(); i++)
    {
        double x1 = double(e1[i]) / sum1;
        double x2 = double(e2[i]) / sum2;
        kl12 = kl12 + x1 * log2(x1 / x2);
        kl21 = kl21 + x2 * log2(x2 / x1);
    }
    kl = (kl12 + kl21) / 2;
    LOG_RESULT("The KL Similarity 1->2: %lf, 2->1: %lf, average: %lf", kl12, kl21, kl);
    return kl;
}

double CE_sim_2_streams(Elastic s1, Elastic s2)
{
    LOG_INFO("Calculate KL similarity using Elastic:");
    auto giant1 = s1.GetItems();
    auto giant2 = s2.GetItems();
    uint64_t sum1 = 0, sum2 = 0;
    std::vector<uint64_t> e1, e2;
    for (auto t : giant1)
    {
        sum1 = sum1 + t.second + 1;
        e1.push_back(t.second + 1);
        auto it = giant2.find(t.first);
        if (it != giant2.end())
        {
            e2.push_back(it->second + 1);
            sum2 = sum2 + it->second + 1;
        }
        else
        {
            e2.push_back(1);
            sum2++;
        }
    }
    for (auto t : giant2)
    {
        auto it = giant1.find(t.first);
        if (it == giant1.end())
        {
            e2.push_back(t.second + 1);
            e1.push_back(1);
            sum2 = sum2 + t.second + 1;
            sum1++;
        }
    }
    assert(e1.size() == e2.size());
    printf("the size of elastic kl is:%lu\n", e1.size());
    double kl = 0, kl12 = 0, kl21 = 0;
    for (int i = 0; i < e1.size(); i++)
    {
        double x1 = double(e1[i]) / sum1;
        double x2 = double(e2[i]) / sum2;
        kl12 = kl12 - x1 * log2(x2);
        kl21 = kl21 - x2 * log2(x1);
    }
    kl = (kl12 + kl21) / 2;
    LOG_RESULT("The CE Similarity 1->2: %lf, 2->1: %lf, average: %lf", kl12, kl21, kl);
    return kl;
}

double kl_divergence_of_distribution_elastic(Elastic s1, Elastic s2)
{
    std::vector<int> A, B;
    int num1 = 0, num2 = 0;
    auto giant1 = s1.GetItems();
    auto giant2 = s2.GetItems();
    for (auto t : giant1)
    {
        A.push_back(t.second);
        num1++;
    }
    for (auto t : giant2)
    {
        B.push_back(t.second);
        num2++;
    }
    int num = std::min(num1, num2);
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