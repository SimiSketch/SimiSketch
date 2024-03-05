#pragma once
#include "minhash.h"
#include "dataset.h"
#include <fstream>

double distribution_minhash_cm(int loop_time, int line_cnt, int len, int hash_cnt, string dataset_name = "caida", int nytimes_dataset_len = 800000, int second_dataset_start = 800003, std::ofstream *fout_time = NULL)
{
    LOG_DEBUG("enter distribution_minhash()");
    bool is_bow = dataset_name == "docword.nytimes";
    Dataset dataset;
    Dataset_BoW dataset_bow1, dataset_bow2;
    int total_packets = 0;

    if (dataset_name == "caida")
    {
        dataset.init("./dataset/caida.dat", 21);
        total_packets = std::max(dataset.stream1.TOTAL_PACKETS, dataset.stream2.TOTAL_PACKETS);
    }
    else if (dataset_name.find("zipf") != string::npos)
    {
        dataset.init("./dataset/" + dataset_name + ".dat", 4);
        total_packets = std::max(dataset.stream1.TOTAL_PACKETS, dataset.stream2.TOTAL_PACKETS);
    }
    else if (dataset_name == "docword.nytimes")
    {
        dataset_bow1.init("./dataset/docword.nytimes.txt", nytimes_dataset_len, 3);
        dataset_bow2.init("./dataset/docword.nytimes.txt", nytimes_dataset_len, second_dataset_start);

        LOG_DEBUG("dataset_bow1.TOTAL_PACKETS: %d", dataset_bow1.TOTAL_PACKETS);
        LOG_DEBUG("dataset_bow2.TOTAL_PACKETS: %d", dataset_bow2.TOTAL_PACKETS);

        total_packets = std::max(dataset_bow1.TOTAL_PACKETS, dataset_bow2.TOTAL_PACKETS);
    }
    else
    {
        dataset.init("./dataset/caida.dat", 21);
        total_packets = std::max(dataset.stream1.TOTAL_PACKETS, dataset.stream2.TOTAL_PACKETS);
    }

    auto start = std::chrono::high_resolution_clock::now(), end = std::chrono::high_resolution_clock::now();
    double similarity_avg = 0;
    for (int i = 0; i < loop_time; i++)
    {
        MINHASH_CM minhash1(line_cnt, len, hash_cnt);
        MINHASH_CM minhash2(line_cnt, len, hash_cnt);
        memcpy(minhash1.slot_seed, minhash2.slot_seed, sizeof(seed_t) * line_cnt);
        memcpy(minhash1.hash_seed, minhash2.hash_seed, sizeof(seed_t) * hash_cnt);

        start = std::chrono::high_resolution_clock::now();
        if (!is_bow)
        {
            for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
                minhash1.insert(dataset.stream1.raw_data[i]);
            for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
                minhash2.insert(dataset.stream2.raw_data[i]);
        }
        else if (is_bow)
        {
            for (int i = 0; i < dataset_bow1.TOTAL_PACKETS; i++)
                minhash1.insert(dataset_bow1.raw_data[i]);
            for (int i = 0; i < dataset_bow2.TOTAL_PACKETS; i++)
                minhash2.insert(dataset_bow2.raw_data[i]);
        }
        end = std::chrono::high_resolution_clock::now();

        double similarity = similarity_minhash_cm(&minhash1, &minhash2);
        similarity_avg += similarity;
    }
    similarity_avg /= loop_time;
    auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    int MIPS = (int)(1. * total_packets / elapsed_time.count() * 1e6);

    LOG_RESULT("minhash with cm similarity_avg: %lf", similarity_avg);
    LOG_RESULT("minhash with cm MIPS: %d", MIPS);
    if (fout_time != NULL)
        *fout_time << MIPS << ",";
    LOG_DEBUG("exit distribution_minhash()\n");
    return similarity_avg;
}

double distribution_minhash_hash(int loop_time, int line_cnt, int len, int hash_cnt, string dataset_name = "caida", int nytimes_dataset_len = 800000, int second_dataset_start = 800003, std::ofstream *fout_time = NULL)
{
    LOG_DEBUG("enter distribution_minhash()");
    bool is_bow = dataset_name == "docword.nytimes";
    Dataset dataset;
    Dataset_BoW dataset_bow1, dataset_bow2;
    int total_packets = 0;

    if (dataset_name == "caida")
    {
        dataset.init("./dataset/caida.dat", 21);
        total_packets = std::max(dataset.stream1.TOTAL_PACKETS, dataset.stream2.TOTAL_PACKETS);
    }
    else if (dataset_name.find("zipf") != string::npos)
    {
        dataset.init("./dataset/" + dataset_name + ".dat", 4);
        total_packets = std::max(dataset.stream1.TOTAL_PACKETS, dataset.stream2.TOTAL_PACKETS);
    }
    else if (dataset_name == "docword.nytimes")
    {
        dataset_bow1.init("./dataset/docword.nytimes.txt", nytimes_dataset_len, 3);
        dataset_bow2.init("./dataset/docword.nytimes.txt", nytimes_dataset_len, second_dataset_start);

        LOG_DEBUG("dataset_bow1.TOTAL_PACKETS: %d", dataset_bow1.TOTAL_PACKETS);
        LOG_DEBUG("dataset_bow2.TOTAL_PACKETS: %d", dataset_bow2.TOTAL_PACKETS);

        total_packets = std::max(dataset_bow1.TOTAL_PACKETS, dataset_bow2.TOTAL_PACKETS);
    }
    else
    {
        dataset.init("./dataset/caida.dat", 21);
        total_packets = std::max(dataset.stream1.TOTAL_PACKETS, dataset.stream2.TOTAL_PACKETS);
    }

    auto start = std::chrono::high_resolution_clock::now(), end = std::chrono::high_resolution_clock::now();
    double similarity_avg = 0;
    for (int i = 0; i < loop_time; i++)
    {
        MINHASH_HASH minhash1( len, hash_cnt);
        MINHASH_HASH minhash2( len, hash_cnt);
        memcpy(&minhash1.index_s, &minhash2.index_s, sizeof(seed_t));
        memcpy(minhash1.hash_seed, minhash2.hash_seed, sizeof(seed_t) * hash_cnt);

        start = std::chrono::high_resolution_clock::now();
        if (!is_bow)
        {
            for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
                minhash1.insert(dataset.stream1.raw_data[i]);
            for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
                minhash2.insert(dataset.stream2.raw_data[i]);
        }
        else if (is_bow)
        {
            for (int i = 0; i < dataset_bow1.TOTAL_PACKETS; i++)
                minhash1.insert(dataset_bow1.raw_data[i]);
            for (int i = 0; i < dataset_bow2.TOTAL_PACKETS; i++)
                minhash2.insert(dataset_bow2.raw_data[i]);
        }
        end = std::chrono::high_resolution_clock::now();

        double similarity = similarity_minhash_hash(&minhash1, &minhash2);
        similarity_avg += similarity;
    }
    similarity_avg /= loop_time;
    auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    int MIPS = (int)(1. * total_packets / elapsed_time.count() * 1e6);

    LOG_RESULT("minhash with hash similarity_avg: %lf", similarity_avg);
    LOG_RESULT("minhash with hash MIPS: %d", MIPS);
    if (fout_time != NULL)
        *fout_time << MIPS << ",";
    LOG_DEBUG("exit distribution_minhash()\n");
    return similarity_avg;
}
