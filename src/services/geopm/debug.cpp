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

///@file debug.cpp 
///@brief Debugging functions for the GEOPM Caliper service

/* -*- Mode: C; tab-width: 2; c-basic-offset: 2 -*- */
#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdarg.h>
#include "uthash.h"
#include <common/Log.h>

//#include "globals.h"
//#include "msr_common.h"

extern int rank;

struct task_t;
struct sample_t;
struct timing_t;

using namespace cali;

void _dlog(const int error, const int loglevel, const char *file, 
					 const char *function,
					 const int line, const char *msg, ...){
	char errStr[] = "error: ";
	char noErrStr[] = "";
	char *useErrStr;
	struct timeval tv;
	uint64_t tsc;
	int core = -1, socket = -1, local = -1;
	int n;
	int status;
	va_list ap;
  
	const int buflen = 2000;
	char buf[buflen];
  
	va_start(ap, msg);
	n = vsnprintf(buf, buflen, msg, ap);
	va_end(ap);
	buf[buflen-1] = 0;

	/*
	if(use_perror){
		char pbuf[buflen];
		snprintf(pbuf, buflen, "%s::%d:\t%s", file, line, buf);
		pbuf[buflen-1] = 0;
		perror(pbuf);
	} else
	*/

	if(error)
		useErrStr = errStr;
	else
		useErrStr = noErrStr;
// !@todo: fix this --> get times from arch MSR
//	get_rdtsc(0, &tsc);

	switch(loglevel){
	default:
	case 1:
		Log(1).stream() << useErrStr << file << "::" << function <<":" << line <<":" << rank << ":\t" << buf << "\n";
//		fprintf(stderr, "%s%s::%s:%d:r%d:\t%s\n", useErrStr, file, function, line, 
//						rank, buf);
		break;
//	case 2:
//		gettimeofday(&tv, 0);
//		status = get_cpuid(&mc_config, &mc_config_initialized, &core, &socket, &local);
//		fprintf(stderr, "%lf\t%s%s::%s:%d:r%d/s%d/c%d:\t%s\n", 
//						tv.tv_sec + tv.tv_usec/1000000.0, useErrStr, file, function, line, 
//						rank, socket, core, buf);
//		break;
//	case 3:
//		status = get_cpuid(&mc_config, &mc_config_initialized, &core, &socket, &local);
//		fprintf(stderr, "%s%s::%s:%d:r%d/s%d/c%d:tsc:0x%lx:\t%s\n", useErrStr, 
//						file, function, line, 
//						rank, socket, core, tsc, buf);
//		break;
	}
}
