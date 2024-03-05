#pragma once
#include "hash.h"
#include <vector>
#include <map>

class Elastic_with_cm
{
public:
    int WID;
    int NSLOT;
    int CMLEN;
    int THRESHOLD;

    struct slot_t
    {
        data_t item;
        count_t cnt;
    };

    slot_t **slots1;
    count_t *votes1;
    seed_t seed;

    seed_t cm_seed;
    count_t *cm1;

    slot_t **slots2;
    count_t *votes2;

    count_t *cm2;

    Elastic_with_cm(int wid, int nslot, int cm_len, int threshold)
        : WID(wid), NSLOT(nslot), CMLEN(cm_len), THRESHOLD(threshold)
    {
        slots1 = new slot_t *[WID];
        slots2 = new slot_t *[WID];
        for (int i = 0; i < WID; i++)
        {
            slots1[i] = new slot_t[NSLOT];
            slots2[i] = new slot_t[NSLOT];
            memset(slots1[i], 0, sizeof(slot_t) * NSLOT);
            memset(slots2[i], 0, sizeof(slot_t) * NSLOT);
        }
        votes1 = new count_t[WID];
        votes2 = new count_t[WID];
        memset(votes1, 0, sizeof(count_t) * WID);
        memset(votes2, 0, sizeof(count_t) * WID);
        srand(clock());
        seed = rand();

        cm_seed = rand();
        cm1 = new count_t[CMLEN];
        memset(cm1, 0, sizeof(count_t) * CMLEN);
        cm2 = new count_t[CMLEN];
        memset(cm2, 0, sizeof(count_t) * CMLEN);
    }

    void insert1(data_t item)
    {
        int pos = HASH::hash(item, seed) % WID;
        int ndx = -1, mn = INT32_MAX;
        for (int i = 0; i < NSLOT; i++)
        {
            if (slots1[pos][i].item == item)
            {
                slots1[pos][i].cnt++;
                return;
            }

            if (slots1[pos][i].cnt < mn)
            {
                mn = slots1[pos][i].cnt;
                ndx = i;
            }
        }

        // Not found
        if (mn == 0) // empty slot
        {
            slots1[pos][ndx].item = item;
            slots1[pos][ndx].cnt = 1;
            return;
        }

        votes1[pos]++;
        if (votes1[pos] > THRESHOLD * mn)
        {
            int index = HASH::hash(slots1[pos][ndx].item, cm_seed) % CMLEN;
            int cnt = slots1[pos][ndx].cnt;
            cm1[index] += cnt;
            slots1[pos][ndx].item = item;
            slots1[pos][ndx].cnt = 1;
            votes1[pos] = 0;
        }
        else
        {
            int index = HASH::hash(item, cm_seed) % CMLEN;
            cm1[index]++;
        }
    }

    void insert2(data_t item)
    {
        int pos = HASH::hash(item, seed) % WID;
        int ndx = -1, mn = INT32_MAX;
        for (int i = 0; i < NSLOT; i++)
        {
            if (slots2[pos][i].item == item)
            {
                slots2[pos][i].cnt++;
                return;
            }

            if (slots2[pos][i].cnt < mn)
            {
                mn = slots2[pos][i].cnt;
                ndx = i;
            }
        }

        // Not found
        if (mn == 0) // empty slot
        {
            slots2[pos][ndx].item = item;
            slots2[pos][ndx].cnt = 1;
            return;
        }

        votes2[pos]++;
        if (votes2[pos] > THRESHOLD * mn)
        {
            int index = HASH::hash(slots2[pos][ndx].item, cm_seed) % CMLEN;
            int cnt = slots2[pos][ndx].cnt;
            cm2[index] += cnt;
            slots2[pos][ndx].item = item;
            slots2[pos][ndx].cnt = 1;
            votes2[pos] = 0;
        }
        else
        {
            int index = HASH::hash(item, cm_seed) % CMLEN;
            cm2[index]++;
        }
    }

    std::map<data_t, count_t> GetItems1()
    {
        std::map<data_t, count_t> rst;
        for (int i = 0; i < WID; i++)
        {
            for (int j = 0; j < NSLOT; j++)
            {
                if (slots1[i][j].cnt > 0)
                    assert(rst.insert(std::make_pair(slots1[i][j].item, slots1[i][j].cnt)).second);
            }
        }

        return rst;
    }

    std::map<data_t, count_t> GetItems2()
    {
        std::map<data_t, count_t> rst;
        for (int i = 0; i < WID; i++)
        {
            for (int j = 0; j < NSLOT; j++)
            {
                if (slots2[i][j].cnt > 0)
                    assert(rst.insert(std::make_pair(slots2[i][j].item, slots2[i][j].cnt)).second);
            }
        }

        return rst;
    }

    double similarity()
    {
        uint64_t intersection = 0;
        uint64_t u = 0;
        for (int i = 0; i < WID; i++)
        {
            for (int j = 0; j < NSLOT; j++)
            {
                if (slots1[i][j].item == slots2[i][j].item)
                {
                    intersection += std::min(slots1[i][j].cnt, slots2[i][j].cnt);
                    u += std::max(slots1[i][j].cnt, slots2[i][j].cnt);
                }
                else
                {
                    u = u + slots1[i][j].cnt + slots2[i][j].cnt;
                }
            }
        }

        for (int i = 0; i < CMLEN; i++)
        {
            intersection += std::min(cm1[i], cm2[i]);
            u += std::max(cm1[i], cm2[i]);
        }

        double res = double(intersection) / u;
        return res;
    }

    double similarity_map()
    {
        // LOG_INFO("Calculate Jaccard similarity using Elastic:");
        auto giant1 = GetItems1();
        auto giant2 = GetItems2();
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

        uint64_t intersection = 0;
        uint64_t union_n = 0;
        for (int i = 0; i < CMLEN; i++)
        {
            intersection += std::min(cm1[i], cm2[i]);
            union_n += std::max(cm1[i], cm2[i]);
        }

        // LOG_RESULT("Intersect: %d, union: %d, Jaccard Similarity: %lf", giant_intersection, giant_union, double(giant_intersection) / giant_union);
        return double(giant_intersection + intersection) / (giant_union + union_n);
    }

    double cosine_similarity()
    {
        uint64_t l1 = 0, l2 = 0, l = 0;
        for (int i = 0; i < WID; i++)
        {
            for (int j = 0; j < NSLOT; j++)
            {
                l1 = l1 + (uint64_t)slots1[i][j].cnt * (uint64_t)slots1[i][j].cnt;
                l2 = l2 + (uint64_t)slots2[i][j].cnt * (uint64_t)slots2[i][j].cnt;
                if (slots1[i][j].item == slots2[i][j].item)
                    l = l + (uint64_t)slots1[i][j].cnt * (uint64_t)slots2[i][j].cnt;
            }
        }
        for (int i = 0; i < CMLEN; i++)
        {
            l1 = l1 + (uint64_t)cm1[i] * (uint64_t)cm1[i];
            l2 = l2 + (uint64_t)cm2[i] * (uint64_t)cm2[i];
            l = l + (uint64_t)cm1[i] * (uint64_t)cm2[i];
        }
        return double(l) / (sqrt(l1) * sqrt(l2));
    }
};
