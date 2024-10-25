


#include "calc_utils.h"


ts_type calc_mean(ts_type *series, int start, int end) {

    ts_type mean = 0;

    if (start >= end) {
        std::cerr<<"error in calc_mean, start > end "<<std::endl;
    } else {
        for (int i = start; i < end; i++) {
            mean += series[i];
        }

        mean /= (end - start);
    }

    return mean;

}

/*
  Using the stdev computational formula.

*/

ts_type calc_stdev(ts_type *series, int start, int end) {

    ts_type sum_x_squares = 0, sum_x = 0, stdev = 0; //sum of x's and sum of x squares
    int i, count_x;

    if (start >= end) {
        printf("error in stdev start >= end\n");
    } else {
        count_x = end - start; //size of the series

        for (int i = start; i < end; i++) {
            sum_x += series[i];
            sum_x_squares += pow(series[i], 2);
        }

        sum_x_squares -= (pow(sum_x, 2) / count_x);

        stdev = sqrt(sum_x_squares / count_x);
    }

    return stdev;

}



/*
 This is the compare function used in the binary search code
 */



ts_type compare_ts_type(const void *a, const void *b) {
    return (*(ts_type *) a - *(ts_type *) b);
}


short compare_short(const void *a, const void *b) {
    if (*(short *) a < *(short *) b)
        return -1;
    else if (*(short *) a == *(short *) b)
        return 0;
    else
        return 1;

}

/*
short compare_file_map_entries (const void * a, const void * b)
{
  char * entry_a = (char *) a;
  struct hercules_file_map *entry_b = (struct hercules_file_map*) b;

  return ( strcmp(entry_a, entry_b->filename));

}
*/
short compare_file_buffer(const void *a, const void *b) {
    struct hercules_file_buffer *entry_a = *((struct hercules_file_buffer **) a);
    struct hercules_file_buffer *entry_b = *((struct hercules_file_buffer **) b);

    if (entry_a->buffered_list_size < entry_b->buffered_list_size)
        return 1;
    else if (entry_a->buffered_list_size == entry_b->buffered_list_size)
        return 0;
    else
        return -1;
}

/*
  returns the current time in string format.
*/

void get_current_time(char *time_buf) {
    time_t timer;

    struct tm *tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(time_buf, 26, "%Y-%m-%d %H:%M:%S", tm_info);
}

ts_type calculate_mean_std_dev_range(struct segment_sketch sketch, int len) {

    ts_type mean_width = sketch.indicators[0] - sketch.indicators[1];

    ts_type stdev_upper = sketch.indicators[2];
    ts_type stdev_lower = sketch.indicators[3];

    return (len * (mean_width * mean_width + stdev_upper * stdev_upper));

}


short get_segment_start(short *points, int idx) {
    if (idx == 0)
        return 0;
    else
        return points[idx - 1];
}

short get_segment_end(short *points, int idx) {
    return points[idx];
}

int get_segment_length(short *points, int i) {

    if (i == 0)
        return points[i];
    else
        return points[i] - points[i - 1];

}
/**
 store the split points ri in short * points, -
 based on equi-length interval of segment(ts_length/segment_size) -
 id_segment == endpoint of interval in ts -
  **/
enum response calc_split_points(short *points, int ts_length, int segment_size) {

    int avg_length = ts_length / segment_size;

    for (int i = 0; i < segment_size; ++i) {
        points[i] = (short) ((i + 1) * avg_length);
    }

    //set the last one
    points[segment_size - 1] = (short) ts_length;

    return SUCCESS;
}

/**
 * split_points[i] = ri /-/ @hs_split_points stores ri and points between each ri to help for vertical split(segmentation fo a segment)
 * **/
enum response
calc_hs_split_points(short *hs_split_points, short *num_hs_split_points, short *split_points, int segment_size,
                     int min_length/*1*/) {
    short c = 0;

    for (int i = 0; i < segment_size; i++) {
        short length = split_points[i]; //i==0
        if (i > 0) {
            length = (short) (split_points[i] - split_points[i - 1]);
        }
        if (length >= min_length * 2)//>= 1*2
        {
            int start = 0;
            if (i > 0) {
                start = split_points[i - 1];
            }
            hs_split_points[c++] = (short) (start + (length / 2));
        }
        hs_split_points[c++] = split_points[i];
    }

    *num_hs_split_points = c;

    return SUCCESS;
}

/**@return  boolean node_segment_split_policy.indicator_split_idx == 0*/
boolean is_split_policy_mean(struct node_segment_split_policy policy) {

    if (policy.indicator_split_idx == 0)
        return true;
    else
        return false;
}

boolean is_split_policy_stdev(struct node_segment_split_policy policy) {

    if (policy.indicator_split_idx == 1)
        return true;
    else
        return false;


}

enum response
mean_node_segment_split_policy_split(struct node_segment_split_policy *policy, struct segment_sketch sketch,
                                     struct segment_sketch *ret) {

    const int num_splits = 2; //default split into 2 node

    ts_type max_mean = sketch.indicators[0];
    ts_type min_mean = sketch.indicators[1];

    policy->indicator_split_idx = 0;  //mean based split
    policy->indicator_split_value = (max_mean + min_mean) / 2;  //the mean value is split value

    ret[0].num_indicators = sketch.num_indicators;
    ret[1].num_indicators = sketch.num_indicators;

    for (int i = 0; i < num_splits; i++) {
        for (int j = 0; j < ret[i].num_indicators; ++j) {
            ret[i].indicators[j] = sketch.indicators[j];
        }
    }

    /* make sure indicator split idx is 0*/
    ret[0].indicators[1] = policy->indicator_split_value;
    ret[1].indicators[0] = policy->indicator_split_value;

    return SUCCESS;
}

enum response
stdev_node_segment_split_policy_split(struct node_segment_split_policy *policy, struct segment_sketch sketch,
                                      struct segment_sketch *ret) {

    const int num_splits = 2; //default split into 2 node

    ts_type max_stdev = sketch.indicators[2];
    ts_type min_stdev = sketch.indicators[3];

    policy->indicator_split_idx = 1;  //stdev based split
    policy->indicator_split_value = (ts_type) (max_stdev + min_stdev) / 2;  //the mean of stdev value is split value

    ret[0].num_indicators = sketch.num_indicators;
    ret[1].num_indicators = sketch.num_indicators;

    for (int i = 0; i < num_splits; i++) {
        for (int j = 0; j < ret[i].num_indicators; ++j) {
            ret[i].indicators[j] = sketch.indicators[j];
        }
    }

    /* make sure indicator split idx is 1*/
    ret[0].indicators[2] = policy->indicator_split_value;
    ret[1].indicators[3] = policy->indicator_split_value;

    return SUCCESS;
}

/** return hs_point_from if the hs_point_to belongs to node_point**/
short get_hs_split_point(short *points, short from, short to, int size_points) {

    short *res;

    res = (short *) bsearch(&to, points, size_points,
                            sizeof(short), reinterpret_cast<__compar_fn_t>(compare_short));


    if (res != NULL) //key found,
    {
        return from;
    } else {
        return to;
    }
}