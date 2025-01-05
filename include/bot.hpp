#pragma once
namespace pokebot::message {
	class MessageDispatcher;
}

namespace pokebot {
	void ParseWeaponJson();
	namespace bot {
		class Bot;

		struct WeaponInfo {
			int max_clip{};
			int max_ammo{};
			int threat{};
		};

		enum class EventType {
			Invalid = -1,
			Found_Enemy,			// Found an enemy with own eyes.
			Heard_Footsteps,		// Heard the enemy's footsteps with own ears.
			Jump,				// Tried to jump.
			Duck,				// Tried to duck.
			Got_Damage,			// Got damage by something(Fall, explosion, or etc...).
			Decide_Destination,	// 
			Received_Help,		// 
			Received_Found_Enemy,	// 
			Received_Clear,		// 
			Sent_Help,			// 
		};

		struct Event final {
			Vector source{};
			EventType type{};
			float time_occurrence{};
			int parent{};
		};

		enum class Message {
			Normal,
			Buy,
			Team_Select,
			Model_Select,
			Selection_Completed
		};

		class Timer final {
			common::Time time{};

			const common::Time& Base_Time = gpGlobals->time;
		public:
			bool IsRunning() const noexcept { return time >= Base_Time; }
			void SetTime(const common::Time t) noexcept { time = t + Base_Time; }
		};

		enum class ActionKey {
			None = 0,
			Run = IN_RUN,
			Attack = IN_ATTACK,
			Jump = IN_JUMP,
			Duck = IN_DUCK,
			Forward = IN_FORWARD,
			Back = IN_BACK,
			Use = IN_USE,
			Cancel = IN_CANCEL,
			Left = IN_LEFT,
			Right = IN_RIGHT,
			Move_Left = IN_MOVELEFT,
			Move_Right = IN_MOVERIGHT,
			Attack2 = IN_ATTACK2,
			Reload = IN_RELOAD,
			ALT1 = IN_ALT1,
			Score = IN_SCORE,
			Shift = 1 << 16
		};
		POKEBOT_ENUM_BIT_OPERATORS(ActionKey);

		enum class State {
			Accomplishment,
			Crisis
		};

		class GoalQueue {
			struct Element final {
				pokebot::node::NodeID ID = node::Invalid_NodeID;
				int Priority{};
			};

			inline static auto Compare = [](const Element& a, const Element& b) noexcept {
				return a.Priority > b.Priority;
			};
			std::set<Element, decltype(Compare)> queue{};
		public:
			bool AddGoalQueue(const node::NodeID ID) noexcept {
				return AddGoalQueue(ID, 0);
			}

			bool AddGoalQueue(const node::NodeID ID, const int Priority) noexcept {
				assert(ID != node::Invalid_NodeID);
				return queue.insert(Element{ .ID = ID, .Priority = Priority }).second;
			}

			bool IsEmpty() const noexcept { return queue.empty(); }
			node::NodeID Get() const noexcept { return (queue.empty() ? node::Invalid_NodeID : queue.cbegin()->ID); }
			void Pop() noexcept { queue.erase(queue.begin()); }
			void Remove(const node::NodeID ID) noexcept { queue.erase(std::find_if(queue.begin(), queue.end(), [ID](const Element& E) { return E.ID == ID; })); }
			void Clear() noexcept { queue.clear(); }
		};

		template<int min, int max>
		class PersonalityItem {
			int value{};
		public:
			inline PersonalityItem& operator=(const auto& v) noexcept { value = std::clamp(v, min, max); return *this; }
			operator int() const noexcept { return value; }
			PersonalityItem() : PersonalityItem(0) {}
			PersonalityItem(const auto& v) { operator=(v); }
		};

		struct Mood {
			PersonalityItem<0, 100> brave{};
			PersonalityItem<0, 100> coop{};
		};

