export module pokebot.bot.squad.staff:member_staff;
import :squad_staff;

import pokebot.util;
import pokebot.bot.squad.strategy;
import pokebot.bot.squad.util;

namespace pokebot::bot::squad::staff {
	export class MemberStaff : public SquadStaff {
		const std::unordered_set<pokebot::util::PlayerName, pokebot::util::PlayerName::Hash> * const Players;

		squad::util::MemberMap player_platoon{};

		void AssignBySequence(size_t max_number) {
			auto player = Players->begin();
			const size_t Number_Of_Members = Players->size();
			if (max_number > Number_Of_Members)
				max_number = Number_Of_Members;
			
			for (int squad = 0; squad < Players->size(); squad++) {
				for (int j = 0; j < max_number && player != Players->end(); j++) {
					auto result = player_platoon.insert({ *player, squad });
					assert(result.second);
					player++;
				}
			}
		}

		void AssignByRemainder(size_t max_number) {
			auto player = Players->begin();
			const size_t Number_Of_Members = Players->size();
			if (max_number > Number_Of_Members)
				max_number = Number_Of_Members;

			int squad = 0;
			for (int j = 0; j < Number_Of_Members && player != Players->end(); j++) {
				auto result = player_platoon.insert({ *player, (squad++ % max_number) });
				assert(result.second);
				player++;
			}
		}

		void (MemberStaff::* assignMethod[2][4])(size_t) = {
			// Offender
			{
				&MemberStaff::AssignBySequence,
				&MemberStaff::AssignByRemainder,
				&MemberStaff::AssignBySequence,
				&MemberStaff::AssignByRemainder
			},
			// Defender
			{
				&MemberStaff::AssignByRemainder,
				&MemberStaff::AssignByRemainder,
				&MemberStaff::AssignByRemainder,
				&MemberStaff::AssignByRemainder
			}
		};

		size_t number_of_goals{};

		auto DecideMethod() const noexcept {
			int index = -1;

			size_t max_number = 5;
			if (bool(base_map_flags & game::MapFlags::Demolition)) {
				index = 0;
				if (base_team == game::Team::CT)
					max_number = number_of_goals;

			} else if (bool(base_map_flags & game::MapFlags::HostageRescue)) {
				index = 1;
			} else if (bool(base_map_flags & game::MapFlags::Assassination)) {
				index = 2;
			} else if (bool(base_map_flags & game::MapFlags::Escape)) {
				index = 3;
			} else {
				assert(false);
			}
			return std::make_pair(index, max_number);
		}
	public:
		MemberStaff(game::Team team, 
					game::MapFlags flag, 
					size_t number_of_goals_,
					const std::unordered_set<pokebot::util::PlayerName, pokebot::util::PlayerName::Hash>* const Players_) : SquadStaff(team, flag), number_of_goals(number_of_goals_), Players(Players_)
		{
			assert(Players != nullptr && "Players is nullptr.");
		}

		[[nodiscard]] squad::util::MemberMap Resolve() {
			if (Players->empty()) {
				return {};
			}

			const auto My_Role = util::JudgeRole(base_team, base_map_flags);
			const auto method = DecideMethod();

			assert(My_Role != util::Role::Both && "Role::Both is not supported currently.");
			(this->*assignMethod[static_cast<int>(My_Role)][method.first])(method.second);
			return player_platoon;
		}
	};
}