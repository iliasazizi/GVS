//
// Created by iazizi on 28‏/5‏/2021.
//

#ifndef TESTS_PTK_H
#define TESTS_PTK_H
#include <chrono>
namespace PTK {

    class Timer {
    private:
        std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<double>> start_point;
        std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<double>> end_point;
    public:
        Timer() {
            start_point = std::chrono::high_resolution_clock::now();
        }
        void printElapsedTime(const char * taskname){
            auto t = getElapsedTime();
            std::cerr << "[TIME|" <<taskname<< "]  ==>  "<<t<<std::endl;
        }
        double getElapsedTime(){
            end_point = std::chrono::high_resolution_clock::now();
            auto elapsed = end_point - start_point;
            return elapsed.count();
        }
        void restart(){
            start_point = std::chrono::high_resolution_clock::now();
        }
    };
    typedef struct querying_stats{
        long distance_computations_bsl;
        long distance_computations_hrl;
        long num_hops_hrl;
        long num_hops_bsl;
        long distance_computations_lb;
        double time_cnmd;
        double time_leaves_search;
        double time_update_knn;
        double time_routing;
        double time_layer0;
        double time_pq;
        querying_stats()
        {
            num_hops_bsl=0;distance_computations_hrl=0;num_hops_hrl=0;distance_computations_bsl=0;
            time_cnmd=0;time_update_knn=0;time_leaves_search=0;time_routing=0;time_layer0=0;time_pq=0;saxdist_computations_bsl=0;saxdist_computations_hsl=0;
        }


        long saxdist_computations_bsl;
        long saxdist_computations_hsl;
    } querying_stats;
}
#endif //TESTS_PTK_H
