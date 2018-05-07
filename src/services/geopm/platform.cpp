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

///@file  platform.cpp 
///@brief Defines the interface to the underlying platform


extern "C" { 
    void SetPKG_Power_Limit(int socket, double pkg_lim);
    void get_rdtsc(int, unsigned long*);
    void get_aperf_mperf(int, unsigned long*, unsigned long*);
    void read_papi(long long *values);
    #include "shift.h"
//    #include "debug.h"
    #include "cpuid.h"
    #include "msr_rapl.h"
    #include "msr_core.h"
    //#include "msr_common.h"
    #include "msr_clocks.h"
    #include "msr_turbo.h"

    extern int eventCount;
    
//    void start_papi(void);
//    void read_papi(long long *values);
//    void stop_papi(long long *values);
//    void wpapi_init();
    
    #define papi_check(fc) if((rc = (fc)) != PAPI_OK) elog(#fc": %d; %s", rc, PAPI_strerror(rc))
    #define papi_check_val(fc, val) if((rc = (fc)) != (val)) elog(#fc": %d != %d; %s", rc, val, PAPI_strerror(val))
    
}

//#include "globals.h"
//#include "shim_time.h"
#include <omp.h>
#include <libunwind.h>
#include "platform.hpp"
#include "md5.h"
#define UNW_LOCAL_ONLY

void hash_backtrace(int fid, md5hash *digest) {
	unw_cursor_t cursor; unw_context_t uc;
	unw_word_t ip;
	//, sp;

	md5_state_t pms;
	int status;

	md5_init(   &pms );

	status = unw_getcontext(&uc);
	if(status != 0)
	  fprintf(stderr, "unw_getcontext() error\n");

	status = unw_init_local(&cursor, &uc);
	if(status != 0)
	  fprintf(stderr, "unw_getcontext() error\n");

	while ((status = unw_step(&cursor)) > 0) {
	  status = unw_get_reg(&cursor, UNW_REG_IP, &ip);
	  if(status != 0)
	    fprintf(stderr, "unw_getcontext() error\n");

	  /*
	  status = unw_get_reg(&cursor, UNW_REG_SP, &sp);
	  if(status != 0)
	    fprintf(stderr, "unw_getcontext() error\n");
	  */

	  md5_append( &pms, (md5_byte_t *)(&ip), sizeof(unw_word_t) );
	  //md5_append( &pms, (md5_byte_t *)(&sp), sizeof(unw_word_t) );
	  status = 0;
	}
	if(status < 0){
		fprintf(stderr, "unw_step() error: %d, ",
			status);
		switch(status){
		case UNW_EUNSPEC:
			fprintf(stderr, "UNW_EUNSPEC\n");
			break;
		case UNW_ENOINFO:
			fprintf(stderr, "UNW_ENOINFO\n");
			break;
		case UNW_EBADVERSION:
			fprintf(stderr, "UNW_EBADVERSION\n");
			break;
		case UNW_EINVALIDIP:
			fprintf(stderr, "UNW_EINVALIDIP\n");
			break;
		case UNW_EBADFRAME:
			fprintf(stderr, "UNW_EBADFRAME\n");
			break;
		case UNW_ESTOPUNWIND:
			fprintf(stderr, "UNW_ESTOPUNWIND\n");
			break;
		default:
			fprintf(stderr, "unknown\n");
			break;
		}
	}

	//! @todo this should be unnecessary
	md5_append( &pms, (md5_byte_t*)(&fid), sizeof(fid) );
	md5_finish( &pms, digest->hash );
}


void SetPKG_Power_Limit(int socket, double pkg_lim) {
    struct rapl_limit pkg_lim1[2];
    struct rapl_limit pkg_lim2[2];

    pkg_lim1[socket].watts = pkg_lim;
    pkg_lim1[socket].seconds = 0.001f;
    pkg_lim1[socket].bits = 0;
    pkg_lim2[socket].watts = pkg_lim;
    pkg_lim2[socket].seconds = 0.001f;
    pkg_lim2[socket].bits = 0;
    set_pkg_rapl_limit(socket, &pkg_lim1[socket], &pkg_lim2[socket]);
}

void Platform::pre_mpi_init() {
    rapl_flags = 0;

//    wlog("--- 1");
////     char *opt = getenv("RAPL");
////     if(opt && !strcmp(opt, "0"))
////         useRAPL = 0;
////     if(init_msr()) {
////         elog("Couldn't initialize MSR interface.\n");
////         exit(0);
////     }
////     if(useRAPL){
//// //      raplFile = initialize_logfile(&raplPtr, &raplSize);
////         rapl_init(&rapl_state, &rapl_flags);
////         rapl_initialized = 1;
////     }
////     else {
////         dlog("RAPL disabled");
////     }
    populate_thread_info();
//    wlog("--- 2");
}

