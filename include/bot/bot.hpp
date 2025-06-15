#pragma once
#include "util/timer.hpp"

namespace pokebot::message {
	class MessageDispatcher;
}

namespace pokebot::bot {
	class Bot;

	struct WeaponInfo {
		int max_clip{};
		int max_ammo{};
		int threat{};
	};
	
	enum class Difficult {
		Easy,
		Normal,
		Hard
	};

	enum class Message {
		Normal,
		Buy,
		Team_Select,
		Model_Select,
		Selection_Completed
	};

	POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(ActionKey,
		None = 0,
		Run = IN_RUN,
		Attack = IN_ATTACK,
		Jump = IN_JUMP,
		Duck = IN_DUCK,
		Forward = IN_FORWARD,
		Back = IN_BACK,
		Use = IN_USE,
		Cancel = IN_CANCEL,
		Left = IN_LEFT,
		Right = IN_RIGHT,
		Move_Left = IN_MOVELEFT,
		Move_Right = IN_MOVERIGHT,
		Attack2 = IN_ATTACK2,
		Reload = IN_RELOAD,
		ALT1 = IN_ALT1,
		Score = IN_SCORE,
		Shift = 1 << 16
	);


	class GoalQueue {
		struct Element final {
			pokebot::node::NodeID ID = node::Invalid_NodeID;
			int Priority{};
		};

		inline static auto Compare = [](const Element& a, const Element& b) POKEBOT_NOEXCEPT{
			return a.Priority > b.Priority;
		};
		std::set<Element, decltype(Compare)> queue{};
	public:
		bool AddGoalQueue(const node::NodeID ID) POKEBOT_NOEXCEPT {
			return AddGoalQueue(ID, 0);
		}

		bool AddGoalQueue(const node::NodeID ID, const int Priority) POKEBOT_NOEXCEPT {
			assert(ID != node::Invalid_NodeID);
			return queue.insert(Element{ .ID = ID, .Priority = Priority }).second;
		}

		bool IsEmpty() const POKEBOT_NOEXCEPT { return queue.empty(); }
		node::NodeID Get() const POKEBOT_NOEXCEPT { return (queue.empty() ? node::Invalid_NodeID : queue.cbegin()->ID); }
		void Pop() POKEBOT_NOEXCEPT { queue.erase(queue.begin()); }
		void Remove(const node::NodeID ID) POKEBOT_NOEXCEPT { queue.erase(std::find_if(queue.begin(), queue.end(), [ID](const Element& E) { return E.ID == ID; })); }
		void Clear() POKEBOT_NOEXCEPT { queue.clear(); }
	};
}