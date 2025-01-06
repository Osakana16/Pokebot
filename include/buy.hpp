#pragma once
#include "database.hpp"

#ifdef TEST_IMPORT
#define TEST_API __declspec(dllimport)
#else
#define TEST_API __declspec(dllexport)
#endif

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

	constexpr std::uint32_t Shield_Flag =			0b000'1'0'0'0'0'00'00'0000000000'0000000000;
	constexpr std::uint32_t NVG_Flag =				0b000'0'1'0'0'0'00'00'0000000000'0000000000;
	constexpr std::uint32_t DefuseKit_Flag =		0b000'0'0'1'0'0'00'00'0000000000'0000000000;
	constexpr std::uint32_t Smoke_Flag =			0b000'0'0'0'1'0'00'00'0000000000'0000000000;
	constexpr std::uint32_t HE_Flag =				0b000'0'0'0'0'1'00'00'0000000000'0000000000;
	constexpr std::uint32_t Flashbang_Flag =		0b000'0'0'0'0'0'11'00'0000000000'0000000000;
	constexpr std::uint32_t Armor_Flag =			0b000'0'0'0'0'0'00'11'0000000000'0000000000;
	constexpr std::uint32_t Helmet_Flag =			0b000'0'0'0'0'0'00'10'0000000000'0000000000;
	constexpr std::uint32_t Kelvar_Flag =			0b000'0'0'0'0'0'00'01'0000000000'0000000000;
	constexpr std::uint32_t Primary_Ammo_Flag =		0b000'0'0'0'0'0'00'00'0111111111'0000000000;
	constexpr std::uint32_t Secondary_Ammo_Flag =	0b000'0'0'0'0'0'00'00'0000000000'0111111111;

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

	class TEST_API BuyPattern {
		BuyNode pattern[32]{};			// Buy patterns for all bots.
	public:
		BuyNode* GetPattern(int round, int win) POKEBOT_NOEXCEPT;
		BuyPattern(const int Money);
	};

	class TEST_API BuyAI {
		BuyPattern pattern;
	public:
		BuyAI(const GameMoney Money);
		BuyNode* GetPattern(int round, int win_state);
	};
}