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

///@file  shim_time.h
///@brief Timing and other accounting services for the GEOPM Caliper service

/* -*- Mode: C; tab-width: 2; c-basic-offset: 2 -*- */
#ifndef SHIM_TIME_H
#define SHIM_TIME_H

#include <sys/time.h>	// struct timeval
#include "globals.h"
#include "debug.h"
//#include "msr_common.h" // rdtsc
#include "msr_clocks.h"
//#include "shim_omp.h"
//#include "wpapi.h"

static inline double
delta_seconds(const struct timeval *s, const struct timeval *e){
	return (double) ( (e->tv_sec - s->tv_sec) + (e->tv_usec - s->tv_usec)/1000000.0 );
}

static inline double
delta_seconds_ts(const struct timespec *s, const struct timespec *e){
  return (double)((e->tv_sec - s->tv_sec) + 
		  (e->tv_nsec - s->tv_nsec)/1000000000.0);
}

// Get cpu frequency in MHz.  Uses core 0.
static inline int getCPU_freq(){
	int status = 
		get_cpuid(&mc_config, &mc_config_initialized, &my_core, &my_socket, &my_local);
	char filename[255];
//	snprintf(filename, 255, 
//					 "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_setspeed", my_core);
//	filename[254] = 0;
//	FILE *file = 
//		fopen(filename, "r");
//	if(!file){
//		elog("failed to open scaling_setspeed");
//		return 0;
//	}
	int cpu_freq = 8;
//	status = fscanf(file, "%d", &cpu_freq);
//	fclose(file);
//	if(status != 1){
//		elog("failed to read scaling_setspeed: %d", status);
//		return 0;
//	}
//	cpu_freq = cpu_freq / 1000;
//	dlog("cpu freq: %d", cpu_freq);
//	if(!cpu_freq)
//		wlog("CPU freq == 0!");
	return cpu_freq;
}

