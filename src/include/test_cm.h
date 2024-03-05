#include "cm_for_2.h"
#include "dataset.h"
#include <fstream>
void test_cm()
{
    int N = 500000;
    int n_array_min = 1, n_array_max = 10;

    Dataset dataset;
    dataset.init("./dataset/caida.dat", 21);
    double real_similarity = dataset.similarity();
    std::ofstream fout("./result/test_cm_cnt.csv", ios::out);
    // fout << "n_array,similarity" << std::endl;
    for (int n_array = n_array_min; n_array <= n_array_max; n_array++)
    {
        int array_len = N / n_array;
        CM_FOR_2 cm(n_array, array_len);
        for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
            cm.insert1(dataset.stream1.raw_data[i]);
        for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
            cm.insert2(dataset.stream2.raw_data[i]);
        fout << n_array << "," << cm.similarity() << std::endl;
    }
    fout.close();
}

bool check_bias_cm(int check_N_times)
{
    Dataset dataset;
    dataset.init("./dataset/caida.dat", 21);
    double real_similarity = dataset.similarity();
    int N_intersect = 0;
    int N_union = 0;
    for (auto t : dataset.stream1.counter)
    {
        auto it = dataset.stream2.counter.find(t.first);
        if (it != dataset.stream2.counter.end())
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
    for (auto t : dataset.stream2.counter)
    {
        N_union += t.second;
    }
    CM_FOR_2 cm(check_N_times, 100000);
    for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
        cm.insert1(dataset.stream1.raw_data[i]);
    for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
        cm.insert2(dataset.stream2.raw_data[i]);
    for (int j = 0; j < cm.N_ARRAY; j++)
    {
        int cur_intersection = 0;
        int cur_union = 0;
        for (int k = 0; k < cm.LEN; k++)
        {
            cur_intersection += std::min(cm.nt1[j][k], cm.nt2[j][k]);
            cur_union += std::max(cm.nt1[j][k], cm.nt2[j][k]);
        }
        if ((cur_intersection - N_intersect) != (N_union - cur_union))
        {
            return false;
        }
    }
    return true;
}

double distribution_cm(int loop_time, int len, string dataset_name = "caida", int line_num = 1, bool separate = true, bool use_known_zipf = true, double zipf_alpha = 1.0)
{
    LOG_DEBUG("into distribution_cm()");
    Dataset dataset;
    if (dataset_name == "caida")
        dataset.init("./dataset/caida.dat", 21, separate);
    else if (dataset_name.find("zipf") != string::npos)
        dataset.init("./dataset/" + dataset_name + ".dat", 4, separate, use_known_zipf, zipf_alpha);
    else
        dataset.init("./dataset/caida.dat", 21);
    std::ofstream fout("./result/cm_len=" + to_string(len) + "-dataset=" + dataset_name + ".csv", ios::out);

    double similarity_avg = 0;
    for (int i = 0; i < loop_time; i++)
    {
        CM_FOR_2 cm(line_num, len);
        for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
        {
            cm.insert1(dataset.stream1.raw_data[i]);
        }
        for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
        {
            cm.insert2(dataset.stream2.raw_data[i]);
        }
        double similarity = cm.similarity();
        fout << similarity << endl;
        LOG_INFO("loop time: %d, similarity: %lf", i, similarity);

        similarity_avg += similarity;
    }
    similarity_avg /= loop_time;
    LOG_DEBUG("exit distribution_cm()\n");
    return similarity_avg;
}

double cosine_distribution_cm(int loop_time, int len)
{
    Dataset dataset;
    dataset.init("./dataset/caida.dat", 21);
    double real_similarity = dataset.similarity();
    std::ofstream fout("./result/cosine_cm_len=" + to_string(len) + ".csv", ios::out);

    double similarity_avg = 0;
    for (int i = 0; i < loop_time; i++)
    {
        CM_FOR_2 cm(1, len);
        for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
        {
            cm.insert1(dataset.stream1.raw_data[i]);
        }
        for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
        {
            cm.insert2(dataset.stream2.raw_data[i]);
        }
        double similarity = cm.cosine_similarity();
        fout << similarity << endl;
        LOG_INFO("loop time: %d, similarity: %lf", i, similarity);

        similarity_avg += similarity;
    }
    similarity_avg /= loop_time;
    return similarity_avg;
}

void print_all_similarities_cm(int len)
{
    Dataset dataset;
    dataset.init("./dataset/caida.dat", 21);

    CM_FOR_2 cm(1, len);
    for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
    {
        cm.insert1(dataset.stream1.raw_data[i]);
    }
    for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
    {
        cm.insert2(dataset.stream2.raw_data[i]);
    }

    LOG_RESULT("Similarity: %lf", cm.similarity());
    LOG_RESULT("Cosine Similarity: %lf", cm.cosine_similarity());
    LOG_RESULT("CE: %lf", cm.cross_entropy());
    LOG_RESULT("KL Divergence: %lf", cm.kl_divergence());
    LOG_RESULT("KL Divergence of distribution: %lf", cm.kl_divergence_of_distribution());
}