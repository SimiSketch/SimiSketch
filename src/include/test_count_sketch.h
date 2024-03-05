#pragma once
#include "count_sketch.h"
#include "dataset.h"
#include <fstream>


double distribution_cs(int loop_time, int len, string dataset_name = "caida", int line_num = 1, bool separate = true, bool use_known_zipf = true, double zipf_alpha = 1.0)
{
    LOG_DEBUG("into distribution_cs()");
    Dataset dataset;
    if (dataset_name == "caida")
        dataset.init("../dataset/caida.dat", 21, separate);
    else if (dataset_name.find("zipf") != string::npos)
        dataset.init("../dataset/" + dataset_name + ".dat", 4, separate, use_known_zipf, zipf_alpha);
    else
        dataset.init("../dataset/caida.dat", 21);
    // std::ofstream fout("./result/cs_len=" + to_string(len) + "-dataset=" + dataset_name + ".csv", ios::out);

    double similarity_avg = 0;
    for (int i = 0; i < loop_time; i++)
    {
        COUNT_SKETCH cs1(line_num, len);
        COUNT_SKETCH cs2(line_num, len);
        memcpy(cs2.slot_seed, cs1.slot_seed, sizeof(seed_t) * line_num);
        cs2.sign_seed = cs1.sign_seed;

        for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
            cs1.insert(dataset.stream1.raw_data[i]);
        for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
            cs2.insert(dataset.stream2.raw_data[i]);
        double similarity = similarity_cs(&cs1, &cs2);
        // fout << similarity << endl;
        LOG_INFO("loop time: %d, similarity: %lf", i, similarity);

        similarity_avg += similarity;
    }
    similarity_avg /= loop_time;
    LOG_DEBUG("exit distribution_cs()\n");
    return similarity_avg;
}

double cosine_distribution_cs(int loop_time, int len)
{
    Dataset dataset;
    dataset.init("../dataset/caida.dat", 21);
    double real_similarity = dataset.similarity();
    std::ofstream fout("./result/cosine_cs_len=" + to_string(len) + ".csv", ios::out);
    double similarity_avg = 0;
    for (int i = 0; i < loop_time; i++)
    {
        COUNT_SKETCH cs1(1, len);
        COUNT_SKETCH cs2(1, len);
        memcpy(cs2.slot_seed, cs1.slot_seed, sizeof(seed_t) * 1);
        cs2.sign_seed = cs1.sign_seed;

        for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
            cs1.insert(dataset.stream1.raw_data[i]);
        for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
            cs2.insert(dataset.stream2.raw_data[i]);
        double similarity = cosine_similarity_cs(&cs1, &cs2);
        fout << similarity << endl;
        LOG_INFO("loop time: %d, similarity: %lf", i, similarity);

        similarity_avg += similarity;
    }
    similarity_avg /= loop_time;
    return similarity_avg;
}

double kl_divergence_distribution_cs(int loop_time, int len, string zipf_skewness_1, string zipf_skewness_2)
{
    Dataset dataset1, dataset2;
    dataset1.init("../dataset/zipf_" + zipf_skewness_1 + ".dat", 4);
    dataset2.init("../dataset/zipf_" + zipf_skewness_2 + ".dat", 4);
    std::ofstream fout("./result/kl_divergence_cs_len=" + to_string(len) + ".csv", ios::out);
    double similarity_avg = 0;
    for (int i = 0; i < loop_time; i++)
    {
        COUNT_SKETCH cs1(1, len);
        COUNT_SKETCH cs2(1, len);
        memcpy(cs2.slot_seed, cs1.slot_seed, sizeof(seed_t) * 1);
        cs2.sign_seed = cs1.sign_seed;

        for (int i = 0; i < dataset1.TOTAL_PACKETS; i++)
            cs1.insert(dataset1.raw_data[i]);
        for (int i = 0; i < dataset2.TOTAL_PACKETS; i++)
            cs2.insert(dataset2.raw_data[i]);
        double similarity = kl_divergence_of_distribution_cs(&cs1, &cs2);
        fout << similarity << endl;
        LOG_INFO("loop time: %d, similarity: %lf", i, similarity);

        similarity_avg += similarity;
    }
    similarity_avg /= loop_time;
    return similarity_avg;
}

