export module pokebot.bot: buy;
import :player_ai;

import pokebot.game.client;
import pokebot.engine;
import pokebot.database;
import pokebot.game;
import pokebot.game.weapon;
import pokebot.util.random;
import pokebot.plugin.console.variable;

namespace pokebot::buy {
	template<typename T, T min_money, T max_money>
	class Money final {
		T money;

		static constexpr int Fix(const T Value) POKEBOT_NOEXCEPT {
			return (Value > max_money ? max_money :
					(Value < min_money ? min_money : Value));
		}
	public:
		using BaseType = T;

		constexpr Money(const T Init_Money) POKEBOT_NOEXCEPT : money(Fix(Init_Money)) {}
		constexpr operator T() const POKEBOT_NOEXCEPT { return money; }
		explicit operator bool() = delete;

		template<typename U>
		constexpr Money& operator=(const U value) POKEBOT_NOEXCEPT {
			money = Fix(value);
			return *this;
		}

		template<>Money& operator=(const bool) = delete;

		constexpr Money& operator+=(const Money& value) POKEBOT_NOEXCEPT { money = Fix(money + value); return *this; }
		constexpr Money& operator-=(const Money& value) POKEBOT_NOEXCEPT { money = Fix(money - value); return *this; }
		constexpr Money& operator*=(const Money& value) POKEBOT_NOEXCEPT { money = Fix(money * value); return *this; }
		constexpr Money& operator/=(const Money& value) POKEBOT_NOEXCEPT { money = Fix(money / value); return *this; }
		constexpr Money& operator%=(const Money& value) POKEBOT_NOEXCEPT { money = Fix(money % value); return *this; }
		constexpr bool operator<(const Money& value) POKEBOT_NOEXCEPT { return money < value; }
		constexpr bool operator>(const Money& value) POKEBOT_NOEXCEPT { return money > value; }
		constexpr bool operator<=(const Money& value) POKEBOT_NOEXCEPT { return money <= value; }
		constexpr bool operator>=(const Money& value) POKEBOT_NOEXCEPT { return money >= value; }
		constexpr bool operator==(const Money& value) POKEBOT_NOEXCEPT { return money == value; }
		constexpr bool operator!=(const Money& value) POKEBOT_NOEXCEPT { return money == value; }

		template<typename U>
		constexpr Money operator+(const U& value) POKEBOT_NOEXCEPT { return (money + value); }
		template<typename U>
		constexpr Money operator-(const U& value) POKEBOT_NOEXCEPT { return (money - value); }
		template<typename U>
		constexpr Money operator*(const U& value) POKEBOT_NOEXCEPT { return (money * value); }
		template<typename U>
		constexpr Money operator/(const U& value) POKEBOT_NOEXCEPT { return (money / value); }

		static constexpr T Min_Money = min_money;
		static constexpr T Max_Money = max_money;
	};

	using GameMoney = Money<int, 0, 16000>;

	constexpr std::uint32_t Shield_Flag = 0b000'1'0'0'0'0'00'00'0000000000'0000000000;
	constexpr std::uint32_t NVG_Flag = 0b000'0'1'0'0'0'00'00'0000000000'0000000000;
	constexpr std::uint32_t DefuseKit_Flag = 0b000'0'0'1'0'0'00'00'0000000000'0000000000;
	constexpr std::uint32_t Smoke_Flag = 0b000'0'0'0'1'0'00'00'0000000000'0000000000;
	constexpr std::uint32_t HE_Flag = 0b000'0'0'0'0'1'00'00'0000000000'0000000000;
	constexpr std::uint32_t Flashbang_Flag = 0b000'0'0'0'0'0'11'00'0000000000'0000000000;
	constexpr std::uint32_t Armor_Flag = 0b000'0'0'0'0'0'00'11'0000000000'0000000000;
	constexpr std::uint32_t Helmet_Flag = 0b000'0'0'0'0'0'00'10'0000000000'0000000000;
	constexpr std::uint32_t Kelvar_Flag = 0b000'0'0'0'0'0'00'01'0000000000'0000000000;
	constexpr std::uint32_t Primary_Ammo_Flag = 0b000'0'0'0'0'0'00'00'0111111111'0000000000;
	constexpr std::uint32_t Secondary_Ammo_Flag = 0b000'0'0'0'0'0'00'00'0000000000'0111111111;
	

	class Buy final {
	public:
		struct Filter {
			enum Kind {
				Price,
				Damage,
				Speed,
				Reload,
				CarryAmmo,
				Clip,
				Random,
				Primary,		// Except secondary weapons
				Secondary,
				Submachinegun,
				Shotgun,
				Rifle,
				Machinegun
			} kind{};

			bool is_lower{};
			union { float f; int i; } value{};
		};
	private:
		// -- Weapon Filters --

