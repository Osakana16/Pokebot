module;
#include <unordered_map>

export module pokebot.bot.squad.strategy:data;
import :strategy_element;

namespace pokebot::bot::squad::strategy {
	export class ObjectiveStorage {
		std::unordered_map<size_t, Objective> objectives{};
	public:
		template<typename T>
		std::optional<size_t> Register(Objective objective, const T& Value) {
			size_t index = objectives.size();
			objectives.insert({ index, Value });
			return index;
		}

		void OnNewRound() {
			objectives.clear();
		}
	};
	
	export struct StrategyData {
		Objective objective;
		int objective_id;
	};
}