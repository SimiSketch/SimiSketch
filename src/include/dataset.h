#pragma once
#ifndef __DATASET_H__

#define __DATASET_H__
#include <map>
#include <unordered_map>
#include <string>
#include <iostream>

#include <math.h>
#include <vector>
#include <fstream>
#include <algorithm> // std::random_shuffle

#include <vector> // std::vector
#include <ctime>  // std::time
#include <random> // std::default_random_engine
#include <chrono>

#include "defs.h"
#include "util.h"
#include "logger.h"
#include "streamgen.h"

using namespace std;

class Stream
{
public:
    count_t TOTAL_PACKETS;
    count_t TOTAL_FLOWS;
    data_t *raw_data = NULL;
    unordered_map<data_t, count_t> counter;

    Stream() = default;

    ~Stream()
    {
        if (raw_data)
            delete[] raw_data;
    }
};

class Dataset
{
public:
    count_t TOTAL_PACKETS;
    count_t TOTAL_PACKETS1, TOTAL_PACKETS2;
    data_t *raw_data = NULL;
    data_t *raw_data1 = NULL, *raw_data2 = NULL;
    Stream stream1, stream2;
    unordered_map<data_t, count_t> counter;
    unordered_map<data_t, count_t> counter1, counter2;

    void init(string PATH, int size_per_item, bool separate = true, bool use_known_zipf = true, double zipf_alpha = 1.0)
    {
        if (!use_known_zipf)
        {
            StreamGen streamgen;
            streamgen.init(zipf_alpha, 200000, 32000000);
            LOG_DEBUG("streamgen.TOTAL_PACKETS=%d, streamgen.TOTAL_FLOWS=%d", streamgen.TOTAL_PACKETS, streamgen.TOTAL_FLOWS);
            std::shuffle(streamgen.raw_data, streamgen.raw_data + streamgen.TOTAL_PACKETS, std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count()));
            // for (int i = 0; i < 10; i++)
            //     LOG_DEBUG("streamgen.raw_data[%d]=%d", i * 10000, streamgen.raw_data[i * 10000]);
            
            stream1.TOTAL_PACKETS = streamgen.TOTAL_PACKETS / 2;
            stream1.raw_data = new data_t[stream1.TOTAL_PACKETS];
            for (count_t i = 0; i < stream1.TOTAL_PACKETS; i++)
            {
                stream1.raw_data[i] = streamgen.raw_data[i];
                stream1.counter[stream1.raw_data[i]]++;
            }
            stream1.TOTAL_FLOWS = stream1.counter.size();
            LOG_DEBUG("[STREAM #1]Total packets: %d, Total flows: %d", stream1.TOTAL_PACKETS, stream1.TOTAL_FLOWS);

            stream2.TOTAL_PACKETS = streamgen.TOTAL_PACKETS - stream1.TOTAL_PACKETS;
            stream2.raw_data = new data_t[stream2.TOTAL_PACKETS];
            for (count_t i = 0; i < stream2.TOTAL_PACKETS; i++)
            {
                stream2.raw_data[i] = streamgen.raw_data[stream1.TOTAL_PACKETS + i];
                stream2.counter[stream2.raw_data[i]]++;
            }
            stream2.TOTAL_FLOWS = stream2.counter.size();
            LOG_DEBUG("[STREAM #2]Total packets: %d, Total flows: %d", stream2.TOTAL_PACKETS, stream2.TOTAL_FLOWS);
            LOG_DEBUG("Dataset initialized.");
            return;
        }
        if (!separate)
        {
            LOG_DEBUG("into Dataset::init()");
            string path1 = "../dataset/130000.dat";
            string path2 = "../dataset/130100.dat";
            int fd1 = Open(path1.c_str(), O_RDONLY);
            int fd2 = Open(path2.c_str(), O_RDONLY);
            struct stat buf1, buf2;
            fstat(fd1, &buf1);
            fstat(fd2, &buf2);
            int n_elements1 = buf1.st_size / size_per_item;
            int n_elements2 = buf2.st_size / size_per_item;
            TOTAL_PACKETS1 = n_elements1;
            TOTAL_PACKETS2 = n_elements2;
            LOG_DEBUG("TOTAL_PACKETS1=%d, TOTAL_PACKETS2=%d", n_elements1, n_elements2);
            LOG_DEBUG("Mmap...");
            void *addr1 = mmap(NULL, buf1.st_size, PROT_READ, MAP_PRIVATE, fd1, 0);
            void *addr2 = mmap(NULL, buf2.st_size, PROT_READ, MAP_PRIVATE, fd2, 0);
            raw_data1 = new data_t[n_elements1];
            raw_data2 = new data_t[n_elements2];
            close(fd1);
            close(fd2);
            if (addr1 == MAP_FAILED || addr2 == MAP_FAILED)
            {
                LOG_ERROR("MMAP FAILED!");
                exit(-1);
            }
            char *ptr1 = reinterpret_cast<char *>(addr1);
            char *ptr2 = reinterpret_cast<char *>(addr2);
            for (int i = 0; i < n_elements1; i++)
            {
                raw_data1[i] = *reinterpret_cast<data_t *>(ptr1);
                ptr1 += size_per_item;
            }
            for (int i = 0; i < n_elements2; i++)
            {
                raw_data2[i] = *reinterpret_cast<data_t *>(ptr2);
                ptr2 += size_per_item;
            }
            munmap(addr1, buf1.st_size);
            munmap(addr2, buf2.st_size);
            for (count_t i = 0; i < n_elements1; i++)
            {
                counter1[raw_data1[i]]++;
            }
            for (count_t i = 0; i < n_elements2; i++)
            {
                counter2[raw_data2[i]]++;
            }
            LOG_DEBUG("Total packets1: %d, Total flows1: %zu", n_elements1, counter1.size());
            LOG_DEBUG("Total packets2: %d, Total flows2: %zu", n_elements2, counter2.size());

            stream1.TOTAL_PACKETS = n_elements1;
            stream1.raw_data = new data_t[stream1.TOTAL_PACKETS];
            for (count_t i = 0; i < stream1.TOTAL_PACKETS; i++)
            {
                stream1.raw_data[i] = raw_data1[i];
                stream1.counter[stream1.raw_data[i]]++;
            }
            stream1.TOTAL_FLOWS = stream1.counter.size();
            LOG_DEBUG("[STREAM #1]Total packets: %d, Total flows: %d", stream1.TOTAL_PACKETS, stream1.TOTAL_FLOWS);

            stream2.TOTAL_PACKETS = n_elements2;
            stream2.raw_data = new data_t[stream2.TOTAL_PACKETS];
            for (count_t i = 0; i < stream2.TOTAL_PACKETS; i++)
            {
                stream2.raw_data[i] = raw_data2[i];
                stream2.counter[stream2.raw_data[i]]++;
            }
            stream2.TOTAL_FLOWS = stream2.counter.size();
            LOG_DEBUG("[STREAM #2]Total packets: %d, Total flows: %d", stream2.TOTAL_PACKETS, stream2.TOTAL_FLOWS);
            LOG_DEBUG("Dataset initialized.");
            LOG_DEBUG("exit Dataset::init()");
            return;
        }
        LOG_DEBUG("into Dataset::init()");
        struct stat buf;
        LOG_DEBUG("Opening file %s", PATH.c_str());
        int fd = Open(PATH.c_str(), O_RDONLY);
        fstat(fd, &buf);
        int n_elements = buf.st_size / size_per_item;
        TOTAL_PACKETS = n_elements;
        LOG_DEBUG("TOTAL_PACKETS=%d", n_elements);
        LOG_DEBUG("Mmap...");
        void *addr = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        raw_data = new data_t[n_elements];
        close(fd);
        if (addr == MAP_FAILED)
        {
            LOG_ERROR("MMAP FAILED!");
            exit(-1);
        }

