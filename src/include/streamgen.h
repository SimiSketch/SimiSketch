#pragma once
#include "defs.h"
#include <unordered_map>
#include <string>
#include <vector>

class StreamGen
{
private:
    // // count_t TOTAL_BUCKETS;
    count_t cnt; // = TOTAL_PACKETS
    // count_t max_freq;
    // count_t *nt; /* nt = count_t[max_freq] */
    // count_t *trunc_start = NULL, *trunc_end = NULL;
    std::vector<count_t> freqs;
    // count_t trunc_min_accumulate_num, trunc_max_accumulate_num;
    std::vector<count_t> freqs_small;
    std::vector<count_t> freqs_big;

    /**
     * @brief Read data from the dataset, store #package in cnt.
     *
     * @param PATH path of the dataset.
     * @param size size of each record.
     * @return array contains all the packages in the daatset, remember to
     * DELETE it after used!
     */
    data_t *read_data(const char *PATH, const int size);

public:
    count_t TOTAL_FLOWS;
    count_t TOTAL_PACKETS;
    count_t TOTAL_SMALL;
    count_t TOTAL_BIG;
    data_t *raw_data;
    std::unordered_map<data_t, count_t> counter;

    /**
     * @brief Should be called before calling other functions defined in this
     * file! Read the CAIDA dataset and figure out its distribution. Set
     * TOTAL_FLOWS and TOTAL_BUCKETS defined in datatype.h
     *
     * @param file path of the data
     */
    void init(std::string file, int size);

    void init(std::vector<double> &dist);

    /**
     * @brief Initial from zipfian distribution
     *
     * @param alpha zipf paramter
     * @param N_FLOWS # flows (distinct items)
     * @param N_PACKETS # total packets
     */
    void init(double alpha, int N_FLOWS, int N_PACKETS);

    /**
     * @brief init() should be called first!
     * Generate a new stream w.r.t. the data distribution
     * @return # apperence of the generated stream
     */
    count_t new_stream();

    /**
     * @brief should be called before trunc_new_stream(). In order to sample
     * flow whose frequence is only in [min_freq, max_freq)
     *
     * @param freq_lower_bound lower bound of the frequency
     * @param freq_upper_bound upper bound of the frequency
     */
    // void trunc_stream_init(count_t freq_lower_bound, count_t freq_upper_bound);

    /**
     * @brief should be called before trunc_new_stream(). In order to sample
     * flow whose frequence is only in [min_freq, max_freq)
     *
     * @param freq_lower_bound lower bound of the frequency
     */
    void trunc_stream_init(count_t freq_lower_bound);

    /**
     * @brief generate stream whose frequency in [1, min_freq)
     *
     * @return count_t frequency of the generated stream
     */
    count_t trunc_tiny_stream();

    /**
     * @brief generate stream whose frequency in [min_freq, max_freq)
     *
     * @return count_t frequency of the generated stream
     */
    // count_t trunc_middle_stream();

    /**
     * @brief generate stream whose frequency in [max_freq, +infty)
     *
     * @return count_t frequency of the generated stream
     */
    // count_t trunc_big_stream();

    /**
     * @brief generate stream whose frequency in [1, max_freq)
     *
     * @return count_t frequency of the generated stream
     */
    // count_t trunc_not_big_stream();

    /**
     * @brief generate stream whose frequency in [min_freq, +infty)
     *
     * @return count_t frequency of the generated stream
     */
    count_t trunc_not_small_stream();

    // /**
    //  * @return double P(frequency>freq)
    //  */
    // double P_bigger_than(count_t freq);

    // /**
    //  * @return P(frequency<freq)
    //  */
    // double P_smaller_than(count_t freq);

    // void print_distribution();

    auto getTotalFlows() const
    {
        return this->TOTAL_FLOWS;
    }

    double L2_Norm()
    {
        double sum = 0;
        for (auto &it : counter)
        {
            sum += it.second * it.second;
        }
        return sqrt(sum);
    }

    ~StreamGen()
    {
        delete[] raw_data;
    }
};