#pragma once

#ifdef NDEBUG
#define assert_or_abort(EXPR) if (!EXPR) abort()
#else
#define assert_or_abort(EXPR) assert(EXPR)
#endif

namespace pokebot::util {
	template<size_t N>
	class fixed_string {
		static_assert(N >= 2);
		char str[N]{};
	public:
		fixed_string() {}
		fixed_string(const char* const a) { operator=(a); }

		char* begin() noexcept { return &str[0]; }
		const char* begin() const noexcept { return &str[0]; }
		const char* cbegin() const noexcept { return &str[0]; }
		char* end() noexcept { return &str[N - 1]; }
		const char* end() const noexcept { return &str[N - 1]; }
		const char* cend() const noexcept { return &str[N - 1]; }
		void clear() noexcept { memset(str, '\0', N); }

		template<size_t M> void push_back(const fixed_string<M>& a) noexcept { operator=(a); }
		void push_back(const char* const a) noexcept { operator+=(a); }
		void push_back(const char a) noexcept { operator+=(a); }
		bool contain(const char* const a) const noexcept { return strstr(str, a) != nullptr; }
		const char* c_str() const noexcept { return str; }
		char* data() noexcept { return str; }
		const char* data() const noexcept { return str; }
		bool empty() const noexcept { return strlen(str) <= 0; }

		static consteval size_t size() noexcept { return N; }

		template<size_t M> bool operator==(const fixed_string<M>& a) const noexcept { return operator==(a.c_str()); }
		template<size_t M> bool operator!=(const fixed_string& a) const noexcept { return operator!=(a.c_str()); }
		bool operator==(const char* const a) const noexcept { return strcmp(str, a) == 0; }
		bool operator!=(const char* const a) const noexcept { return !operator==(a); }

		char operator[](const int i) noexcept { assert_or_abort(i >= 0 && i < N); return str[i]; }
		char operator[](const int i) const noexcept { assert_or_abort(i >= 0 && i < N); return str[i]; }
		char at(const int i) noexcept { assert_or_abort(i >= 0 && i < N); return str[i]; }
		char at(const int i) const noexcept { assert_or_abort(i >= 0 && i < N); return str[i]; }

		template<size_t M>
		fixed_string& operator=(const fixed_string<M>& a) noexcept {
			static_assert(M <= N);
			return operator=(a.c_str());
		}

		fixed_string& operator=(const char* const a) noexcept {
			assert_or_abort(strlen(a) <= N);
			strcpy(str, a);
			return *this;
		}

		fixed_string& operator=(const char a) noexcept {
			const char res[2]{ a, '\0' };
			return operator=(res);
		}

		template<size_t M>
		fixed_string& operator+=(const fixed_string<M>& a) noexcept {
			static_assert(M <= N);
			return operator+=(a.c_str());
		}

		fixed_string& operator+=(const char* const a) noexcept {
			assert_or_abort(strlen(str) + strlen(a) <= N);
			strcat(str, a);
			return *this;
		}

		fixed_string& operator+=(const char a) noexcept {
			const char res[2]{ a, '\0' };
			return operator+=(res);
		}

		template<size_t M>
		fixed_string operator+(const fixed_string<M>& a) const noexcept {
			static_assert(M <= N);
			return operator+(a.c_str());
		}

		fixed_string operator+(const char* const a) const noexcept {
			char res[N]{};
			assert_or_abort(strlen(str) + strlen(a) <= N);
			strcat(res, str);
			strcat(res, a);
			return fixed_string<N>{res};
		}

		fixed_string operator+(const char a) const noexcept {
			char res[N]{};
			char a_to_str[2]{ a, '\0' };
			return operator+(a_to_str);
		}

		struct Hash {
			inline size_t operator()(const fixed_string& s) const {
				return std::hash<std::string>()(s.c_str());
			}
		};
	};

	// Maximum player length is 32.
	using PlayerName = fixed_string<32u>;
}