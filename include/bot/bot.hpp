#pragma once
#include "util/timer.hpp"

namespace pokebot::message {
	class MessageDispatcher;
}

namespace pokebot::bot {
	class Bot;

	using PlatoonID = std::optional<int>;
	constexpr auto Not_Joined_Any_Platoon = std::nullopt;

	struct WeaponInfo {
		int max_clip{};
		int max_ammo{};
		int threat{};
	};
	
	enum class Difficult {
		Easy,
		Normal,
		Hard
	};

	enum class Message {
		Normal,
		Buy,
		Team_Select,
		Model_Select,
		Selection_Completed
	};

	POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(ActionKey,
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
	);

	enum class State {
		Accomplishment,	// The bot mainly does accomplish the mission
		Crisis,			// The bot is in dangerous situation.
		Follow,			// Follow the leader
		Stuck			// 
	};

	class GoalQueue {
		struct Element final {
			pokebot::node::NodeID ID = node::Invalid_NodeID;
			int Priority{};
		};

		inline static auto Compare = [](const Element& a, const Element& b) POKEBOT_NOEXCEPT{
			return a.Priority > b.Priority;
		};
		std::set<Element, decltype(Compare)> queue{};
	public:
		bool AddGoalQueue(const node::NodeID ID) POKEBOT_NOEXCEPT {
			return AddGoalQueue(ID, 0);
		}

		bool AddGoalQueue(const node::NodeID ID, const int Priority) POKEBOT_NOEXCEPT {
			assert(ID != node::Invalid_NodeID);
			return queue.insert(Element{ .ID = ID, .Priority = Priority }).second;
		}

		bool IsEmpty() const POKEBOT_NOEXCEPT { return queue.empty(); }
		node::NodeID Get() const POKEBOT_NOEXCEPT { return (queue.empty() ? node::Invalid_NodeID : queue.cbegin()->ID); }
		void Pop() POKEBOT_NOEXCEPT { queue.erase(queue.begin()); }
		void Remove(const node::NodeID ID) POKEBOT_NOEXCEPT { queue.erase(std::find_if(queue.begin(), queue.end(), [ID](const Element& E) { return E.ID == ID; })); }
		void Clear() POKEBOT_NOEXCEPT { queue.clear(); }
	};

	template<int min, int max>
	class PersonalityItem {
		int value{};
	public:
		inline PersonalityItem& operator=(const auto& v) POKEBOT_NOEXCEPT { value = std::clamp(v, min, max); return *this; }
		operator int() const POKEBOT_NOEXCEPT { return value; }
		PersonalityItem() : PersonalityItem(0) {}
		PersonalityItem(const auto& v) { operator=(v); }
	};

	struct Mood {
		PersonalityItem<0, 100> brave{};
		PersonalityItem<0, 100> coop{};
	};

	class Bot {
		friend class Manager;
		friend class Troops;

		util::Time stopping_time{};

		util::Time
			frame_interval{},
			last_command_time{};

		// This variable is used to prevent freezing the whole game.
		util::Timer spawn_cooldown_time{ util::GetRealGlobalTime };

		// Make the bot to do nothing except buying while round freeze.
		util::Timer freeze_time{ util::GetRealGlobalTime };
		// 
		util::Timer buy_wait_timer{ util::GetRealGlobalTime };

		// - Stuck Checker -
		// 
		util::Timer stuck_check_interval_timer{ util::GetRealGlobalTime };
		Vector stuck_check_origin{};

		// - Lock -
		bool is_locked_by_bomb{};

		Message start_action{};

		PlatoonID platoon = Not_Joined_Any_Platoon;
		game::Team team{};
		game::Model model{};

		game::Weapon current_weapon = game::Weapon::Knife;

		Vector movement_angle{};
		float move_speed{}, strafe_speed{};

		std::uint8_t ComputeMsec() POKEBOT_NOEXCEPT;

		void SelectionUpdate() POKEBOT_NOEXCEPT;
		void NormalUpdate() POKEBOT_NOEXCEPT;
		void BuyUpdate() POKEBOT_NOEXCEPT;
		void OnSelectionCompleted() POKEBOT_NOEXCEPT;
		void CheckAround();

		void CheckBlocking() noexcept;

		void TurnViewAngle(), TurnMovementAngle();

		ActionKey press_key{};
		util::Timer danger_time{ util::GetRealGlobalTime };

		std::vector<util::PlayerName> target_enemies{};

		State state = State::Accomplishment;
		void AccomplishMission() POKEBOT_NOEXCEPT;
		void Combat() POKEBOT_NOEXCEPT;
		void Follow() POKEBOT_NOEXCEPT;

		/**
		* @brief Try to unstuck.
		* 
		*/
		void TryToUnstuck() POKEBOT_NOEXCEPT;

		void (Bot::*doObjective[4])() = {
			// Basic
			&Bot::AccomplishMission,
			&Bot::Combat,
			&Bot::Follow,
			&Bot::TryToUnstuck
		};

		void OnTerroristDemolition() noexcept, 
			OnTerroristHostage() noexcept, 
			OnTerroristAssasination() noexcept, 
			OnTerroristEscape() noexcept;

		void OnCTDemolition() noexcept, 
			OnCTHostage() noexcept, 
			OnCTAssasination() noexcept, 
			OnCTEscape() noexcept;

		void (Bot::*accomplishState[2][5])() = {
			{
				&Bot::OnTerroristDemolition,
				&Bot::OnTerroristHostage,
				&Bot::OnTerroristAssasination,
				&Bot::OnTerroristEscape
			},
			{
				&Bot::OnCTDemolition,
				&Bot::OnCTHostage,
				&Bot::OnCTAssasination,
				&Bot::OnCTEscape
			}
		};