		struct TroopsStrategy {
			node::NodeID objective_goal_node = node::Invalid_NodeID;
			enum class Strategy {
				/*- Demolition -*/
				Plant_C4_Specific_Bombsite,		// Plant the bomb on the specific bomb site.
				Defend_Bombsite_Concentrative,	// Defend the specific bombsite
				Defend_Bombsite_Divided,		// Divide the team and defend the bomb.
				/*- Hostage Rescue -*/
				Prevent_Hostages,
				Prevent_Rescuezones,
				Rush_And_Rescue,
				/*- Assasination -*/
				Prevent_Safety,
				Rush_Safety,
				/*- Escape -*/
				Rush_Escapezone,
				Prevent_Escapezone,
			} strategy;
		};

		class Troops final {
			TroopsStrategy strategy;
			TroopsStrategy old_strategy;
			std::function<bool(const std::pair<std::string, Bot>& target)> leader_condition;
			std::function<bool(const std::pair<std::string, Bot>& target)> condition;

			common::Team team{};
			Troops* parent{};
			std::vector<Troops> platoons{};
		public:
			common::Team Team() { return team; }
			Troops(decltype(condition) target_condition, decltype(leader_condition) target_leader_condition, decltype(team) target_team) : condition(target_condition), leader_condition(target_leader_condition), team(target_team) {}
			bool IsRoot() const noexcept { return parent == nullptr; }
			int CreatePlatoon(decltype(condition) target_condition, decltype(condition) target_leader_condition);
			bool DeletePlatoon(const int Index);

			void DecideStrategy(std::unordered_map<std::string, Bot>* bots);
			void Command(std::unordered_map<std::string, Bot>* bots);
			void SetNewStrategy(const TroopsStrategy&);
			bool HasGoalBeenDevised(const node::NodeID) const noexcept;
			bool HasGoalBeenDevisedByOtherPlatoon(const node::NodeID) const noexcept;
			bool NeedToDevise() const noexcept;

			node::NodeID GetGoalNode() const noexcept { return strategy.objective_goal_node; }
			
			Troops& operator[](const int index) { return platoons[index]; }
			const Troops& at(const int index) const { return platoons[index]; }

			auto begin() { return platoons.begin(); }
			auto end() { return platoons.end(); }
			auto GetPlatoonSize() { return platoons.size(); }
		};

		class Bot {
			friend class Manager;
			friend class Troops;

			common::Time 
				frame_interval{}, 
				last_command_time{};

			// This variable is used to prevent freezing the whole game.
			Timer spawn_cooldown_time{};

			// Make the bot to do nothing except buying while round freeze.
			Timer freeze_time{};
			Timer buy_wait_timer{};

			Message start_action{};
			
			int platoon = -1;
			common::Team team{};
			common::Model model{};

			game::Weapon current_weapon = game::Weapon::Knife;

			common::AngleVector movement_angle{};
			float move_speed{}, strafe_speed{};

			std::uint8_t ComputeMsec() noexcept;
			
			void SelectionUpdate() noexcept;
			void NormalUpdate() noexcept;
			void BuyUpdate() noexcept;
			void OnSelectionCompleted() noexcept;
			void CheckAround();

			void TurnViewAngle(), TurnMovementAngle();

			Timer danger_time{};

			std::vector<std::string> target_enemies{};

			State state = State::Accomplishment;
			void AccomplishMission() noexcept, Combat() noexcept;
			std::string name{};
		public:
			void ReceiveCommand(const TroopsStrategy&);

			Mood personality{};
			Mood mood{};
			Timer behavior_wait_timer{};

			GoalQueue goal_queue{};
#if !USE_NAVMESH
			node::PathWalk<node::NodeID> routes{};
#else
			node::PathWalk<std::uint32_t> routes{};
#endif
			std::optional<Vector> goal_vector{};
			pokebot::node::NodeID goal_node{};
			pokebot::node::NodeID next_dest_node{};

			bool IsHelpingMate() const noexcept { return false; }

