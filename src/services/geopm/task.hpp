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

///@file  task.hpp
///@brief Class to define attributes and methods to define task-level configuration parameters

#ifndef TASK_HPP
#define TASK_HPP
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

#include <mpi.h>
#include "platform.hpp"
//#include "stacktrace.h"
//#include "shim_enumeration.h"
#define START_TIMER 1
#define STOP_TIMER 0
#define max(a,b) (a > b ? a : b)
#define min(a,b) (a < b ? a : b)

#define MAX_SOCKETS 2
using namespace std;


void get_rdtsc(int cpu, uint64_t *tsc);
void get_aperf_mperf(int core, uint64_t *aperf, uint64_t *mperf);

class Task;

struct task_t;

typedef struct {
	uint64_t Pkg_joules_start, PP0_joules_start, DRAM_joules_start;
	uint64_t Pkg_joules_stop, PP0_joules_stop, DRAM_joules_stop;
	double Pkg_joules, PP0_joules, DRAM_joules;
} power_t;

typedef struct {
	struct timeval start, stop;
	uint64_t aperf_start, aperf_stop, aperf_delta,
		mperf_start, mperf_stop, mperf_delta,
		tsc_start, tsc_stop, tsc_delta;

#ifdef _DEBUG
	uint64_t aperfs_start[16], mperfs_start[16], tscs_start[16];
	uint64_t aperfs_stop[16], mperfs_stop[16], tscs_stop[16];
#endif

	int cpu_freq; // in MHz
    int mem_freq;

	//!@todo rewinding will render these invalid.
	double parallel_time, parallel_thread_time;

	power_t power[MAX_SOCKETS];
	double Pkg_watts, PP0_watts, DRAM_watts; // calculated over omp_sockets

	double ratio, elapsed_time, c0;
//	long long events_start[MAX_PAPI_EVENTS],
//		events_stop[MAX_PAPI_EVENTS];
	Task *parent;
} Timing_t;

typedef struct {
	int16_t threads;
	int16_t freq_idx;
	double power;
	double elapsed_time;
} conf_t;

typedef struct {
	struct timeval start, stop;
	uint64_t aperf_start, aperf_stop, aperf_delta,
		mperf_start, mperf_stop, mperf_delta,
		tsc_start, tsc_stop, tsc_delta;

#ifdef _DEBUG
	uint64_t aperfs_start[16], mperfs_start[16], tscs_start[16];
	uint64_t aperfs_stop[16], mperfs_stop[16], tscs_stop[16];
#endif

	int cpu_freq; // in MHz

	//!@todo rewinding will render these invalid.
	double parallel_time, parallel_thread_time;

	power_t power[MAX_SOCKETS];
	double Pkg_watts, PP0_watts, DRAM_watts; // calculated over omp_sockets

	double ratio, elapsed_time, c0;
	task_t *parent;
} timing_t;

struct task_t {
	timing_t time;

	long ID; // -1 for computation
	
	// MPI only

//	int msgSize; // number of elements!
//	int msgSrc, msgDest;
//	int tag;
//	//!@todo find a portable representation for communicators
//	MPI_Comm comm;

	union {
		struct {
			// bit 0
			int omp:1;
			// bit 1...
			//!@todo this cannot handle tasks with multiple frequencies
			int spin:1;
			// bit 2
			int estPower:1;
			// bit 3
			int multiSocket:1;
			// bit 4
			int threads:8;
			// bit 12
			int newComm:1;
			// bit 13
		} flags;
		int iFlags;
	};

	MPI_Request *reqs;
	int *reqIndices;
	int reqCount;

	int numthreads;

	MPI_Status *stats;

	// end MPI only

	// task that followed this task last time
	task_t *next;

	md5hash hashKey;

	conf_t *frontier;
	int frontier_length;
    int optConfig;
    
    int preOptConfig;
    timing_t prevTime;

	int curConfigThreads;
	int curConfigFreq;

	UT_hash_handle hh;
};




class Task {
    public:
        Timing_t time;
        long ID; // -1 for computation
        
        // MPI only
        
        int msgSize; // number of elements!
        int msgSrc, msgDest;
        int tag;
        //!@todo find a portable representation for communicators
        MPI_Comm comm;
        int SMT;
        
        union {
        	struct {
        		// bit 0
        		int omp:1;
        		// bit 1...
        		//!@todo this cannot handle tasks with multiple frequencies
        		int spin:1;
        		// bit 2
        		int estPower:1;
        		// bit 3
        		int multiSocket:1;
        		// bit 4
        		int threads:8;
        		// bit 12
        		int newComm:1;
        		// bit 13
        	} flags;
        	int iFlags;
        };
        
        MPI_Request *reqs;
        int *reqIndices;
        int reqCount;
//        pthread_mutex_t powerMutex;
        
        int numthreads;
        
        MPI_Status *stats;
        
        // end MPI only
        
        // task that followed this task last time
        Task *next;
        
        md5hash hashKey;
        
        conf_t *frontier;
        int frontier_length;
        int optConfig;
        
        int preOptConfig;
        timing_t prevTime;
        
        int curConfigThreads;
        int curConfigFreq;
        
        UT_hash_handle hh;


        void log_time_sample(Platform *mc, int rank);
        void print();
        void mark_time(Platform *, int, int);
        int getCPU_freq();
        double delta_seconds_ts(const struct timespec*,const struct timespec*);
        void accumulate_slack(double *, float);
        Task() { 
            SMT = 0; 
        }

};


#endif
