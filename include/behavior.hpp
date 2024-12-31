#pragma once
#define BEHAVIOR_PRIVATE namespace
#define BEHAVIOR_IF(A) [](const Bot* const Self) noexcept { return A; }
#define BEHAVIOR_IFELSE(AIF,A_PROCESS,BIF,B_PROCESS) Condition::If(AIF, A_PROCESS), Condition::If(BIF, B_PROCESS)
#define BEHAVIOR_IFELSE_TEMPLATE(IF,A_PROCESS,B_PROCESS) Condition::If(IF<true>, A_PROCESS), Condition::If(IF<false>, B_PROCESS)


#define RETURN_BEHAVIOR_TRUE_OR_FALSE(B,PROCESS) if constexpr (B) return (PROCESS); else return !(PROCESS)

#define BEHAVIOR_CREATE(TYPE,NAME) std::shared_ptr<TYPE> NAME = TYPE::Create(#NAME)
#define BEHAVIOR_CREATE_INLINE(TYPE,NAME) inline std::shared_ptr<TYPE> NAME = TYPE::Create(#NAME)

namespace pokebot::bot {
	namespace behavior {
		enum class Status {
			Failed,
			Success,
			Running
		};

		void DefineBehavior();
		void DefineAction();
		void DefineObjective();
		void DefineCombat() ;

		using BehaviorFunc = std::function<Status(Bot* const)>;
		using Activator = std::function<bool(const Bot* const)>;

		class BehaviorNode {
		protected:
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
			Status Evalute(Bot* const self) override;

			using BehaviorNode::BehaviorNode;

			void Define(std::initializer_list<std::shared_ptr<BehaviorNode>> behaviors) {
				assert(children.empty());
				children = behaviors;
			}

			static std::shared_ptr<Sequence> Create(const char* const Name) {
				auto node = std::make_shared<Sequence>(Name);
				return node;
			}

			static std::shared_ptr<Sequence> Create(std::initializer_list<std::shared_ptr<BehaviorNode>> behaviors) {
				auto result = Create("Sequence");
				result->Define(behaviors);
				return result;
			}
		};

		class Priority : public BehaviorNode {
			std::vector<std::shared_ptr<BehaviorNode>> children;
		public:
			using BehaviorNode::BehaviorNode;

			Status Evalute(Bot* const self) override;

			void Define(std::initializer_list<std::shared_ptr<BehaviorNode>> behaviors) {
				assert(children.empty());
				children = behaviors;
			}

			static std::shared_ptr<Priority> Create(const char* const Name) {
				auto node = std::make_shared<Priority>(Name);
				return node;
			}

			static std::shared_ptr<Priority> Create(std::initializer_list<std::shared_ptr<BehaviorNode>> behaviors) {
				auto result = Create("Priority");
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
				return Status::Failed;
			}

			Condition(Activator activator, std::shared_ptr<BehaviorNode> behavior) : BehaviorNode("If"), CanActive(activator), child(behavior){}

			static std::shared_ptr<Condition> If(Activator activator, std::shared_ptr<BehaviorNode> behavior) { return std::make_shared<Condition>(activator, behavior);  }
		};

		class Action : public BehaviorNode {
			BehaviorFunc action;
		public:
			using BehaviorNode::BehaviorNode;

			Status Evalute(Bot* const self) noexcept override {
				// SERVER_PRINT(std::format("{}\n", name).c_str());
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

		template<bool b>
		bool IsBombPlanted(const Bot* const Self) {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, manager.C4Origin().has_value());
		}

		template<bool b>
		bool IsTickingToExplosion(const Bot* const Self) noexcept {
			return false;
		}

		template<bool b>
		bool IsOnBomb(const Bot* const Self) noexcept {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (common::Distance(Self->Origin(), *manager.C4Origin()) <= 50.0f));
		}

		template<bool b>
		bool HasBomb(const Bot* const Self) noexcept {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, Self->HasWeapon(game::Weapon::C4));
		}

