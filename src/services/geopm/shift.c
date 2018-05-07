/* -*- Mode: C; tab-width: 2; c-basic-offset: 2 -*- */

#include <assert.h>
#include <stdio.h>
#include "shift.h"
#include "cpuid.h"
#include "msr_core.h"
#include "msr_rapl.h"
#include <stdint.h>
//#include "shim.h"
//#include "globals.h"

//#define BLR_USE_SHIFT

/*! @todo FASTEST_FREQ, turboboost_present, current_freq are initialized
  assuming turboboost is present.
 */
int NUM_FREQS, SLOWEST_FREQ;
const int FASTEST_FREQ = 15;
int turboboost_present = 1;

//static const char *cpufreq_path[] = {"/sys/devices/system/cpu/cpu",
//				     "/cpufreq/"};
//static const char cpufreq_governor[] = "scaling_governor";
//static const char cpufreq_speed[] = "scaling_setspeed";
//static const char cpufreq_min[] = "scaling_min_freq";
//static const char cpufreq_max[] = "scaling_max_freq";
       
static const char cpufreq_frequencies[] = "scaling_available_frequencies";
int freqs[MAX_NUM_FREQUENCIES];// in kHz, fastest to slowest
double frequencies[MAX_NUM_FREQUENCIES];// in GHz
static int prev_freq_idx = -1;
int shift_initialized = 0;

int shift_get_freq_index(const int targetFreq_kHz){
		int freqIndex;
		for(freqIndex = SLOWEST_FREQ; freqIndex >= FASTEST_FREQ; freqIndex--)
			if(freqs[freqIndex] >= targetFreq_kHz)
				break;
		if(freqIndex < FASTEST_FREQ)
			freqIndex = FASTEST_FREQ;
		return freqIndex;
}

int shift_set_socket_governor(int socket, const char *governor_str){
	char filename[100];
//	FILE *sfp;
//
//	assert(socket >= 0);
//	assert(socket < mc_config.sockets);
//	assert(governor_str);
//
//	int core_index;
//#ifdef BLR_USE_SHIFT
//	for(core_index = 0; core_index < mc_config.cores_per_socket; core_index++){
//		snprintf(filename, 100, "%s%u%s%s", cpufreq_path[0], 
//			 mc_config.map_socket_to_core[socket][core_index], 
//			 cpufreq_path[1], cpufreq_governor);
//		sfp = fopen(filename, "w");
//		if(!sfp){
//			fprintf(stderr, 
//				"!!! unable to open governor selector %s\n", 
//				filename);
//			return 1;
//		}
//		//assert(sfp);
//		fprintf(sfp, "%s", governor_str);
//		fclose(sfp);
//	}
//#endif
	return 0;
}

void set_msr_by_idx(int coreid, unsigned int msr_idx, unsigned int start_bit, unsigned int end_bit, unsigned int curr_msr_val, unsigned long val) {
     unsigned int bits = start_bit - end_bit + 1;
     unsigned long msr_val;
     unsigned long mask_val=0xFFFFFFFFFFFFFFFF;
     mask_val >>= (64 - bits);
     msr_val = curr_msr_val;
     msr_val &= ~(mask_val << end_bit);
     msr_val |= (val << end_bit);

     if((start_bit-end_bit+1) < 64)
         write_msr_by_idx(coreid, msr_idx, msr_val);
     else
         write_msr_by_idx(coreid, msr_idx, val);
}

//void set_freq(uint64_t freqval){
//    int core;
//    uint64_t val;
//    static uint64_t numDevs = 0;
//    if (!numDevs)
//    {
//        numDevs = num_devs();
//    }
//    for(core=0; core<numDevs; core++){
//        read_msr_by_idx(core, IA32_PERF_CTL, &val);
//        //fprintf(stderr, "PERF_CTL: 0x%x", val);
//        //write_msr_by_idx(core, IA32_PERF_CTL, val);
//        set_msr_by_idx(core, IA32_PERF_CTL, 15, 8, val, freqval);
//    }
//}

//void set_freq_by_idx(int socket, int core, uint64_t freqval){
void set_freq_by_idx(int core, uint64_t freqval){
    uint64_t val;
    //int act_coreid = socket*MAX_THREADS + core;
    read_msr_by_idx(core, IA32_PERF_CTL, &val);
    //fprintf(stderr, "PERF_CTL: 0x%x", val);
    //write_msr_by_idx(core, IA32_PERF_CTL, val);
    set_msr_by_idx(core, IA32_PERF_CTL, 15, 8, val, freqval);
}


