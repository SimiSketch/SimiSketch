#include "include/dataset.h"
#include "include/test_cm.h"
#include "include/test_cu.h"
#include "include/test_count_sketch.h"
#include "include/test_elastic.h"
#include "include/test_cycle.h"
#include "include/test_minhash.h"
#include "include/test_hll.h"
#include "include/test_maxloghash.h"

int main(){
  Dataset dataset;
  dataset.init("./dataset/zipf_0.5.dat",4);
  dataset.similarity();
  distribution_cycle(1,100000,"zipf_0.5");
}