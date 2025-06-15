#pragma once
#define POKEBOT_ENUM_BIT_OPERATORS(TYPE) inline constexpr TYPE operator|(const TYPE& n1, const TYPE& n2) noexcept { return static_cast<TYPE>((int)n1 | (int)n2); } inline constexpr TYPE operator&(const TYPE& n1, const TYPE& n2) noexcept { return static_cast<TYPE>((int)n1 & (int)n2); } inline constexpr TYPE operator^(const TYPE& n1, const TYPE& n2) noexcept { return static_cast<TYPE>((int)n1 ^ (int)n2); } inline constexpr TYPE operator|=(TYPE& n1, const TYPE& n2) noexcept { n1 = (n1 | n2); return n1; } inline constexpr TYPE operator&=(TYPE& n1, const TYPE& n2) noexcept { n1 = (n1 & n2); return n1; } inline constexpr TYPE operator^=(TYPE& n1, const TYPE& n2) noexcept { n1 = (n1 ^ n2); return n1; } inline constexpr TYPE operator~(TYPE n1) noexcept { return static_cast<TYPE>(~static_cast<int>(n1)); }
#define POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(TYPE,...) enum class TYPE { __VA_ARGS__ }; POKEBOT_ENUM_BIT_OPERATORS(TYPE)

namespace pokebot::game {
	POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(
		MapFlags,
		Demolition = 1 << 0,
		HostageRescue = 1 << 1,
		Assassination = 1 << 2,
		Escape = 1 << 3,
		Other = 1 << 4
	);
}