		/**
		* Narrow down weapon candidates by the price.
		* @param filter The filter for the weapon list.
		*/
		void FilterWeaponByPrice(const Filter* filter, std::vector<const database::WeaponData*>* result) POKEBOT_NOEXCEPT {
			assert(!result->empty());
			std::vector<const database::WeaponData*> datas = std::move(*result);
			for (auto data : datas) {
				if (data->price > filter->value.i)
					continue;

				result->push_back(data);
			}
		}

		/**
		* Narrow down weapon candidates by the damage amount.
		* @param filter The filter for the weapon list.
		*/
		void FilterWeaponByDamage(const Filter* filter, std::vector<const database::WeaponData*>* result) POKEBOT_NOEXCEPT {
			assert(!result->empty());
			std::vector<const database::WeaponData*> datas = std::move(*result);
			for (auto data : datas) {
				if (data->damage > filter->value.i)
					continue;

				result->push_back(data);
			}
		}

		/**
		* Narrow down weapon candidates by weight.
		* @param filter The filter for the weapon list.
		*/
		void FilterWeaponBySpeed(const Filter* filter, std::vector<const database::WeaponData*>* result) POKEBOT_NOEXCEPT {
			assert(!result->empty());
			std::vector<const database::WeaponData*> datas = std::move(*result);
			for (auto data : datas) {
				if (data->movement_decay > filter->value.i)
					continue;

				result->push_back(data);
			}
		}

		/**
		* Narrow down weapon candidates by primary.
		* @param filter The filter for the weapon list.
		*/
		void FilterWeaponByPrimary(const Filter* filter, std::vector<const database::WeaponData*>* result) POKEBOT_NOEXCEPT {
			assert(!result->empty());
			std::vector<const database::WeaponData*> datas = std::move(*result);
			for (auto& data : datas) {
				if (data->label.contains("primary")) {
					result->push_back(data);
				}
			}
		}

		/**
		* Narrow down weapon candidates by secondary.
		* @param filter The filter for the weapon list.
		*/
		void FilterWeaponBySecondary(const Filter* filter, std::vector<const database::WeaponData*>* result) POKEBOT_NOEXCEPT {
			assert(!result->empty());
			std::vector<const database::WeaponData*> datas = std::move(*result);
			for (auto& data : datas) {
				if (data->label.contains("secondary") || data->label.contains("handgun") || data->label.contains("pistol")) {
					result->push_back(data);
				}
			}
		}

		/**
		* Narrow down weapon candidates by shotgun.
		* @param filter The filter for the weapon list.
		*/
		void FilterWeaponByShotgun(const Filter* filter, std::vector<const database::WeaponData*>* result) POKEBOT_NOEXCEPT {
			assert(!result->empty());
			std::vector<const database::WeaponData*> datas = std::move(*result);
			for (auto& data : datas) {
				if (data->label.contains("shotgun")) {
					result->push_back(data);
				}
			}
		}

		/**
		* Narrow down weapon candidates by submachinegun.
		* @param filter The filter for the weapon list.
		*/
		void FilterWeaponBySubmachinegun(const Filter* filter, std::vector<const database::WeaponData*>* result) POKEBOT_NOEXCEPT {
			assert(!result->empty());
			std::vector<const database::WeaponData*> datas = std::move(*result);
			for (auto& data : datas) {
				if (data->label.contains("submachinegun")) {
					result->push_back(data);
				}
			}
		}

		/**
		* Narrow down weapon candidates by rifle.
		* @param filter The filter for the weapon list.
		*/
		void FilterWeaponByRifle(const Filter* filter, std::vector<const database::WeaponData*>* result) POKEBOT_NOEXCEPT {
			assert(!result->empty());
			std::vector<const database::WeaponData*> datas = std::move(*result);
			for (auto& data : datas) {
				if (data->label.contains("rifle")) {
					result->push_back(data);
				}
			}
		}

		/**
		* Narrow down weapon candidates by rifle.
		* @param filter The filter for the weapon list.
		*/
		void FilterWeaponByMachinegun(const Filter* filter, std::vector<const database::WeaponData*>* result) POKEBOT_NOEXCEPT {
			assert(!result->empty());
			std::vector<const database::WeaponData*> datas = std::move(*result);
			for (auto& data : datas) {
				if (data->label.contains("machinegun")) {
					result->push_back(data);
				}
			}
		}

		/**
		* Narrow down weapon candidates by reload time.
		* @param filter The filter for the weapon list.
		*/
		void FilterWeaponByReload(const Filter* filter, std::vector<const database::WeaponData*>* result) POKEBOT_NOEXCEPT {
			assert(!result->empty());
			std::vector<const database::WeaponData*> datas = std::move(*result);
			for (auto data : datas) {
				if (data->movement_decay > filter->value.i)
					continue;

				result->push_back(data);
			}
		}

