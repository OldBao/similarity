#ifndef LOG_H_
#define LOG_H_

#include "ul_log.h"

#ifdef DEBUG

#include <stdio.h>
#define SM_LOG_FATAL(fmt, arg...) printf(fmt "\n", ##arg) 
#define SM_LOG_WARNING(fmt, arg...) printf(fmt "\n", ##arg) 
#define SM_LOG_NOTICE(fmt, arg...) printf(fmt "\n", ##arg) 
#define SM_LOG_DEBUG(fmt, arg...) printf(fmt "\n", ##arg) 
#define SM_LOG_TRACE(fmt, arg...) printf(fmt "\n", ##arg)

#define __SM_CHECK(cond, fmt, arg...) {                                 \
    if (!cond) {                                                        \
      printf ("assert (" #cond ") fails : " fmt "\n", ##arg); \
      abort();                                                          \
    }                                                                   \
  }

#endif
#ifdef NDEBUG
#define SM_LOG_FATAL(fmt, arg...) ul_writelog (UL_LOG_FATAL, fmt, ##arg)
#define SM_LOG_WARNING(fmt, arg...) ul_writelog (UL_LOG_WARNING, fmt, ##arg)
#define SM_LOG_NOTICE(fmt, arg...) ul_writelog (UL_LOG_NOTICE, fmt, ##arg)
#define SM_LOG_DEBUG(fmt, arg...) ul_writelog (UL_LOG_DEBUG, fmt, ##arg)
#define SM_LOG_TRACE(fmt, arg...) ul_writelog (UL_LOG_TRACE, fmt, ##arg)
#define __SM_CHECK(cond, fmt, arg...) {           \
    SM_LOG_WARNING ("assert (" #cond ") fails : " fmt "\n", ##arg); \
  }
#endif

#define SM_CHECK_RET(ret, cond, fmt, arg...)  {       \
    __SM_CHECK(cond, fmt, ##arg);                     \
    return ret;                                        \
  }

#define SM_CHECK_RET_ERR(cond, fmt, arg...) SM_CHECK_RET(-1, cond, fmt, ##arg)
#define SM_CHECK_RET_OK(cond, fmt, arg...) SM_CHECK_RET(0, cond, fmt, ##arg)

#define SM_CHECK __SM_CHECK
#endif
