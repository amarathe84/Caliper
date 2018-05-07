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

///@file  shim_config.cpp 
///@brief Main C++ file that defines the class and methods to implement configuration exploration

/* -*- Mode: C; tab-width: 2; c-basic-offset: 2 -*- */

extern "C" { 
    double estPower(int SMT, int threads, int cpu_freq, int mem_freq);
    void SetPKG_Power_Limit(int socket, double pkg_lim);
//    void get_rdtsc(int, unsigned long*);
//    void get_aperf_mperf(int, unsigned long*, unsigned long*);
//    void read_papi(long long *values);
    #include "shift.h"
//    #include "debug.h"
    #include "cpuid.h"
//    #include "shim_omp.h"
    #include "uthash.h"
}
#include <mpi.h>
#include <libunwind.h>
#include <unistd.h>
#include <stdio.h>
#include "shim_config.hpp"
#include "shim_time.h"
#include "task.hpp"
#include "debug.h"
#include "md5.h"
#define UNW_LOCAL_ONLY


ConfigList::ConfigList() {

}

ConfigList::~ConfigList() {
    config_obj_t *taskiter;
    for(taskiter = taskhead; taskiter = (config_obj_t *) taskiter->hh.next; taskiter!=NULL) {
        free(taskiter->list);
    }
    if(taskhead) {
        delete taskhead;   
    }
}

sample_t *config_obj_t::GetSample(int idx) {
    return &list[idx];    
}

int config_obj_t::GetSampleIndex() {
    return index;
}

void config_obj_t::IncSamples() {
    index++;
}

void config_obj_t::SetIndex(int idx) {
    index = idx;
}

md5hash config_obj_t::GetHashKey() {
    return hashKey;
}

void config_obj_t::SetHashKey(md5hash ihashkey) {
    hashKey = ihashkey;
}

sample_t *ConfigList::GetSampleEntry(Task *curTask, int total_configs_per_rank) {
    sample_t *sample_entry = NULL;
    config_obj_t *task_entry = NULL; 
    
    //CPP hash
    if(taskhead) {
        HASH_FIND(hh, taskhead, &curTask->hashKey, sizeof(md5hash), task_entry);
    }

    if(task_entry) {
//        wlog("Found hash entry, appending");
//        sample_entry = &task_entry->list[task_entry->index];
        sample_entry = task_entry->GetSample(task_entry->GetSampleIndex());
        task_entry->IncSamples();
    } 
    else {
        task_entry = new config_obj_t;
        task_entry->list = (sample_t *) calloc(total_configs_per_rank, sizeof(sample_t));
        int initIter;
        for(initIter = 0; initIter < total_configs_per_rank; initIter++) {
            task_entry->GetSample(initIter)->elapsed_time = 9999999.9f;
            task_entry->GetSample(initIter)->Pkg_watts = 2000.0f;
        }
        task_entry->SetIndex(1);
        task_entry->SetHashKey(curTask->hashKey);
        HASH_ADD(hh, taskhead, hashKey, sizeof(md5hash), task_entry);
//        wlog("Added hash");
        sample_entry = task_entry->GetSample(0);
    }
    return sample_entry;
}


void ShimConfig::log_pareto_configs() {
    char *path = getenv("dataDir");
    int iter;
    if(!path)
        path = ".";
    const int filenameLength = 1000;
    char filename[filenameLength];
    snprintf(filename, filenameLength, "%s/pareto.%d.dat", path, rank);
    filename[filenameLength - 1] = 0;
    FILE *fp = fopen(filename, "w");
    assert(fp);
    fprintf(fp, "task\tthreads\tfreq\tpower\ttime\n");

    config_obj_t *taskIter; 
    for(taskIter = task_list->taskhead; taskIter != NULL; taskIter = (config_obj_t *) taskIter->hh.next) {
        Task *task_ptr;
        HASH_FIND(hh, tasks, &taskIter->hashKey.hash, 16, task_ptr);
        if(task_ptr && task_ptr->frontier) {
            for(iter = task_ptr->frontier_length - 1; iter >= 0; iter--) {
                fprintf(fp, "%lx\t%d\t%d\t%0.3lf\t%0.6lf\n", 
                        task_ptr, task_ptr->frontier[iter].threads,
                        task_ptr->frontier[iter].freq_idx,
                        task_ptr->frontier[iter].power,
                        task_ptr->frontier[iter].elapsed_time);
            }
        }
    }
    wlog("Dumped all configs and pareto");
    fclose(fp);
}