		template<bool b>
		bool IsPlayerMate(const Bot* const Self) noexcept {
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, (Self->JoinedTeam() == common::GetTeamFromModel(game::game.host.AsEdict())));
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
			RETURN_BEHAVIOR_TRUE_OR_FALSE(b, true);
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

		template<game::MapFlags flag>
		bool IsCurrentMode(const Bot* const Self) noexcept {
			return game::game.IsCurrentMode(flag);
		}

		template<bool b>
		bool IsHelper(const Bot* const) noexcept {
			return false;
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
		extern std::shared_ptr<Action> tap_fire;
		extern std::shared_ptr<Action> jump;
		extern std::shared_ptr<Action> duck;
		extern std::shared_ptr<Action> walk;
		extern std::shared_ptr<Action> change_silencer;
		extern std::shared_ptr<Action> adjust_scope;
		extern std::shared_ptr<Action> set_goal_team_objective;
		extern std::shared_ptr<Action> set_goal_from_c4_within_range;
		extern std::shared_ptr<Action> set_goal_bombspot;
		extern std::shared_ptr<Action> set_goal_hostage;
		extern std::shared_ptr<Action> set_goal_rescuezone;
		extern std::shared_ptr<Action> set_goal_escapezone;
		extern std::shared_ptr<Action> set_goal_vipsafety;
		extern std::shared_ptr<Action> set_goal_tspawn;
		extern std::shared_ptr<Action> set_goal_ctspawn;
		extern std::shared_ptr<Action> set_goal_weapon;
		extern std::shared_ptr<Action> find_goal;
		extern std::shared_ptr<Action> head_to_goal;
		BEHAVIOR_CREATE_INLINE(Action, set_goal_c4_node);
		BEHAVIOR_CREATE_INLINE(Action, set_goal_c4_vector);
		BEHAVIOR_CREATE_INLINE(Action, rapid_fire);
		BEHAVIOR_CREATE_INLINE(Action, move_vector);
		BEHAVIOR_CREATE_INLINE(Action, set_goal_from_team_objective_within_range);
		std::shared_ptr<Action> wait(std::uint32_t, float);

		namespace fight {
			extern std::shared_ptr<Priority> while_spotting_enemy;
		}

		// - DEmolition Behaviors - 
		namespace demolition {
			BEHAVIOR_CREATE_INLINE(Priority, t_plant);			// The terrorist plant the bomb.
			BEHAVIOR_CREATE_INLINE(Priority, t_planted_wary);	// Terrorists make the rounds to defend the bomb.
			BEHAVIOR_CREATE_INLINE(Priority, t_planted_camp);	// Terrorirts camp around c4 to defend the bomb.
			BEHAVIOR_CREATE_INLINE(Priority, t_defusing);
			
			BEHAVIOR_CREATE_INLINE(Priority, ct_defend);
			BEHAVIOR_CREATE_INLINE(Priority, ct_defend_wary);
			BEHAVIOR_CREATE_INLINE(Priority, ct_defend_camp);
			BEHAVIOR_CREATE_INLINE(Priority, ct_planted);
			BEHAVIOR_CREATE_INLINE(Priority, ct_defusing);
		}

		// - Rescue Behaviors -
		namespace rescue {
			extern std::shared_ptr<Sequence> ct_try;
			extern std::shared_ptr<Sequence> ct_leave;
			extern std::shared_ptr<Sequence> lead_hostage;
		}

		// - ASsasination Behaviors -
		namespace assist {
			extern std::shared_ptr<Sequence> ct_cover;
			extern std::shared_ptr<Sequence> ct_take_point;
			extern std::shared_ptr<Sequence> ct_vip_escape;
		}
	
		// - EScape Behaviors -
		namespace escape {
			extern std::shared_ptr<Sequence> t_get_primary;
			extern std::shared_ptr<Sequence> t_take_point;
		}

	}
}