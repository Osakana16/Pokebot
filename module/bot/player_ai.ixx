export module pokebot.bot: player_ai;
import :goal_queue;
import :message;
import :difficult;
import :personality_item;
import :mood;
import :state_machine;

import pokebot.terrain.graph.path;
import pokebot.game.player;

export namespace pokebot::bot {
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

		pokebot::game::player::ActionKey press_key{};
		util::Timer danger_time{ util::GetRealGlobalTime };

		std::vector<pokebot::util::PlayerName> target_enemies{};

		State state = State::Accomplishment;
		void AccomplishMission() POKEBOT_NOEXCEPT;
		void Combat() POKEBOT_NOEXCEPT;
		void Follow() POKEBOT_NOEXCEPT;

		/**
		* @brief Try to unstuck.
		*
		*/
		void TryToUnstuck() POKEBOT_NOEXCEPT;

		void (Bot::* doObjective[4])() = {
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

		void (Bot::* accomplishState[2][5])() = {
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

		pokebot::util::PlayerName name{};

		void (Bot::* updateFuncs[5])() = {
			&Bot::NormalUpdate,
			&Bot::BuyUpdate,
			&Bot::SelectionUpdate,	// Team Select
			&Bot::SelectionUpdate,	// Model Select
			&Bot::OnSelectionCompleted
		};
	public:
		void LockByBomb() { is_locked_by_bomb = true; }
		void UnlockByBomb() { is_locked_by_bomb = false; }

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

		void PressKey(game::player::ActionKey pressable_key) {
			press_key |= pressable_key;
		}

		bool IsPressingKey(const game::player::ActionKey Key) const noexcept;
		bool IsFollowing() const POKEBOT_NOEXCEPT { return false; }

		// -- Getters --

		const pokebot::util::PlayerName& Name() const POKEBOT_NOEXCEPT;
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