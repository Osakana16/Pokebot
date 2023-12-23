#pragma once
#define BEHAVIOR_PRIVATE namespace
#define BEHAVIOR_IF(A) [](const Bot* const Self) noexcept { return A; }

#define RETURN_BEHAVIOR_TRUE_OR_FALSE(B,PROCESS) if constexpr (B) return PROCESS; else return !PROCESS

#define BEHAVIOR_CREATE(TYPE,NAME) std::shared_ptr<TYPE> NAME = TYPE::Create(#NAME)

namespace pokebot::bot {
	namespace behavior {
		enum class Status {
			Not_Ready,	// Cannot start the behavior because of the invalid parameter.
			Executed,	// 
			Enough,		// Need not to execute the behavior.
			Failed		// A part of behavior process is executed but failed.
		};

		void DefineBehavior();
		void DefineAction();
		void DefineObjective();


		using BehaviorFunc = std::function<Status(Bot* const)>;
		using Activator = std::function<bool(const Bot* const)>;

		class BehaviorNode {
			std::string_view name{};
		public:
			BehaviorNode(std::string_view self_name) : name(self_name) {}
			virtual Status Evalute(Bot* const) = 0;
		};

		class Root : public BehaviorNode {
			std::shared_ptr<BehaviorNode> child{};
			Status Evalute(Bot* const self) override {
				return child->Evalute(self);
			}
		public:
			Root(std::shared_ptr<BehaviorNode> child_) : BehaviorNode("Root"), child(child_) {}
		};

		class Sequence : public BehaviorNode {
			std::vector<std::shared_ptr<BehaviorNode>> children;
		public:
			Status Evalute(Bot* const self) override {
				for (auto child : children) {
					switch (child->Evalute(self)) {
						case Status::Not_Ready:
							return Status::Not_Ready;
					}
				}
				return Status::Executed;
			}

			using BehaviorNode::BehaviorNode;

			void Define(std::initializer_list<std::shared_ptr<BehaviorNode>> behaviors) {
				assert(children.empty());
				children = behaviors;
			}

			static std::shared_ptr<Sequence> Create() {
				return std::make_shared<Sequence>("Sequence");
			}

			static std::shared_ptr<Sequence> Create(std::initializer_list<std::shared_ptr<BehaviorNode>> behaviors) {
				auto result = Create();
				result->Define(behaviors);
				return result;
			}
		};

		class Priority : public BehaviorNode {
			std::vector<std::shared_ptr<BehaviorNode>> children;
		public:
			using BehaviorNode::BehaviorNode;

			Status Evalute(Bot* const self) override {
				for (auto child : children) {
					switch (child->Evalute(self)) {
						case Status::Not_Ready:
						case Status::Failed:
						case Status::Enough:
							continue;
					}
					break;
				}
				return Status::Executed;
			}

			void Define(std::initializer_list<std::shared_ptr<BehaviorNode>> behaviors) {
				assert(children.empty());
				children = behaviors;
			}

			static std::shared_ptr<Priority> Create() {
				return std::make_shared<Priority>("Priority");
			}

			static std::shared_ptr<Priority> Create(std::initializer_list<std::shared_ptr<BehaviorNode>> behaviors) {
				auto result = Create();
				result->Define(behaviors);
				return result;
			}
		};

		class Random : public BehaviorNode {
			std::vector<std::shared_ptr<BehaviorNode>> children;
		public:
			using BehaviorNode::BehaviorNode;

			Status Evalute(Bot* const self) override{
				static auto index = common::Random<int>(0, children.size() - 1);
				for (int i = index; children[i]->Evalute(self) == Status::Failed; i = index);
				return Status::Executed;
			}

			void Define(std::initializer_list<std::shared_ptr<BehaviorNode>> behaviors) {
				assert(children.empty());
				children = behaviors;
			}

			static std::shared_ptr<Random> Create() {
				return std::make_shared<Random>("Random");
			}

			static std::shared_ptr<Random> Create(std::initializer_list<std::shared_ptr<BehaviorNode>> behaviors) {
				auto result = Create();
				result->Define(behaviors);
				return result;
			}
		};

		class Condition : public BehaviorNode {
			std::shared_ptr<BehaviorNode> child{};
			Activator CanActive;
		public:
			Status Evalute(Bot* const self) override {
				if (CanActive(self)) {
					return child->Evalute(self);
				}
				return Status::Not_Ready;
			}