		void FilterWeaponByCarryingAmmo(const Filter* filter, std::vector<const database::WeaponData*>* result) POKEBOT_NOEXCEPT {
			assert(!result->empty());
			std::vector<const database::WeaponData*> datas = std::move(*result);
			for (auto data : datas) {
				if (database::database.GetCartridge(data->cartridge.c_str())->max > filter->value.i)
					continue;

				result->push_back(data);
			}
		}

		void FilterWeaponByClip(const Filter* filter, std::vector<const database::WeaponData*>* result) {
			assert(!result->empty());
			std::vector<const database::WeaponData*> datas = std::move(*result);
			for (auto data : datas) {
				if (database::database.GetCartridge(data->cartridge.c_str())->per > filter->value.i)
					continue;

				result->push_back(data);
			}
		}

		void FilterWeaponByRandom(const Filter* filter, std::vector<const database::WeaponData*>* result) {
			assert(!result->empty());
			std::vector<const database::WeaponData*> datas = std::move(*result);
			result->push_back(datas[util::Random<int>(0, datas.size() - 1)]);
		}

		static void PrepareData(std::vector<const database::WeaponData*>* datas) {
			for (auto name : game::weapon::Weapon_CVT) {
				if (auto data = database::database.GetWeaponData(std::get<const char* const>(name)); data != nullptr)
					datas->push_back(data);
			}
		}

		void(Buy::* FilterBuys[13])(const Filter* filter, std::vector<const database::WeaponData*>* result) {
			&Buy::FilterWeaponByPrice,
				& Buy::FilterWeaponByDamage,
				& Buy::FilterWeaponBySpeed,
				& Buy::FilterWeaponByReload,
				& Buy::FilterWeaponByCarryingAmmo,
				& Buy::FilterWeaponByClip,
				& Buy::FilterWeaponByRandom,
				& Buy::FilterWeaponByPrimary,
				& Buy::FilterWeaponBySecondary,
				& Buy::FilterWeaponByShotgun,
				& Buy::FilterWeaponBySubmachinegun,
				& Buy::FilterWeaponByRifle,
				& Buy::FilterWeaponByMachinegun
		};
	public:
		void BuyWeapon(const database::WeaponData** destination, std::initializer_list<Filter> filters) {
			std::vector<const database::WeaponData*> datas{};
			PrepareData(&datas);
			for (const auto& filter : filters) {
				assert(!datas.empty());
				(this->*FilterBuys[static_cast<int>(filter.kind)])(&filter, &datas);
				if (datas.size() == 1)
					break;
			}
			// Primary weapon
			*destination = datas.front();
		}
	};

	struct BuyNode final {
		const database::WeaponData* data[2]{};
		/*
			This is the bit flag to buy equipment. If the bit is 1, bots buy it.
			The bit is composed like below:
				0b000'0'0'0'0'0'00'00'0000000000'0000000000

			000			- unused
			0			- tactical sheild
			0			- nightvision
			0			- defusal kit
			0			- smoke
			0			- he grenade
			00			- Flashbang
			00			- helmet(left bit) and kelvar(right bit)
			0000000000	- Primary ammo(Bots can buy it 10 times)
			0000000000	- Secondary ammo(Bots can buy it 10  times)
		*/
		std::uint32_t equipment_flag = 0b000'0'0'0'0'0'00'00'0000000000'0000000000;

		std::pair<GameMoney, GameMoney> money{ 0, 0 };		// Prediction of min/max
	};

	class BuyPattern {
		BuyNode pattern[32]{};			// Buy patterns for all bots.
	public:
		BuyNode* GetPattern(int round, int win_state) POKEBOT_NOEXCEPT {
			if ((round < 0 || round >= 16) || (win_state < 0 || win_state > 1))
				return nullptr;
			else
				return &pattern[(round * 2) + win_state];
		}

		BuyPattern(const int Money) {
			constexpr int rewards[2] = {
				1400,
				3500
			};

			Buy buy;
			for (int round = 0; round <= 15; round++) {
				for (int win_state = 0; win_state < 2; win_state++) {
					BuyNode* node = GetPattern(round, win_state);
					node->money.first = Money + rewards[win_state];
					node->money.second = Money + rewards[win_state] + (300 * 32) + (150 * 4) + (1000 * 4);
					buy.BuyWeapon(&node->data[0],
								  {
									  Buy::Filter{.kind = Buy::Filter::Kind::Primary, .is_lower = false, .value = {.i = 0 } },
									  Buy::Filter{.kind = Buy::Filter::Kind::Random, .is_lower = false, .value = {.i = 0 } },
								  });

					buy.BuyWeapon(&node->data[1],
								  {
									  Buy::Filter{.kind = Buy::Filter::Kind::Secondary, .is_lower = false, .value = {.i = 0 } },
									  Buy::Filter{.kind = Buy::Filter::Kind::Random, .is_lower = false, .value = {.i = 0 } },
								  });

					node->equipment_flag |= Primary_Ammo_Flag;
					node->equipment_flag |= Secondary_Ammo_Flag;
					node->equipment_flag |= DefuseKit_Flag;
				}
			}
		}
	};

