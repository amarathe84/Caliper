#!/bin/bash
# Copyright (c) 2015, Lawrence Livermore National Security, LLC.
# Produced at the Lawrence Livermore National Laboratory.
#
# This file is part of Caliper.
# Written by Aniruddha Marathe, marathe1@llnl.gov.
# All rights reserved.
#
# For details, see https://github.com/scalability-llnl/Caliper.
# Please also see the LICENSE file for our additional BSD notice.
#
# Redistribution and use in source and binary forms, with or without modification, are
# permitted provided that the following conditions are met:
#
#  * Redistributions of source code must retain the above copyright notice, this list of
#    conditions and the disclaimer below.
#  * Redistributions in binary form must reproduce the above copyright notice, this list of
#    conditions and the disclaimer (as noted below) in the documentation and/or other materials
#    provided with the distribution.
#  * Neither the name of the LLNS/LLNL nor the names of its contributors may be used to endorse
#    or promote products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
# LAWRENCE LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# @file  test_geo_omp.sh
# @brief Srun launch script to launch unit-test code for GEOPM Caliper service

source geopm-env.sh
numnodes=$1
numtasks=$2

trace_dump=~/ecp_traces/test
runiter=1
for pcap in ${pcaplist}; #240 220 200 180 160 140 120;
do
    sed "s/POWER_BUDGET/${pcap}/g" policy_balanced.json > temp_policy.json

    tracepath=${trace_dump}.${pcap}.${runiter}.balanced.geo_omp_only
    mkdir -p ${tracepath}
    (
    OMP_NUM_THREADS=3 \
    geopmsrun --geopm-rm=SrunLauncher \
            --geopm-ctl=process \
            --geopm-policy=./temp_policy.json \
            --geopm-report=${tracepath}/report \
            --geopm-trace=${tracepath}/trace \
            --geopm-profile="test" \
            --geopm-shmkey="shm-marathe1" \
            -N ${numnodes} -n ${numtasks} -m block -l -- \
            ./main.geo.omp \
            2>& 1) >& ${tracepath}/output

#    2>& 1) >& ${tracepath}/output
done