void ShimConfig::log_opt_config(Task *task_ptr, int freq, uint16_t threads, float power, float time) {
    char *path = getenv("dataDir");
    int exists = 0;
    if(!path)
        path = ".";
    char filename[1000];
    snprintf(filename, 1000, "%s/opt_config.%d.dat", path, rank);
    if(access(filename, F_OK) != -1)
        exists = 1;
    FILE *fp = fopen(filename, "a+");
    assert(fp);
    if(!exists) fprintf(fp, "task\tthr\tfreq\tpower\ttime\n");
    fprintf(fp, "%lx\t%d\t%d\t%0.2lf\t%0.3lf\n", task_ptr,
            threads,
            freq,
            power,
            time*(1000.0f));
    fclose(fp);
}


void ShimConfig::log_explored_config(int freq, uint16_t threads, float power, float time) {
    char *path = getenv("dataDir");
    int exists = 0;
    if(!path)
        path = ".";
    const int filenameLength = 1000;
    char filename[filenameLength];
    snprintf(filename, filenameLength, "%s/explored.%d.dat", path, rank);
    filename[filenameLength - 1] = 0;
    if(access(filename, F_OK) != -1)
        exists = 1;
    FILE *fp = fopen(filename, "a+");
    assert(fp);
    if(!exists) fprintf(fp, "task\tthr\tfreq\tpower\ttime\n");
    fprintf(fp, "%lx\t%d\t%d\t%0.2lf\t%0.3lf\n", curTask,
            threads,
            freq,
            power,
            time*(1000.0f));

    fclose(fp);
}

void ShimConfig::log_initial_config() {
    char *path = getenv("dataDir");
    if(!path) path = ".";
    char filename[1000];
    snprintf(filename, 1000, "%s/initial.%d.dat\0", path, rank);
    FILE *fp = fopen(filename, "w+");
    fprintf(fp, "SAMPLE_MAX_THREADS:%d, localConfig:%d, rank:%d\n", SAMPLE_MAX_THREADS, localConfig, rank);
    fclose(fp);
}


ShimConfig::ShimConfig(Platform *mc, int localconf) { 
//    wlog("-");
    task_list = new ConfigList(); 
    task_list->taskhead = NULL;
    SAMPLE_MAX_THREADS = mc->get_max_threads(); 
    localConfig = localconf; 
    curTask = NULL;
    tasks = NULL;
    prevTask = NULL;
    
//    wlog("--------");
}

void ShimConfig::post_mpi_init(int m_rank, int m_size) {
    rank = m_rank;
    size = m_size;
//    log_initial_config();
//    exit(1);
}

void ShimConfig::slow_down_task(Platform *mc, int *cur_omp_threads, int *cur_freq_idx) {
    int iter;
    double new_target_time = curTask->time.elapsed_time + curTask->next->time.elapsed_time;
    for(iter = curTask->frontier_length - 1; iter > 0; iter--) {
        if(curTask->frontier[iter].elapsed_time < new_target_time - 0.001)
            break;
    }
    if(curTask->optConfig != iter) {
        curTask->optConfig = iter;
        *cur_omp_threads = curTask->frontier[curTask->optConfig].threads;
        *cur_freq_idx = curTask->frontier[curTask->optConfig].freq_idx;
        mc->set_num_threads(*cur_omp_threads);
        mc->set_cur_freq(*cur_freq_idx);
    }
}

void ShimConfig::speed_up_task(Platform *mc, int *cur_omp_threads, int *cur_freq_idx, float cur_powerlimit, float power_scale_up_margin) {
    if(curTask->optConfig - 1 >= 0 &&
       curTask->frontier[curTask->optConfig - 1].power <= cur_powerlimit + power_scale_up_margin) {
        curTask->optConfig = curTask->optConfig - 1;
        *cur_omp_threads = curTask->frontier[curTask->optConfig].threads;
        *cur_freq_idx = curTask->frontier[curTask->optConfig].freq_idx;
        mc->set_num_threads(*cur_omp_threads);
        mc->set_cur_freq(*cur_freq_idx);
    }
}

