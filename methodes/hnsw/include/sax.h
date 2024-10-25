//
// Created by ilias.azizi on 26/01/23.
//

#ifndef ELPISAX_SAX_H
#define ELPISAX_SAX_H
#include "math.h"

#define MINVAL -2000000
#define MAXVAL 2000000
typedef unsigned char sax_type;
typedef unsigned long long root_mask_type;
typedef float ts_type;

int compare(const void *a, const void *b);
void sax_from_ts(ts_type *ts_in, sax_type *sax_out, int ts_values_per_segment,
                 int segments, int cardinality, int bit_cardinality);
void paa_from_ts (ts_type *ts_in, ts_type *paa_out, int segments, int ts_values_per_segment);
float   minidist_paa_to_isax_raw_SIMD(const float *paa, const sax_type *sax,
                                      const sax_type *sax_cardinalities,
                                      sax_type max_bit_cardinality,
                                      double max_cardinality,
                                      int number_of_segments,
                                      int min_val,
                                      int max_val,
                                      float ratio_sqrt);

ts_type minidist_paa_to_isax_raw(const float *paa, const sax_type *sax,
                                 const sax_type *sax_cardinalities,
                                 sax_type max_bit_cardinality,
                                 double max_cardinality,
                                 int number_of_segments,
                                 int min_val,
                                 int max_val,
                                 float ratio_sqrt);
#endif //ELPISAX_SAX_H
