/* -*- Mode: C; tab-width: 2; c-basic-offset: 2 -*- */
/*
  2600 Mhz, no turbo, 1-16 cores
  Coefficients:
  (Intercept)        cores  
  39.19         5.70  

  Turbo, 1-16 cores
  Coefficients:
  (Intercept)        cores  
  43.735        7.581  
*/

/*!@todo: Need to update the model for Broadwell */
//!@todo: Pick the model based on the architecture
// one socket
/*
  2014-06-04, cab debug node, one socket, 1-8 threads, all freqs, no turbo
  test_omp

  freqs in MHz
  coef(pkg_m)
  (Intercept)  OMP_NUM_THREADS   cpuFreq OMP_NUM_THREADS:cpuFreq 
  10.646851392     -2.083719850      0.004600185      0.002981610 

  coef(pp0_m)
  (Intercept)  OMP_NUM_THREADS   cpuFreq OMP_NUM_THREADS:cpuFreq 
  -2.582608826     -2.077257263      0.004611694      0.002977944 

  ---
  2014-06-05, cab debug, one socket, 1-8 threads, all freqs, no turbo
  heartwall (gnu)

  coef(pkg_m)
  (Intercept)         threads         cpuFreq threads:cpuFreq 
  10.893117109    -1.492897383     0.004851315     0.002343866 

  coef(pp0_m)
  (Intercept)         threads         cpuFreq threads:cpuFreq 
  -2.023628386    -1.500958476     0.004779861     0.002343656 

  ---

  (Intercept)  mem_freq:SMTFALSE mem_freq:SMTTRUE threads:mctp     mcr:mctr     mctp:mcp    mcp:mcptr
  6.900180e+01     -1.771959e-02    -2.359040e-02 2.499607e-08 9.376432e+00 2.335512e-13 6.074700e-12 
*/

/* Need to know threads, mem freq, cpu freq, SMT, cores.  SMT is not
   enabled on cab.
   
   Frequencies are in MHz.
   
   Per socket! To compute for two sockets, determine how many threads
   on each socket, call the function twice, and add the results.
*/
double estPower(int SMT, int threads, int cpu_freq, int mem_freq){
	int cores;
	/*
	  double mcp = mem_freq * cpu_freq;
	  double mcr = cpu_freq / mem_freq;
	  double mctp = mcp * threads;
	  double mctr = mcr / threads;
	  double mcptr = mcp / threads;
	  double mcrtp = mcr * threads;
	  double result = 
	  6.900180e+01 + 
	  2.499607e-08 * threads * mctp + 
	  9.376432e+00 * mcr * mctr + 
	  2.335512e-13 * mctp * mcp + 
	  6.074700e-12 * mcp * mcptr;
	  if(SMT)
	  result += 
	  -2.359040e-02 * mem_freq;
	  else
	  result += 
	  -1.771959e-02 * mem_freq;
	*/
	/*
	  if(cpu_freq == 2601)
	  //result = 43.735 + 7.581 * cores;
	  *pkg_w = 50.796 + 6.112 * cores; // with GOMP_CPU_AFFINITY='0-15'
	  else if (cpu_freq == 2600)
	  //result = 39.19 + 5.7 * cores;
	  *pkg_w = 39.675 + 5.378 * cores; // with GOMP_CPU_AFFINITY='0-15'
	  else {
	  elog("cpu freq %d not supported yet", cpu_freq);
	  }
	*/
	// ignore turbo for now
	if(cpu_freq == 2601)
		cpu_freq = 2600;
	double result = 
		10.89312 + -1.4929 * cores + .0048513 * cpu_freq + 
		.002343866 * cores * cpu_freq;
	/*
	  double pp0_w = 
	  -2.58261 + -2.07725 * cores + .004611694 * cpu_freq + 
	  .002977944 * cores * cpu_freq;
	*/
	return (result/(2.0f/3.0f));
}
