// Copyright (c) 2017, Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory.
//
// This file is part of Caliper.
// Written by Aniruddha Marathe, marathe1@llnl.gov.
// All rights reserved.
//
// For details, see https://github.com/scalability-llnl/Caliper.
// Please also see the LICENSE file for our additional BSD notice.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the disclaimer below.
//  * Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the disclaimer (as noted below) in the documentation and/or other materials
//    provided with the distribution.
//  * Neither the name of the LLNS/LLNL nor the names of its contributors may be used to endorse
//    or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// LAWRENCE LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

///@file  shim_config.hpp 
///@brief Header file that defines the class and methods to implement configuration exploration


#ifndef SHIM_CONFIG_HPP
#define SHIM_CONFIG_HPP
#include <assert.h>
#include <string.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <omp.h>
#include <iostream>
#include <cmath>
//#include "shift.h"
//#include "shim.h"
#include "uthash.h"
//#include "cpuid.h"
//#include "msr_rapl.h"
//#include "shim.h"
//#include "globals.h"
//#include "shim_power.h"
//#include "shim_time.h"

#include "platform.hpp"
#include "task.hpp"
#define SAMPLE_MIN_THREADS 6
#define MIN_CONFIG_THREADS 9
#define MIN_CONFIG_FREQ 0 

using namespace std;

typedef struct {
	int freq;
	int nthreads;
	double elapsed_time;
	double ipc;
	double Pkg_watts;
	md5hash hashKey;
} sample_t;

class config_obj_t {
    public:
        md5hash hashKey;
        sample_t *list;
        int index;
        UT_hash_handle hh;
        sample_t *GetSample(int);
        int  GetSampleIndex();
        void IncSamples();
        void SetIndex(int);
        md5hash GetHashKey();
        void SetHashKey(md5hash);
};

//typedef struct {
//    int16_t threads;
//    int16_t freq_idx;
//    double power;
//    double elapsed_time;
//} conf_t;


class ConfigList {
    public:
        config_obj_t *taskhead;
        ConfigList();
        ~ConfigList();
        
        sample_t *GetSampleEntry(Task *, int);
};

class ShimConfig {
    public:
        ConfigList *task_list;
        sample_t *sample_map;
        int localConfig;
        int SAMPLE_MAX_THREADS;
        int myrank;
        int total_configs;
        int total_configs_per_rank;
        int config_freq;
        int config_threads;
        int optFreqIndex;
        int optThreadCount;
        int rank;
        int size;
        double cur_powerlimit;
        int sampleIter;
        int curIter;
        Task *curTask, *prevTask, *tasks;

    public:
        void thread_config_init_comp_sample(Platform *mc);
        void thread_config_pre_comp_sample(Platform *mc);
        void thread_config_post_comp_sample();
        void thread_config_post_pcontrol(Platform *mc);
        void thread_config_set_opt_config(Platform *mc, double cur_powerlimit, Task *task_ptr);
        void thread_config_set_opt_config_all_tasks(Platform *mc, double cur_powerlimit);
        conf_t *thread_config_construct_pareto_list(sample_t *sample_data, int *pareto_count);
//        int thread_config_compare_pareto_func(const void * a, const void * b);
//        void thread_config_release_comp_sample();
        void power_release_comp_sample();
        void getTask(long);
        void setTask();
        ShimConfig(Platform *mc, int );
        void slow_down_task(Platform *, int *, int *);
        void speed_up_task(Platform *, int *, int *, float, float);
        void post_mpi_init(int, int);
        void log_pareto_configs();
        void log_explored_config(int, uint16_t, float, float);
        void log_opt_config(Task *, int, uint16_t, float, float);
        void log_initial_config();
////        void set_opt_config_under_power(Platform *mc, float upowerlimit);
};

#endif