void ShimConfig::thread_config_set_opt_config_all_tasks(Platform *mc, double powerlimit) {
//    wlog("-");
    //#LIBMSR
    Task *ptr;
    for(ptr = tasks; ptr != NULL; ptr = (Task *) ptr->hh.next){
        if(ptr && ptr->flags.omp && ptr->frontier) {
            thread_config_set_opt_config(mc, powerlimit, ptr);
        }
    }
}

void ShimConfig::thread_config_set_opt_config(Platform *mc, double powerlimit, Task *task_ptr) {
//    wlog("-");
    //#LIBMSR
    int iter;
    task_ptr->optConfig = -1;
    if(task_ptr->frontier[0].power <= powerlimit) {
        task_ptr->optConfig = 0;
    } else {
        for(iter = 0; iter < task_ptr->frontier_length-1; iter++) {
            if((task_ptr->frontier[iter].power > powerlimit) && 
                (task_ptr->frontier[iter+1].power <= powerlimit)) {
                task_ptr->optConfig = iter;
                break;
            }
        }
    }
    if(task_ptr->optConfig == -1) {
        task_ptr->optConfig = 0;
    }

    //!@todo: change 8 to max number of cores per processor on the system
    //!@todo: Get the max number of cores per processor on the system
    if(task_ptr->frontier[task_ptr->optConfig].threads > mc->get_max_threads() || 
       task_ptr->frontier[task_ptr->optConfig].freq_idx < mc->get_slowest_freq() ||
       task_ptr->frontier[task_ptr->optConfig].threads < 1 || 
       task_ptr->frontier[task_ptr->optConfig].freq_idx > mc->get_fastest_freq()) {
        task_ptr->frontier[task_ptr->optConfig].threads = mc->get_max_threads(); 
        task_ptr->frontier[task_ptr->optConfig].freq_idx = mc->get_fastest_freq();
    }

    log_opt_config(task_ptr, 
                             task_ptr->frontier[task_ptr->optConfig].freq_idx,
                             task_ptr->frontier[task_ptr->optConfig].threads,
                             task_ptr->frontier[task_ptr->optConfig].power,
                             task_ptr->frontier[task_ptr->optConfig].elapsed_time);
////    //CPP file IO
////    if(outputThreadConfig && (curIter == sampleIter)) {
////        char configoutfile[1024];
////        char *path = getenv("dataDir");
////        sprintf(configoutfile, "%s/opt_config%d.dat", path, rank);
////        FILE *fp = fopen(configoutfile, "a+");
////        fprintf(fp, "Task:%lx\tThread:%d Frequency:%d Power:%0.3lf Time:%0.3lf\n", 
////                    task_ptr->frontier[task_ptr->optConfig].threads, 
////                    task_ptr->frontier[task_ptr->optConfig].freq_idx,
////                    task_ptr->frontier[task_ptr->optConfig].power,
////                    task_ptr->frontier[task_ptr->optConfig].elapsed_time*1000.0f);
////        fclose(fp);
////    }
//    wlog("--------");
    
}

int thread_config_compare_pareto_func(const void * a, const void * b) {
    sample_t *p = (sample_t *)a;
    sample_t *q = (sample_t *)b;
    if((p->elapsed_time - q->elapsed_time) < 0)  //If power_a < power_b
        return -1;                          // a should be before b
    else if((p->elapsed_time - q->elapsed_time) == 0) { 
        if((p->Pkg_watts - q->Pkg_watts) <= 0)   //Then, if exec_a < exec_b
            return -1;
        else 
            return 1;                           // else, b should be before a
    }
    else 
        return 1;
    
}