/*! start = 1, stop = 0 
	 @todo don't need gettimeofday if tsc rate is constant.
	 @todo get mperf, aperf, temperature

	 @todo only call mark_time once for each task.  This will prevent us
	 from measuring overhead time, but may provide more accurate power
	 estimates.

	 calculate aperf/mperf ratio and corresponding frequency, elapsed time
	 @todo deal with counter overflow; see turbostat code
	 //!@todo finish aperf/mperf
*/
static inline void mark_time(timing_t *t, int start_stop){
	assert(t);

//#ifndef OMPT_DISABLED
//	if(inParallel){
//		elog("MPI in OpenMP!");
//	}
//#endif
	
	get_cpuid(&mc_config, &mc_config_initialized, &my_core, &my_socket, &my_local);

	if(start_stop){ // start
		gettimeofday(&t->start, 0);
		get_rdtsc(0, &t->tsc_start);
		dlog("tsc start: %llu", t->tsc_start);
		if(rapl_initialized){
			get_aperf_mperf(my_core, &t->aperf_start, &t->mperf_start);
#ifdef _DEBUG
			if(omp_sockets){
				int core;
				for(core = 0; core < mc_config.cores_per_socket; core++){
					get_aperf_mperf(mc_config.map_socket_to_core[omp_sockets[0]][core], 
													 t->aperfs_start+core, t->mperfs_start+core);
					get_rdtsc(mc_config.map_socket_to_core[omp_sockets[0]][core], // tsc
									 t->tscs_start+core);
				}
			}
#endif
		}
		if(rapl_initialized && omp_sockets){
			// measure all relevant sockets and sum their per-domain powers
			int socket_index;
			//!@todo if we don't correctly segregate two ranks running on
			//!the same node, this will report incorrect power for each
			//!rank.
			for(socket_index = 0; socket_index < num_omp_sockets; socket_index++){
				int omp_socket = omp_sockets[socket_index];
				// write raw joules to rapl_state, then copy to local timing_t
                poll_rapl_data();
			}
		} else if(rapl_initialized){
			// measure all sockets
			int socket_index;
			for(socket_index = 0; socket_index < mc_config.sockets; socket_index++){
			}
		}
	} else { // stop
		gettimeofday(&t->stop, 0);
		get_rdtsc(0, &t->tsc_stop);
		dlog("tsc stop: %llu", t->tsc_stop);
		if(rapl_initialized)
			get_aperf_mperf(my_core, &t->aperf_stop, &t->mperf_stop);
		t->elapsed_time = delta_seconds(&t->start, &t->stop);

		//!@todo handle overflow for tsc, mperf, and aperf. Check counter widths!
		t->tsc_delta = t->tsc_stop - t->tsc_start;
		if(rapl_initialized){
			t->aperf_delta = t->aperf_stop - t->aperf_start;
			t->mperf_delta = t->mperf_stop - t->mperf_start;
			t->c0 = t->mperf_delta/(double)t->tsc_delta;
			t->ratio = t->aperf_delta/(double)t->mperf_delta;
#ifdef _DEBUG
			if(omp_sockets){
				int core;
				for(core = 0; core < mc_config.cores_per_socket; core++){
					get_aperf_mperf(mc_config.map_socket_to_core[omp_sockets[0]][core], 
													 t->aperfs_stop+core, t->mperfs_stop+core);
					get_rdtsc(mc_config.map_socket_to_core[omp_sockets[0]][core], 
									 t->tscs_stop+core);
				}
			}
#endif
		}

		t->cpu_freq = getCPU_freq();

		//!@todo configurable time threshold
		if(omp_sockets && num_omp_sockets > 1 && t->parent)
			t->parent->flags.multiSocket = 1;
		if(rapl_initialized && omp_sockets && t->elapsed_time > .01){
			int socket_index;
			if(t->parent)
				t->parent->flags.estPower = 0;
			t->Pkg_watts = t->PP0_watts = t->DRAM_watts = 0.0;
			for(socket_index = 0; socket_index < num_omp_sockets; socket_index++){
				int omp_socket = omp_sockets[socket_index];
				
				t->Pkg_watts += t->power[omp_socket].Pkg_joules/t->elapsed_time;
				t->PP0_watts += t->power[omp_socket].PP0_joules/t->elapsed_time;
				t->DRAM_watts += t->power[omp_socket].DRAM_joules/t->elapsed_time;

#ifdef _DEBUG
				//!@todo watts, joules, c0 are incorrect if thread affinity is
				//!not specified and the master thread changes cores.
				elog("pkg[%d] watts: %f joules: %f seconds: %f"
						 " d_mperf: %lld d_aperf: %lld, d_tsc: %lld"
						 " c0: %f eff. freq: %f",
						 omp_socket, 
						 t->power[omp_socket].Pkg_joules/t->elapsed_time,
						 t->power[omp_socket].Pkg_joules,
						 t->elapsed_time,
						 t->mperf_delta, t->aperf_delta, t->tsc_delta,
						 t->c0, // c0 ratio
						 t->ratio // eff. freq. ratio
					);
				int core;
				for(core = 0; core < mc_config.cores_per_socket; core++){
					double md = t->mperfs_stop[core] - t->mperfs_start[core];
					double ad = t->aperfs_stop[core] - t->aperfs_start[core];;
					double td = t->tscs_stop[core] - t->tscs_start[core];
					elog("socket %d core %d c0: %f eff. freq: %f",
							 omp_sockets[0], core, md/td, ad/md);
				}
#endif
			}
		} else { // model power
			int threads;
			if(t->parent){
				t->parent->flags.estPower = 1;
				threads = t->parent->flags.threads;
			} else
				threads = lround(ceil(t->parallel_thread_time / t->parallel_time));
			//! this is only valid for Intel(R) Xeon(R) CPU E5-2670.
			double serialPower = // 67.3
				estPower(SMT, 1, t->cpu_freq, mem_freq);
			if(t->parallel_time){
				//!@todo treat sockets separately
				double parallelPower = 
					estPower(SMT, threads, t->cpu_freq, mem_freq);
				double serialTime = t->elapsed_time - t->parallel_time;
				dlog("total time: %e, serial power: %e, serial time: %e, "
						 "parallel power: %e, parallel time: %e",
						 t->elapsed_time,
						 serialPower, serialTime, parallelPower, t->parallel_time);
				t->Pkg_watts =
					(serialTime * serialPower + t->parallel_time * parallelPower) /
					t->elapsed_time;
			} else
				t->Pkg_watts = serialPower;
		}
	}
}

static inline void clear_time(timing_t *t){
	memset(t, 0, sizeof(timing_t));
}

#endif
