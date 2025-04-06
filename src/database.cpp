#include "database.hpp"
namespace pokebot::database {
	Database::Database() {
		std::filesystem::current_path("D:\\SteamLibrary\\steamapps\\common\\Half-Life");

		[&] {
			if (FILE* fp = fopen("pokebot/database/bot.json", "r"); fp) {
				nlohmann::json data{};
				try {
					data = nlohmann::json::parse(fp);
				} catch (nlohmann::json::parse_error& e) {
					SERVER_PRINT(std::format("Parse error at {}\n", e.byte).c_str());
					return;
				}

				for (const auto item : data["template"].items()) {
					auto ArrayPushBack = [](std::list<util::fixed_string<64u>>* const target, const nlohmann::json_abi_v3_11_2::json& Datas) POKEBOT_NOEXCEPT {
						for (int i = 0; i < Datas.size(); i++) {
							target->push_back(Datas[i].get<std::string>().c_str());
						}
					};

					auto ArraySet = [](int* target, const nlohmann::json_abi_v3_11_2::json& Datas) POKEBOT_NOEXCEPT {
						*target = Datas.get<int>();
					};

					if (data["template"][item.key()].contains("weapon"))
						ArrayPushBack(&bot_template_database[item.key().c_str()].weapon, data["template"][item.key()]["weapon"]);
					if (data["template"][item.key()].contains("map"))
						ArrayPushBack(&bot_template_database[item.key().c_str()].map, data["template"][item.key()]["map"]);
					if (data["template"][item.key()].contains("skin"))
						ArrayPushBack(&bot_template_database[item.key().c_str()].skin, data["template"][item.key()]["skin"]);
					if (data["template"][item.key()].contains("brave"))
						ArraySet(&bot_template_database[item.key().c_str()].brave, data["template"][item.key()]["brave"]);
					if (data["template"][item.key()].contains("coop"))
						ArraySet(&bot_template_database[item.key().c_str()].brave, data["template"][item.key()]["coop"]);

					if (data["template"][item.key()].contains("reaction")) {
						for (int i = 0; i < data["template"][item.key()]["reaction"].size(); i++) {
							for (int j = 0; j < data["template"][item.key()]["reaction"][i].size(); j++) {
								bot_template_database[item.key().c_str()].reaction[i][j] = data["template"][item.key()]["reaction"][i][j].get<float>();
							}
						}
					}
				}

				for (const auto item : data["bot"].items()) {

				}
				fclose(fp);
			}
		}();

		[&] {
			if (FILE* fp = fopen("pokebot/database/weapon.json", "r"); fp != nullptr) {
				nlohmann::json data{};
				try {
					data = nlohmann::json::parse(fp);
				} catch (nlohmann::json::parse_error& e) {
					SERVER_PRINT(std::format("Parse error at {}\n", e.byte).c_str());
					return;
				}

				auto GetItem = [&data](std::string_view key, std::string_view item) -> nlohmann::json& {
					return data[key][item];
				};

				std::function<nlohmann::json&(std::string_view)> GetCartridge = std::bind(GetItem, "cartridge", std::placeholders::_1);
				for (const auto item : data["cartridge"].items()) {
					auto* const cartridge = &cartridges[item.key().c_str()];
					cartridge->per = GetCartridge(item.key())["per"].get<int>();
					cartridge->max = GetCartridge(item.key())["max"].get<int>();
					cartridge->price = GetCartridge(item.key())["price"].get<int>();
				}

				std::function<nlohmann::json& (std::string_view)> GetWeapon = std::bind(GetItem, "weapon", std::placeholders::_1);
				for (const auto item : data["weapon"].items()) {
					util::fixed_string<64u> id{};
					if (data["weapon"][item.key()].contains("id")) {
						id = GetWeapon(item.key())["id"].get<std::string>().c_str();
					} else {
						id = ("weapon_" + item.key()).c_str();
					}
					auto* const weapon = &weapons[id.data()];
					weapon->id = id.c_str();
					weapon->capacity		= GetWeapon(item.key())["capacity"].get<int>();
					weapon->damage			= GetWeapon(item.key())["damage"].get<int>();
					weapon->reload			= GetWeapon(item.key())["reload"].get<float>();
					weapon->movement_decay	= GetWeapon(item.key())["movement-decay"].get<int>();
					weapon->price			= GetWeapon(item.key())["price"].get<int>();
					weapon->rate			= GetWeapon(item.key())["rate"].get<int>();
					weapon->accuracy		= GetWeapon(item.key())["accuracy"].get<float>();
					weapon->cartridge		= GetWeapon(item.key())["cartridge"].get<std::string>().c_str();
					weapon->underwater		= GetWeapon(item.key())["underwater"].get<bool>();
					weapon->rapidfire		= GetWeapon(item.key())["rapidfire"].get<bool>();

					for (const auto model : GetWeapon(item.key())["model"].items())
						weapon->model.insert(model.value().get<std::string>().c_str());

					for (const auto label : GetWeapon(item.key())["label"].items())
						weapon->label.insert(label.value().get<std::string>().c_str());

					int i = 0;
					for (const auto menu : GetWeapon(item.key())["menu"].items()) {
						for (const auto sub : menu.value().items()) {
							const int s = sub.value().get<int>();
							weapon->menu[i].push_back(s);
						}
						i++;
					}
					if (i < 2)
						weapon->menu[1] = weapon->menu[0];
				}
				fclose(fp);
			}
		}();
	}

	const WeaponData* const Database::GetWeaponData(const std::string_view Item_ID) const POKEBOT_NOEXCEPT {
		if (auto it = weapons.find(Item_ID.data()); it != weapons.end())
			return &it->second;
		else
			return nullptr;
	}

	const Cartridge* const Database::GetCartridge(const std::string_view Item_ID) const POKEBOT_NOEXCEPT {
		if (auto it = cartridges.find(Item_ID.data()); it != cartridges.end())
			return &it->second;
		else
			return nullptr;
	}
}