conf_t *ShimConfig::thread_config_construct_pareto_list(sample_t *sample_data, int *pareto_count) {
//    wlog("-");
    int sample_count = 0;
    int iterSamples, iter1 = 1, iter2 = 0;
    conf_t *pareto_list = NULL;
    int local_comm_size = size;

    if(rank == 0) { wlog(" ----- thread_config_construct_pareto_list: processing recs: %d\n", total_configs_per_rank * local_comm_size); }

    if(localConfig)
        local_comm_size = 1;

//    if(rank == 0) {
//        wlog("Power Time");
//        for(iter1 = 0; iter1 < total_configs_per_rank * local_comm_size; iter1++) {
//            wlog("Task:0x%x   Threads:%d, Freq:%d\t%0.3lf %0.3lf", sample_data[iter1].hashKey.hash, sample_data[iter1].nthreads, sample_data[iter1].freq, sample_data[iter1].Pkg_watts, sample_data[iter1].elapsed_time);
//        }
//        wlog("--------------------------");
//    }

    qsort(sample_data, total_configs_per_rank * local_comm_size, sizeof(sample_t),  thread_config_compare_pareto_func);

    int pareto_index = 0;
    pareto_list = (conf_t *) calloc(total_configs_per_rank * local_comm_size, sizeof(conf_t));

    pareto_list[pareto_index].threads       = sample_data[0].nthreads;
    pareto_list[pareto_index].freq_idx      = sample_data[0].freq;
    pareto_list[pareto_index].power         = sample_data[0].Pkg_watts;
    pareto_list[pareto_index].elapsed_time  = sample_data[0].elapsed_time;
    
    for(iterSamples = 1; iterSamples < total_configs_per_rank * local_comm_size; iterSamples++) {
        if(sample_data[iterSamples].Pkg_watts < pareto_list[pareto_index].power) {
            //Add to pareto list
            pareto_index += 1;
            pareto_list[pareto_index].threads = sample_data[iterSamples].nthreads;
            pareto_list[pareto_index].freq_idx = sample_data[iterSamples].freq;
            pareto_list[pareto_index].power = sample_data[iterSamples].Pkg_watts;
            pareto_list[pareto_index].elapsed_time  = sample_data[iterSamples].elapsed_time;
        }
    }

    *pareto_count = pareto_index+1;
    return pareto_list;
}

void ShimConfig::thread_config_init_comp_sample(Platform *mc) {
//    wlog("-");
    //#LIBMSR
//    task_list->InitConfigs();
     int iter1, iter2, iter3 = 0;
     int total_thread_count = SAMPLE_MAX_THREADS - SAMPLE_MIN_THREADS + 1;
     int fastest_freq = mc->get_fastest_freq();
     int slowest_freq = mc->get_slowest_freq();
     int total_freq_count = fastest_freq - slowest_freq + 1;
     total_configs = total_thread_count*total_freq_count;
     sample_map = (sample_t *) calloc(total_configs, sizeof(sample_t));

     if(rank == 0) { wlog("******* total_thread_count:%d   total_freq_count:%d  total_configs:%d", total_thread_count, total_freq_count, total_configs); }
     if(rank == 0) { wlog("*** SAMPLE_MAX_THREADS:%d, SAMPLE_MIN_THREADS:%d", SAMPLE_MAX_THREADS, SAMPLE_MIN_THREADS); }
     if(rank == 0) { wlog("*** fastest_freq:%d, slowest_freq:%d", fastest_freq, slowest_freq); }
     if(localConfig) {
         for(iter1 = SAMPLE_MAX_THREADS; iter1 >= SAMPLE_MIN_THREADS; iter1 -= 1) {
             for(iter2 = fastest_freq; iter2 >= slowest_freq; iter2 -= 1) {
                 sample_map[iter3].nthreads = iter1;
                 sample_map[iter3].freq = iter2;
                 iter3++;
             }
         }
         total_configs = iter3;
         total_configs_per_rank = total_configs;
         sampleIter = total_configs;
     }
     else {
         for(iter1 = SAMPLE_MAX_THREADS; iter1 >= SAMPLE_MIN_THREADS; iter1 -= 1) {
             for(iter2 = fastest_freq; iter2 >= slowest_freq; iter2 -= 1) {
                 sample_map[iter3].nthreads = iter1;
                 sample_map[iter3].freq = iter2;
                 iter3++;
             }
         }
 
         total_configs_per_rank  = ((int)((int)total_configs/(int)size));
         total_configs_per_rank += ((int)(total_configs % size)?(int)1:(int)0); 
 
         sampleIter = (int)((total_thread_count * total_freq_count)/size);
         sampleIter += ((total_thread_count * total_freq_count)%size)?1:0;
         if(rank == 0) { wlog("******* total_configs:%d total_configs_per_rank:%d", total_configs, total_configs_per_rank); }
     }
 
     curIter = -1; //first OMP task
     //For Cab, set this to 8 threads
     mc->set_num_threads(mc->get_max_threads());
     //omp_set_num_threads(MAX_THREADS); 
     mc->set_cur_freq(mc->get_fastest_freq());
//     mc->set_pkg_power(cur_powerlimit);
     if(rank == 0) { wlog("Out of thread_config_init_comp_sample"); }
     wlog("--------");
 
}

