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

		class Memory final {
			std::vector<Event> memories{};
			float Evaluate(const int);
			bool Save();
		public:
			void Memorize(const Vector&, EventType, float);
			void Clear();
		};

		enum class Message {
			Normal,
			Buy,
			Team_Select,
			Model_Select,
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
				assert(ID >= 0);
				if (ID == node::Invalid_NodeID)
					return false;

				return queue.insert(Element{ .ID = ID, .Priority = Priority }).second;
			}

			bool IsEmpty() const noexcept { return queue.empty(); }
			node::NodeID Get() const noexcept { return (queue.empty() ? node::Invalid_NodeID : queue.cbegin()->ID); }
			void Pop() noexcept { queue.erase(queue.begin()); }
			void Remove(const node::NodeID ID) noexcept { queue.erase(std::find_if(queue.begin(), queue.end(), [ID](const Element& E) { return E.ID == ID; })); }
			void Clear() noexcept { queue.clear(); }
		};

		struct Mood {
			int brave = 100;
			int coop;
		};

		class Bot {
			friend class Manager;

			std::shared_ptr<game::Client> client{};
			pokebot::common::Time 
				frame_interval{}, 
				last_command_time{};

			bool can_listen_radio{},
				can_follow_radio{};

			Timer update_timer{},
				allow_listen_radio_timer{};

			Timer freeze_time{};
			Timer buy_wait_timer{};

			Memory memory{};
			Message start_action{};

			common::Team team{};
			common::Model model{};

			game::Weapon current_weapon = game::Weapon::Knife;

			common::AngleVector movement_angle{};
			float move_speed{}, strafe_speed{};

			std::unordered_set<const game::Client*> target{};


			void DecideBehavior();
			void BehaviorUpdate() noexcept;

			std::uint8_t ComputeMsec() noexcept;
			
			void SelectionUpdate() noexcept;
			void NormalUpdate() noexcept;
			void BuyUpdate() noexcept;
			void CheckAround();

			void TurnViewAngle(), TurnMovementAngle();

			Timer danger_time{};

			static constexpr int MATE = 0, ENEMY = 1;
			std::vector<const edict_t*> entities[2]{};
		public:
			int squad = -1;
			Mood mood{};
			Timer behavior_wait_timer{};

			bool need_to_update{};
			GoalQueue goal_queue{};

			node::PathWalk routes{};
			pokebot::node::NodeID goal_node{};
			pokebot::node::NodeID next_dest_node{};

			bool IsHelpingMate() const noexcept { return false; }

			struct { 
				std::optional<common::PositionVector> view{}, movement{}; 
				void Clear() noexcept { view = movement = std::nullopt; }
			} look_direction{}, ideal_direction{};


			Bot(std::shared_ptr<game::Client>, const common::Team, const common::Model) POKEBOT_DEBUG_NOEXCEPT;

			void OnNewRound() POKEBOT_DEBUG_NOEXCEPT;
			void Run() POKEBOT_DEBUG_NOEXCEPT;

			void SelectWeapon(const game::Weapon);

			void LookAtClosestEnemy();
			bool IsLookingAtEnemy() const noexcept;
			bool IsEnemyFar() const noexcept;

			void OnRadioRecieved(const std::string& Sender_Name, const std::string& Radio_Sentence) noexcept;

			void OnBombPlanted() noexcept;

			// -- Setters --

			void SetGoal(const node::NodeID) noexcept;
			void PressKey(ActionKey);
			bool IsPressingKey(const ActionKey) const noexcept;

			// -- Getters --

			std::string_view Name() const noexcept { return client->Name(); }
			Vector Origin() const noexcept;
			float Health() const noexcept;

			bool IsLookingAt(const Vector& Dest, const float Range) const noexcept;
		
			bool HasPrimaryWeapon() const noexcept { return bool(client->Edict()->v.weapons & game::Primary_Weapon_Bit); }
			bool HasSecondaryWeapon() const noexcept { return bool(client->Edict()->v.weapons & game::Secondary_Weapon_Bit); }
			bool HasWeapon(const game::Weapon Weapon_ID) const noexcept { return bool(client->Edict()->v.weapons & common::ToBit<int>(Weapon_ID)); }
			bool IsCurrentWeapon(const game::Weapon Weapon_ID) const noexcept { return (current_weapon == Weapon_ID); }

			bool HasGoalToHead() const noexcept;
			bool IsInBuyzone() const noexcept { return client->IsShowingIcon(game::StatusIcon::Buy_Zone); }

			bool IsDucking() const noexcept { return (client->IsDucking()); }
			bool IsDriving() const noexcept { return (client->IsOnTrain()); }
			bool IsSwimming() const noexcept { return (client->IsInWater()); }
			bool IsOnFloor() const noexcept { return (client->IsOnFloor()); }

			bool IsFighting() const noexcept { return danger_time.IsRunning(); }
			bool CanSeeEnemy() const noexcept;

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

		class Squad {
			Policy policy{};

			std::string leader_name{};
			std::unordered_set<std::string> members{};
			size_t limit{};
		public:
			const std::string& LeaderName() const noexcept { return leader_name; }
			const Policy GetPolicy() const noexcept { return policy; }

			bool Join(const std::string&);
			bool Left(const std::string&);

			void Update();

			Squad(const std::string& Leader_Name, const size_t Number_Limit, const Policy Initial_Policy);
		};

		struct RadioMessage {
			common::Team team;
			std::string sender;
			std::string message;
		};

		struct BotBalancer final {
			Vector gap{};
		};

		inline class Manager {
			std::optional<Vector> c4_origin{};

			friend class pokebot::message::MessageDispatcher;
			std::unordered_map<std::string, Bot> bots{};
			std::unordered_map<std::string, BotBalancer> balancer{};

			std::vector<Squad> squads[2]{};
			Bot* const Get(const std::string&) noexcept;
			RadioMessage radio_message{};
		public:
			void OnNewRound();
			const Vector& GetCompensation(const std::string& Bot_Name) { return balancer[Bot_Name].gap; }

			void Insert(std::string bot_name, const common::Team, const common::Model, const bot::Difficult) POKEBOT_DEBUG_NOEXCEPT;
			void Kick(const std::string& Bot_Name) noexcept;
			void Remove(const std::string& Bot_Name) noexcept;
			void Update() noexcept;

			bool IsExist(const std::string& Bot_Name) const noexcept;
			void Assign(const std::string_view Bot_Name, Message message) noexcept;
			void OnDied(const std::string& Bot_Name) noexcept;
			void OnDamageTaken(const std::string_view Bot_Name, const edict_t* Inflictor, const int Damage, const int Armor, const int Bit) noexcept;
			void OnJoinedTeam(const std::string&) noexcept;
			void OnChatRecieved(const std::string&) noexcept;
			void OnTeamChatRecieved(const std::string&) noexcept;
			void OnRadioRecieved(const std::string& Sender_Name, const std::string& Radio_Sentence) noexcept;

			void OnBombPlanted() noexcept;

			int SetupLonelySquad(const std::string&);
			int SetupVipSquad(const std::string&);
			int SetupHelperSquad(const std::string&);
			int SetupDefenseSquad(const std::string&);
			int SetupOffenseSquad(const std::string&);
			int SetupPlayerSquad();

			// Join a bot to a squad with matching policies. 
			int JoinSquad(const std::string&, Policy Will_Policy);
			void LeftSquad(const std::string&);
			std::shared_ptr<game::Client> GetSquadLeader(const common::Team, const int Squad_Index);

			bool IsBotLeader(const std::string&, const int) const noexcept;

			Policy SquadPolicy(const int) const noexcept;

			// -- Getters --

			void GetManagingNames(std::forward_list<std::string>* names);
			Vector GetOrigin(const std::string& Bot_Name) const noexcept;
			float GetHealth(const std::string& Bot_Name) const noexcept;

			// --

			const std::optional<Vector>& C4Origin() const noexcept { return c4_origin; }
		} manager{};
	}
}