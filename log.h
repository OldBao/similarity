#ifndef LOG_H_
#define LOG_H_

#include "ul_log.h"

#define SM_LOG_FATAL(fmt, arg...) ul_writelog (UL_LOG_FATAL, fmt, ##arg)
#define SM_LOG_WARNING(fmt, arg...) ul_writelog (UL_LOG_WARNING, fmt, ##arg)
#define SM_LOG_NOTICE(fmt, arg...) ul_writelog (UL_LOG_NOTICE, fmt, ##arg)
#define SM_LOG_DEBUG(fmt, arg...) ul_writelog (UL_LOG_DEBUG, fmt, ##arg)
#define SM_LOG_TRACE(fmt, arg...) ul_writelog (UL_LOG_TRACE, fmt, ##arg)

#endif
