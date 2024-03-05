#pragma once
#include "cycle.h"
#include "dataset.h"
#include <fstream>

double distribution_cycle(int loop_time, int len, string dataset_name = "caida", int nytimes_dataset_len = 800000, int second_dataset_start = 800003, std::ofstream *fout_time = NULL)
{
    LOG_DEBUG("enter distribution_cycle()");
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

    double similarity_avg = 0;
    auto start = std::chrono::high_resolution_clock::now(), end = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < loop_time; i++)
    {
        cycle_ours cs(len);

        start = std::chrono::high_resolution_clock::now();
        if (!is_bow)
        {
            for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
                cs.insert1(dataset.stream1.raw_data[i]);
            for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
                cs.insert2(dataset.stream2.raw_data[i]);
        }
        else if (is_bow)
        {
            for (int i = 0; i < dataset_bow1.TOTAL_PACKETS; i++)
                cs.insert1(dataset_bow1.raw_data[i]);
            for (int i = 0; i < dataset_bow2.TOTAL_PACKETS; i++)
                cs.insert2(dataset_bow2.raw_data[i]);
        }
        end = std::chrono::high_resolution_clock::now();

        double similarity = cs.similarity();
        similarity_avg += similarity;
    }
    similarity_avg /= loop_time;
    auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    int MIPS = (int)(1. * total_packets / elapsed_time.count() * 1e6);

    LOG_RESULT("cycle similarity_avg: %lf", similarity_avg);
    LOG_RESULT("cycle MIPS: %d", MIPS);
    if (fout_time != NULL)
        *fout_time << MIPS << ",";
    LOG_DEBUG("exit distribution_cycle()\n");
    return similarity_avg;
}
