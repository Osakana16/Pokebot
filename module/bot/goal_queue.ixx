module;
#include <set>
#include <unordered_set>
export module pokebot.bot: goal_queue;
import pokebot.terrain.graph.node.id;

namespace pokebot::bot {

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