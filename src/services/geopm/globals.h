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

///@file  globals.h
///@brief Global variables used in the GEOPM Caliper service

/* -*- Mode: C; tab-width: 2; c-basic-offset: 2 -*- */
#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include "cpuid.h"
//#include "shim.h"
#include "msr_rapl.h"
#include "uthash.h"
#include <mpi.h>

#define COMM_SLACK_THRESHOLD 0.0001
#define SLOWDOWN_THRESHOLD 0.01
#define SPEEDUP_THRESHOLD 0.001
#define THREAD_SCHEDULE_THRESHOLD 0.01
#define POWER_SCALE_UP 2

#define MAX_POWER_CAP 110.0
#define POWER_HEADROOM_FRACTION 0.25

#define SCALE_UP_STEP 2
#define SCALE_DOWN_STEP 1

#define MAX_THREADS 12

#define THREAD_STEP 1
#define FREQ_STEP 1
#define POWER_SCALE_THRESHOLD 2

//For Cab, min and max thread counts for configuration space
#define SAMPLE_MIN_THREADS 6
#define MAX_THREAD_CONFIG 4096
#define MAX_THREAD_CONFIGS_PER_RANK 64

//typedef struct {
//	int freq;
//	int nthreads;
//	double elapsed_time;
//	double ipc;
//	double Pkg_watts;
//	md5hash hashKey;
//} sample_t;

//typedef struct task_t;
//typedef struct sample_t;
//typedef struct timing_t;

typedef struct config_list_t config_list_t;
struct config_list_t {
	md5hash hashKey;
	sample_t *list;
	int index;
	UT_hash_handle hh;
};

extern config_list_t *task_list; 
extern sample_t *sample_map;
extern int total_configs;
extern int total_configs_per_rank;
extern sample_t *sample_entry;
extern int enableThreadConfig; //!@todo: could be replaced by 'threadScaling'
extern int cur_freq_idx;
extern int cur_omp_threads;

extern int config_freq;
extern int config_threads;

extern int pcontrol_level;

extern int localConfig;

extern FILE *papi_raplFile;
extern FILE *perf_file;
extern FILE *rapl_file;

extern int rapl_initialized;

extern FILE *glogFile;

extern struct rapl_data *rapl_state; 

/*------------------------------------------------------------------
	Environment options
	------------------------------------------------------------------*/

/*!@todo OpenMP threads don't go to sleep immediately; the runtime
	keeps them alive for a configurable interval after a parallel
	section. If an interval is long enough or follows a
	parallel section, it will include the active->idle transition for
	some number of threads.
*/

//!@todo these are machine-specific, single-core
//const double idlePowerPKG_w = 17.1;
//const double idlePowerPP0_w = 4.1;

//!@todo these depend on cpu frequency
//const double activePowerPKG_w = ;
//const double activePowerPP0_w = ;

extern int useRAPL;


//!@todo set these somewhere
extern int g_trace;
extern int g_algo;
extern int g_bind;
extern int binding_stable;

extern int static_cpuFreq; // flag
extern int static_cpuFreqIndex; // index

/*------------------------------------------------------------------
  End environment options
  ------------------------------------------------------------------*/

extern char hostname[100];

extern const int mem_freq;

//!@todo infer SMT from mc_config or 
//! /sys/devices/system/cpu/present and /sys/devices/system/cpu/online
extern const int SMT;

extern mcsup_nodeconfig_t mc_config;
extern int mc_config_initialized;

extern int my_core;
extern int my_local;
extern int my_socket;

extern int *omp_sockets;
extern int num_omp_sockets;

extern int optThreadCount;
extern int optFreqIndex;
extern long runCpuFreq;

//extern long curIter;
//extern int sampleIter;

extern int pcontrol;

//!@todo not thread-safe
extern timing_t globalTime;

extern timing_t collectiveTime;
extern timing_t configOverheadTime1;
extern timing_t configOverheadTime2;

extern float accum_power_time;
extern float accum_slack_time;

struct ppacket {
	float headroom;
	float power_fraction;
	float Pkg_watts;
	float rank;
};


//!@todo not thread-safe
extern task_t *tasks;
extern task_t *antepenultimateTask, *prevTask, *curTask;
extern task_t *replayTasks;
extern task_t *curReplayTask;

//!@todo not thread-safe
//extern req_t *reqMap;

// for mapping log requests to live requests
//extern reqMatch_t *replayReqMap;

extern MPI_Request *curReqs;
extern int curReqsLength; // number elements, not bytes
extern int curReqsSpace; // number elements, not bytes

/* potential column names: start duration name size dest src tag
	 comm hash flags reqs OMP_NUM_THREADS cpuFreq
*/
extern const char * const replayColNames[];

struct replayColsPresent_s {
	unsigned start:1;
	unsigned duration:1;
	unsigned name:1;
	unsigned size:1;
	unsigned dest:1;
	unsigned src:1;
	unsigned tag:1;
	unsigned comm:1;
	unsigned hash:1;
	unsigned flags:1;
	unsigned reqs:1;
	unsigned OMP_NUM_THREADS:1;
	unsigned cpuFreq:1;
};

extern struct replayColsPresent_s replayColsPresent;
extern FILE *logFile;
extern float upowerlimit;

#ifdef POWER_SCHEDULE
extern int g_power_thread_run;
extern pthread_t t_handle;

extern int cp_ranks;
extern float cur_powerlimit;
// per-rank power limit in watts

// microsecond interval between power samples in the power thread
extern long usecinterval;

extern float collThreshold;
extern int collcount; 
extern int powerSampling;
extern int powerBalancing;
extern int outputThreadConfig;
extern FILE *powerFile;
extern int resetAdagio;
//extern int threadScaling;
extern int enableAdagio;

// protects slackAccum 
extern pthread_mutex_t powerMutex;

// protected by powerMutex
extern double slackAccum;

extern const double slackThreshold;

extern volatile int g_all_reached_finalize; /* Counter for all MPI ranks */
extern volatile int g_reached_finalize;   /* Flag to indicate local allreduce state */
extern volatile MPI_Comm g_dup_comm;    /* communicator for power-allreduce */

extern int maxNumThreads; /* Maximum number of threads defined by the user */



#endif

#endif
