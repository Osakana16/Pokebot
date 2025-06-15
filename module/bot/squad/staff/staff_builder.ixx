module;
#include <vector>

export module pokebot.bot.squad.staff:staff_builder;
import :squad_staff;
import :member_staff;
import :goal_staff;
import pokebot.bot.squad.strategy;

namespace pokebot::bot::squad::staff {
	export class StaffBuilder {
		game::Team team{};
		game::MapFlags map_flags{};
		node::Graph* graph{};
		game::GameBase* game{};

		size_t number_of_goals{};
		std::unordered_set<pokebot::util::PlayerName, pokebot::util::PlayerName::Hash>* players{};

		void AssertBuild() {
			if (!bool(map_flags & game::MapFlags::Other)) {
				assert(number_of_goals > 0 && "The number of goal is not set.");
			} else {
				assert(number_of_goals == 0 && "The number of goal is set.");
			}
		}

	public:
		StaffBuilder(game::Team team_, game::MapFlags map_flags_) : team(team_), map_flags(map_flags_) {}

		StaffBuilder& SetPlayers(std::unordered_set<pokebot::util::PlayerName, pokebot::util::PlayerName::Hash> *players) {
			assert(players != nullptr && "nullptr is assigned.");
			assert(players->size() <= 16u && "The number of the player is upper than 17.");
			assert(!players->empty() && "The player is empty.");
			this->players = players;
			return *this;
		}

		StaffBuilder& SetNumberOfGoals(size_t number_of_goals) {
			this->number_of_goals = number_of_goals;
			return *this;
		}
		
		StaffBuilder& SetGraph(node::Graph* graph) {
			assert(graph != nullptr && "nullptr is assigned.");
			this->graph = graph;
			return *this;
		}
		
		StaffBuilder& SetGame(game::GameBase* game) {
			assert(game != nullptr && "nullptr is assigned.");
			this->game = game;
			return *this;
		}

		staff::MemberStaff BuildStaffForMember() {
			AssertBuild();
			return staff::MemberStaff(
				team,
				map_flags,
				number_of_goals,
				players
			);
		}

		staff::GoalStaff BuildStaffForGoal() {
			AssertBuild();
			return staff::GoalStaff(
				graph,
				game,
				team, 
				map_flags
			);
		}
	};
}