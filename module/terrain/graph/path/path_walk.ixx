export module pokebot.terrain.graph.path.path_walk;

export namespace pokebot::node {
	template<typename NodeIdentity>
	class PathWalk {
		std::list<NodeIdentity> nodes{};
		std::list<NodeIdentity>::const_iterator cursor{};
	public:
		bool Contains(const NodeIdentity n) const noexcept {
			return std::find(nodes.cbegin(), nodes.cend(), n) != nodes.cend();
		}

		size_t Size() const {
			return nodes.size();
		}

		void PushFront(const NodeIdentity n) POKEBOT_NOEXCEPT {
			nodes.push_front(n);
			cursor = nodes.begin();
		}

		void PushBack(const NodeIdentity n) POKEBOT_NOEXCEPT {
			nodes.push_back(n);
			cursor = nodes.begin();
		}

		NodeIdentity Current() const POKEBOT_NOEXCEPT {
			return *cursor;
		}

		NodeIdentity Destination() const POKEBOT_NOEXCEPT {
			return nodes.back();
		}

		bool Next() POKEBOT_NOEXCEPT {
			cursor++;
			return !IsEnd();
		}

		void Previous() POKEBOT_NOEXCEPT {
			cursor--;
		}

		bool IsEnd() const POKEBOT_NOEXCEPT {
			return cursor == nodes.cend();
		}

		bool Empty() const POKEBOT_NOEXCEPT {
			return nodes.empty();
		}

		void Clear() POKEBOT_NOEXCEPT {
			nodes.clear();
		}
	};
}