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

///@file  shift.h
///@brief Frequency control interfaces for the GEOPM Caliper service

#ifndef SHIFT_H
#define SHIFT_H

#define MAX_NUM_FREQUENCIES 16

int shift_get_freq_index(const int kHz);

int shift_set_socket_governor(int socket, const char *governor_str);
int shift_parse_freqs();
int shift_core(int core, int freq_idx);
int shift_socket(int sock, int freq_idx);
int shift_init_socket(int socket, const char *governor_str, int freq_idx);
void shift_set_initialized(int);

extern int NUM_FREQS, SLOWEST_FREQ;
extern const int FASTEST_FREQ;
extern int turboboost_present;
extern int freqs[MAX_NUM_FREQUENCIES]; // in kHz
extern int shift_initialized;
extern double frequencies[MAX_NUM_FREQUENCIES];// in GHz

#endif  //SHIFT_H

