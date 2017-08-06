
#pragma pack (push, 1)
	
namespace std140 {
	struct float_ {
		static_assert(sizeof(f32) == 4, "");
		static constexpr uptr	align () { return 4; }
		f32						raw;
		
		void set (f32 f) {
			raw = f;
		}
		f32 get () const {
			return raw;
		}
	};
	struct bool_ {
		static constexpr uptr	align () { return 4; };
		u32						raw;
		
		void set (bool b) {
			STATIC_ASSERT(	static_cast<u32>(false) == 0 && static_cast<u32>(true) == 1);
			raw = static_cast<u32>(b);
		}
	};
	struct sint_ {
		static_assert(sizeof(GLint) == 4, "");
		static constexpr uptr	align () { return 4; };
		GLint					raw;
		
		void set (GLint i) {
			raw = i;
		}
	};
	struct uint_ {
		static_assert(sizeof(GLuint) == 4, "");
		static constexpr uptr	align () { return 4; };
		GLuint					raw;
		
		void set (GLuint u) {
			raw = u;
		}
	};
	
	struct ivec2 {
		static constexpr uptr	align () { return 2 * sizeof(sint_); };
		tv2<GLint>				raw;
		
		void set (tv2<GLint> vp v) {
			raw = v;
		}
	};
	struct uvec2 {
		static constexpr uptr	align () { return 2 * sizeof(uint_); };
		tv2<GLuint>				raw;
		
		void set (tv2<GLuint> vp v) {
			raw = v;
		}
	};
	struct uvec4 {
		static constexpr uptr	align () { return 4 * sizeof(uint_); };
		tv4<GLuint>				raw;
		
		void set (tv4<GLuint> vp v) {
			raw = v;
		}
	};
	
	struct vec2 {
		static constexpr uptr	align () { return 2 * sizeof(float_); };
		v2						raw;
		
		void set (v2 vp v) {
			raw = v;
		}
	};
	struct vec3 {
		static constexpr uptr	align () { return 4 * sizeof(float_); };
		v3						raw;
		
		void set (v3 vp v) {
			raw = v;
		}
	};
	struct vec4 {
		static constexpr uptr	align () { return 4 * sizeof(float_); };
		static_assert((4 * sizeof(float_)) >= 16, "");
		v4						raw;
		
		void set (v4 vp v) {
			raw = v;
		}
	};
	
	template <typename T, uptr LEN_>
	struct array {
		static constexpr uptr	LEN = LEN_;
		static constexpr uptr	align () { return align_up(T::align(), vec4::align()); };
		static constexpr uptr	stride () { return align_up(sizeof(T), vec4::align()); };
		byte					raw[stride() * LEN];
		
		T& operator[] (uptr indx) {
			assert(indx < LEN);
			return reinterpret_cast<T&>(*reinterpret_cast<T*>(&raw[0] +(indx * stride())));
		}
		template <typename INDEXABLE>
		void set_all (INDEXABLE val) {
			for (uptr i=0; i<LEN; ++i)
				this->operator[](i).set(val[i]);
		}
	};
	
	struct mat3 {
		static constexpr uptr	align () { return array<vec3, 3>::align(); };
		static_assert(array<vec3, 3>::align() >= 16, "");
		v4						arr[3];
		
		void set (m3 mp m) {
			arr[0] = v4(m.column(0), 0.0f);
			arr[1] = v4(m.column(1), 0.0f);
			arr[2] = v4(m.column(2), 0.0f);
		}
	};
	struct mat4 {
		static constexpr uptr	align () { return array<vec4, 4>::align(); };
		static_assert(array<vec4, 4>::align() >= 16, "");
		m4						raw;
		
		void set (m4 mp m) {
			raw = m;
		}
		void set (hm mp m) {
			raw = m.m4();
		}
	};
	
	template <uptr L, uptr R> // MSVC somehow can compile this, but not ASSERT_STD140_ALIGNMENT with a resular %
	struct _Modulus {
		static constexpr uptr VAL = L % R;
	};
}
	
	#define ASSERT_STD140_STRUCT_SIZE(structure) \
		static_assert(std140::_Modulus<sizeof(structure), structure::align()>::VAL == 0, "std140 alignment failed, add padding.")
	#define ASSERT_STD140_ALIGNMENT(structure, member) \
		static_assert(std140::_Modulus<offsetof(structure, member), decltype(structure::member)::align()>::VAL == 0, "std140 alignment failed, add padding.")
	#define ASSERT_STD140_ALIGNMENT_IDENTICAL(std140_struct, structure, member) \
		static_assert(offsetof(std140_struct, member) == offsetof(structure, member), "std140 alignment failed, add padding.")
	
#pragma pack (pop)