			struct { 
				std::optional<common::PositionVector> view{}, movement{}; 
				void Clear() noexcept { view = movement = std::nullopt; }
			} look_direction{}, ideal_direction{};


			Bot(const std::string&, const common::Team, const common::Model) noexcept;

			void OnNewRound() noexcept;
			void Run() noexcept;

			void SelectWeapon(const game::Weapon), SelectPrimaryWeapon(), SelectSecondaryWeapon();

			bool HasEnemy() const noexcept;
			void LookAtClosestEnemy();
			bool IsLookingAtEnemy() const noexcept;
			bool IsEnemyFar() const noexcept;

			inline bool IsGoodCondition() const noexcept { return Health() >= 50; }

			void OnRadioRecieved(const std::string& Sender_Name, const std::string& Radio_Sentence) noexcept;

			void OnBombPlanted() noexcept;

			// -- Setters --

			void SetGoal(const node::NodeID) noexcept;
			void PressKey(ActionKey);
			bool IsPressingKey(const ActionKey) const noexcept;
			bool IsFollowing() const noexcept { return false; }

			// -- Getters --

			std::string_view Name() const noexcept;
			Vector Origin() const noexcept;
			float Health() const noexcept;

			bool IsLookingAt(const Vector& Dest, const float Range) const noexcept;
		
			bool HasPrimaryWeapon() const noexcept;
			bool HasSecondaryWeapon() const noexcept;
			bool HasWeapon(const game::Weapon Weapon_ID) const noexcept;
			bool IsCurrentWeapon(const game::Weapon Weapon_ID) const noexcept { return (current_weapon == Weapon_ID);  }

			bool HasGoalToHead() const noexcept;
			bool IsInBuyzone() const noexcept;

			bool IsInBombTargetZone() const noexcept;

			bool IsFighting() const noexcept { return danger_time.IsRunning(); }
			std::string GetEnemyWithinView() const noexcept;
			bool CanSeeEntity() const noexcept;

			int JoinedPlatoon() const noexcept;
			common::Team JoinedTeam() const noexcept;
			float GetSecondLeftToCompleteReloading() const noexcept;

			/* - Client Wrapper - */

			bool IsDucking() const noexcept;
			bool IsDriving() const noexcept;
			bool IsInWater() const noexcept;
			bool IsSwimming() const noexcept;
			bool IsOnFloor() const noexcept;
			bool IsClimbingLadder() const noexcept;
			bool IsReloading() const noexcept;
			bool IsPlantingBomb() const noexcept;
			bool IsChangingWeapon() const noexcept;
			bool IsFalling() const noexcept;
			bool Jumped() const noexcept;
			bool IsJumping() const noexcept;
			bool IsLeadingHostages() const noexcept;
			bool IsLookingThroughScope() const noexcept;
			bool IsLookingThroughCamera() const noexcept;
			bool IsChangingSilencer() const noexcept;
			bool IsEnabledFlashlight() const noexcept;
			bool IsEnabledNightvision() const noexcept;
		};

		enum class Difficult {
			Easy,
			Normal,
			Hard
		};

		enum class Policy {
			Elimination,
			Survival,		// Survive until the round ends.
			Defense,
			Offense,
			Player
		};

		struct RadioMessage {
			common::Team team;
			std::string sender;
			std::string message;
		};

		struct BotBalancer final {
			Vector gap{};
		};

        /**
        * @brief Manager class for Bot.
        */
		class Manager final : private common::Singleton<Manager> {
			std::optional<Vector> c4_origin{};

			Troops troops[2];
			friend class pokebot::message::MessageDispatcher;
			std::unordered_map<std::string, Bot> bots{};
			std::unordered_map<std::string, BotBalancer> balancer{};

			Bot* const Get(const std::string&) noexcept;
			RadioMessage radio_message{};
			Manager();
		public:
			/**
			* @brief Get the instance of Manager
			* @return The instance of Manager
			*/
			static Manager& Instance() noexcept {
				static Manager manager{};
				return manager;
			}

