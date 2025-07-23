#if 0
#include <fstream>
import std;
export module pokebot.terrain.graph.node.point;
import :node_id;
import :node_flag;


export namespace pokebot::node {
	// Waypoint
	class Point final {
		inline operator Vector() const POKEBOT_NOEXCEPT { return Origin(); }


		static constexpr float MAX_COST = std::numeric_limits<float>::max();
		std::unordered_set<NodeID> connections{};

		NodeFlag flag{};

		Vector point{};
	public:
		float time{};
	
		std::pair<float, float> Length() const POKEBOT_NOEXCEPT {
			return std::make_pair(Range<float>, Range<float>);
		}

		Point(const Vector& Initialize_Point) : point(Initialize_Point) {}
				
		std::pair<std::unordered_set<NodeID>::const_iterator, std::unordered_set<NodeID>::const_iterator> GetConnections() const POKEBOT_NOEXCEPT {
			return std::make_pair(connections.begin(), connections.end());
		}

		bool AddConnection(const NodeID node_id) POKEBOT_NOEXCEPT {
			connections.insert(node_id);
			return true;
		}

		bool AddFlag(const NodeFlag flag) POKEBOT_NOEXCEPT {
			this->flag |= flag;
			return true;
		}

		void Write(std::ofstream* const ofs) {
			ofs->write(reinterpret_cast<const char*>(&point), sizeof(point));
			ofs->write(reinterpret_cast<const char*>(&flag), sizeof(flag));
			size_t size = connections.size();
			ofs->write(reinterpret_cast<const char*>(&size), sizeof(size));
			for (auto connection : connections) {
				ofs->write(reinterpret_cast<const char*>(&connection), sizeof(connection));
			}
		}

		void Read(std::ifstream* const ifs) {
			ifs->read(reinterpret_cast<char*>(&point), sizeof(point));
			ifs->read(reinterpret_cast<char*>(&flag), sizeof(flag));
			size_t size{};
			ifs->read(reinterpret_cast<char*>(&size), sizeof(size));
			for (size_t i = 0; i < size; i++) {
				NodeID id{};
				ifs->read(reinterpret_cast<char*>(&id), sizeof(id));
				connections.insert(id);
			}
		}

		const Vector& Origin() const POKEBOT_NOEXCEPT {
			return point;
		}
	};
}
#endif