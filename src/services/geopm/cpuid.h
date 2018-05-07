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

///@file  cpuid.h
///@brief CPU and platform enumeration interfaces for the GEOPM Caliper service

#ifndef BLR_CPUID_H
#define BLR_CPUID_H

/*! on Xeon E5640, core ids are 0, 1, 9, 10, 
  and apic ids range from 0 to 53.
  Thus, some array maps must be longer than the number of cores.
 */
typedef struct 
{
  int sockets;
  int cores;
  int max_apicid;
  int cores_per_socket;
  int *map_core_to_socket;   /* length: cores */
  int *map_core_to_local;    /* length: cores */
  int **map_socket_to_core;  /* length: sockets, cores_per_socket */
  int *map_core_to_per_socket_core; /* length: cores */
  int *map_apicid_to_core;   /* length: max apic id + 1 */
  int *map_core_to_apicid;   /* length: cores */
} mcsup_nodeconfig_t;

extern mcsup_nodeconfig_t config;
extern int config_initialized;

int get_cpuid(mcsup_nodeconfig_t *config, int *config_initialized, 
							int *core, int *socket, int *local);
int parse_proc_cpuinfo(mcsup_nodeconfig_t *config, int *config_initialized);
#endif
