#include <chrono>
#include "TimeUtil.h"

namespace phb::util {

int64_t now_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

}