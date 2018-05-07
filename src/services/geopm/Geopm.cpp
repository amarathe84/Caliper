// Copyright (c) 2015, Lawrence Livermore National Security, LLC.  
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

///@file  Geopm.cpp
///@brief GEOPM service to relay updates to phase and loop annotation attributes to GEOPM runtime

#include "CaliperService.h"

#include <Caliper.h>
#include <SnapshotRecord.h>

#include <common/RuntimeConfig.h>
#include <common/ContextRecord.h>
#include <common/Log.h>

#include <cassert>
#include <chrono>
#include <map>
#include <mutex>
#include <string>
#include <type_traits>
#include <vector>
#include "geopm.h"
#include <omp.h>
#include <mpi.h>

//#include "stacktrace.h"
//#include "shim_enumeration.h"
#include "shim_config.hpp"
//#include <mpi.h>
#include "cpuid.h"
#include "shift.h"
#include "platform.hpp"
#include <functional>
#define GEOPM_NULL_VAL -1

using namespace cali;
using namespace std;

int rank;
int size;

namespace 
{
//    Platform *mc;

std::map<std::string, uint64_t> geopm_phase_map; 
std::map<std::string, uint64_t> geopm_loop_list; 

Platform *mc;
ShimConfig *shim;

void geopm_init_region(Caliper* c, const Attribute& attr, const Variant& value) {
   uint64_t sumatoms_rid;
   int err;
   Log(1).stream() << "Init GEOPM " << endl;
}

void geopm_set_iteration(Caliper* c, const Attribute& attr, const Variant& val) {
//    Log(1).stream() << "-----------------------GEOPM: Iteration: " << attr 
//    << "  Val: " << val 
//    << endl;

    std::string sAttrName(attr.name());
//    Log(1).stream() << "---GEOPM: Set event: " << attr << "  Val: " << val << endl;
    /* Check if the attribute is an iteration */
    if(sAttrName.find(".loopcount") != std::string::npos) { 
        std::string sLoopName = sAttrName.substr(0, sAttrName.find(".")); 
         geopm_loop_list.insert(std::pair<std::string, uint64_t>(sLoopName, val.to_uint()));
        /* Safe-guarding condition for never-before-seen loop region */
    } /* Check if the attribute is of type 'annotation' */
    if(omp_get_max_threads() > 1) {
    } else {
        
    }
}

void geopm_begin_region(Caliper* c, const Attribute& attr, const Variant& regionname) {
    int err;
    std::string sRegionName(regionname.to_string());
    std::string sAttrName(attr.name());
//    Log(1).stream() << "---GEOPM: Begin statements: " << attr << "  Val: " << regionname << endl;
// 
//     /* Check if the attribute is an iteration, if so, find its name and update 
//      * GEOPM progress 
//      */

//     if(sAttrName.find("iteration#") != std::string::npos) { 
// //        Log(1).stream() << "--- Attribute:" << attr << "   Thread ID: " << omp_get_thread_num() << "\n";
//          std::string sAttrList(sAttrName);
//          std::string token("");
//          int pos = 0;
//          while((token != "iteration") && (pos = sAttrList.find("#")) != std::string::npos) {
//              token = sAttrList.substr(0, pos);
//              sAttrList.erase(0, pos + 1);
//          }
//          std::string sLoopName;
//          if(sAttrList.find("#") != std::string::npos) {
//              sLoopName = sAttrList.substr(sAttrList.find("#"));
//          } else {
//              sLoopName = sAttrList;
//          }
// //         /* Safe-guarding condition for iteration of a never-before-seen loop */
// //         if(geopm_loop_list.end() == geopm_loop_list.find(sLoopName) || -1 == geopm_loop_list[sLoopName]) {
// //             geopm_loop_list.insert(std::pair<std::string, uint64_t>(sRegionName, 1));
// //         }
//     } /* Check if the attribute is of type 'annotation' */

    /* check if the attribute type is annotation, loop or function, and create a region ID. */
    if(attr.name() == "annotation" 
        ) {
         if(geopm_phase_map.end() == geopm_phase_map.find(sRegionName)) {
             uint64_t phase_rid = GEOPM_NULL_VAL; 
             geopm_prof_region(sRegionName.c_str(), GEOPM_REGION_HINT_COMPUTE, &phase_rid);
             geopm_phase_map.insert(std::pair<std::string, uint64_t>(sRegionName, phase_rid));
//             Log(1).stream() << " ---- BEGIN GEOPM added:" << sRegionName
//                         << "  for type:" << attr.name()
//                         << "  phase_rid: " << geopm_phase_map[sRegionName]
//                         << endl;
         }
    }

    if(attr.name() == "annotation") {
         /* Check if the attribute is already present in the phase map */
//        Log(1).stream() << "---GEOPM: Start annotation: " 
//            << sRegionName 
//            << "  Phase name: " << geopm_phase_map.find(sRegionName)->first
//            << "  Phase rID: " << geopm_phase_map.find(sRegionName)->second 
//            << "  Size: " << geopm_phase_map.size() 
//            << endl;
        geopm_prof_enter(geopm_phase_map[sRegionName]);
//        Log(1).stream() << " ---- OMP Threads: " << omp_get_max_threads() << endl; 
    } else if(attr.name() == "loop") {
        /* Start of a loop in the application, initialize loop count
         * to update fractional progress later
         */
        if(regionname.to_string() == "mainloop") {
            /* Nothing to do at the start of the mainloop */
        } else if(omp_get_max_threads() > 1) {
            geopm_loop_list.insert(std::pair<std::string, uint64_t>(sRegionName, -1));
            std::hash<std::string> hasher;
            auto hashed = hasher(sRegionName);
            shim->getTask(hashed);
            shim->curTask->flags.omp = 1;
            shim->thread_config_post_comp_sample();
            shim->curTask->mark_time(mc,shim->rank, 1); //Start the timer
        }

    } else if(attr.name() == "statement") {
    } else if(attr.name() == "function") {
    }
}

void geopm_end_region(Caliper* c, const Attribute& attr, const Variant& regionname) {
    int err;
    std::string sRegionName(regionname.to_string());
    std::string sAttrName(attr.name());
//    Log(1).stream() << "---GEOPM: End statement: " << attr << "  Val: " << regionname << endl;
    /* Check if the attribute is an iteration */
    if(sAttrName.find("iteration#") != std::string::npos) { 
        std::string sAttrList(sAttrName);
        std::string token("");
        int pos = 0;
        while((token != "iteration") && (pos = sAttrList.find("#")) != std::string::npos) {
            token = sAttrList.substr(0, pos);
            sAttrList.erase(0, pos + 1);
        }

        /* If this is the main loop, mark end of timestep */
        std::string sLoopName = sAttrList.substr(0, sAttrList.find("#"));
        if(omp_get_max_threads() > 1) {
            if(regionname.to_uint() == 1) {
                int num_thread = omp_get_max_threads();
                int thread_idx = omp_get_thread_num();

                /* Caliper scope is not thread-level. Caliper must expose 
                 * thread ID in order for geopm_tprof_* markup to function
                 * as expected.
                 */
//                Log(1).stream() << "---GEOPM: Initializing tprof for loop: " << sLoopName << ", threads: " << num_thread << endl;
                geopm_tprof_init_loop(num_thread, thread_idx, geopm_loop_list[sLoopName], 1);
            }

            /* If this is an OpenMP loop, mark thread-level progress */ 
            geopm_tprof_post();
        } else {
            /* This is neither the main loop nor an OpenMP loop, 
             * therefore, mark process-level progress */
            geopm_prof_progress(geopm_phase_map[sRegionName], 
                                regionname.to_double()/(double) geopm_loop_list[sRegionName]);
        }
    } else { 
        /* This event marks the end of region */
        if(attr.name() == "annotation"
            ) {
            /* Check if the attribute is already present in the phase map */
            if(geopm_phase_map.end() == geopm_phase_map.find(sRegionName)) {
                /* Missing phase begin, throw error */
                uint64_t phase_rid = GEOPM_NULL_VAL; 
                geopm_phase_map[sRegionName] = phase_rid;
                Log(1).stream() << "---GEOPM: missing phase found. Please add the missing 'begin' mark-up for " << sRegionName << endl; 
            }
        }
        /* Check if the attribute is of type 'annotation' */
        if(attr.name() == "annotation") {
            if(geopm_phase_map[sRegionName] != GEOPM_NULL_VAL) { 
                geopm_prof_exit(geopm_phase_map[sRegionName]);
            }
//            Log(1).stream() << "---GEOPM: End annotation: " << attr << "  Val: " << regionname << endl;
        } else if(attr.name() == "loop") {
            if(regionname.to_string() == "mainloop") {
                geopm_prof_epoch();
                shim->thread_config_post_pcontrol(mc);
            } else if(omp_get_max_threads() > 1) {
                shim->thread_config_pre_comp_sample(mc);
                shim->curTask->mark_time(mc,shim->rank,0); //Start the timer
                geopm_loop_list.erase(sRegionName);
            } else {
                geopm_loop_list.erase(sRegionName);
            }
        } else if(attr.name() == "statement") {
        } else if(attr.name() == "function") {
        }
    }
}

void geopm_post_init(Caliper* c) {
    /* Initiate Conductor's configuration selection logic */
    int powerBalancing=0;
    int enableAdagio=0;
    int powerSampling=0;
    int enableThreadConfig=1;
    int optFreq=0;
    int upowerlimit=120;
    int prun=0;
    int rank;
    int size;

    mc = new Platform(powerBalancing, enableAdagio, powerSampling, enableThreadConfig, optFreq, upowerlimit, prun);
    mc->pre_mpi_init();
    mc->post_mpi_init();
    shim = new ShimConfig(mc, 0);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    shim->post_mpi_init(rank, size);
    shim->thread_config_init_comp_sample(mc);
    shim->log_initial_config();
    Log(1).stream() << "Registered GEOPM service" << endl;
//    mc = new Platform(0, 0, 0, 1, FASTEST_FREQ, MAX_POWER_CAP_NONARCH, 0);

}

/// Initialization handler
void geopm_service_register(Caliper* c)
{
    c->events().pre_begin_evt.connect(&geopm_begin_region);
    c->events().pre_end_evt.connect(&geopm_end_region);
    c->events().pre_set_evt.connect(&geopm_set_iteration);
    c->events().post_init_evt.connect(&geopm_post_init);
}

} // namespace


namespace cali
{
    CaliperService geopm_service = { "geopm", &::geopm_service_register };
} // namespace cali
