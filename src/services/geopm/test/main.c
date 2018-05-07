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

///@file  main.c
///@brief GEOPM service to unit-test code for GEOPM service for Caliper.

/* Test application for GEOPM Caliper service.
 * The application tests the following GEOPM annotations and mappings:
 * (GEOPM) geopm_prof_region   -- (Caliper) first occurrence of 
 *                              phase start, loop begin, function start
 * (GEOPM) geopm_prof_enter    -- (Caliper) phase start, loop begin, 
 *                              function start
 * (GEOPM) geopm_prof_exit     -- (Caliper) phase end, loop end, 
 *                              function end
 * (GEOPM) geopm_prof_progress -- (Caliper) end of iteration#xxx attribute
 * (GEOPM) geopm_prof_epoch    -- (Caliper) iteration#mainloop end
 * (GEOPM) geopm_tprof_create  -- (Caliper) update to xxx.loopcount
 * (GEOPM) geopm_tprof_destroy -- (Caliper) loop end
 * (GEOPM) geopm_tprof_increment -- (Caliper) end of iteration#xxx attribute
 *
 * The application builds five binaries: 
 *  main.orig:          original application (without OpenMP),
 *  main.geo:           with GEOPM markup,
 *  main.geo.omp:       with GEOPM markup and OpenMP computation phase,
 *  main.caligeo:       with Caliper markup, and
 *  main.caligeo.omp:   with Caliper markup and OpenMP computation phase
 *  
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <mpi.h>

#ifdef __D_CALI__
#include <cali.h>  
#include <cali_macros.h>  
#endif

#ifdef __D_GEO__
#include <geopm.h>
#endif

#ifdef __OPENMP__
#include <omp.h>
#endif

//#ifdef __D_GEO__
//uint64_t func_send_recv_rid;
//#endif

void do_send_recv(int rank, char *buff_ptr, char **lbuff, long buffsize, int proto, long u_sleep_time) {
#ifdef __D_CALI__
    CALI_MARK_FUNCTION_BEGIN;
#endif
    int tag=1;
    MPI_Request req;
    MPI_Status stat;
    int liter;

    for(liter = 0; liter < u_sleep_time * 10; liter++) {
        if(rank%2 != 0) {
            if(proto==1) {
                MPI_Send(buff_ptr, buffsize, MPI_CHAR, rank-1, tag, MPI_COMM_WORLD);
            } else {
                memcpy(*lbuff, buff_ptr, buffsize);
                MPI_Isend(*lbuff, buffsize, MPI_CHAR, rank-1, tag, MPI_COMM_WORLD, &req);
            }
        }
        else {
            usleep(u_sleep_time);
            MPI_Recv(buff_ptr, buffsize, MPI_CHAR, rank+1, tag, MPI_COMM_WORLD, &stat);
        }
    }
#ifdef __D_CALI__
    CALI_MARK_FUNCTION_END;
#endif

}

#ifdef __D_GEO__
void do_compute_p1(long liter, long biter, uint64_t comp_rid, char *buff_ptr, long buffsize, long u_sleep_time) {
#else
void do_compute_p1(long liter, long biter, char *buff_ptr, long buffsize, long u_sleep_time) {
#endif    
        #ifdef __D_CALI__           
            /* Loop begin must be followed by total loop count to 
             * enable fractional progress measurement in the GEOPM
             * service 
             */
            CALI_MARK_LOOP_BEGIN(comp_loop1, "compute-loop1");
            cali_set_int_byname("compute-loop1.loopcount", u_sleep_time * 10);
        #endif
            for(liter = 0; liter < u_sleep_time * 10; liter++) {
            #ifdef __D_CALI__           
//                CALI_MARK_ITERATION_BEGIN_DOUBLE(mainloop, liter/(u_sleep_time * 10));
                CALI_MARK_ITERATION_BEGIN(comp_loop1, liter);
            #endif
                for(biter = 0; biter < buffsize; biter++) {
                    buff_ptr[biter] += (u_sleep_time + liter) * biter;
                }
                #ifdef __D_GEO__
                    geopm_prof_progress(comp_rid, liter);
                #endif
            #ifdef __D_CALI__           
                CALI_MARK_ITERATION_END(comp_loop1);
            #endif
            }
        #ifdef __D_CALI__           
            CALI_MARK_LOOP_END(comp_loop1);
        #endif
}


