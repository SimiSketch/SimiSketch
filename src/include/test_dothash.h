#pragma once
#include "dothash.h"
#include "dataset.h"

void test_dothash_cm(){
    Dataset dataset;
    dataset.init("./dataset/caida.dat", 21);
    double real_similarity = dataset.similarity();
    LOG_INFO("The real size of 1:%d,real size of 2:%d", dataset.stream1.TOTAL_PACKETS, dataset.stream2.TOTAL_PACKETS);
    dothash_cm dot(2000,100000);

    for (int i = 0; i < dataset.stream1.TOTAL_PACKETS; i++)
    {
        dot.insert1(dataset.stream1.raw_data[i]);
    }
    for (int i = 0; i < dataset.stream2.TOTAL_PACKETS; i++)
    {
        dot.insert2(dataset.stream2.raw_data[i]);
    }
    double similarity = dot.simiarity();
    LOG_INFO("TEST SIMILARITY THROUGH DOTHASH WITH EXCAT CM");
    LOG_RESULT("similarity:%lf",similarity);

}