void ShimConfig::thread_config_pre_comp_sample(Platform *mc) {
//    wlog("--++--++");
//    wlog("-");
    //#LIBMSR
////     if((curIter >= sampleIter + 1) && curTask) {
////         if(curTask->time.elapsed_time > 0.001) {
////             if(curTask->flags.omp && curTask->frontier != NULL) {
//// //                if(rank == 0) {
////                     wlog("---- Iter:%d 0x%x Freq:%d  Threads:%d  P:%0.3lf PrevP:%0.3lf T:%0.3lf  PrevT:%0.3lf CPL:%0.3lf", 
////                         curIter, curTask->hashKey.hash, 
////                         curTask->frontier[curTask->optConfig].freq_idx, 
////                         curTask->frontier[curTask->optConfig].threads, 
////                         curTask->frontier[curTask->optConfig].power, 
////                         curTask->time.Pkg_watts, 
////                         curTask->frontier[curTask->optConfig].elapsed_time, 
////                         curTask->time.elapsed_time, cur_powerlimit);
//// //                }
////                 //If the observed power usage is higher than on the frontier,
////                 //select the next best configuration on the frontier
////                 if(curTask->time.Pkg_watts > cur_powerlimit + 3.0) {
////                     if(curTask->optConfig + 1 < curTask->frontier_length) {
////                         curTask->optConfig += 1;
////                         curTask->frontier[curTask->optConfig-1].power = curTask->time.Pkg_watts;
////                     }
////                 } else if(curTask->time.Pkg_watts < cur_powerlimit - 3.0) {
////                     if(powerBalancing) { 
////                         //You may have to speed up task only when PB is enabled
////                         while((curTask->optConfig - 1 >= 0) && 
////                               (curTask->frontier[curTask->optConfig-1].power < cur_powerlimit - 3.0)) {
////                             curTask->optConfig--;
////                             curTask->frontier[curTask->optConfig+1].power = curTask->time.Pkg_watts;
////                         }
//// ////                         //If running at a lower power than on the frontier
//// ////                         if(curTask->optConfig - 1 >= 0) { 
//// //// //                           && curTask->frontier[curTask->optConfig - 1].power <= cur_powerlimit)
//// ////                             curTask->optConfig -= 1;
//// ////                             curTask->frontier[curTask->optConfig+1].power = curTask->time.Pkg_watts;
//// ////                         }
////                     }
////                 }
////                 //Change the thread count and frequency index if required
////                 
////                 if(mc->get_num_threads() != curTask->frontier[curTask->optConfig].threads)
////                     mc->set_num_threads(curTask->frontier[curTask->optConfig].threads);
////                 mc->set_cur_freq(curTask->frontier[curTask->optConfig].freq_idx);
//// //                wlog("        --- Freq:%d    Threads:%d", curTask->frontier[curTask->optConfig].freq_idx, curTask->frontier[curTask->optConfig].threads);  
////             } 
////         }
////     }
}