double distribution_cs_weighted_by_cm(int loop_time, int len, string dataset_name = "caida", int line_num = 1, bool separate = true, bool use_known_zipf = true, double zipf_alpha = 1.0)
{
    LOG_DEBUG("into distribution_cs_weighted_by_cm()");
    Dataset dataset;

    if (dataset_name == "caida")
        dataset.init("../dataset/caida.dat", 21, separate);
    else if (dataset_name.find("zipf") != string::npos)
        dataset.init("../dataset/" + dataset_name + ".dat", 4, separate, use_known_zipf, zipf_alpha);
    else
        dataset.init("../dataset/caida.dat", 21);
    std::ofstream fout("./result/cs_weighted_by_cm_len=" + to_string(len) + "-dataset=" + dataset_name + ".csv", ios::out);

    double similarity_avg = 0;
    for (int i = 0; i < loop_time; i++)
    {
        COUNT_SKETCH_WEIGHTED_BY_CM cs1(line_num, len);
        COUNT_SKETCH_WEIGHTED_BY_CM cs2(line_num, len);
        memcpy(cs2.slot_seed, cs1.slot_seed, sizeof(seed_t) * line_num);
        cs2.sign_seed = cs1.sign_seed;

        for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
            cs1.insert(dataset.stream1.raw_data[i]);
        for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
            cs2.insert(dataset.stream2.raw_data[i]);
        double similarity = similarity_cs_weighted_by_cm(&cs1, &cs2);
        fout << similarity << endl;
        LOG_INFO("loop time: %d, similarity: %lf", i, similarity);

        similarity_avg += similarity;
    }
    similarity_avg /= loop_time;
    LOG_DEBUG("exit distribution_cs_weighted_by_cm()\n");
    return similarity_avg;
}

double kl_divergence_distribution_cs_weighted_by_cm(int loop_time, int len, string zipf_skewness_1, string zipf_skewness_2)
{
    Dataset dataset1, dataset2;
    dataset1.init("../dataset/zipf_" + zipf_skewness_1 + ".dat", 4);
    dataset2.init("../dataset/zipf_" + zipf_skewness_2 + ".dat", 4);
    std::ofstream fout("./result/kl_divergence_cs_weighted_by_cm_len=" + to_string(len) + ".csv", ios::out);
    double similarity_avg = 0;
    for (int i = 0; i < loop_time; i++)
    {
        COUNT_SKETCH_WEIGHTED_BY_CM cs1(1, len);
        COUNT_SKETCH_WEIGHTED_BY_CM cs2(1, len);
        memcpy(cs2.slot_seed, cs1.slot_seed, sizeof(seed_t) * 1);
        cs2.sign_seed = cs1.sign_seed;

        for (int i = 0; i < dataset1.TOTAL_PACKETS; i++)
            cs1.insert(dataset1.raw_data[i]);
        for (int i = 0; i < dataset2.TOTAL_PACKETS; i++)
            cs2.insert(dataset2.raw_data[i]);
        double similarity = kl_divergence_distribution_cs_weighted_by_cm(&cs1, &cs2);
        fout << similarity << endl;
        LOG_INFO("loop time: %d, similarity: %lf", i, similarity);

        similarity_avg += similarity;
    }
    similarity_avg /= loop_time;
    return similarity_avg;
}

double distribution_cs_merge(int loop_time, int len, string dataset_name = "caida")
{
    Dataset dataset;

    if (dataset_name == "caida")
        dataset.init("../dataset/caida.dat", 21);
    else if (dataset_name.find("zipf") != string::npos)
        dataset.init("../dataset/" + dataset_name + ".dat", 4);
    else
        dataset.init("../dataset/caida.dat", 21);
    std::ofstream fout("./result/cs_merge_len=" + to_string(len) + "-dataset=" + dataset_name + ".csv", ios::out);

    double similarity_avg = 0;
    for (int i = 0; i < loop_time; i++)
    {
        COUNT_SKETCH_MERGE cs1(1, len);
        COUNT_SKETCH_MERGE cs2(1, len);
        memcpy(cs2.hash_seeds, cs1.hash_seeds, sizeof(seed_t) * 1);
        cs2.sign_seed = cs1.sign_seed;

        for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
            cs1.insert(dataset.stream1.raw_data[i]);
        for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
            cs2.insert(dataset.stream2.raw_data[i]);
        double similarity = similarity_cs_merge(&cs1, &cs2);
        fout << similarity << endl;
        LOG_INFO("loop time: %d, similarity: %lf", i, similarity);

        similarity_avg += similarity;
    }
    similarity_avg /= loop_time;
    return similarity_avg;
}

