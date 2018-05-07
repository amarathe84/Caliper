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

#include "../CaliperService.h"

#include <Caliper.h>
#include <SnapshotRecord.h>

#include <RuntimeConfig.h>
#include <ContextRecord.h>
#include <Log.h>

#include <cassert>
#include <chrono>
#include <map>
#include <mutex>
#include <string>
#include <type_traits>
#include <vector>
#include "geopm.h"
#include <omp.h>
//#include <shim_config.h>

//#include "conductor.h"

#define GEOPM_NULL_VAL -1

using namespace cali;
using namespace std;

namespace 
{
    struct GeopmOmpLoop { 
        uint64_t loopcount;
        struct geopm_tprof_c *tprof;
    };

std::map<std::string, uint64_t> geopm_phase_map; 
std::map<std::string, GeopmOmpLoop> geopm_loop_map; 

void geopm_init_region(Caliper* c, const Attribute& attr, const Variant& value) {
   uint64_t sumatoms_rid;
   int err;
   Log(1).stream() << "Init GEOPM " << endl;
}

void geopm_set_iteration(Caliper* c, const Attribute& attr, const Variant& val) {
    Log(1).stream() << "-----------------------GEOPM: Iteration: " << attr 
    << "  Val: " << val 
    << endl;

    std::string sAttrName(attr.name());
    /* Check if the attribute is an iteration */
    if(sAttrName.find(".loopcount") != std::string::npos) { 
        std::string sLoopName = sAttrName.substr(0, sAttrName.find(".")); 
        /* Safe-guarding condition for never-before-seen loop region */
        geopm_loop_map[sLoopName].loopcount = val.to_uint();
        geopm_loop_map[sLoopName].tprof = NULL;
        if(omp_get_max_threads() > 1) {
            int num_thread = omp_get_num_threads();
            geopm_tprof_create(num_thread, val.to_uint(), 0, &geopm_loop_map[sLoopName].tprof);
        }
    } /* Check if the attribute is of type 'annotation' */
}

void geopm_begin_region(Caliper* c, const Attribute& attr, const Variant& regionname) {
    int err;
    std::string sRegionName(regionname.to_string());
    std::string sAttrName(attr.name());
    /* Check if the attribute is an iteration */
    if(sAttrName.find("iteration#") != std::string::npos) { 
        std::string sAttrList = sAttrName;
        std::string token = "";
        int pos = 0;
        while((token != "iteration") && (pos = sAttrList.find("#")) != std::string::npos) {
            token = sAttrList.substr(0, pos);
            sAttrList.erase(0, pos + 1);
        }
        std::string sLoopName = sAttrList.substr(0, sAttrList.find("#"));
        /* Safe-guarding condition for never-before-seen loop region */
        if(!geopm_phase_map[sLoopName]) {
            uint64_t phase_rid;
            geopm_prof_region(sLoopName.c_str(), GEOPM_REGION_HINT_COMPUTE, &phase_rid);
            geopm_phase_map[sLoopName] = phase_rid;
        }
    } /* Check if the attribute is of type 'annotation' */

    if(attr.name() == "annotation" || attr.name() == "loop" || attr.name() == "function") {
         if(!geopm_phase_map[sRegionName]) {
             uint64_t phase_rid = GEOPM_NULL_VAL; 
             geopm_prof_region(sRegionName.c_str(), GEOPM_REGION_HINT_COMPUTE, &phase_rid);
             geopm_phase_map[sRegionName] = phase_rid;
             Log(1).stream() << " ---- BEGIN GEOPM added:" << sRegionName
                         << "  for type:" << attr.name()
                         << "  phase_rid: " << geopm_phase_map[sRegionName]
                         << endl;
         }
    }

    if(attr.name() == "annotation") {
         /* Check if the attribute is already present in the phase map */
        geopm_prof_enter(geopm_phase_map[sRegionName]);
    } else if(attr.name() == "loop") {
        geopm_prof_enter(geopm_phase_map[sRegionName]);
    } else if(attr.name() == "statement") {
    } else if(attr.name() == "function") {
        geopm_prof_enter(geopm_phase_map[sRegionName]);
    }
}

void geopm_end_region(Caliper* c, const Attribute& attr, const Variant& regionname) {
    int err;
    std::string sRegionName(regionname.to_string());
    std::string sAttrName(attr.name());
    /* Check if the attribute is an iteration */
    if(sAttrName.find("iteration#") != std::string::npos) { 
        std::string sAttrList = sAttrName;
        std::string token = "";
        int pos = 0;
        while((token != "iteration") && (pos = sAttrList.find("#")) != std::string::npos) {
            token = sAttrList.substr(0, pos);
            sAttrList.erase(0, pos + 1);
        }
        if(omp_get_max_threads() > 1) {
            int num_thread = omp_get_num_threads();
            geopm_tprof_increment(
                geopm_loop_map[sRegionName].tprof, 
                geopm_phase_map[sRegionName],
                num_thread);
        } else {
            geopm_prof_progress(geopm_phase_map[sRegionName], regionname.to_double());
        }

        std::string sLoopName = sAttrList.substr(0, sAttrList.find("#"));
        if(sLoopName == "mainloop") {
            geopm_prof_epoch();

//            /* Conductor code */
//            if(1 == loopID) {
//                thread_config_set_opt_config(plimit, task_ptr);
//            } else if(NUM_SAMPLES == loopID) { 
//                confTList = thread_config_construct_pareto_list(sampl1, sampl2);
//            } else {
//                thread_config_post_pcontrol();
//            }
//
//            /* Need to handle shutdown case */
//            // power_release_comp_sample();
        }
    } 
    if(attr.name() == "annotation" || attr.name() == "loop" || attr.name() == "function") {
        /* Check if the attribute is already present in the phase map */
        if(!geopm_phase_map[sRegionName]) {
            /* Missing phase begin, throw error */
            uint64_t phase_rid = GEOPM_NULL_VAL; 
            geopm_phase_map[sRegionName] = phase_rid;
            Log(1).stream() << "---GEOPM: missing phase begin. Added phase:" << sRegionName << endl; 
        }

//        /* Conductor code */
//        if(1 == loopID) {
//            thread_config_init_comp_sample();
//            thread_config_pre_comp_sample();
//        } else if(NUM_SAMPLES == loopID) {
//            thread_config_pre_comp_sample();
//        } else {
//            thread_config_post_comp_sample();
//        }

    }
    /* Check if the attribute is of type 'annotation' */
    if(attr.name() == "annotation") {
        geopm_prof_exit(geopm_phase_map[sRegionName]);
    } else if(attr.name() == "loop") {
        if(omp_get_max_threads() > 1) {
            geopm_tprof_destroy(geopm_loop_map[sRegionName].tprof);
            geopm_loop_map[sRegionName].tprof = NULL;
            geopm_loop_map[sRegionName].loopcount = 0;
        }
        geopm_prof_exit(geopm_phase_map[sRegionName]);
    } else if(attr.name() == "statement") {
    } else if(attr.name() == "function") {
        geopm_prof_exit(geopm_phase_map[sRegionName]);
    }
}


/// Initialization handler
void geopm_service_register(Caliper* c)
{
    c->events().pre_begin_evt.connect(&geopm_begin_region);
    c->events().pre_end_evt.connect(&geopm_end_region);
    c->events().pre_set_evt.connect(&geopm_set_iteration);

    Log(1).stream() << "Registered GEOPM service" << endl;
}

} // namespace


namespace cali
{
    CaliperService geopm_service = { "geopm", &::geopm_service_register };
} // namespace cali
