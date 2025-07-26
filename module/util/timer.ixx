module;
#include "goldsrc.hpp"
export module pokebot.util: timer;

export namespace pokebot::util {


	using Time = float;

	class Timer final {
		Time time{};
		float(*GetGlobalTime)();
	public:
		Timer(float(*GetGlobalTime_)()) : GetGlobalTime(GetGlobalTime_) {}

		bool IsRunning() const noexcept {
			return time >= GetGlobalTime();
		}

		void SetTime(const Time t) noexcept {
			time = t + GetGlobalTime();
		}
	};

	util::Time GetRealGlobalTime() noexcept {
		return gpGlobals->time;
	}
}