export module pokebot.bot.squad:troops;

import pokebot.game;
import pokebot.game.util;
import pokebot.bot.squad.staff;
import pokebot.bot.squad.strategy;
import pokebot.bot.squad.goal_decision_strategy;
import pokebot.bot.squad.util;
import pokebot.terrain.graph;
import pokebot.terrain.graph.node;

namespace pokebot::bot::squad {
	export class Troops {
		pokebot::game::Team team;

		squad::util::MemberMap members{};
		std::vector<pokebot::bot::squad::strategy::Objective> objectives{};
		
		void Clear() {
			members.clear();
		}

		bool AddMember(const pokebot::util::PlayerName& Bot_Name) noexcept {
			auto result = members.insert({ Bot_Name, -1 });
			return result.second;
		}

		size_t GetNumberOfGoals(const pokebot::game::CSGameBase* const game, const pokebot::node::Graph* const graph) const noexcept {
			auto map_flag = game->GetScenario();
			size_t number{};
			if (bool(map_flag & pokebot::game::MapFlags::Demolition)) {
				number = graph->GetNumberOfGoals(pokebot::node::GoalKind::Bombspot);
			} else if (bool(map_flag & game::MapFlags::HostageRescue)) {
				number = game->GetNumberOfLivingHostages();
			} else if (bool(map_flag & game::MapFlags::Assassination)) {
				number = graph->GetNumberOfGoals(pokebot::node::GoalKind::Vip_Safety);
			} else if (bool(map_flag & game::MapFlags::Escape)) {
				number = graph->GetNumberOfGoals(pokebot::node::GoalKind::Escape_Zone);
			} else {
				assert(false && "The map flag is not supported.");
			}
			return number;
		}

		void AddMembers(const squad::util::MemberSet& Members) {
			for (auto& member : Members) {
				const bool Is_Added = AddMember(member);
				assert(Is_Added && "Failed to add member.");
			}
		}
	public:
		Troops(game::Team team_) : team(team_) {}
		
		Troops(game::Team team_,
			   const std::unordered_set<pokebot::util::PlayerName, pokebot::util::PlayerName::Hash>& Members) : Troops(team_) {
			AddMembers(Members);
			assert(!members.empty());
		}

		Troops(game::Team team_,
			   const squad::util::MemberSet& Members,
			   pokebot::game::CSGameBase* game,
			   pokebot::node::Graph* graph) : Troops(team_) {
			OnNewRound(Members, graph, game);
		}
		
		void Establish(pokebot::game::CSGameBase* const game, pokebot::node::Graph* const graph) {
			assert(game != nullptr && "nullptr is assigned.");
			assert(graph != nullptr && "nullptr is assigned.");
			if (members.empty()) {
				assert(false);
				return;
			}

			size_t number_of_goals = GetNumberOfGoals(game, graph);
			squad::util::MemberSet new_members{};
			for (auto& member : members) {
				new_members.insert(member.first);
			}
			auto map_flag = game->GetScenario();
			auto builder = pokebot::bot::squad::staff::StaffBuilder{ team, map_flag }.SetPlayers(&new_members).SetNumberOfGoals(number_of_goals).SetGraph(graph).SetGame(game);

			members = builder.BuildStaffForMember().Resolve();

			auto min_max = std::minmax_element(members.cbegin(), members.cend(), [](const auto& a, const auto& b) {
				return a.second < b.second;
			});
			auto length = std::distance(min_max.first, min_max.second) + 1;

			auto goal_staff = builder.BuildStaffForGoal();
			for (size_t i = 0; i < length; ++i) {
				if (!bool(i & 1u)) {
					goal_staff.SetCondition(pokebot::bot::squad::goal_decision_strategy::FoundOrderCondition::First);
				} else {
					goal_staff.SetCondition(pokebot::bot::squad::goal_decision_strategy::FoundOrderCondition::Last);
				}
				objectives.push_back(goal_staff.Resolve());
			}
		}

		/**
		* @brief Add a member
		* @param Bot_Name 
		*/
		void OnNewRound(const squad::util::MemberSet& Members, pokebot::node::Graph* graph, pokebot::game::CSGameBase* game) {
			Clear();
			for (auto& member : Members) {
				const bool Is_Added = AddMember(member);
				assert(Is_Added);
			}
			Establish(game, graph);
		}

		/**
		* @brief Add a member
		* @param Bot_Name 
		*/
		void OnPostNewRound(const pokebot::util::PlayerName& Bot_Name) noexcept  {
			AddMember(Bot_Name);
		}

		bool IsJoining(const pokebot::util::PlayerName& Bot_Name) const noexcept {
			return members.contains(Bot_Name);
		}
		
		pokebot::node::NodeID GetPlatoonGoal(const pokebot::util::PlayerName& Bot_Name) const noexcept {
			return std::get<pokebot::bot::squad::strategy::GoalNode>(GetObjective(Bot_Name));
		}

		pokebot::bot::squad::strategy::LeaderName GetLeaderGoal(const pokebot::util::PlayerName& Bot_Name) const noexcept {
			return std::get<pokebot::bot::squad::strategy::LeaderName>(GetObjective(Bot_Name));
		}

		int GetTargetHostageID(const pokebot::util::PlayerName& Bot_Name) const noexcept {
			return std::get<pokebot::bot::squad::strategy::HostageIDs>(GetObjective(Bot_Name));
		}

		size_t GetNumberOfMembers() const noexcept {
			return members.size();
		}

		const squad::strategy::Objective& GetObjective(const pokebot::util::PlayerName& Bot_Name) const noexcept {
			return objectives[members.at(Bot_Name)];
		}
	};
}