double cosine_distribution_cs_merge(int loop_time, int len)
{
    Dataset dataset;
    dataset.init("../dataset/caida.dat", 21);
    double real_similarity = dataset.similarity();
    std::ofstream fout("./result/cosine_cs_merge_len=" + to_string(len) + ".csv", ios::out);
    double similarity_avg = 0;
    for (int i = 0; i < loop_time; i++)
    {
        COUNT_SKETCH_MERGE cs1(1, len);
        COUNT_SKETCH_MERGE cs2(1, len);
        memcpy(cs2.hash_seeds, cs1.hash_seeds, sizeof(seed_t) * 1);
        cs2.sign_seed = cs1.sign_seed;

        for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
            cs1.insert(dataset.stream1.raw_data[i]);
        for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
            cs2.insert(dataset.stream2.raw_data[i]);
        double similarity = cosine_similarity_cs_merge(&cs1, &cs2);
        fout << similarity << endl;
        LOG_INFO("loop time: %d, similarity: %lf", i, similarity);

        similarity_avg += similarity;
    }
    similarity_avg /= loop_time;
    return similarity_avg;
}

double distribution_cs_merge_weighted_by_cm(int loop_time, int len, string dataset_name = "caida", int nytimes_dataset_len = 800000, int second_dataset_start = 800003, std::ofstream *fout_time = NULL)
{
    LOG_DEBUG("into distribution_cs_merge_weighted_by_cm()");
    bool is_bow = dataset_name == "docword.nytimes";
    Dataset dataset;
    Dataset_BoW dataset_bow1, dataset_bow2;
    int total_packets = 0;

    if (dataset_name == "caida")
    {
        dataset.init("../dataset/caida.dat", 21);
        total_packets = std::max(dataset.stream1.TOTAL_PACKETS, dataset.stream2.TOTAL_PACKETS);
    }
    else if (dataset_name.find("zipf") != string::npos)
    {
        dataset.init("../dataset/" + dataset_name + ".dat", 4);
        total_packets = std::max(dataset.stream1.TOTAL_PACKETS, dataset.stream2.TOTAL_PACKETS);
    }
    else if (dataset_name == "docword.nytimes")
    {
        dataset_bow1.init("../dataset/docword.nytimes.txt", nytimes_dataset_len, 3);
        dataset_bow2.init("../dataset/docword.nytimes.txt", nytimes_dataset_len, second_dataset_start);

        LOG_DEBUG("dataset_bow1.TOTAL_PACKETS: %d", dataset_bow1.TOTAL_PACKETS);
        LOG_DEBUG("dataset_bow2.TOTAL_PACKETS: %d", dataset_bow2.TOTAL_PACKETS);

        total_packets = std::max(dataset_bow1.TOTAL_PACKETS, dataset_bow2.TOTAL_PACKETS);
    }
    else
    {
        dataset.init("../dataset/caida.dat", 21);
        total_packets = std::max(dataset.stream1.TOTAL_PACKETS, dataset.stream2.TOTAL_PACKETS);
    }

    auto start = std::chrono::high_resolution_clock::now(), end = std::chrono::high_resolution_clock::now();
    double similarity_avg = 0;
    for (int i = 0; i < loop_time; i++)
    {
        COUNT_SKETCH_MERGE_WEIGHTED_BY_CM cs1(1, len);
        COUNT_SKETCH_MERGE_WEIGHTED_BY_CM cs2(1, len);

        memcpy(cs2.hash_seeds, cs1.hash_seeds, sizeof(seed_t) * 1);
        cs2.sign_seed = cs1.sign_seed;

        start = std::chrono::high_resolution_clock::now();
        if (!is_bow)
        {
            for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
                cs1.insert(dataset.stream1.raw_data[i]);
            for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
                cs2.insert(dataset.stream2.raw_data[i]);
        }
        else if (is_bow)
        {
            for (int i = 0; i < dataset_bow1.TOTAL_PACKETS; i++)
                cs1.insert(dataset_bow1.raw_data[i]);
            for (int i = 0; i < dataset_bow2.TOTAL_PACKETS; i++)
                cs2.insert(dataset_bow2.raw_data[i]);
        }
        end = std::chrono::high_resolution_clock::now();

        double similarity = similarity_cs_merge_weighted_by_cm(&cs1, &cs2);
        similarity_avg += similarity;
    }
    similarity_avg /= loop_time;
    auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    int MIPS = (int)(1. * total_packets / elapsed_time.count() * 1e6);

    LOG_RESULT("cs_merge_weighted_by_cm similarity_avg: %lf", similarity_avg);
    LOG_RESULT("cs_merge_weighted_by_cm MIPS: %d", MIPS);
    if (fout_time != NULL)
        *fout_time << MIPS << ",";
    LOG_DEBUG("exit distribution_cs_merge_weighted_by_cm()\n");
    return similarity_avg;
}