        char *ptr = reinterpret_cast<char *>(addr);
        for (int i = 0; i < n_elements; i++)
        {
            raw_data[i] = *reinterpret_cast<data_t *>(ptr); 
            ptr += size_per_item;
        }
        munmap(addr, buf.st_size);
        for (count_t i = 0; i < n_elements; i++)
        {
            counter[raw_data[i]]++;
        }
        LOG_DEBUG("Total packets: %d, Total flows: %zu", n_elements, counter.size());

        stream1.TOTAL_PACKETS = n_elements / 2;
        stream1.raw_data = new data_t[stream1.TOTAL_PACKETS];
        for (count_t i = 0; i < stream1.TOTAL_PACKETS; i++)
        {
            stream1.raw_data[i] = raw_data[i];
            stream1.counter[stream1.raw_data[i]]++;
        }
        stream1.TOTAL_FLOWS = stream1.counter.size();
        LOG_DEBUG("[STREAM #1]Total packets: %d, Total flows: %d", stream1.TOTAL_PACKETS, stream1.TOTAL_FLOWS);

        stream2.TOTAL_PACKETS = n_elements - stream1.TOTAL_PACKETS;
        stream2.raw_data = new data_t[stream2.TOTAL_PACKETS];
        for (count_t i = 0; i < stream2.TOTAL_PACKETS; i++)
        {
            stream2.raw_data[i] = raw_data[n_elements / 2 + i];
            stream2.counter[stream2.raw_data[i]]++;
        }
        stream2.TOTAL_FLOWS = stream2.counter.size();
        LOG_DEBUG("[STREAM #2]Total packets: %d, Total flows: %d", stream2.TOTAL_PACKETS, stream2.TOTAL_FLOWS);
        LOG_DEBUG("Dataset initialized.");
        LOG_DEBUG("exit Dataset::init()");
        return;
    }

    double similarity()
    {
        count_t N_intersect = 0;
        count_t N_union = 0;
        for (auto t : stream1.counter)
        {
            auto it = stream2.counter.find(t.first);
            if (it != stream2.counter.end())
            {
                if (t.second > it->second)
                {
                    N_union += t.second - it->second;
                    N_intersect += it->second;
                }
                else
                {
                    N_intersect += t.second;
                }
            }
            else
            {
                N_union += t.second;
            }
        }

        for (auto t : stream2.counter)
        {
            N_union += t.second;
        }
        LOG_RESULT("Real jaccard similarity: %lf\n", double(N_intersect) / N_union);
        return double(N_intersect) / N_union;
    }

    double cosine_similarity()
    {
        uint64_t numerator = 0, denominator_1 = 0, denominator_2 = 0;
        for (auto t : stream1.counter)
        {
            denominator_1 += uint64_t(t.second) * t.second;
            auto it = stream2.counter.find(t.first);
            if (it != stream2.counter.end())
            {
                numerator += uint64_t(t.second) * it->second;
            }
        }
        for (auto t : stream2.counter)
        {
            denominator_2 += uint64_t(t.second) * t.second;
        }

        // printf("numerator: %llu, denominator_1: %llu, denominator_2: %llu\n", numerator, denominator_1, denominator_2);
        return double(numerator) / (sqrt(denominator_1) * sqrt(denominator_2));
    }

    double cross_entropy()
    {
        double ans = 0;
        for (auto t : stream1.counter)
        {
            auto it = stream2.counter.find(t.first);
            if (it != stream2.counter.end()) 
            {
                ans -= double(t.second) / stream1.TOTAL_PACKETS * log(double(it->second) / stream2.TOTAL_PACKETS) / log(2);
            }
            else 
            {
                ans -= double(t.second) / stream1.TOTAL_PACKETS * log(1. / stream2.TOTAL_FLOWS) / log(2);
            }
        }
        for (auto t : stream2.counter)
        {
            auto it = stream1.counter.find(t.first);
            if (it != stream1.counter.end())
            {
                ans -= double(t.second) / stream2.TOTAL_PACKETS * log(double(it->second) / stream1.TOTAL_PACKETS) / log(2);
            }
            else
            {
                ans -= double(t.second) / stream2.TOTAL_PACKETS * log(1. / stream1.TOTAL_FLOWS) / log(2);
            }
        }
        return ans / 2;
    }

    double kl_divergence()
    {
        double ans = 0;
        for (auto t : stream1.counter)
        {
            auto it = stream2.counter.find(t.first);
            if (it != stream2.counter.end())
            {
                ans += double(t.second) / stream1.TOTAL_PACKETS * log((double(t.second) / stream1.TOTAL_PACKETS) / (double(it->second) / stream2.TOTAL_PACKETS)) / log(2);
            }
            else
            {
                ans += double(t.second) / stream1.TOTAL_PACKETS * log((double(t.second) / stream1.TOTAL_PACKETS) / (1. / stream2.TOTAL_FLOWS)) / log(2);
            }
        }
        for (auto t : stream2.counter)
        {
            auto it = stream1.counter.find(t.first);
            if (it != stream1.counter.end())
            {
                ans += double(t.second) / stream2.TOTAL_PACKETS * log((double(t.second) / stream2.TOTAL_PACKETS) / (double(it->second) / stream1.TOTAL_PACKETS)) / log(2);
            }
            else
            {
                ans += double(t.second) / stream2.TOTAL_PACKETS * log((double(t.second) / stream2.TOTAL_PACKETS) / (1. / stream1.TOTAL_FLOWS)) / log(2);
            }
        }
        return ans / 2;
    }

    ~Dataset()
    {
        // LOG_DEBUG("into ~Dataset()");
        if (raw_data)
        {
            // LOG_DEBUG("delete[] raw_data");
            delete[] raw_data;
        }
    }
};