#ifdef __OPENMP__
  #ifdef __D_GEO__
    static int do_compute_p2(uint64_t comp_rid, size_t num_stream, double scalar, double *a, double *b, double *c)
  #else
    static int do_compute_p2(size_t num_stream, double scalar, double *a, double *b, double *c)
  #endif
{
    int count = 64;
    const size_t block = 16;
    int err = 0;
    // Mark the loop
#ifdef __D_CALI__
    CALI_CXX_MARK_LOOP_BEGIN(comp_loop2, "compute-loop2");
    cali_set_int_byname("compute-loop2.loopcount", count);
//    CALI_MARK_LOOP_BEGIN(comp_loop2, "compute-loop2");
#endif

    #pragma omp parallel for
    for (int i = 0; i < count; ++i) {
        // Mark each loop iteration
        #ifdef __D_CALI__
//            CALI_CXX_MARK_LOOP_ITERATION(comp_loop2, i);
//            CALI_MARK_ITERATION_BEGIN(comp_loop2, i);
        #endif

        #ifdef __D_GEO__
            int num_thread = 1;
            num_thread = omp_get_num_threads();
            err = geopm_tprof_init(num_thread);
        #endif
        // A Caliper snapshot taken at this point will contain
        // { "function"="main", "loop"="main loop", "iteration#main loop"=<i> }

        size_t j;
        for (j = 0; j < block; ++j) {
                 a[i * block + j] = b[i * block + j] + scalar * c[i * block + j];
                 a[i * block + j] = b[i * block + j] + scalar * c[i * block + j];
        }
        #ifdef __D_CALI__
//        CALI_MARK_ITERATION_END(comp_loop2);
        #endif
        #ifdef __D_GEO__
            geopm_tprof_post();
        #endif

    }
#ifdef __D_CALI__
    CALI_CXX_MARK_LOOP_END(comp_loop2);
//    CALI_MARK_LOOP_END(comp_loop2);
#endif
    return err;
}
#endif

int main(int argc, char *argv[]) {
    
    int numtasks, source=0, dest, tag=1, i;
    int provided;

    int val = 1;
    int iter;
    char *buff_ptr, *local_buff;
    long buffsize=200000;
    int total_iter=40;
    int proto=1;
    long u_sleep_time=1;

    double t1, t2;
    MPI_Status stat;
    MPI_Request req;

    int rank;
    MPI_Init( &argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    buff_ptr   = (char *) new char[buffsize];
    local_buff = (char *) new char[buffsize];

    double w1 = MPI_Wtime();
    long liter, biter;

#ifdef __D_GEO__
    uint64_t comm_rid;
    uint64_t comp_rid;
    geopm_prof_region("comm-phase", GEOPM_REGION_HINT_COMPUTE, &comm_rid);
    geopm_prof_region("comp-phase", GEOPM_REGION_HINT_COMPUTE, &comp_rid);
//    geopm_prof_region("f_send_recv", GEOPM_REGION_HINT_COMPUTE, &func_send_recv_rid);
#endif
    
#ifdef __OPENMP__
    //size_t num_stream = 0.2 * 500000000;
    size_t num_stream = 0.2 * 5000;
    double *a = NULL;
    double *b = NULL;
    double *c = NULL;

    size_t cline_size = 64;
    size_t mem_size = sizeof(double) * num_stream;
    int err = posix_memalign((void **)&a, cline_size, mem_size);
    if (!err) {
        err = posix_memalign((void **)&b, cline_size, mem_size);
    }
    if (!err) {
        err = posix_memalign((void **)&c, cline_size, mem_size);
    }
    if (!err) {
        int i;
  #pragma omp parallel for
        for (i = 0; i < num_stream; i++) {
            a[i] = 0.0;
            b[i] = 1.0;
            c[i] = 2.0;
        }
    }
#endif

    for(proto = 1; proto < 2; proto++) {
        for(iter = 0;iter<total_iter;iter++) {
        #ifdef __D_CALI__           
            CALI_MARK_LOOP_BEGIN(mainloop, "mainloop");
        #endif

        #ifdef __D_CALI__
            CALI_MARK_BEGIN("comm-phase");
        #endif            
            #ifdef __D_GEO__
                geopm_prof_enter(comm_rid);
            #endif
            do_send_recv(rank, buff_ptr, &local_buff, buffsize, proto, u_sleep_time);
            #ifdef __D_GEO__
                geopm_prof_exit(comm_rid);
            #endif
        #ifdef __D_CALI__
            CALI_MARK_END("comm-phase");
        #endif

        #ifdef __D_CALI__
            CALI_MARK_BEGIN("comp-phase");
        #endif            
            #ifdef __D_GEO__
                geopm_prof_enter(comp_rid);
            #endif
                #ifdef __OPENMP__
                    #ifdef __D_GEO__
                    do_compute_p2(comp_rid, num_stream,3,a,b,c);
                    #else
                    do_compute_p2(num_stream,3,a,b,c);
                    #endif
                #else
                    #ifdef __D_GEO__
                    do_compute_p1(liter, biter, comp_rid, buff_ptr, buffsize, u_sleep_time);
                    #else
                    do_compute_p1(liter, biter, buff_ptr, buffsize, u_sleep_time);
                    #endif
                #endif
            #ifdef __D_GEO__
                geopm_prof_exit(comp_rid);
            #endif
        #ifdef __D_CALI__
            CALI_MARK_END("comp-phase");
        #endif            
        #ifdef __D_GEO__
            geopm_prof_epoch();
        #endif            
        #ifdef __D_CALI__           
            CALI_MARK_LOOP_END(mainloop);
        #endif

            MPI_Barrier(MPI_COMM_WORLD);
        }
    }

    printf("calling Finalize\n");
    MPI_Finalize();
    return 0;
}

