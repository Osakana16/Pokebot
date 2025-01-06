#pragma once
namespace pokebot {
	namespace database {
		struct BotProfile final {
			std::list<std::string> bases{};
			std::list<std::string> weapon{};
			std::list<std::string> map{};
			std::list<std::string> skin{};
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
			std::string id{};
			std::string cartridge{};
			int capacity{};
			int damage{};
			float reload{};
			int movement_decay{};
			int rate{};
			int price{};
			float accuracy{};
			bool underwater{};
			bool rapidfire{};

			std::unordered_set<std::string> model{};
			std::unordered_set<std::string> label{};
			std::vector<int> menu[2]{};
		};

#ifdef TEST_IMPORT
#define TEST_API __declspec(dllimport)
#else
#define TEST_API __declspec(dllexport)
#endif


		inline static class TEST_API Database {
			std::unordered_map<std::string, BotProfile> bot_template_database{};
			std::unordered_map<std::string, Cartridge> cartridges{};
			std::unordered_map<std::string, WeaponData> weapons{};
		public:
			Database();

			const WeaponData* const GetWeaponData(const std::string_view) const POKEBOT_NOEXCEPT;
			const Cartridge* const GetCartridge(const std::string_view) const POKEBOT_NOEXCEPT;

		} database{};
	}
}