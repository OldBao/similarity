#ifndef LOG_H_
#define LOG_H_

#include "ul_log.h"

#ifdef DEBUG

#include <stdio.h>
#define SM_LOG_FATAL printf
#define SM_LOG_WARNING printf
#define SM_LOG_NOTICE printf
#define SM_LOG_DEBUG(fmt, arg...) printf(fmt "\n", ##arg) 
#define SM_LOG_TRACE printf

#endif
#ifdef NDEBUG
#define SM_LOG_FATAL(fmt, arg...) ul_writelog (UL_LOG_FATAL, fmt, ##arg)
#define SM_LOG_WARNING(fmt, arg...) ul_writelog (UL_LOG_WARNING, fmt, ##arg)
#define SM_LOG_NOTICE(fmt, arg...) ul_writelog (UL_LOG_NOTICE, fmt, ##arg)
#define SM_LOG_DEBUG(fmt, arg...) ul_writelog (UL_LOG_DEBUG, fmt, ##arg)
#define SM_LOG_TRACE(fmt, arg...) ul_writelog (UL_LOG_TRACE, fmt, ##arg)
#endif

#endif
