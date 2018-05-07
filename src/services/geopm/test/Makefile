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
# @file  Makefile
# @brief Makefile to build unit-test benchmarks for the GEOPM Caliper service

include ../InstallPath

CC=mpicxx
CPP=mpicxx
all: main.c
	${CC} -o main.orig main.c
	${CC} -o main.geo main.c -D__D_GEO__ -I$(GEO_INSTALL_PATH)/include \
				-L$(GEO_INSTALL_PATH)/lib -L$(LIBMSR_INSTALL_PATH)/lib \
	            -Xlinker -rpath $(GEO_INSTALL_PATH)/lib \
				-Xlinker -rpath $(LIBMSR_INSTALL_PATH)/lib -lgeopm -lmsr -qopenmp
	${CC} -o main.geo.omp main.c -D__OPENMP__ -D__D_GEO__ -I$(GEO_INSTALL_PATH)/include \
				-L$(GEO_INSTALL_PATH)/lib -L$(LIBMSR_INSTALL_PATH)/lib \
	            -Xlinker -rpath $(GEO_INSTALL_PATH)/lib \
				-Xlinker -rpath $(LIBMSR_INSTALL_PATH)/lib -lgeopm -lmsr -qopenmp
	${CC} -o main.caliapp main.c -D__D_CALI__ -I$(CALIPER_INSTALL_PATH)/include/caliper \
				-L$(CALIPER_INSTALL_PATH)/lib -L$(CALIPER_INSTALL_PATH)/lib64 -lcaliper -lcaliper-common \
				-lcaliper-reader -Xlinker \
				-rpath $(CALIPER_INSTALL_PATH)/lib -Xlinker -rpath $(CALIPER_INSTALL_PATH)/lib64
	${CC} -o main.caliapp.omp main.c -D__OPENMP__ -D__D_CALI__ -I$(CALIPER_INSTALL_PATH)/include/caliper \
				-L$(CALIPER_INSTALL_PATH)/lib -L$(CALIPER_INSTALL_PATH)/lib64 -lcaliper -lcaliper-common \
				-lcaliper-reader -Xlinker \
				-rpath $(CALIPER_INSTALL_PATH)/lib -Xlinker -rpath $(CALIPER_INSTALL_PATH)/lib64 -qopenmp
	${CC} -o main.caligeo main.c -D__D_CALI__ -I$(CALIPER_INSTALL_PATH)/include/caliper \
				-L$(CALIPER_INSTALL_PATH)/lib -L$(CALIPER_INSTALL_PATH)/lib64 -lcaliper -lcaliper-common \
				-lcaliper-reader -Xlinker \
				-rpath $(CALIPER_INSTALL_PATH)/lib -Xlinker -rpath $(CALIPER_INSTALL_PATH)/lib64 \
				-L$(GEO_INSTALL_PATH)/lib -L$(LIBMSR_INSTALL_PATH)/lib \
				-Xlinker -rpath $(GEO_INSTALL_PATH)/lib \
				-Xlinker -rpath $(LIBMSR_INSTALL_PATH)/lib -lgeopm -lmsr
	${CC} -o main.caligeo.omp main.c -D__OPENMP__ -D__D_CALI__ -I$(CALIPER_INSTALL_PATH)/include/caliper \
				-L$(CALIPER_INSTALL_PATH)/lib -L$(CALIPER_INSTALL_PATH)/lib64 -lcaliper -lcaliper-common \
				-lcaliper-reader -Xlinker \
				-rpath $(CALIPER_INSTALL_PATH)/lib -Xlinker -rpath $(CALIPER_INSTALL_PATH)/lib64 \
				-L$(GEO_INSTALL_PATH)/lib -L$(LIBMSR_INSTALL_PATH)/lib \
				-Xlinker -rpath $(GEO_INSTALL_PATH)/lib \
				-Xlinker -rpath $(LIBMSR_INSTALL_PATH)/lib -lgeopm -lmsr -qopenmp
#	${CC} -o main.geo geo-only.c -I$(GEO_INSTALL_PATH)/include \
#				-L$(GEO_INSTALL_PATH)/lib -L$(LIBMSR_INSTALL_PATH)/lib \
#	            -Xlinker -rpath $(GEO_INSTALL_PATH)/lib \
#				-Xlinker -rpath $(LIBMSR_INSTALL_PATH)/lib -lgeopm -lmsr -qopenmp
test_cali: test_cali.cpp
	${CPP} -o test_cali test_cali.cpp -I$(CALIPER_INSTALL_PATH)/include/caliper \
				-L$(CALIPER_INSTALL_PATH)/lib -L$(CALIPER_INSTALL_PATH)/lib64 -lcaliper -lcaliper-common \
				-lcaliper-reader \
				-L$(GEO_INSTALL_PATH)/lib -L$(LIBMSR_INSTALL_PATH)/lib \
				-Xlinker -rpath $(GEO_INSTALL_PATH)/lib \
				-Xlinker -rpath $(LIBMSR_INSTALL_PATH)/lib -lgeopm -lmsr -qopenmp \
				-Xlinker -rpath $(CALIPER_INSTALL_PATH)/lib -Xlinker -rpath $(CALIPER_INSTALL_PATH)/lib64

clean:
	/bin/rm -f main.geo main.orig main.caligeo.omp main.caligeo main.geo.omp