double jaccard_similarity_2_dataset(Dataset *dataset1, Dataset *dataset2)
{
    count_t N_intersect = 0;
    count_t N_union = 0;
    for (auto t : dataset1->counter)
    {
        auto it = dataset2->counter.find(t.first);
        if (it != dataset2->counter.end())
        {
            if (t.second > it->second)
            {
                N_union += t.second - it->second;
                N_intersect += it->second;
            }
            else
            {
                N_intersect += t.second;
            }
        }
        else
        {
            N_union += t.second;
        }
    }

    for (auto t : dataset2->counter)
    {
        N_union += t.second;
    }
    LOG_RESULT("Real jaccard similarity: %lf\n", double(N_intersect) / N_union);
    return double(N_intersect) / N_union;
}

double kl_divergence_2_dataset(Dataset *dataset1, Dataset *dataset2)
{
    vector<int> A, B;
    int num1 = 0, num2 = 0;
    for (auto t : dataset1->counter)
    {
        A.push_back(t.second);
        num1++;
    }
    for (auto t : dataset2->counter)
    {
        B.push_back(t.second);
        num2++;
    }

    int num = min(num1, num2);
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

    LOG_RESULT("Real kl divergence: %lf\n", kl / 2);
    return kl / 2;
}

class Dataset_BoW
{
public:
    count_t TOTAL_PACKETS; 
    count_t TOTAL_FLOWS;  
    uint32_t *raw_data = NULL;

