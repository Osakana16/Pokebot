#pragma once

#include "common.hpp"
#include "util/global_time_provider.hpp"

namespace pokebot::util {
	class Timer final {
		common::Time time{};
		common::Time(*GetGlobalTime)();
	public:
		Timer(common::Time(*GetGlobalTime_)()) : GetGlobalTime(GetGlobalTime_) {}

		bool IsRunning() const noexcept {
			return time >= GetGlobalTime();
		}

		void SetTime(const common::Time t) noexcept {
			time = t + GetGlobalTime();
		}
	};
}