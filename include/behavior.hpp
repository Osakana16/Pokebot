#pragma once
#define BEHAVIOR_PRIVATE namespace
#define BEHAVIOR_IF(A) [](const Bot* const Self) POKEBOT_NOEXCEPT { return A; }
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
			Running,
			Not_Executed
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
			virtual Status Evaluate(Bot* const) = 0;
		};

		class Root : public BehaviorNode {
			std::shared_ptr<BehaviorNode> child{};
			Status Evaluate(Bot* const self) override {
				return child->Evaluate(self);
			}
		public:
			Root(std::shared_ptr<BehaviorNode> child_) : BehaviorNode("Root"), child(child_) {}
		};

		class Sequence : public BehaviorNode {
			std::vector<std::shared_ptr<BehaviorNode>> children;
		public:
			Status Evaluate(Bot* const self) override;

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

			Status Evaluate(Bot* const self) override;

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
			Status Evaluate(Bot* const self) override {
				if (CanActive(self)) {
					return child->Evaluate(self);
				}
				return Status::Not_Executed;
			}

			Condition(Activator activator, std::shared_ptr<BehaviorNode> behavior) : BehaviorNode("If"), CanActive(activator), child(behavior){}

			static std::shared_ptr<Condition> If(Activator activator, std::shared_ptr<BehaviorNode> behavior) { return std::make_shared<Condition>(activator, behavior);  }
		};

		class Action : public BehaviorNode {
			BehaviorFunc action;
		public:
			using BehaviorNode::BehaviorNode;

			Status Evaluate(Bot* const self) POKEBOT_NOEXCEPT override {
				// SERVER_PRINT(std::format("{}\n", name).c_str());
				return action(self);
			}
			
			void Define(BehaviorFunc action_) POKEBOT_NOEXCEPT {
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

		BEHAVIOR_CREATE_INLINE(Action, change_primary);
		BEHAVIOR_CREATE_INLINE(Action, change_secondary);
		BEHAVIOR_CREATE_INLINE(Action, change_melee);
		BEHAVIOR_CREATE_INLINE(Action, change_grenade);
		BEHAVIOR_CREATE_INLINE(Action, change_flashbang);
		BEHAVIOR_CREATE_INLINE(Action, change_smoke);
		BEHAVIOR_CREATE_INLINE(Action, change_c4);
		BEHAVIOR_CREATE_INLINE(Action, look_c4);
		BEHAVIOR_CREATE_INLINE(Action, look_hostage);
		BEHAVIOR_CREATE_INLINE(Action, look_enemy);
		BEHAVIOR_CREATE_INLINE(Action, look_door);
		BEHAVIOR_CREATE_INLINE(Action, look_button);
		BEHAVIOR_CREATE_INLINE(Action, move_forward);
		BEHAVIOR_CREATE_INLINE(Action, use);
		BEHAVIOR_CREATE_INLINE(Action, tap_fire);
		BEHAVIOR_CREATE_INLINE(Action, jump);
		BEHAVIOR_CREATE_INLINE(Action, duck);
		BEHAVIOR_CREATE_INLINE(Action, walk);
		BEHAVIOR_CREATE_INLINE(Action, change_silencer);
		BEHAVIOR_CREATE_INLINE(Action, adjust_scope);
		BEHAVIOR_CREATE_INLINE(Action, set_goal_hostage_node);
		BEHAVIOR_CREATE_INLINE(Action, set_goal_hostage_vector);
		BEHAVIOR_CREATE_INLINE(Action, set_goal_bombspot);
		BEHAVIOR_CREATE_INLINE(Action, set_goal_rescuezone);
		BEHAVIOR_CREATE_INLINE(Action, set_goal_escapezone);
		BEHAVIOR_CREATE_INLINE(Action, set_goal_vipsafety);
		BEHAVIOR_CREATE_INLINE(Action, set_goal_tspawn);
		BEHAVIOR_CREATE_INLINE(Action, set_goal_ctspawn);
		BEHAVIOR_CREATE_INLINE(Action, set_goal_weapon);
		BEHAVIOR_CREATE_INLINE(Action, find_goal);
		BEHAVIOR_CREATE_INLINE(Action, head_to_goal);
		BEHAVIOR_CREATE_INLINE(Action, set_goal_team_objective);
		BEHAVIOR_CREATE_INLINE(Action, set_goal_from_c4_within_range);
		BEHAVIOR_CREATE_INLINE(Action, reset_goal);
		BEHAVIOR_CREATE_INLINE(Action, set_goal_c4_node);
		BEHAVIOR_CREATE_INLINE(Action, set_goal_c4_vector);
		BEHAVIOR_CREATE_INLINE(Action, set_goal_backpack_node);
		BEHAVIOR_CREATE_INLINE(Action, set_goal_backpack_vector);
		BEHAVIOR_CREATE_INLINE(Action, rapid_fire);
		BEHAVIOR_CREATE_INLINE(Action, lock);
		BEHAVIOR_CREATE_INLINE(Action, move_vector);
		BEHAVIOR_CREATE_INLINE(Action, set_goal_from_team_objective_within_range);

		std::shared_ptr<Action> wait(std::uint32_t, float);

		namespace fight {
			BEHAVIOR_CREATE_INLINE(Sequence, beat_enemies);
			BEHAVIOR_CREATE_INLINE(Priority, retreat);
		}

		// - DEmolition Behaviors - 
		namespace demolition {
			/*
				The bomber plants the bomb.
			*/
			BEHAVIOR_CREATE_INLINE(Priority, t_plant);

			BEHAVIOR_CREATE_INLINE(Priority, t_planted_wary);	// Terrorists make the rounds to defend the bomb.
			BEHAVIOR_CREATE_INLINE(Priority, t_planted_camp);	// Terrorirts camp around c4 to defend the bomb.
			BEHAVIOR_CREATE_INLINE(Priority, t_pick_up_bomb);	// Terrorirts try to pick up the bomb.

			BEHAVIOR_CREATE_INLINE(Priority, t_defusing);
			
			BEHAVIOR_CREATE_INLINE(Priority, ct_defend);
			BEHAVIOR_CREATE_INLINE(Priority, ct_defend_wary);
			BEHAVIOR_CREATE_INLINE(Priority, ct_defend_camp);
			BEHAVIOR_CREATE_INLINE(Priority, ct_planted);
			BEHAVIOR_CREATE_INLINE(Sequence, ct_defusing);
		}

		// - Rescue Behaviors -
		namespace rescue {
			BEHAVIOR_CREATE_INLINE(Priority, t_defend_hostage);

			BEHAVIOR_CREATE_INLINE(Priority, ct_try);
			BEHAVIOR_CREATE_INLINE(Priority, ct_leave);
			BEHAVIOR_CREATE_INLINE(Sequence, lead_hostage);
		}

		// - ASsassination Behaviors -
		namespace assassination {
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