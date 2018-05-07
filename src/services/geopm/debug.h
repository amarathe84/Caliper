#ifndef DEBUG_H
#define DEBUG_H

#define __FILEN__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#if DEBUGLEVEL > 0
#define dlog(msg...) _dlog(0, DEBUGLEVEL, __FILEN__, __FUNCTION__, __LINE__, msg)
#else
#define dlog(msg...) 
#endif

#define elog(msg...) _dlog(1, DEBUGLEVEL, __FILEN__, __FUNCTION__, __LINE__, msg)
#define wlog(msg...) _dlog(0, DEBUGLEVEL, __FILEN__, __FUNCTION__, __LINE__, msg)
void _dlog(const int error, const int loglevel, const char *file, 
					 const char *function,
					 const int line, const char *msg, ...);

#endif