			/**
			* @brief Called when a new round starts.
			*/
			void OnNewRound() noexcept;

			/**
			* @brief Get the compensation vector for a bot.
			* @param Bot_Name The name of the bot.
			* @return The compensation vector.
			*/
			const Vector& GetCompensation(const std::string& Bot_Name) { return balancer[Bot_Name].gap; }

			/**
			* @brief Update the state of all bots.
			*/
			void Update() noexcept;

			/**
			* @brief Insert a new bot into the game.
			* @param bot_name The name of the bot.
			* @param team The team of the bot.
			* @param model The model of the bot.
			* @param difficulty The difficulty level of the bot.
			*/
			void Insert(std::string bot_name, const common::Team team, const common::Model model, const bot::Difficult difficulty) noexcept;

			/**
			* @brief Kick a bot from the game.
			* @param Bot_Name The name of the bot.
			*/
			void Kick(const std::string& Bot_Name) noexcept;

			/**
			* @brief Remove a bot from the game.
			* @param Bot_Name The name of the bot.
			*/
			void Remove(const std::string& Bot_Name) noexcept;

			/**
			* @brief Check if a bot exists by name.
			* @param Bot_Name The name of the bot.
			* @return true if the bot exists, false otherwise.
			*/
			bool IsExist(const std::string& Bot_Name) const noexcept;

			/**
			* @brief Assign an engine message to a bot.
			* @param Bot_Name The name of the bot.
			* @param message The message to assign.
			*/
			void Assign(const std::string_view Bot_Name, Message message) noexcept;

			/**
			* @brief Called when a bot dies.
			* @param Bot_Name The name of the bot.
			*/
			void OnDied(const std::string& Bot_Name) noexcept;

			/**
			* @brief Called when a bot takes damage.
			* @param Bot_Name The name of the bot.
			* @param Inflictor The entity that inflicted the damage.
			* @param Damage The amount of damage taken.
			* @param Armor The amount of armor remaining.
			* @param Bit Additional information about the damage.
			*/
			void OnDamageTaken(const std::string_view Bot_Name, const edict_t* Inflictor, const int Damage, const int Armor, const int Bit) noexcept;

			/**
			* @brief Called when a bot joins a team.
			* @param Bot_Name The name of the bot.
			*/
			void OnJoinedTeam(const std::string&) noexcept;

			/**
			* @brief Called when a chat message is received.
			* @param Bot_Name The name of the bot.
			*/
			void OnChatRecieved(const std::string&) noexcept;

			/**
			* @brief Called when a team chat message is received.
			* @param Bot_Name The name of the bot.
			*/
			void OnTeamChatRecieved(const std::string&) noexcept;

			/**
			* @brief Called when a radio message is received.
			* @param Sender_Name The name of the sender.
			* @param Radio_Sentence The radio message.
			*/
			void OnRadioRecieved(const std::string& Sender_Name, const std::string& Radio_Sentence) noexcept;

			/**
			* @brief Called when the bomb is planted.
			*/
			void OnBombPlanted() noexcept;

			/**
			* @brief Called when a bot has completely joined the game.
			* @param bot The bot that joined.
			*/
			void OnBotJoinedCompletely(Bot* const) noexcept;

			/**
			* @brief Get the goal node ID for a troop or platoon.
			* @param Target_Team The team of the troop.
			* @param Index The platoon index.
			* @return The goal node ID. If Index is less than 0, returns the troop's goal node ID.
			*/
			node::NodeID GetGoalNode(const common::Team Target_Team, const int Index) const noexcept;

			/**
			* @brief Get the origin of the C4 bomb.
			* @return The origin of the C4 bomb.
			*/
			const std::optional<Vector>& C4Origin() const noexcept { return c4_origin; }
		};
	}
}