			Condition(Activator activator, std::shared_ptr<BehaviorNode> behavior) : BehaviorNode("If"), CanActive(activator), child(behavior) {}

			static std::shared_ptr<Condition> If(Activator activator, std::shared_ptr<BehaviorNode> behavior) {
				return std::make_shared<Condition>(activator, behavior); 
			}
		};

		class Action : public BehaviorNode {
			BehaviorFunc action;
		public:
			using BehaviorNode::BehaviorNode;

			Status Evalute(Bot* const self) noexcept override {
				return action(self);
			}
			
			void Define(BehaviorFunc action_) noexcept {
				assert(action == nullptr);
				action = action_;
			}

			static std::shared_ptr<Action> Create(std::string_view name) {
				return std::make_shared<Action>(name);
			}

			static std::shared_ptr<Action> Create(BehaviorFunc action_, std::string_view name) {
				auto result = Create(name);
				result->Define(action_);
				return result;
			}
		};

		template<Status status>
		class After final : public BehaviorNode {
			std::shared_ptr<BehaviorNode> child{}, after{};
		public:
			Status Evalute(Bot* const self) override {
				auto result = child->Evalute(self) ;
				if (result == status) {
					return after->Evalute(self);
				}
				return result;
			}

			After(std::shared_ptr<BehaviorNode> behavior, std::shared_ptr<BehaviorNode> after_node) : BehaviorNode("After"), child(behavior), after(after_node) {}

			static std::shared_ptr<After<status>> With(std::shared_ptr<BehaviorNode> behavior, std::shared_ptr<BehaviorNode> after_node) {
				return std::make_shared<After<status>>(behavior, after_node); 
			}
		};

		template<bool b>
		bool Always(const Bot* const) noexcept { return b; }

		template<common::Team value>
		bool Is(const Bot* const Self) { return Self->JoinedTeam() == value; }

		template<bool b>
		bool IsPlayingGame(const Bot* const Self) {
			return false;
		}

		template<bool b>
		bool HasEnemy(const Bot* const Self) {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->IsFighting());
		}

		template<bool b, node::GoalKind kind>
		bool IsOnGoal(const Bot* const Self) {
			bool is_on_a_goal{};
			auto goals = node::world.GetGoal(kind);
			for (auto goal = goals.first; goal != goals.second; goal++) {
				if ((is_on_a_goal = node::world.IsOnNode(Self->Origin(), goal->second))) {
					break;
				}
			}
			return is_on_a_goal;
		}

