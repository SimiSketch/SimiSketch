#include "defs.h"
#include "streamgen.h"
#include "logger.h"
#include "util.h"
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <cstring>
#include <string>
#include <assert.h>
using namespace std;

data_t *StreamGen::read_data(const char *PATH, const int size)
{
    struct stat buf;
    LOG_DEBUG("Opening file %s", PATH);
    int fd = Open(PATH, O_RDONLY);
    fstat(fd, &buf);
    int n_elements = buf.st_size / size;
    cnt = n_elements;
    TOTAL_PACKETS = n_elements;
    LOG_DEBUG("\tcnt=%d", cnt);
    LOG_DEBUG("Mmap...");
    void *addr = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    data_t *data_array = new data_t[n_elements];
    close(fd);
    if (addr == MAP_FAILED)
    {
        LOG_ERROR("MMAP FAILED!");
        exit(-1);
    }

    char *ptr = reinterpret_cast<char *>(addr);
    for (int i = 0; i < n_elements; i++)
    {
        data_array[i] = *reinterpret_cast<data_t *>(ptr);
        ptr += size;
    }

    munmap(addr, buf.st_size);
    return data_array;
}

void StreamGen::init(string file, int size)
{
    raw_data = read_data(file.c_str(), size);
    for (count_t i = 0; i < cnt; i++)
    {
        auto it = counter.find(raw_data[i]);
        if (it == counter.end())
        {
            counter.insert(std::make_pair(raw_data[i], 1));
        }
        else
        {
            it->second++;
        }
    }
    TOTAL_FLOWS = counter.size();
    // TOTAL_BUCKETS = TOTAL_FLOWS / 10;
    LOG_INFO("Total flows: %d", TOTAL_FLOWS);
    LOG_INFO("Total packets: %d", TOTAL_PACKETS);

    for (auto it : counter)
    {
        freqs.push_back(it.second);
    }

    // Find max frequences
    // max_freq = 0;
    // for (auto it = counter.begin(); it != counter.end();it++)
    // {
    //     max_freq = std::max(max_freq, it->second);
    // }
    // max_freq++;
    // nt = new count_t[max_freq];
    // memset(nt, 0, max_freq * sizeof(count_t));
    // for (auto it = counter.begin(); it != counter.end(); it++)
    // {
    //     nt[it->second] += 1;
    // }
    // for (int i = 1; i < max_freq;i++)
    // {
    //     nt[i] += nt[i - 1];
    // }
    srand(clock());
}

void StreamGen::init(vector<double> &dist)
{
    raw_data = NULL;
    TOTAL_PACKETS = 0;
    TOTAL_FLOWS = 0;
    for (int i = 0; i < dist.size(); i++)
    {
        if (dist[i] > 0)
        {
            TOTAL_PACKETS += i * dist[i];
            TOTAL_FLOWS += dist[i];
            for (int k = 0; k < dist[i]; k++)
                freqs.push_back(i);
        }
    }
    LOG_INFO("Total flows: %d", TOTAL_FLOWS);
    LOG_INFO("Total packets: %d", TOTAL_PACKETS);
    cnt = TOTAL_PACKETS;
}

void StreamGen::init(double alpha, int N_FLOWS, int N_PACKETS)
{
    TOTAL_FLOWS = 0;
    TOTAL_PACKETS = 0;
    raw_data = new data_t[N_PACKETS];
    double M = 0;
    for (int i = 1; i <= N_FLOWS; i++)
        M += 1.0 / pow(i, alpha);

    int curpos = 0;
    for (int i = 1; i <= N_FLOWS; i++)
    {
        count_t curfreq = N_PACKETS * 1.0 / (pow(i, alpha) * M);
        if (curfreq < 1)
            break;
        TOTAL_FLOWS++;
        TOTAL_PACKETS += curfreq;
        assert(counter.insert(std::make_pair(i, curfreq)).second);
        freqs.push_back(curfreq);
        for (int ii = 0; ii < curfreq; ii++)
        {
            raw_data[curpos] = i;
            curpos++;
        }
    }

    LOG_INFO("Total flows: %d", TOTAL_FLOWS);
    LOG_INFO("Total packets: %d", TOTAL_PACKETS);
    cnt = TOTAL_PACKETS;
}

