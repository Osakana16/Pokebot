#pragma once
#define _WIN32 1
#include <extdll.h>
#include <dllapi.h>
#include <meta_api.h>
#include <entity_state.h>

#undef max
#undef min

#include <algorithm>
#include <stdexcept>
#include <memory>
#include <functional>
#include <random>

#include <limits>
#include <format>
#include <optional>
#include <utility>
#include <variant>

#include <set>
#include <unordered_set>
#include <unordered_map>
#include <valarray>
#include <vector>
#include <stack>
#include <queue>

#include <cassert>
#include <cstring>
#include <cstdint>

#ifndef MSVC
#undef snprintf
#endif

#include "nlohmann/json.hpp"