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

		static constexpr int Fix(const T Value) noexcept {
			return (Value > max_money ? max_money :
				(Value < min_money ? min_money : Value));
		}
	public:
		using BaseType = T;

		constexpr Money(const T Init_Money) noexcept : money(Fix(Init_Money)) {}
		constexpr operator T() const noexcept { return money; }
		explicit operator bool() = delete;
		
		template<typename U>
		constexpr Money& operator=(const U value) noexcept {
			money = Fix(value);
			return *this;
		}

		template<>Money& operator=(const bool) = delete;

		constexpr Money& operator+=(const Money& value) noexcept { money = Fix(money + value); return *this; }
		constexpr Money& operator-=(const Money& value) noexcept { money = Fix(money - value); return *this; }
		constexpr Money& operator*=(const Money& value) noexcept { money = Fix(money * value); return *this; }
		constexpr Money& operator/=(const Money& value) noexcept { money = Fix(money / value); return *this; }
		constexpr Money& operator%=(const Money& value) noexcept { money = Fix(money % value); return *this; }
		constexpr bool operator<(const Money& value) noexcept { return money < value; }
		constexpr bool operator>(const Money& value) noexcept { return money > value; }
		constexpr bool operator<=(const Money& value) noexcept { return money <= value; }
		constexpr bool operator>=(const Money& value) noexcept { return money >= value; }
		constexpr bool operator==(const Money& value) noexcept { return money == value; }
		constexpr bool operator!=(const Money& value) noexcept { return money == value; }

		template<typename U>
		constexpr Money operator+(const U& value) noexcept { return (money + value); }
		template<typename U>
		constexpr Money operator-(const U& value) noexcept { return (money - value); }
		template<typename U>
		constexpr Money operator*(const U& value) noexcept { return (money * value); }
		template<typename U>
		constexpr Money operator/(const U& value) noexcept { return (money / value); }

		static constexpr T Min_Money = min_money;
		static constexpr T Max_Money = max_money;
	};

	using GameMoney = Money<int, 0, 16000>;

	struct BuyNode final {
		const database::WeaponData* data[11]{};
		std::pair<GameMoney, GameMoney> money{ 0, 0 };		// Prediction of min/max
	};

	class TEST_API BuyPattern {
		BuyNode pattern[32]{};
	public:
		BuyNode* GetPattern(int round, int win) noexcept;
		BuyPattern(const int Money);
	};

	class TEST_API BuyAI {
		BuyPattern pattern;
	public:
		BuyAI(const GameMoney Money);
		BuyNode* GetPattern(int round, int win_state);
	};
}