		util::PlayerName name{};

		void JoinPlatoon(const PlatoonID Target_Platoon) noexcept;
		void LeavePlatoon() noexcept { platoon = Not_Joined_Any_Platoon; }

		void (Bot::*updateFuncs[5])() = {
			&Bot::NormalUpdate,
			&Bot::BuyUpdate,
			&Bot::SelectionUpdate,	// Team Select
			&Bot::SelectionUpdate,	// Model Select
			&Bot::OnSelectionCompleted 
		};
	public:
		void LockByBomb() { is_locked_by_bomb = true; }
		void UnlockByBomb() { is_locked_by_bomb = false; }

		void ReceiveCommand(const TroopsStrategy&);

		Mood personality{};
		Mood mood{};
		util::Timer behavior_wait_timer{ util::GetRealGlobalTime };

		GoalQueue goal_queue{};
#if !USE_NAVMESH
		node::PathWalk<node::NodeID> routes{};
#else
		node::PathWalk<std::uint32_t> routes{};
#endif
		std::optional<Vector> goal_vector{};
		pokebot::node::NodeID goal_node{};
		pokebot::node::NodeID next_dest_node{};

		bool IsHelpingMate() const POKEBOT_NOEXCEPT { return false; }

		struct {
			std::optional<Vector> view{}, movement{};
			void Clear() POKEBOT_NOEXCEPT { view = movement = std::nullopt; }
		} look_direction{}, ideal_direction{};


		Bot(const std::string_view&, const game::Team, const game::Model) POKEBOT_NOEXCEPT;

		void OnNewRound() POKEBOT_NOEXCEPT;
		void Run() POKEBOT_NOEXCEPT;

		void SelectWeapon(const game::Weapon), SelectPrimaryWeapon(), SelectSecondaryWeapon();

		bool HasEnemy() const POKEBOT_NOEXCEPT;
		void LookAtClosestEnemy();
		bool IsLookingAtEnemy() const POKEBOT_NOEXCEPT;
		bool IsEnemyFar() const POKEBOT_NOEXCEPT;

		inline bool IsGoodCondition() const POKEBOT_NOEXCEPT { return Health() >= 50; }

		void OnRadioRecieved(const std::string_view& Sender_Name, const std::string_view& Radio_Sentence) POKEBOT_NOEXCEPT;

		void OnBombPlanted() POKEBOT_NOEXCEPT;

		// -- Setters --

		void SetGoal(const node::NodeID) POKEBOT_NOEXCEPT;
		void PressKey(ActionKey);
		bool IsPressingKey(const ActionKey) const POKEBOT_NOEXCEPT;
		bool IsFollowing() const POKEBOT_NOEXCEPT { return false; }

		// -- Getters --

		const util::PlayerName& Name() const POKEBOT_NOEXCEPT;
		Vector Origin() const POKEBOT_NOEXCEPT;
		float Health() const POKEBOT_NOEXCEPT;

		bool IsLookingAt(const Vector& Dest, const float Range) const POKEBOT_NOEXCEPT;

		bool HasPrimaryWeapon() const POKEBOT_NOEXCEPT;
		bool HasSecondaryWeapon() const POKEBOT_NOEXCEPT;
		bool HasWeapon(const game::Weapon Weapon_ID) const POKEBOT_NOEXCEPT;
		bool IsCurrentWeapon(const game::Weapon Weapon_ID) const POKEBOT_NOEXCEPT { return (current_weapon == Weapon_ID); }

		bool HasGoalToHead() const POKEBOT_NOEXCEPT;
		bool IsInBuyzone() const POKEBOT_NOEXCEPT;

		bool IsInBombTargetZone() const POKEBOT_NOEXCEPT;

		bool IsFighting() const POKEBOT_NOEXCEPT { return danger_time.IsRunning(); }
		bool CanSeeEntity(const edict_t*) const POKEBOT_NOEXCEPT;

		PlatoonID JoinedPlatoon() const POKEBOT_NOEXCEPT;
		game::Team JoinedTeam() const POKEBOT_NOEXCEPT;
		float GetSecondLeftToCompleteReloading() const POKEBOT_NOEXCEPT;

		/* - Client Wrapper - */

		bool IsDucking() const POKEBOT_NOEXCEPT;
		bool IsDriving() const POKEBOT_NOEXCEPT;
		bool IsInWater() const POKEBOT_NOEXCEPT;
		bool IsSwimming() const POKEBOT_NOEXCEPT;
		bool IsOnFloor() const POKEBOT_NOEXCEPT;
		bool IsClimbingLadder() const POKEBOT_NOEXCEPT;
		bool IsReloading() const POKEBOT_NOEXCEPT;
		bool IsPlantingBomb() const POKEBOT_NOEXCEPT;
		bool IsChangingWeapon() const POKEBOT_NOEXCEPT;
		bool IsFalling() const POKEBOT_NOEXCEPT;
		bool Jumped() const POKEBOT_NOEXCEPT;
		bool IsJumping() const POKEBOT_NOEXCEPT;
		bool IsLeadingHostages() const POKEBOT_NOEXCEPT;
		bool IsLookingThroughScope() const POKEBOT_NOEXCEPT;
		bool IsLookingThroughCamera() const POKEBOT_NOEXCEPT;
		bool IsChangingSilencer() const POKEBOT_NOEXCEPT;
		bool IsEnabledFlashlight() const POKEBOT_NOEXCEPT;
		bool IsEnabledNightvision() const POKEBOT_NOEXCEPT;
	};
}