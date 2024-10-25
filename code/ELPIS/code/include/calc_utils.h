//
//  cals_utils.h
//  ds-tree C version
//
//  Created by Karima Echihabi on 18/12/2016
//  Copyright 2012 Paris Descartes University. All rights reserved.
//  
//  Based on isax code written by Zoumpatianos on 3/12/12.
//  Copyright 2012 University of Trento. All rights reserved.
//

#ifndef herculeslib_hercules_calc_utils_h
#define herculeslib_hercules_calc_utils_h


#include <iostream>
#include "globals.h"
#include "Node.h"
#include "math.h"

ts_type calc_mean(ts_type *, int start, int end);


short compare_short(const void *a, const void *b);


ts_type calculate_mean_std_dev_range(struct segment_sketch sketch, int len);


short get_segment_start(short *points, int idx);

short get_segment_end(short *points, int idx);

int get_segment_length(short *points, int i);

enum response calc_split_points(short *points, int ts_length, int segment_size);
enum response calc_hs_split_points(short *hs_split_points, short *num_hs_split_points, short *split_points, int segment_size,
                     int min_length/*1*/);
boolean is_split_policy_mean(struct node_segment_split_policy policy) ;

boolean is_split_policy_stdev(struct node_segment_split_policy policy) ;
enum response
mean_node_segment_split_policy_split(struct node_segment_split_policy *policy, struct segment_sketch sketch,
                                     struct segment_sketch *ret) ;

enum response
stdev_node_segment_split_policy_split(struct node_segment_split_policy *policy, struct segment_sketch sketch,
                                      struct segment_sketch *ret) ;

/** return hs_point_from if the hs_point_to belongs to node_point**/
short get_hs_split_point(short *points, short from, short to, int size_points) ;
#endif
