#ifndef PHOTOBOOTH_LOGGING_H
#define PHOTOBOOTH_LOGGING_H

#include <boost/log/trivial.hpp>

#define TRIVIAL()   BOOST_LOG_TRIVIAL(trace)
#define DBG()       BOOST_LOG_TRIVIAL(debug)
#define INFO()      BOOST_LOG_TRIVIAL(info)
#define WARN()      BOOST_LOG_TRIVIAL(warning)
#define ERR()       BOOST_LOG_TRIVIAL(error)
#define FATAL()     BOOST_LOG_TRIVIAL(fatal)

#endif //PHOTOBOOTH_LOGGING_H
