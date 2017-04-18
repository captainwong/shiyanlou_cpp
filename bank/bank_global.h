#pragma once

#include <chrono>
#include <memory>

namespace bank {

using clock = std::chrono::steady_clock;
using time_point = clock::time_point;
using duration = std::chrono::minutes;

};