void ShimConfig::thread_config_post_comp_sample() {
    //1. Get the task, check if omp
//    wlog("--++--++---");
    if(curIter > -1 && curIter < sampleIter) {
        wlog("curTask:%x, curTask->flags.omp:%d", curTask, curTask->flags.omp);
        if(curTask && curTask->flags.omp) { 
            curTask->flags.omp = 1;
            sample_t *sample_entry = NULL;
            if(task_list) { 
//    wlog("***** curTask:%d  total_configs_per_rank:%d", curTask, total_configs_per_rank );
                sample_entry = task_list->GetSampleEntry(curTask, total_configs_per_rank); 
            }
            sample_entry->freq = config_freq;
            sample_entry->nthreads = config_threads;
 
            if(config_threads == (-1 * (curIter + 1))) {
                sample_entry->Pkg_watts = 2000.0f;
                sample_entry->elapsed_time = 9999999.9f;
            } 
            else {
                sample_entry->Pkg_watts = curTask->time.Pkg_watts;
                sample_entry->elapsed_time = curTask->time.elapsed_time;
            }
            log_explored_config(config_freq,config_threads,curTask->time.Pkg_watts,curTask->time.elapsed_time);

            curTask->curConfigThreads = config_threads;
            curTask->curConfigFreq = config_freq;
            //wlog("curTask->curConfigThreads:%d\t \
            //      config_threads:%d\t,  \
            //      curTask->curConfigFreq:%d\t \
            //      config_freq:%d,\t \
            //      ->Pkg_watts:%f\t \
            //      ->elapsed_time:%f", \
            //      curTask->curConfigThreads, \
            //      config_threads, \
            //      curTask->curConfigFreq, \
            //      config_freq,\
            //      sample_entry->Pkg_watts,\
            //      sample_entry->elapsed_time \
            //      );
        }
//        }
    }
}

void ShimConfig::thread_config_post_pcontrol(Platform *mc) {
    //!@todo:#LIBMSR
    if(rank == 0) wlog("sampleIter:%d curIter:%d", sampleIter, curIter);
    if(rank == 0) { wlog("End of timestep: %d", curIter); }
    curIter++;
    //This is the first iteration and this is the controller rank
    if(curIter == 0) {
///        if(rank == 0) {
///            mark_time(&configOverheadTime1, 0);
///        }
    }
    if(curIter > -1 && curIter < sampleIter) {
        if(curIter == 0) {
///            if(rank == 0) {
///                mark_time(&configOverheadTime2, 1);
///            }
            //Reset RAPL for configuration data collection
            mc->set_pkg_power(MAX_POWER_CAP_NONARCH);
        }

        //1. Get configuration map index for this (rank, curIter)
        int map_index;

        if(localConfig)
            map_index = curIter;
        else
            map_index = curIter + rank*sampleIter;
        //!@todo: Select between power or frequency model
        //2. Set frequency, threads
        if(map_index < total_configs) {
            config_freq = sample_map[map_index].freq;
            config_threads = sample_map[map_index].nthreads;
            wlog("curIter:%d Setting threads:%d  Freq:%d", curIter, config_threads, config_freq);
            mc->set_num_threads(config_threads);
            mc->set_cur_freq(config_freq);
        } else {
            config_freq = (int)(mc->get_fastest_freq() - mc->get_slowest_freq())/2;
            config_threads = -1 * (curIter + 1); //Multiply with curIter to indicate that the configuration 
                                           //is invalid, but the next iteration has started
        }
    }
    else if(curIter == sampleIter) {

        //All samples have been collected, create the pareto frontier
        int iter1, iter2;
        int tempiter;
        //Allocate task sample buffers
        config_obj_t *taskIter;

        if(localConfig) {
            for(taskIter = task_list->taskhead; taskIter != NULL; taskIter = (config_obj_t *) taskIter->hh.next) {
                int pareto_count = 0;
                conf_t *pareto_list = thread_config_construct_pareto_list(taskIter->list, &pareto_count);
                Task *task_ptr;
                HASH_FIND(hh, tasks, &taskIter->hashKey.hash, 16, task_ptr);
                task_ptr->frontier = pareto_list;
                task_ptr->frontier_length = pareto_count;
                thread_config_set_opt_config(mc, cur_powerlimit, task_ptr);
            }
        }
        else {
            sample_t *sample_data = (sample_t *) calloc(total_configs_per_rank*size, sizeof(sample_t));
            if(NULL == task_list->taskhead) { 
                wlog("task_list->taskhead:%x", task_list->taskhead);
            }
            for(taskIter = task_list->taskhead; taskIter != NULL; taskIter = (config_obj_t *) taskIter->hh.next) {
                //Reset sample buffer
                memset(sample_data, 0, sizeof(sample_t) * total_configs_per_rank*size);
                //Select the best config for each region
                //Collect rank-level sample data for the task
                MPI_Allgather(taskIter->list, total_configs_per_rank * sizeof(sample_t), MPI_BYTE, sample_data, sizeof(sample_t) * total_configs_per_rank, MPI_BYTE, MPI_COMM_WORLD);

                int pareto_count = 0;
                conf_t *pareto_list = thread_config_construct_pareto_list(sample_data, &pareto_count);
                //Find hash entry with threads and frequency for the task
                Task *task_ptr;
                HASH_FIND(hh, tasks, &taskIter->hashKey.hash, 16, task_ptr);
                //Update hash entry with the pareto list and set threads and frequency for the task
                task_ptr->frontier = pareto_list;
                task_ptr->frontier_length = pareto_count;
                thread_config_set_opt_config(mc, cur_powerlimit, task_ptr);

                if(rank == 0) wlog("-------- Task: %lx \t Pareto count: %d", taskIter, pareto_count);
            }
            log_pareto_configs();
            free(sample_data);
        }
        free(sample_map);

        //Increment curIter so that the run-time won't invoke thread config logic
        //after this timestep
        curIter++;
        
        //Incorrectly termed as 'overhead'. This is the total time for 
        //the configuration phase. Should derive overhead from this value.
//         if(rank == 0) {
//             mark_time(&configOverheadTime2, 0);
//             //wlog("----------------------Overhead: %0.4lf", (configOverheadTime2.elapsed_time/sampleIter) - configOverheadTime1.elapsed_time);
//         }

        mc->set_pkg_power(cur_powerlimit);
    }
}

