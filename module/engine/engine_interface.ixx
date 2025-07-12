export module pokebot.engine;

import pokebot.game.player;
import pokebot.common.event_handler;
import pokebot.util;

export namespace pokebot::engine {
	using TextCache = pokebot::util::fixed_string<120u>;
	
	enum ScoreStatus : int {
		Nothing = 0,
		Dead = 1 << 0,
		Bomb = 1 << 1,
		VIP = 1 << 2,
		Defuse_Kit = 1 << 3
	};

	template <typename... T>
	using EdictParameter = std::tuple<const edict_t* const, T...>;
	
	struct Observables {
		common::NormalObservable<void> game_commercing_observable{};

		common::NormalObservable<EdictParameter<TextCache>> show_menu_observable{};
		common::NormalObservable<EdictParameter<ScoreStatus>> score_attribute_observable{};
		common::NormalObservable<EdictParameter<game::StatusIcon>> status_icon_observable{};
		common::NormalObservable<EdictParameter<const edict_t* const, int, int, int>> player_damage_taken_observable{};
		common::NormalObservable<void> new_round_observable{};


		common::NormalObservable<void> bomb_dropped_observable{};
		common::NormalObservable<void> bomb_pickedup_observable{};
		common::NormalObservable<void> bomb_planted_observable{};

		common::NormalObservable<void> money_changed_observable{};
		common::NormalObservable<void> nvg_toggled_observable{};
		common::NormalObservable<void> public_chat_observable{};
		common::NormalObservable<void> flashbang_observable{};

		common::NormalObservable<void> join_t_observable{}, join_ct_observable{};

		common::NormalObservable<void> ct_win_observable{}, t_win_observable{};
		common::NormalObservable<void> ct_lose_observable{}, t_lose_observable{};
	};

	enum class ShowMenuPhase {
		Team_Select,
		Model_Select
	};

	class EngineInterface {
	public:
		static void OnDllAttached() noexcept;

		static void OnMessageBegin(int msg_dest, int msg_type, const float* pOrigin, edict_t* edict);
		static void OnMessageEnd();

		static void	OnWriteByte(int value);
		static void OnWriteChar(int value);
		static void OnWriteShort(int value);
		static void OnWriteLong(int value);
		static void OnWriteAngle(float value);
		static void OnWriteCoord(float value);
		static void OnWriteString(const char* sz);
		static void OnWriteEntity(int value);

		static void OnClientCommand(edict_t* pEdict, const char* szFmt, ...);

		static const char* OnArgs();
		static const char* OnArgv(int argc);
		static int OnArgc();

		static void InputFakeclientCommand(edict_t*, util::fixed_string<32u> sentence);
		
		inline static Observables observables{};
	private:
		static int current_message;
		inline static std::vector<std::variant<int, float, TextCache>> args{};

		static const edict_t* engine_target_edict;
		inline static bool is_bot{};
		inline static bool is_host{};
		static bool is_bot_command;

		static void PushArg(int value);
		static void PushArg(float value);
		static void PushArg(const char* const value);

		static void OnVGUIMenuShown();
		static void OnShowMenu();
		static void OnStatusIconShown();
		static void OnWeaponListCalled();
		static void OnTeamInfoCalled();
		static void OnMoneyChanged();
		static void OnPlayerAmmoPickup();
		static void OnPlayerDamageTaken();
		static void OnTextMessageSent();
		static void OnScreenFade();
		static void OnHLTVStart();

		inline static std::vector<util::fixed_string<32u>> bot_args{};
	};
}