#pragma once
#define USE_NAVMESH true


namespace pokebot::node {
	
	template<typename T>
	constexpr T World_Size = 8192;
	constexpr size_t Split_Size = 256;
	constexpr size_t Tree_Size = World_Size<size_t> / Split_Size;


	enum class RouteState {
		New,
		Open,
		Closed
	};

	


	void AStarSearch(std::pair<const Vector&, const Vector&>);
}