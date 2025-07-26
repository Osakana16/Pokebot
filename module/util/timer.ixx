module;
#include "goldsrc.hpp"
export module pokebot.util: timer;
import pokebot.engine.hlvector;

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

	engine::HLVector& GetRealRight() noexcept {
		return reinterpret_cast<engine::HLVector&>(gpGlobals->v_right);
	}

	engine::HLVector& GetRealForward() noexcept {
		return reinterpret_cast<engine::HLVector&>(gpGlobals->v_forward);
	}

	engine::HLVector& GetRealUp() noexcept {
		return reinterpret_cast<engine::HLVector&>(gpGlobals->v_up);
	}
}