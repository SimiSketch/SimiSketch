#include "cu.h"
#include "dataset.h"
#include <fstream>


void test_cu()
{
    int N = 500000;
    int n_array_min = 1, n_array_max = 10;

    Dataset dataset;
    dataset.init("./dataset/caida.dat", 21);
    double real_similarity = dataset.similarity();
    std::ofstream fout("./result/test_cu_cnt.csv", ios::out);
    fout << "n_array,similarity" << std::endl;
    for (int n_array = n_array_min; n_array <= n_array_max; n_array++)
    {
        int array_len = N / n_array;
        CU cu(n_array, array_len);
        for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
            cu.insert1(dataset.stream1.raw_data[i]);
        for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
            cu.insert2(dataset.stream2.raw_data[i]);
        fout << n_array << "," << cu.similarity() << std::endl;
    }
    fout.close();
}

bool check_bias_cu(int check_N_times)
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
    CU cu(check_N_times, 100000);
    for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
        cu.insert1(dataset.stream1.raw_data[i]);
    for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
        cu.insert2(dataset.stream2.raw_data[i]);
    for (int j = 0; j < cu.N_ARRAY; j++)
    {
        int cur_intersection = 0;
        int cur_union = 0;
        for (int k = 0; k < cu.LEN; k++)
        {
            cur_intersection += std::min(cu.nt1[j][k], cu.nt2[j][k]);
            cur_union += std::max(cu.nt1[j][k], cu.nt2[j][k]);
        }
        if ((cur_intersection - N_intersect) != (N_union - cur_union))
        {
            return false;
        }
    }
    return true;
}

void distribution_of_similarity_cu(int loop_time)
{
    Dataset dataset;
    dataset.init("./dataset/caida.dat", 21);
    double real_similarity = dataset.similarity();
    std::ofstream fout("./result/test_cu_similarity_distribtion.csv", ios::out);
    fout << "cu_similarity_distribtion" << std::endl;
    for (int i = 0; i < loop_time; i++)
    {
        CU cu(1, 100000);
        for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
        {
            cu.insert1(dataset.stream1.raw_data[i]);
        }
        for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
        {
            cu.insert2(dataset.stream2.raw_data[i]);
        }
        fout << cu.similarity() << endl;
    }
}