void ShimConfig::power_release_comp_sample() {
//    Task *task_ptr;
//    for(task_ptr = tasks; task_ptr != NULL; task_ptr=task_ptr->next) {
//        if(task_ptr->frontier) {
//            delete task_ptr->frontier;
//        }
//    }
}

void ShimConfig::getTask(long shimID) {
	//!@todo can use fid argument to hash_backtrace to distinguish... something?
	md5hash digest;
	hash_backtrace(0, &digest);
	
	//antepenultimateTask = prevTask;
	prevTask = curTask;
	curTask = 0;
	
	HASH_FIND(hh, tasks, &digest, sizeof(md5hash), curTask);

	// if not in table, insert
	if(!curTask){
		curTask = new Task; 
		assert(curTask);
		curTask->ID = shimID;
		curTask->time.parent = curTask;
		curTask->hashKey = digest;
		curTask->frontier = NULL;
		curTask->frontier_length = 0;
		/* by default, all tasks start with omp=0 and threads=1. The omp
			 and threads flags are set in shim_omp.c by an OpenMP event
			 callback, and are valid after the task executes. */
		curTask->flags.omp = 0;
		curTask->flags.threads = 1;
		curTask->curConfigThreads = 0;
		curTask->curConfigFreq = 0;
		HASH_ADD(hh, tasks, hashKey, sizeof(md5hash), curTask);
	} else { // if we've seen this task before
		curTask->time.parallel_time = 0.0;
		curTask->time.parallel_thread_time = 0.0;

		// don't reset omp flag or threads
		//curTask->iFlags = 0;
		curTask->flags.spin = 0;
		curTask->flags.estPower = 0;
		curTask->flags.multiSocket = 0;
		curTask->flags.newComm = 0;
	}
#ifndef OMPT_DISABLED
//	resetTimes(); // omp-specific
#endif
	if(prevTask){
#ifdef _DEBUG
		if(prevTask && prevTask->next && prevTask->next != curTask)
			dlog("task misprediction: prev: 0x%x, last successor: 0x%x, cur: 0x%x\n",
					 prevTask->hashKey, prevTask->next->hashKey, curTask->hashKey);
#endif
		prevTask->next = curTask;
	}
}

void ShimConfig::setTask() {
//    curTask->msgSrc = msgSrc;
//    curTask->msgSize = msgSize;
//    curTask->msgDest = msgDest;
//    curTask->tag = tag;
//    curTask->comm = comm;
//    curTask->reqCount = reqCount;
//    curTask->reqIndices = 0;
}

