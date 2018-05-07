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

///@file  platform.hpp
///@brief Defines the interface to the underlying platform

#ifndef PLATFORM_HPP
#define PLATFORM_HPP
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
//#include "uthash.h"
#include "cpuid.h"
#include "msr_rapl.h"
//#include "shim.h"
//#include "globals.h"
//#include "shim_power.h"
//#include "shim_time.h"

//#define SAMPLE_MIN_THREADS 8
//#define MIN_CONFIG_THREADS 8
//#define MIN_CONFIG_FREQ 15

using namespace std;

#define MAX_POWER_CAP_NONARCH 150 

#include "md5.h"

typedef struct {
  md5_byte_t hash[16];
} md5hash;

void hash_backtrace(int fid, md5hash *digest);

class Platform {
    public:
        int *omp_sockets;
        int useRAPL;
        int rapl_initialized;
        mcsup_nodeconfig_t mc_config;
        int mc_config_initialized;
        int MAX_THREADS;
        int cur_omp_threads;
        int num_omp_sockets;
        float upowerlimit;
        int static_cpuFreq;
        int static_cpuFreqIndex;
        uint64_t * rapl_flags;
        struct rapl_data *rapl_state;
        int powerBalancing;
        int enableAdagio;
        int powerSampling;
        int enableThreadConfig;
        long int optfreq;
        double powercap;
        int powerrun;

    public:
        void pre_mpi_init();
        void post_mpi_init();
        void post_mpi_finalize();
        
        int get_max_threads();
        int get_num_threads();
        void set_cur_freq(int freqidx);
        void set_pkg_power(float power);
        void set_num_threads(int numthreads);
        int get_slowest_freq();
        int get_fastest_freq();
        void populate_thread_info();
        void set_opt_config();
        void get_aperf_mperf(int core, uint64_t *aperf, uint64_t *mperf);
        void get_rdtsc(int core, uint64_t *tsc); 
        long get_freq_by_idx(int);
        void get_joules(int, uint64_t *, uint64_t *, uint64_t *);
        int mc_get_cpuid(int *, int *, int *);
        int get_num_sockets();
        int * get_omp_sockets();
        int get_omp_socket_idx(int);
        Platform(int pbalance, int enable_adagio, int psampling, int threadconf, long int opt_freq, double pcap, int prun) { 
            powerBalancing = pbalance; 
            enableAdagio = enable_adagio;
            powerSampling = psampling;
            enableThreadConfig = threadconf;
            optfreq = opt_freq;
            powercap = pcap;
            powerrun = prun;
        }
};
#endif