count_t StreamGen::new_stream()
{
    // count_t cur = static_cast<count_t>(rand());
    // cur = cur % nt[max_freq-1];
    // auto it = std::lower_bound(nt, nt + max_freq, cur);
    // return static_cast<count_t>(it - nt);

    int pos = double(rand()) * TOTAL_FLOWS / RAND_MAX;
    return freqs[pos];
}

// void StreamGen::trunc_stream_init(count_t freq_lower_bound, count_t freq_upper_bound)
// {
//     trunc_start=nt+freq_lower_bound;
//     trunc_end=nt+freq_upper_bound;
//     trunc_min_accumulate_num = *trunc_start;
//     trunc_max_accumulate_num = *trunc_end;
// }

void StreamGen::trunc_stream_init(count_t freq_lower_bound)
{
    for (int i = 0; i < TOTAL_FLOWS; i++)
    {
        if (freqs[i] >= freq_lower_bound)
            freqs_big.push_back(freqs[i]);
        else
            freqs_small.push_back(freqs[i]);
    }
    TOTAL_BIG = freqs_big.size();
    TOTAL_SMALL = freqs_small.size();
    if (TOTAL_SMALL == 0)
        LOG_ERROR("No small flows!");
}

count_t StreamGen::trunc_tiny_stream()
{
    if (TOTAL_SMALL == 0)
        return new_stream();
    double pos = double(rand()) * TOTAL_SMALL / RAND_MAX;
    return freqs_small[pos];
}

// count_t StreamGen::trunc_middle_stream()
// {
//     count_t cur = static_cast<count_t>(rand());
//     cur = cur % (trunc_max_accumulate_num-trunc_min_accumulate_num);
//     cur += trunc_min_accumulate_num;
//     auto it = std::lower_bound(trunc_start, trunc_end, cur);
//     return static_cast<count_t>(it - nt);
// }

// count_t StreamGen::trunc_big_stream()
// {
//     count_t cur = static_cast<count_t>(rand());
//     cur = cur % (nt[max_freq-1]-trunc_max_accumulate_num);
//     cur += trunc_max_accumulate_num;
//     auto it = std::lower_bound(trunc_end, nt+max_freq, cur);
//     return static_cast<count_t>(it - nt);
// }

// count_t StreamGen::trunc_not_big_stream()
// {
//     count_t cur = static_cast<count_t>(rand());
//     cur = cur % (trunc_max_accumulate_num-1);
//     cur += 1;
//     auto it = std::lower_bound(nt, trunc_end, cur);
//     return static_cast<count_t>(it - nt);
// }

count_t StreamGen::trunc_not_small_stream()
{
    double pos = double(rand()) * TOTAL_BIG / RAND_MAX;
    return freqs_big[pos];
}

// double StreamGen::P_bigger_than(count_t freq)
// {
//     return double(nt[max_freq-1]-nt[freq])/nt[max_freq-1];
// }

// double StreamGen::P_smaller_than(count_t freq)
// {
//     return double(nt[freq-1])/nt[max_freq-1];
// }

// void StreamGen::print_distribution()
// {
//     std::vector<count_t> freq;
//     for (auto it : counter)
//     {
//         freq.push_back(it.second);
//     }
//     sort(freq.begin(),freq.end());
//     LOG_INFO("Flow distribution:");
//     LOG_INFO("Percentile | Frequency");
//     LOG_INFO("\t50\t%d",freq[int(0.5*TOTAL_FLOWS)]);
//     LOG_INFO("\t60\t%d",freq[int(0.6*TOTAL_FLOWS)]);
//     LOG_INFO("\t70\t%d",freq[int(0.7*TOTAL_FLOWS)]);
//     LOG_INFO("\t80\t%d",freq[int(0.8*TOTAL_FLOWS)]);
//     LOG_INFO("\t90\t%d",freq[int(0.9*TOTAL_FLOWS)]);
//     LOG_INFO("\t95\t%d",freq[int(0.95*TOTAL_FLOWS)]);
//     LOG_INFO("\t99\t%d",freq[int(0.99*TOTAL_FLOWS)]);
// }