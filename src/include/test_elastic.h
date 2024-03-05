#include "elastic.h"
#include "elastic_with_cm.h"
#include "dataset.h"
#include <fstream>

using namespace std;

double distribution_elastic(int loop_time, int len, string dataset_name = "caida")
{
    Dataset dataset;
    if (dataset_name == "caida")
        dataset.init("./dataset/caida.dat", 21);
    else if (dataset_name.find("zipf") != string::npos)
        dataset.init("./dataset/" + dataset_name + ".dat", 4);
    else
        dataset.init("./dataset/caida.dat", 21);
    std::ofstream fout("./result/elastic_cnt=4_len=" + to_string(len) + "-dataset=" + dataset_name + ".csv", ios::out);

    double similarity_avg = 0;
    for (int i = 0; i < loop_time; i++)
    {
        Elastic elastic1(len, 4, 8);
        Elastic elastic2(len, 4, 8);
        for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
        {
            elastic1.insert(dataset.stream1.raw_data[i]);
        }
        for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
        {
            elastic2.insert(dataset.stream2.raw_data[i]);
        }
        double test_similarity = similarity(elastic1, elastic2);
        fout << test_similarity << endl;
        LOG_INFO("loop time: %d, similarity: %lf", i, test_similarity);

        similarity_avg += test_similarity;
    }
    similarity_avg /= loop_time;
    return similarity_avg;
}

double cosine_distribution_elastic(int loop_time, int len)
{
    Dataset dataset;
    dataset.init("./dataset/caida.dat", 21);
    std::ofstream fout("./result/cosine_elastic_cnt=4_len=" + to_string(len) + ".csv", ios::out);

    double similarity_avg = 0;
    for (int i = 0; i < loop_time; i++)
    {
        Elastic elastic1(len, 4, 8);
        Elastic elastic2(len, 4, 8);
        for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
        {
            elastic1.insert(dataset.stream1.raw_data[i]);
        }
        for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
        {
            elastic2.insert(dataset.stream2.raw_data[i]);
        }
        double test_similarity = Cosine_sim_2_streams(elastic1, elastic2);
        fout << test_similarity << endl;
        LOG_INFO("loop time: %d, similarity: %lf", i, test_similarity);

        similarity_avg += test_similarity;
    }
    similarity_avg /= loop_time;
    return similarity_avg;
}

double kl_divergence_distribution_elastic(int loop_time, int len, string zipf_skewness_1, string zipf_skewness_2)
{
    Dataset dataset1, dataset2;
    dataset1.init("./dataset/zipf_" + zipf_skewness_1 + ".dat", 4);
    dataset2.init("./dataset/zipf_" + zipf_skewness_2 + ".dat", 4);
    std::ofstream fout("./result/kl_divergence_elastic_cnt=" + to_string(len) + "_len=4.csv", ios::out);

    double similarity_avg = 0;
    for (int i = 0; i < loop_time; i++)
    {
        Elastic elastic1(len, 4, 8);
        Elastic elastic2(len, 4, 8);
        for (int i = 0; i < dataset1.TOTAL_PACKETS; i++)
        {
            elastic1.insert(dataset1.raw_data[i]);
        }
        for (int i = 0; i < dataset2.TOTAL_PACKETS; i++)
        {
            elastic2.insert(dataset2.raw_data[i]);
        }
        double test_similarity = kl_divergence_of_distribution_elastic(elastic1, elastic2);
        fout << test_similarity << endl;
        LOG_INFO("loop time: %d, similarity: %lf", i, test_similarity);

        similarity_avg += test_similarity;
    }
    similarity_avg /= loop_time;
    return similarity_avg;
}

double distribution_elastic_with_cm(int loop_time, int len, string dataset_name = "caida")
{
    Dataset dataset;

    if (dataset_name == "caida")
        dataset.init("./dataset/caida.dat", 21);
    else if (dataset_name == "zipf_1.0")
        dataset.init("./dataset/zipf_1.0.dat", 4);
    else if (dataset_name == "zipf_0.0")
        dataset.init("./dataset/zipf_0.0.dat", 4);
    else
        dataset.init("./dataset/caida.dat", 21);
    std::ofstream fout("./result/elastic_with_cm_cnt=3_len=" + to_string(len) + "-dataset=" + dataset_name + ".csv", ios::out);

    double similarity_avg = 0;
    for (int i = 0; i < loop_time; i++)
    {
        Elastic_with_cm elastic(len, 4, len, 8);
        for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
        {
            elastic.insert1(dataset.stream1.raw_data[i]);
        }
        for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
        {
            elastic.insert2(dataset.stream2.raw_data[i]);
        }

        double test_similarity = elastic.similarity_map();
        fout << test_similarity << endl;
        LOG_INFO("loop time: %d, similarity: %lf", i, test_similarity);

        similarity_avg += test_similarity;
    }
    similarity_avg /= loop_time;
    return similarity_avg;
}

double cosine_distribution_elastic_with_cm(int loop_time, int len)
{
    Dataset dataset;
    dataset.init("./dataset/caida.dat", 21);
    // double real_similarity = dataset.similarity();
    std::ofstream fout("./result/elastic_with_cm_cnt=3_len=" + to_string(len) + ".csv", ios::out);

    double similarity_avg = 0;
    for (int i = 0; i < loop_time; i++)
    {
        Elastic_with_cm elastic(len, 4, len, 8);
        for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
        {
            elastic.insert1(dataset.stream1.raw_data[i]);
        }
        for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
        {
            elastic.insert2(dataset.stream2.raw_data[i]);
        }

        double test_similarity = elastic.cosine_similarity();
        fout << test_similarity << endl;
        LOG_INFO("loop time: %d, similarity: %lf", i, test_similarity);

        similarity_avg += test_similarity;
    }
    similarity_avg /= loop_time;
    return similarity_avg;
}

void print_all_similarities_elastic(int len)
{
    Dataset dataset;
    dataset.init("./dataset/caida.dat", 21);

    Elastic s1(len, 4, 8);
    Elastic s2(len, 4, 8);
    for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
    {
        s1.insert(dataset.stream1.raw_data[i]);
    }
    for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
    {
        s2.insert(dataset.stream2.raw_data[i]);
    }

    LOG_RESULT("Similarity: %lf", similarity(s1, s2));
    LOG_RESULT("Cosine Similarity: %lf", Cosine_sim_2_streams(s1, s2));
    LOG_RESULT("KL Divergence: %lf", KL_sim_2_streams(s1, s2));
    LOG_RESULT("KL Divergence of distribution: %lf", kl_divergence_of_distribution_elastic(s1, s2));
}