void Platform::populate_thread_info() { 
    //!@todo the socket selection will change if we alter the process or
    //thread mapping dynamically
        int numThreads = omp_get_max_threads();
        int *t_sockets = (int*)malloc(numThreads * sizeof(int));
        int i;

//        elog("OMP init");
        // dummy loop to set up thread affinity if applicable
#pragma omp parallel for private(i)
        for(i = 0; i < numThreads; i++){
            volatile int j = i;
            j++;
        }

#pragma omp parallel for private(i)
        for(i = 0; i < numThreads; i++)
            get_cpuid(&mc_config, &mc_config_initialized, 0, t_sockets + i, 0);
#pragma omp parallel
        {
            cpu_set_t mask;
            sched_getaffinity(0, sizeof(mask), &mask);
//          elog("thread %d socket %d affinity 0x%x",
//                   omp_get_thread_num(),
//                   t_sockets[omp_get_thread_num()],
//                   *((int*)&mask));
        }

        //!@todo: Check if hyperthreading is enabled.
        //Current setting is for system with hyperthreading enabled
        MAX_THREADS = mc_config.cores_per_socket/2;
//        SAMPLE_MAX_THREADS = mc_config.cores_per_socket/2;
        cur_omp_threads = MAX_THREADS;

        // figure out which sockets we need to measure
        int *sockets = (int*)calloc(mc_config.sockets, sizeof(int));
        for(i = 0; i < numThreads; i++)
            sockets[t_sockets[i]]++;
        num_omp_sockets = 0;
        for(i = 0; i < mc_config.sockets; i++)
            if(sockets[i])
                num_omp_sockets++;

        omp_sockets = (int*)malloc(num_omp_sockets * sizeof(int));
        num_omp_sockets = 0;
        for(i = 0; i < mc_config.sockets; i++)
            if(sockets[i])
                omp_sockets[num_omp_sockets++] = i;
        free(sockets);
        free(t_sockets);
}

//Platform::Platform() {
//}

void Platform::set_opt_config() {
    {
        // get cpu frequency settings from environment in kHz, apply to all cores
        shift_parse_freqs();
        int socket;
//      !@todo: LIBMOD Turn this back on once everything is fixed
//      for(socket = 0; socket < num_omp_sockets; socket++)
//          disable_turbo();
//          //disable_turbo(omp_sockets[socket]); //#LIBMOD -- old disable_turbo

        if(powercap != 0.0f && optfreq != 0){
//            elog("Setting both static CPU frequency and RAPL power cap not supported.");
            //MPI_Abort(MPI_COMM_WORLD, 1);
        }
        if(powercap){
            upowerlimit = powercap;
            if(upowerlimit <= 0 || upowerlimit > MAX_POWER_CAP_NONARCH){
//                wlog("Power limit out of expected range: %f", upowerlimit);
            } else {
                if(powerBalancing) {
//                    wlog("Setting power cap to %fw", upowerlimit);
                    set_pkg_power(upowerlimit);
                }
            }
//            if(powerruns) {
//                upowerlimit += 10.0f;
//            }
        } else if(optfreq){
            static_cpuFreq = 1;

            // find closest frequency
            int targetFreq_kHz = optfreq;
            static_cpuFreqIndex = shift_get_freq_index(targetFreq_kHz);
        }
        if(!optfreq)
            static_cpuFreqIndex = FASTEST_FREQ;
    }
    //!@note: We shift to the fastest frequency even when RAPL clamping
    //!is enabled. This is to avoid accidentally constraining RAPL to
    //!low frequencies.
    set_cur_freq(static_cpuFreqIndex);
}


void Platform::post_mpi_init() {
    if(init_msr()) {
//        elog("Couldn't initialize MSR interface.\n");
        exit(0);
    }
    rapl_init(&rapl_state, &rapl_flags);
    rapl_initialized = 1;


    // LC systems don't allow writing to anything except setspeed, so
    // we can't call shift_init_socket in its current form
    shift_initialized = 1;
    set_opt_config();

}

void Platform::post_mpi_finalize() {
    int core, socket;
    get_cpuid(&mc_config, &mc_config_initialized, &core, &socket, 0);
    for(socket = 0; socket < mc_config.sockets; socket++)
        shift_socket(socket, FASTEST_FREQ);

}

void Platform::set_cur_freq(int freqidx) {
    int socket;
    for(socket = 0; socket < num_omp_sockets; socket++)
        shift_socket(omp_sockets[socket], static_cpuFreqIndex);
}

void Platform::set_pkg_power(float power) {
    int socket;
    for(socket = 0; socket < num_omp_sockets; socket++)
        SetPKG_Power_Limit(omp_sockets[socket], power);
}

void Platform::set_num_threads(int numthreads) {
    omp_set_num_threads(numthreads);
}

int Platform::get_max_threads() {
    return MAX_THREADS;
}

int Platform::get_num_threads() {
    return omp_get_num_threads();
}

int Platform::get_slowest_freq() {
    return SLOWEST_FREQ;
}

int Platform::get_fastest_freq() {
    return FASTEST_FREQ;
}

void Platform::get_aperf_mperf(int core, uint64_t *aperf, uint64_t *mperf) {
    get_aperf_mperf(mc_config.map_socket_to_core[omp_sockets[0]][core],
            aperf+core, mperf+core);
}

void Platform::get_rdtsc(int core, uint64_t *tsc) {
// !@todo: call the version of this function from the platform    
//    get_rdtsc(mc_config.map_socket_to_core[omp_sockets[0]][core],
//            tsc+core);
    
}

long Platform::get_freq_by_idx(int freq) {
    return freqs[freq];
}

void Platform::get_joules(int s, uint64_t *pkg, uint64_t *dram, uint64_t *pp0) {
    get_rapl_joules(s, pkg, dram, pp0);
}

int Platform::mc_get_cpuid(int *my_core, int *my_socket, int *my_local) {
    return get_cpuid(&mc_config, &mc_config_initialized, my_core, my_socket, my_local);
}

int Platform::get_num_sockets() {
    return num_omp_sockets;
}

int * Platform::get_omp_sockets() {
    return omp_sockets; 
}

int Platform::get_omp_socket_idx(int idx) {
    return omp_sockets[idx];
}
