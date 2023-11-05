#include "database.hpp"
#include "buy.hpp"

namespace pokebot {
	namespace bot {
		void Bot::BuyUpdate() noexcept {
			if (client->IsShowingIcon(game::StatusIcon::Buy_Zone) && !buy_wait_timer.IsRunning()) {
				auto ai = buy::BuyAI(this->client->Money);
				auto node = ai.GetPattern(0, 0);
				for (auto data : node->data) {
					if (data == nullptr)
						continue;

					game::game.IssueCommand(*client, "buy");
					for (auto menu : data->menu[JoinedTeam() == common::Team::CT]) {
						game::game.IssueCommand(*client, std::format("menuselect {}", menu));
					}
				}
				start_action = Message::Normal;
			}
		}
	}

	namespace buy {
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
			/**
			* Narrow down weapon candidates by the price.
			* @param filter The filter for the weapon list.
			*/
			void FilterWeaponByPrice(const Filter* filter, std::vector<const database::WeaponData*>* result) noexcept {
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
			void FilterWeaponByDamage(const Filter* filter, std::vector<const database::WeaponData*>* result) noexcept {
				std::vector<const database::WeaponData*> datas{};
				for (int i = 0; i < 32; i++) {

				}

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
			void FilterWeaponBySpeed(const Filter* filter, std::vector<const database::WeaponData*>* result) noexcept {

			}

			/**
			* Narrow down weapon candidates by secondary.
			* @param filter The filter for the weapon list.
			*/
			void FilterWeaponBySecondary(const Filter* filter, std::vector<const database::WeaponData*>* result) noexcept {
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
			void FilterWeaponByShotgun(const Filter* filter, std::vector<const database::WeaponData*>* result) noexcept {
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
			void FilterWeaponBySubmachinegun(const Filter* filter, std::vector<const database::WeaponData*>* result) noexcept {
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
			void FilterWeaponByRifle(const Filter* filter, std::vector<const database::WeaponData*>* result) noexcept {
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
			void FilterWeaponByMachinegun(const Filter* filter, std::vector<const database::WeaponData*>* result) noexcept {
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
			void FilterWeaponByReload(const Filter* filter, std::vector<const database::WeaponData*>* result) noexcept {
				assert(!result->empty());
			}

			void FilterWeaponByCarryingAmmo(const Filter* filter, std::vector<const database::WeaponData*>* result) noexcept {
				assert(!result->empty());
			}

			void FilterWeaponByClip(const Filter* filter, std::vector<const database::WeaponData*>* result) {
				assert(!result->empty()); 
			}

			void FilterWeaponByRandom(const Filter* filter, std::vector<const database::WeaponData*>* result) {
				assert(!result->empty());
				std::vector<const database::WeaponData*> datas = std::move(*result);
				result->push_back(datas[common::Random<int>(0, datas.size() - 1)]);
			}

			static void PrepareData(std::vector<const database::WeaponData*>* datas) {
				for (auto name : game::Weapon_CVT) {
					if (auto data = database::database.GetWeaponData(name); data != nullptr)
						datas->push_back(data);
				}
			}

			void(Buy::* FilterBuys[12])(const Filter* filter, std::vector<const database::WeaponData*>* result) {
				&Buy::FilterWeaponByPrice,
				&Buy::FilterWeaponByDamage,
				&Buy::FilterWeaponBySpeed,
				&Buy::FilterWeaponByReload,
				&Buy::FilterWeaponByCarryingAmmo,
				&Buy::FilterWeaponByClip,
				&Buy::FilterWeaponByRandom,
				&Buy::FilterWeaponBySecondary,
				&Buy::FilterWeaponByShotgun,
				&Buy::FilterWeaponBySubmachinegun,
				&Buy::FilterWeaponByRifle,
				&Buy::FilterWeaponByMachinegun
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

		BuyNode* BuyPattern::GetPattern(int round, int win_state) noexcept {
			if ((round < 0 || round >= 16) || (win_state < 0 || win_state > 1)) 
				return nullptr;
			else
				return &pattern[(round * 2) + win_state];
		}

		BuyPattern::BuyPattern(const int Money) {
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
							Buy::Filter{.kind = Buy::Filter::Kind::Rifle, .is_lower = false, .value = {.i = 0 } },
							Buy::Filter{.kind = Buy::Filter::Kind::Random, .is_lower = false, .value = {.i = 0 } },
						});

					buy.BuyWeapon(&node->data[1],
						{
							Buy::Filter{.kind = Buy::Filter::Kind::Secondary, .is_lower = false, .value = {.i = 0 } },
							Buy::Filter{.kind = Buy::Filter::Kind::Random, .is_lower = false, .value = {.i = 0 } },
						});
				}
			}
		}

		BuyAI::BuyAI(const GameMoney Money) : pattern(BuyPattern(Money)) {

		}

		BuyNode* BuyAI::GetPattern(int round, int win_state) {
			return pattern.GetPattern(round, win_state);
		}
	}
}