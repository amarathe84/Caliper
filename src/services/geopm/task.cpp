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

///@file  task.cpp
///@brief Class to define attributes and methods to define task-level configuration parameters
/* -*- Mode: C; tab-width: 2; c-basic-offset: 2 -*- */

extern "C" { 
    double estPower(int SMT, int threads, int cpu_freq, int mem_freq);
//    void SetPKG_Power_Limit(int socket, double pkg_lim);
//    void get_rdtsc(int, unsigned long*);
//    void get_aperf_mperf(int, unsigned long*, unsigned long*);
    #include "uthash.h"
//    void read_papi(long long *values);
//    #include "shift.h"
//    #include "debug.h"
    #include "cpuid.h"
}

//#include "shim_config.hpp"
#include <assert.h>
#include <math.h>
//#include "shim_power.h"
//#include "shim_time.h"
//#include "shim_enumeration.h"
#include "task.hpp"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "power_model.hpp"
#include "debug.h"

double
delta_seconds(const struct timeval *s, const struct timeval *e){
	return (double) ( (e->tv_sec - s->tv_sec) + (e->tv_usec - s->tv_usec)/1000000.0 );
}

double
delta_seconds_ts(const struct timespec *s, const struct timespec *e){
  return (double)((e->tv_sec - s->tv_sec) + 
		  (e->tv_nsec - s->tv_nsec)/1000000000.0);
}

//double
//delta_seconds(const struct timeval s, const struct timeval e){
//	return (double) ( (e.tv_sec - s.tv_sec) + (e.tv_usec - s.tv_usec)/1000000.0 );
//}
//
//double Task::
//delta_seconds_ts(const struct timespec *s, const struct timespec *e){
//  return (double)((e->tv_sec - s->tv_sec) + 
//		  (e->tv_nsec - s->tv_nsec)/1000000000.0);
//}


////// Get cpu frequency in MHz.  Uses core 0.
////int Task::getCPU_freq(){
////	int status = 
////		get_cpuid(&mc_config, &mc_config_initialized, &my_core, &my_socket, &my_local);
////	char filename[255];
//////	filename[254] = 0;
//////	FILE *file = 
//////		fopen(filename, "r");
////	int cpu_freq = 15;
//////	status = fscanf(file, "%d", &cpu_freq);
//////	fclose(file);
//////	cpu_freq = cpu_freq / 1000;
//////	dlog("cpu freq: %d", cpu_freq);
//////	if(!cpu_freq)
//////		wlog("CPU freq == 0!");
////	return cpu_freq;
////}


void Task::print(){
	dlog(
//        "%s\t"  // name
//			 "%d\t"  // size
//			 "%d\t"  // dest
//			 "%d\t"  // src
//			 "%d\t"  // tag
//			 "%p\t"  // comm
			 "0x%x\t"// hash
			 "0x%x", // flags
//			 fnName(ID),
//			 msgSize,
//			 msgDest,
//			 msgSrc,
//			 tag,
//			 comm,
			 hashKey,
			 iFlags
		);
}

void Task::log_time_sample(Platform *mc, int rank) {
    if(rank > -1) {
        int exists = 1;
        char *path = getenv("dataDir");
        if(!path) path = ".";
        char filename[1000];
        snprintf(filename, 1000, "%s/task.%d.dat\0", path, rank);
        struct stat buf1;
        if(stat(filename, &buf1) != 0) {
            exists = 0;
}
        
        FILE *fp = fopen(filename, "a+");
        if(!exists) {
            fprintf(fp, "task time threads freq pkgwatt pp0watt dramwatt\n");
        } 
        fprintf(fp, "%p %0.6lf %d %d %0.6lf %lf %lf\n", (unsigned int *) &(*this), time.elapsed_time, mc->get_num_threads(), time.cpu_freq, time.Pkg_watts, time.PP0_watts, time.DRAM_watts);
            
        fclose(fp);
    }
}

