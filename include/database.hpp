#pragma once
namespace pokebot {
	namespace database {
		struct BotProfile final {
			std::list<common::fixed_string<64u>> bases{};
			std::list<common::fixed_string<64u>> weapon{};
			std::list<common::fixed_string<64u>> map{};
			std::list<common::fixed_string<64u>> skin{};
			int brave{};
			int coop{};
			int light_level{};
			float reaction[2][2]{};
		};

		struct Cartridge final {
			int per{};
			int max{};
			int price{};
		};

		struct WeaponData final {
			common::fixed_string<64u> id{};
			common::fixed_string<64u> cartridge{};
			int capacity{};
			int damage{};
			float reload{};
			int movement_decay{};
			int rate{};
			int price{};
			float accuracy{};
			bool underwater{};
			bool rapidfire{};

			std::unordered_set<common::fixed_string<64u>, common::fixed_string<64u>::Hash> model{};
			std::unordered_set<common::fixed_string<64u>, common::fixed_string<64u>::Hash> label{};
			std::vector<int> menu[2]{};
		};

#ifdef TEST_IMPORT
#define TEST_API __declspec(dllimport)
#else
#define TEST_API __declspec(dllexport)
#endif


		inline static class TEST_API Database {
			std::unordered_map<common::fixed_string<64u>, BotProfile, common::fixed_string<64u>::Hash> bot_template_database{};
			std::unordered_map<common::fixed_string<64u>, Cartridge, common::fixed_string<64u>::Hash> cartridges{};
			std::unordered_map<common::fixed_string<64u>, WeaponData, common::fixed_string<64u>::Hash> weapons{};
		public:
			Database();

			const WeaponData* const GetWeaponData(const std::string_view) const POKEBOT_NOEXCEPT;
			const Cartridge* const GetCartridge(const std::string_view) const POKEBOT_NOEXCEPT;

		} database{};
	}
}