int shift_parse_freqs(){
	char filename[100];
	FILE *sfp;
    int iter;

    ////LIBMSR: Update with PERF_CTRL states
    //!@todo: Read MSR_PLATFORM_INFO to get max non-turbo ratio frequenecies
    for(iter = 0; iter < MAX_NUM_FREQUENCIES; iter++) { 
        freqs[iter] = 1200000 + (iter*100000);
        frequencies[iter] = (double)freqs[iter] / 1000000.0;
    }
    NUM_FREQS=MAX_NUM_FREQUENCIES;

	SLOWEST_FREQ = 0;
    //!@todo: Read MSR_PLATFORM_INFO to get turbo enable status
	if(NUM_FREQS > 1){
		turboboost_present = (frequencies[0] / frequencies[1]) < 1.01;
	} else {
		turboboost_present = 0;
	}

	return 0;
}

int shift_core(int core, int freq_idx){
////  #ifdef BLR_USE_SHIFT
////  	char filename[100];
////  	FILE *sfp;
////  #endif
////  //	 if(!shift_initialized){
////  //		 elog("Shift not initialized!");
////  //		 MPI_Abort(MPI_COMM_WORLD, 1);
////  //	 }
////  //	 if(freq_idx < SLOWEST_FREQ){
////  //		 elog("invalid frequency index requested: %d [%d, %d]",
////  //					FASTEST_FREQ, SLOWEST_FREQ);
////  //		 MPI_Abort(MPI_COMM_WORLD, 1);
////  //	 }		 
////  
////  	int temp_cpuid;
////  	get_cpuid(&mc_config, &mc_config_initialized, &temp_cpuid, 0, 0);
////  	if(binding_stable)
////  		assert( temp_cpuid == my_core );
////  
////  	//shm_mark_freq(freq_idx);
////  
////  	if(my_core == core){
////  		if( freq_idx == prev_freq_idx)
////  			return freq_idx;
////  		
////  		prev_freq_idx = freq_idx;
////  	}
////  
////  	//Make the change
////  #ifdef BLR_USE_SHIFT
////      set_freq_by_idx(core, freq_idx);
////      //dlog("rank %d socket %d rank %d core %d shifting core %d to %d", 
////      //		 rank, my_socket, socket_rank, my_core, core, freq_idx);
////  	//fprintf(sfp, "%u\n", freqs[ freq_idx ]);
////  #endif
////      //set_cpu_freq()
	return freq_idx;
}

int shift_socket(int sock, int freq_idx){
///  	 assert(sock >= 0);
///  	 assert(sock < mc_config.sockets);
///  
///  //	 if(!shift_initialized){
///  //		 elog("Shift not initialized!");
///  //		 MPI_Abort(MPI_COMM_WORLD, 1);
///  //	 }
///  //	 if(freq_idx < SLOWEST_FREQ){
///  //		 elog("invalid frequency index requested: %d [%d, %d]",
///  //					FASTEST_FREQ, SLOWEST_FREQ);
///  //		 MPI_Abort(MPI_COMM_WORLD, 1);
///  //	 }		 
///  	 
///  	 int core_index;
///  	 dlog("rank %d shifting %d cores on socket %d to freq %d (%f GHz)", 
///  				rank,
///  				mc_config.cores_per_socket, 
///  				sock, freq_idx, frequencies[freq_idx]);
///  	 for(core_index = 0; core_index < mc_config.cores_per_socket; core_index++)
///  		 shift_core(mc_config.map_socket_to_core[sock][core_index], freq_idx);
	 return 0;
}

int shift_set_socket_min_freq(int socket){
//// 	 assert(socket >= 0);
//// 	 assert(socket < mc_config.sockets);
//// 	 
//// 	 int core_index;
//// 	 FILE *sfp;
//// 	 char filename[100];
//// #ifdef BLR_USE_SHIFT
////     set_freq_by_idx(core, SLOWEST_FREQ);
//// #endif
	 return 0;
}

int shift_set_socket_max_freq(int socket){
//// 	 assert(socket >= 0);
//// 	 assert(socket < mc_config.sockets);
//// 	 
//// 	 int core_index;
//// 	 FILE *sfp;
//// 	 char filename[100];
//// #ifdef BLR_USE_SHIFT
////     set_freq_by_idx(core, FASTEST_FREQ);
//// #endif
	 return 0;
}

int shift_init_socket(int socket, const char *governor_str, int freq_idx){
//// 	assert(socket >= 0);
//// 	assert(socket < mc_config.sockets);
//// 	assert(governor_str);

	shift_set_socket_governor(socket, governor_str);
	shift_set_socket_min_freq(socket);
	shift_set_socket_max_freq(socket);
	shift_initialized = 1;
	
	//!@todo check to see if unrelated cores are affecting selected
	//frequency

	/* set all cores to slowest frequency, then let individual cores
	   select a higher frequency	*/
	shift_socket(socket, freq_idx);
	return 0;
}

void shift_set_initialized(int init){
	shift_initialized = init;
}
