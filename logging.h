#ifndef PHOTOBOOTH_LOGGING_H
#define PHOTOBOOTH_LOGGING_H

#include <boost/log/trivial.hpp>

#define LOG_RECORD_INFO "[ " << __FILENAME__ << ":" << __LINE__ << " ]  "

#define TRIVIAL()   BOOST_LOG_TRIVIAL(trace) << LOG_RECORD_INFO
#define DBG()       BOOST_LOG_TRIVIAL(debug) << LOG_RECORD_INFO
#define INFO()      BOOST_LOG_TRIVIAL(info) << LOG_RECORD_INFO
#define WARN()      BOOST_LOG_TRIVIAL(warning) << LOG_RECORD_INFO
#define ERR()       BOOST_LOG_TRIVIAL(error) << LOG_RECORD_INFO
#define FATAL()     BOOST_LOG_TRIVIAL(fatal) << LOG_RECORD_INFO

#endif //PHOTOBOOTH_LOGGING_H
