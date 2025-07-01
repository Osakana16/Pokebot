export module pokebot.game.weapon: weapon_definition;
import :weapon_type;
import :weapon_id;
import :ammo_id;

export namespace pokebot::game::weapon {
	using WeaponName = const char* const;
	constexpr WeaponType Weapon_Type[31]{
			WeaponType::Special,
			WeaponType::Secondary,//1
			WeaponType::Secondary,//2
			WeaponType::Primary,//3
			WeaponType::Grenade,//4
			WeaponType::Primary,//5
			WeaponType::Special,//6
			WeaponType::Primary,//7
			WeaponType::Primary,//8
			WeaponType::Grenade,//9
			WeaponType::Secondary,//10
			WeaponType::Secondary,//11
			WeaponType::Primary,//12
			WeaponType::Primary,//13
			WeaponType::Primary,//14
			WeaponType::Primary,//15
			WeaponType::Secondary,//16
			WeaponType::Secondary,//17
			WeaponType::Primary,//18
			WeaponType::Primary,//19
			WeaponType::Primary,//20
			WeaponType::Primary,//21
			WeaponType::Primary,//22
			WeaponType::Primary,//23
			WeaponType::Primary,//24
			WeaponType::Grenade,//25
			WeaponType::Secondary,//26
			WeaponType::Primary,//27
			WeaponType::Primary,//28
			WeaponType::Melee,//29
			WeaponType::Primary//30
	};

	constexpr std::tuple<WeaponName, ammo::ID> Weapon_CVT[30]{
		std::make_tuple<WeaponName, ammo::ID>("weapon_p228", ammo::ID::SIG357),
		std::make_tuple<WeaponName, ammo::ID>("weapon_shield", ammo::ID::None),
		std::make_tuple<WeaponName, ammo::ID>("weapon_scout", ammo::ID::Nato776),
		std::make_tuple<WeaponName, ammo::ID>("weapon_hegrenade", ammo::ID::None),
		std::make_tuple<WeaponName, ammo::ID>("weapon_xm1014", ammo::ID::Buckshot),
		std::make_tuple<WeaponName, ammo::ID>("weapon_c4", ammo::ID::None),
		std::make_tuple<WeaponName, ammo::ID>("weapon_aug", ammo::ID::Nato556),
		std::make_tuple<WeaponName, ammo::ID>("weapon_mac10", ammo::ID::ACP45),
		std::make_tuple<WeaponName, ammo::ID>("weapon_smoke", ammo::ID::None),
		std::make_tuple<WeaponName, ammo::ID>("weapon_elite", ammo::ID::MM9),
		std::make_tuple<WeaponName, ammo::ID>("weapon_fiveseven", ammo::ID::MM57),
		std::make_tuple<WeaponName, ammo::ID>("weapon_ump45", ammo::ID::ACP45),
		std::make_tuple<WeaponName, ammo::ID>("weapon_sg550", ammo::ID::Nato556),
		std::make_tuple<WeaponName, ammo::ID>("weapon_galil", ammo::ID::Nato556),
		std::make_tuple<WeaponName, ammo::ID>("weapon_famas", ammo::ID::Nato556),
		std::make_tuple<WeaponName, ammo::ID>("weapon_usp", ammo::ID::ACP45),
		std::make_tuple<WeaponName, ammo::ID>("weapon_glock18", ammo::ID::MM9),
		std::make_tuple<WeaponName, ammo::ID>("weapon_awp", ammo::ID::Magnum338),
		std::make_tuple<WeaponName, ammo::ID>("weapon_mp5navy", ammo::ID::MM9),
		std::make_tuple<WeaponName, ammo::ID>("weapon_m249", ammo::ID::NatoBox556),
		std::make_tuple<WeaponName, ammo::ID>("weapon_m3", ammo::ID::Buckshot),
		std::make_tuple<WeaponName, ammo::ID>("weapon_m4a1", ammo::ID::Nato556),
		std::make_tuple<WeaponName, ammo::ID>("weapon_tmp", ammo::ID::MM9),
		std::make_tuple<WeaponName, ammo::ID>("weapon_g3sg1", ammo::ID::Nato776),
		std::make_tuple<WeaponName, ammo::ID>("weapon_flashbang", ammo::ID::None),
		std::make_tuple<WeaponName, ammo::ID>("weapon_deagle", ammo::ID::AE50),
		std::make_tuple<WeaponName, ammo::ID>("weapon_sg552", ammo::ID::Nato556),
		std::make_tuple<WeaponName, ammo::ID>("weapon_ak47", ammo::ID::Nato776),
		std::make_tuple<WeaponName, ammo::ID>("weapon_knife", ammo::ID::None),
		std::make_tuple<WeaponName, ammo::ID>("weapon_p90", ammo::ID::MM57)
	};

	constexpr int Primary_Weapon_Bit = (game::ToBit<int>(ID::M3) | game::ToBit<int>(ID::XM1014) | game::ToBit<int>(ID::MAC10) | game::ToBit<int>(ID::TMP) | game::ToBit<int>(ID::MP5) | game::ToBit<int>(ID::UMP45) | game::ToBit<int>(ID::P90) | game::ToBit<int>(ID::Famas) | game::ToBit<int>(ID::Galil) | game::ToBit<int>(ID::AK47) | game::ToBit<int>(ID::M4A1) | game::ToBit<int>(ID::AUG) | game::ToBit<int>(ID::SG552) | game::ToBit<int>(ID::SG550) | game::ToBit<int>(ID::G3SG1) | game::ToBit<int>(ID::Scout) | game::ToBit<int>(ID::AWP) | game::ToBit<int>(ID::M249));
	constexpr int Secondary_Weapon_Bit = (game::ToBit<int>(ID::P228) | game::ToBit<int>(ID::USP) | game::ToBit<int>(ID::Deagle) | game::ToBit<int>(ID::FiveSeven) | game::ToBit<int>(ID::Glock18) | game::ToBit<int>(ID::Elite));
	constexpr int Melee_Bit = (game::ToBit<int>(ID::Knife));
	constexpr int Grenade_Bit = (game::ToBit<int>(ID::HEGrenade) | game::ToBit<int>(ID::Flashbang) | game::ToBit<int>(ID::Smoke));
	constexpr int C4_Bit = (game::ToBit<int>(ID::C4));
}