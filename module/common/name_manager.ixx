export module pokebot.common.name_manager;


export namespace pokebot::common {
	template<typename T>
	class NameManger {
	public:
		virtual ~NameManger() = 0 {}
		virtual const T* const Get(const std::string_view&) const = 0;
		virtual T* const Get(const std::string_view&) = 0;
	};
}