    void init(string PATH, int total_packets, int startline = 3)
    {
        LOG_DEBUG("into Dataset_BoW()");
        LOG_DEBUG("Opening file %s", PATH.c_str());
        TOTAL_PACKETS = total_packets;
        TOTAL_FLOWS = 0;
        raw_data = new uint32_t[TOTAL_PACKETS];
        FILE *file = fopen(PATH.c_str(), "r");
        if (!file)
        {
            cout << "Error opening file." << endl;
            return;
        }

        int lineCount = 0;
        while (lineCount < startline)
        {
            char ch = fgetc(file);
            if (ch == '\n')
            {
                lineCount++;
            }
        }

        int packet_cnt = 0, flow_cnt = 0;
        while (packet_cnt < TOTAL_PACKETS && !feof(file))
        {
            uint32_t wordNum;
            int word_cnt;
            fscanf(file, "%*d %u %u\n", &wordNum, &word_cnt);
            raw_data[packet_cnt] = wordNum;
            packet_cnt++;
            flow_cnt++;
            while (word_cnt > 1) 
            {
                raw_data[packet_cnt] = wordNum;
                packet_cnt++;
                word_cnt--;
                if (packet_cnt >= TOTAL_PACKETS)
                    break;
            }
        }
        TOTAL_FLOWS = flow_cnt;
        fclose(file);

        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(raw_data, raw_data + TOTAL_PACKETS, std::default_random_engine(seed));

        LOG_DEBUG("Total packets: %d, Total flows: %d", TOTAL_PACKETS, TOTAL_FLOWS);
        LOG_DEBUG("%u, %u ,%u, %u, %u, %u, %u, %u, %u, %u", raw_data[0], raw_data[1], raw_data[2], raw_data[3], raw_data[4], raw_data[5], raw_data[6], raw_data[7], raw_data[8], raw_data[9]);
        LOG_DEBUG("%u, %u ,%u, %u, %u, %u, %u, %u, %u, %u", raw_data[TOTAL_PACKETS - 10], raw_data[TOTAL_PACKETS - 9], raw_data[TOTAL_PACKETS - 8], raw_data[TOTAL_PACKETS - 7], raw_data[TOTAL_PACKETS - 6], raw_data[TOTAL_PACKETS - 5], raw_data[TOTAL_PACKETS - 4], raw_data[TOTAL_PACKETS - 3], raw_data[TOTAL_PACKETS - 2], raw_data[TOTAL_PACKETS - 1]);
        LOG_DEBUG("exit Dataset_BoW()");
        printf("\n");
    }

    ~Dataset_BoW()
    {
        // LOG_DEBUG("into ~Dataset_BoW()");
        if (raw_data)
        {
            // LOG_DEBUG("delete[] raw_data");
            delete[] raw_data;
        }
    }
};

double jaccard_similarity_2_dataset_BoW(Dataset_BoW *dataset1, Dataset_BoW *dataset2)
{
    LOG_DEBUG("into jaccard_similarity_2_dataset_BoW()");
    std::map<uint32_t, count_t> counter1, counter2;
    for (count_t i = 0; i < dataset1->TOTAL_PACKETS; i++)
    {
        counter1[dataset1->raw_data[i]]++;
    }
    for (count_t i = 0; i < dataset2->TOTAL_PACKETS; i++)
    {
        counter2[dataset2->raw_data[i]]++;
    }

    count_t N_intersect = 0;
    count_t N_union = 0;
    for (auto t : counter1)
    {
        auto it = counter2.find(t.first);
        if (it != counter2.end())
        {
            if (t.second > it->second)
            {
                N_union += t.second - it->second;
                N_intersect += it->second;
            }
            else
            {
                N_intersect += t.second;
            }
        }
        else
        {
            N_union += t.second;
        }
    }

    for (auto t : counter2)
    {
        N_union += t.second;
    }
    LOG_DEBUG("N_intersect: %d, N_union: %d", N_intersect, N_union);
    LOG_RESULT("Real jaccard similarity: %lf", double(N_intersect) / N_union);
    LOG_DEBUG("exit jaccard_similarity_2_dataset_BoW()\n");
    return double(N_intersect) / N_union;
}
#endif