	class BuyAI {
		BuyPattern pattern;
	public:
		BuyAI(const GameMoney Money) : pattern(BuyPattern(Money)) {}

		BuyNode* GetPattern(int round, int win_state) {
			return pattern.GetPattern(round, win_state);
		}
	};

}

namespace pokebot::bot {
	void Bot::BuyUpdate() POKEBOT_NOEXCEPT {
		if (!plugin::console::poke_buy) {
			return;
		}

		if (client.IsInBuyzone() && !buy_wait_timer.IsRunning()) {
			auto ai = buy::BuyAI(client.Money);
			auto node = ai.GetPattern(0, 0);
			for (auto data : node->data) {
				if (data == nullptr)
					continue;

				engine::EngineInterface::InputFakeclientCommand(client.client, "buy");
				for (auto menu : data->menu[JoinedTeam() == game::Team::CT]) {
					engine::EngineInterface::InputFakeclientCommand(client.client, std::format("menuselect {}", menu).c_str());
				}
			}

			if (bool(node->equipment_flag & buy::Shield_Flag)) {
				engine::EngineInterface::InputFakeclientCommand(client.client, "shield");
				node->equipment_flag &= ~buy::Shield_Flag;
			}

			if (bool(node->equipment_flag & buy::DefuseKit_Flag)) {
				engine::EngineInterface::InputFakeclientCommand(client.client, "defuser");
				node->equipment_flag &= ~buy::DefuseKit_Flag;
			}

			if (bool(node->equipment_flag & buy::NVG_Flag)) {
				engine::EngineInterface::InputFakeclientCommand(client.client, "nvgs");
				node->equipment_flag &= ~buy::NVG_Flag;
			}

			if (bool(node->equipment_flag & buy::Smoke_Flag)) {
				engine::EngineInterface::InputFakeclientCommand(client.client, "sgren");
				node->equipment_flag &= ~buy::Smoke_Flag;
			}

			if (bool(node->equipment_flag & buy::HE_Flag)) {
				engine::EngineInterface::InputFakeclientCommand(client.client, "hegren");
				node->equipment_flag &= ~buy::HE_Flag;
			}

			if (bool(node->equipment_flag & buy::Kelvar_Flag)) {
				engine::EngineInterface::InputFakeclientCommand(client.client, "vest");
				node->equipment_flag &= ~buy::Kelvar_Flag;
			}

			if (bool(node->equipment_flag & buy::Helmet_Flag)) {
				engine::EngineInterface::InputFakeclientCommand(client.client, "vesthelm");
				node->equipment_flag &= ~buy::Helmet_Flag;
			}

			std::uint32_t flashbang_buy_bit = 0b000'0'0'0'0'0'10'00'0000000000'0000000000;
			while ((node->equipment_flag & buy::Flashbang_Flag) > 0) {
				if (bool(node->equipment_flag & flashbang_buy_bit)) {
					engine::EngineInterface::InputFakeclientCommand(client.client, "flash");
					// Set 0 to a bit to decrement the number of buy.
					node->equipment_flag &= ~flashbang_buy_bit;
					flashbang_buy_bit >>= 1;
				}
			}

			std::uint32_t primaryammo_buy_bit = 0b000'0'0'0'0'0'00'00'0100000000'0000000000;
			while ((node->equipment_flag & buy::Primary_Ammo_Flag) > 0) {
				if (bool(node->equipment_flag & primaryammo_buy_bit)) {
					engine::EngineInterface::InputFakeclientCommand(client.client, "buyammo1");

					// Set 0 to a bit to decrement the number of buy.
					node->equipment_flag &= ~primaryammo_buy_bit;
					primaryammo_buy_bit >>= 1;
				}
			}

			std::uint32_t secondaryammo_buy_bit = 0b000'0'0'0'0'0'00'00'0000000000'0100000000;
			while ((node->equipment_flag & buy::Secondary_Ammo_Flag) > 0) {
				if (bool(node->equipment_flag & secondaryammo_buy_bit)) {
					engine::EngineInterface::InputFakeclientCommand(client.client, "buyammo2");

					// Set 0 to a bit to decrement the number of buy.
					node->equipment_flag &= ~secondaryammo_buy_bit;
					secondaryammo_buy_bit >>= 1;
				}
			}
			start_action = Message::Normal;
		}
	}
}