void Task::mark_time(Platform *mc, int rank, int start_stop) {
#ifndef OMPT_DISABLED
//	if(inParallel){
//		elog("MPI in OpenMP!");
//	}
#endif

//    wlog("**** 1");
    //CPP: needs to be its own class such as 'system'
    int my_core;
    int my_socket;
    int my_local;
    mc->mc_get_cpuid(&my_core, &my_socket, &my_local);
//    wlog("  **** 2");

	if(start_stop){ // start
		gettimeofday(&time.start, 0);
//		get_rdtsc(0, &time.tsc_start);
//		wlog("tsc start: %llu, mc->rapl_initialized:%d",time.tsc_start,mc->rapl_initialized);
//		if(mc->rapl_initialized){
//            wlog("------ 1");
/// -----> put this back			get_aperf_mperf(my_core, &time.aperf_start, &time.mperf_start);
#ifdef _DEBUG
////			if(omp_sockets){
////				int core;
////				for(core = 0; core < mc_config.cores_per_socket; core++){
////					get_aperf_mperf(mc_config.map_socket_to_core[omp_sockets[0]][core], 
////													time.aperfs_start+core,time.mperfs_start+core);
////					get_rdtsc(mc_config.map_socket_to_core[omp_sockets[0]][core], // tsc
////									time.tscs_start+core);
////				}
////			}
#endif
//		}
//		if(envPAPI_EVENTS)
//			read_papi(&time.events_start);
		if(//mc->rapl_initialized 
            mc->get_omp_sockets()){
//            if(rank == 0) wlog("------ 2");
			// measure all relevant sockets and sum their per-domain powers
			int socket_index;
			//!@todo if we don't correctly segregate two ranks running on
			//!the same node, this will report incorrect power for each
			//!rank.
//            if(rank == 0) wlog("------ 3");
            int omp_socket = 0;
			for(socket_index = 0; socket_index < mc->get_num_sockets(); socket_index++){
//                if(rank == 0) wlog("------ 4");
				omp_socket = mc->get_omp_socket_idx(socket_index);
				// write raw joules to rapl_state, then copy to local timing_t
                read_rapl_data();
                get_rapl_joules(omp_socket,   &time.power[omp_socket].Pkg_joules_start,
                                              &time.power[omp_socket].DRAM_joules_start,
                                              &time.power[omp_socket].PP0_joules_start);
                dlog("OMP: PKG raw j: %llu, PP0 raw j: %llu, DRAM raw j: %llu",
                         time.power[omp_socket].Pkg_joules_start,
                         time.power[omp_socket].PP0_joules_start,
                         time.power[omp_socket].DRAM_joules_start);
			}
//            if(rank == 0) wlog("Start DRAM 1: %lf", time.power[omp_socket].DRAM_joules_start);
		} else if(mc->rapl_initialized){
			// measure all sockets
			int socket_index;
			for(socket_index = 0; socket_index < mc->get_num_sockets(); socket_index++){
				// write raw joules to rapl_state, then copy to local timing_t
                read_rapl_data();
                get_rapl_joules(socket_index,   &time.power[socket_index].Pkg_joules_start,
                                                &time.power[socket_index].DRAM_joules_start,
                                                &time.power[socket_index].PP0_joules_start);
                dlog("Non-OMP: PKG raw j: %llu, PP0 raw j: %llu, DRAM raw j: %llu",
                         time.power[socket_index].Pkg_joules_start,
                         time.power[socket_index].PP0_joules_start,
                         time.power[socket_index].DRAM_joules_start);

			}
//            if(rank == 0) wlog("Start DRAM 2 : %lf", time.power[socket_index].DRAM_joules_start);
		}
	} else { // stop
//        wlog("      **** 3");
		gettimeofday(&time.stop, 0);
//		get_rdtsc(0, &time.tsc_stop);
//		dlog("tsc stop: %llu",time.tsc_stop);
		if(mc->rapl_initialized)
///// -------> put this back			get_aperf_mperf(my_core, &time.aperf_stop, &time.mperf_stop);
//        wlog("          **** 4");
//		if(envPAPI_EVENTS)
//			read_papi(&time.events_stop);
		time.elapsed_time = delta_seconds(&time.start, &time.stop);

		//!@todo handle overflow for tsc, mperf, and aperf. Check counter widths!
//		time.tsc_delta =time.tsc_stop -time.tsc_start;
		if(mc->rapl_initialized){
			time.aperf_delta =time.aperf_stop -time.aperf_start;
			time.mperf_delta =time.mperf_stop -time.mperf_start;
			time.c0 = 0; //time.mperf_delta/(double)time.tsc_delta;
			time.ratio =time.aperf_delta/(double)time.mperf_delta;
		}

		time.cpu_freq = mc->get_freq_by_idx(my_core);
//        wlog("            **** 5");

		//!@todo configurable time threshold
		if(mc->get_omp_sockets() && mc->get_num_sockets() > 1 && time.parent)
			time.parent->flags.multiSocket = 1;
		if(//mc->rapl_initialized && 
            mc->get_omp_sockets() && time.elapsed_time > .005){
//            wlog("            **** 5");
			int socket_index;
			if(time.parent)
				time.parent->flags.estPower = 0;
			time.Pkg_watts =time.PP0_watts =time.DRAM_watts = 0.0;
//            wlog("              **** 6: _num_sockets:%d", mc->get_num_sockets());
            int omp_socket;
			for(socket_index = 0; socket_index < mc->get_num_sockets(); socket_index++){
				omp_socket = mc->get_omp_socket_idx(socket_index);
                //get_rapl_data(omp_socket, &time.Pkg_watts, &time.DRAM_watts, &time.PP0_watts);
//                wlog("                  **** 7: omp_socket:%d", omp_socket);
                read_rapl_data();
                get_rapl_joules(omp_socket,  &time.power[omp_socket].Pkg_joules_stop,
                                             &time.power[omp_socket].DRAM_joules_stop,
                                             &time.power[omp_socket].PP0_joules_stop);

                time.power[omp_socket].Pkg_joules =
                                                 time.power[omp_socket].Pkg_joules_stop -
                                                 time.power[omp_socket].Pkg_joules_start;

                time.power[omp_socket].PP0_joules =
                                                 time.power[omp_socket].PP0_joules_stop -
                                                 time.power[omp_socket].PP0_joules_start;

                time.power[omp_socket].DRAM_joules =
                                                 time.power[omp_socket].DRAM_joules_stop -
                                                 time.power[omp_socket].DRAM_joules_start;

                time.Pkg_watts  += time.power[omp_socket].Pkg_joules /time.elapsed_time;
                time.PP0_watts  += time.power[omp_socket].PP0_joules /time.elapsed_time;
                time.DRAM_watts += time.power[omp_socket].DRAM_joules/time.elapsed_time;

                dlog("End: PKG raw j: %llu, PP0 raw j: %llu, DRAM raw j: %llu",
                         time.power[omp_socket].Pkg_joules_start,
                         time.power[omp_socket].PP0_joules_start,
                         time.power[omp_socket].DRAM_joules_start);

                dlog("End: PKG watts: %0.5lf, DRAM watts: %0.5lf",
                         time.Pkg_watts,
                         time.DRAM_watts);
			}
//            if(rank == 0) wlog("End DRAM: %lf", time.power[omp_socket].DRAM_joules_start);
//            if(rank == 0) wlog("------Pkg_watts: %lf, time.DRAM_watts:%lf", time.Pkg_watts, time.DRAM_watts);
	    } else { //Model power
			int threads;
			if(time.parent){
				time.parent->flags.estPower = 1;
				threads = time.parent->flags.threads;
			} else
				threads = lround(ceil(time.parallel_thread_time / time.parallel_time));
			//! this is only valid for Intel(R) Xeon(R) CPU E5-2670.
			double serialPower = // 67.3
				estPower(SMT, 1, time.cpu_freq, time.mem_freq);
			if(time.parallel_time){
				//!@todo treat sockets separately
				double parallelPower = 
					estPower(SMT, threads, time.cpu_freq, time.mem_freq);
				double serialTime = time.elapsed_time - time.parallel_time;
//				dlog("total time: %e, serial power: %e, serial time: %e, "
//						 "parallel power: %e, parallel time: %e",
//						 time.elapsed_time,
//						 serialPower, serialTime, parallelPower, time.parallel_time);
				time.Pkg_watts =
					(serialTime * serialPower + time.parallel_time * parallelPower) /
					time.elapsed_time;
                time.Pkg_watts = time.Pkg_watts / 1000.0f;
			} else
				time.Pkg_watts = serialPower / 1000.0f;
        }
        log_time_sample(mc, rank);
    }
}

void Task::accumulate_slack(double *slackAccum, float slackThreshold) {
//    if(ID != GMPI_Init_thread && ID != GMPI_Init){
//        pthread_mutex_lock(&powerMutex);
        *slackAccum +=
            max(0, delta_seconds(&time.start, &time.stop) -
                    slackThreshold);
//        pthread_mutex_unlock(&powerMutex);
//    }
}