		template<bool b>
		bool IsBombPlanted(const Bot* const Self) {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, manager.Is_Bomb_Planted);
		}

		template<bool b>
		bool IsTickingToExplosion(const Bot* const Self) noexcept {
			return false;
		}

		template<bool b>
		bool IsOnBomb(const Bot* const Self) noexcept {
			return false;
		}

		template<bool b>
		bool HasBomb(const Bot* const Self) noexcept {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->HasWeapon(game::Weapon::C4));
		}

		template<bool b>
		bool IsPlayerMate(const Bot* const Self) noexcept {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (Self->JoinedTeam() == common::GetTeamFromModel(game::game.GetHost())));
		}

		template<bool b>
		bool IsJumping(const Bot* const Self) noexcept {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, !Self->IsOnFloor());
		}

		template<bool b>
		bool IsDucking(const Bot* const Self) noexcept {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->IsDucking());
		}

		template<bool b>
		bool IsSwimming(const Bot* const Self) noexcept {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->IsSwimming());
		}

		template<bool b>
		bool IsBlind(const Bot* const Self) noexcept {
			return false;
		}

		template<bool b>
		bool IsUsing(const Bot* const Self) noexcept {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->IsPressingKey(ActionKey::Use));
		}

		template<bool b>
		bool IsDriving(const Bot* const Self) noexcept {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->IsDriving());
		}

		template<bool b>
		bool IsInDark(const Bot* const) noexcept {
			return false;
		}

		template<bool b>
		bool HasMoneyEnough(const Bot* const) noexcept {
			return false;
		}

		template<bool b>
		bool IsTimeEarly(const Bot* const) noexcept {
			return false;
		}

		template<bool b>
		bool NeedToJoinSquad(const Bot* const) noexcept {
			return false;
		}

		template<bool b>
		bool IsJoinedSquad(const Bot* const Self) noexcept {
			if constexpr (b) {
				return Self->squad != -1;
			} else {
				return Self->squad == -1;
			}
		}

		template<bool b>
		bool IsSquadLeader(const Bot* const Self) noexcept {
			if constexpr (b)
				return manager.IsBotLeader(Self->Name().data(), Self->squad);
			else
				return !manager.IsBotLeader(Self->Name().data(), Self->squad);
		}

		template<bool b>
		bool IsHelper(const Bot* const) noexcept {
			return false;
		}

		template<bool b>
		bool IsFollower(const Bot* const Self) noexcept {
			return !IsSquadLeader<true>(Self);
		}

		template<bool b>
		bool IsFeelingLikeBravely(const Bot* const Self) {
			return Self->mood.brave >= 50;
		}

		template<bool b>
		bool IsFeelingLikeCooperation(const Bot* const Self) {
			return Self->mood.coop >= 50;
		}

		template<bool b>
		bool IsVip(const Bot* const Self) {
			return false;
		}

		template<bool b>
		bool IsEnoughSquadEstablished(const Bot* const Self) {
			if constexpr (b)	
				return false;
			else
				return true;
		}

		template<bool b>
		bool IsVipSquadEnoughJoined(const Bot* const Self) {
			return false;
		}


		extern std::shared_ptr<Action> change_primary;
		extern std::shared_ptr<Action> change_secondary;
		extern std::shared_ptr<Action> change_melee;
		extern std::shared_ptr<Action> change_grenade;
		extern std::shared_ptr<Action> change_flashbang;
		extern std::shared_ptr<Action> change_smoke;
		extern std::shared_ptr<Action> change_c4;
		extern std::shared_ptr<Action> look_c4;
		extern std::shared_ptr<Action> look_hostage;
		extern std::shared_ptr<Action> look_enemy;
		extern std::shared_ptr<Action> look_door;
		extern std::shared_ptr<Action> look_button;
		extern std::shared_ptr<Action> use;
		extern std::shared_ptr<Action> fire;
		extern std::shared_ptr<Action> jump;
		extern std::shared_ptr<Action> duck;
		extern std::shared_ptr<Action> walk;
		extern std::shared_ptr<Action> change_silencer;
		extern std::shared_ptr<Action> adjust_scope;
		extern std::shared_ptr<Action> set_goal_c4;
		extern std::shared_ptr<Action> set_goal_bombspot;
		extern std::shared_ptr<Action> set_goal_hostage;
		extern std::shared_ptr<Action> set_goal_rescuezone;
		extern std::shared_ptr<Action> set_goal_escapezone;
		extern std::shared_ptr<Action> set_goal_vipsafety;
		extern std::shared_ptr<Action> set_goal_tspawn;
		extern std::shared_ptr<Action> set_goal_ctspawn;
		extern std::shared_ptr<Action> set_goal_weapon;
		extern std::shared_ptr<Action> head_to_goal;
		extern std::shared_ptr<Action> discard_latest_goal;

		extern std::shared_ptr<Action> follow_squad_leader;

		extern std::shared_ptr<Action> create_lonely_squad;
		extern std::shared_ptr<Action> create_offense_squad;
		extern std::shared_ptr<Action> create_defense_squad;
		extern std::shared_ptr<Action> create_vip_squad;
		extern std::shared_ptr<Action> join_vip_squad;
		extern std::shared_ptr<Action> join_player_squad;
		extern std::shared_ptr<Action> join_offense_squad;
		extern std::shared_ptr<Action> join_defense_squad;
		extern std::shared_ptr<Action> left_squad;

		extern std::shared_ptr<After<Status::Enough>> head_and_discard_goal;

		namespace demolition {
			extern std::shared_ptr<Priority> objective;
		}

		namespace rescue {
			extern std::shared_ptr<Priority> objective;
		}

		namespace assist {
			extern std::shared_ptr<Priority> objective;
		}

		namespace escape {
			extern std::shared_ptr<Priority> objective;
		}

		namespace coop {
			extern std::shared_ptr<Sequence> objective;
		}

		namespace elimination {
			extern std::shared_ptr<Priority> objective;
		}
	}
}