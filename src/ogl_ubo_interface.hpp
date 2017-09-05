
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

	
#pragma pack (push, 1)

////
DECLD constexpr u32 MAX_LIGHTS_COUNT = 8;

struct std140_Transforms {
	static constexpr uptr					align () { return std140::mat4::align(); };
	std140::mat4							model_to_cam;
	std140::mat3							normal_model_to_cam;
};
ASSERT_STD140_STRUCT_SIZE(std140_Transforms);
ASSERT_STD140_ALIGNMENT(std140_Transforms, model_to_cam);
ASSERT_STD140_ALIGNMENT(std140_Transforms, normal_model_to_cam);

struct std140_Material {
	static constexpr uptr					align () { return align_up(std140::vec3::align(), std140::vec4::align()); };
	
	std140::vec3							albedo;
	std140::float_							metallic;
	std140::float_							roughness;
	std140::float_							IOR;
	std140::float_							pad_00[2];
	
	std140_Material (): IOR{1.5f} {} // IOR defaults to Epic Games value of 1.5, -> F0 (dielectric) = 0.04
};
ASSERT_STD140_STRUCT_SIZE(std140_Material);
ASSERT_STD140_ALIGNMENT(std140_Material, albedo);
ASSERT_STD140_ALIGNMENT(std140_Material, metallic);
ASSERT_STD140_ALIGNMENT(std140_Material, roughness);
ASSERT_STD140_ALIGNMENT(std140_Material, IOR);

struct std140_Light {
	static constexpr uptr					align () { return std140::vec4::align(); };
											// if w == 0.0:	xyz: direction to directional light source
											// else:		xyz: pos of point light
	std140::vec4							light_vec_cam;
	std140::vec3							luminance;
	std140::uint_							shad_i;
	std140::mat4							cam_to_light;
};
ASSERT_STD140_STRUCT_SIZE(std140_Light);
ASSERT_STD140_ALIGNMENT(std140_Light, light_vec_cam);
ASSERT_STD140_ALIGNMENT(std140_Light, luminance);
ASSERT_STD140_ALIGNMENT(std140_Light, shad_i);
ASSERT_STD140_ALIGNMENT(std140_Light, cam_to_light);

struct std140_Global {
	std140::mat4							cam_to_clip;
	
	std140::ivec2							target_res;
	std140::vec2							mouse_cursor_pos; // in 0-1 range
	
	std140_Transforms						transforms;
	
	std140_Material							mat;
	
	std140::uint_							lights_count;
	std140::uint_							lightbulb_indx;
	std140::float_							pad_00[2];
	
	std140::array<std140_Light, MAX_LIGHTS_COUNT>	lights;
	
	std140::mat3							camera_to_world;
	
	// PBR dev
	std140::float_							luminance_prefilter_mip_count;
	std140::uint_							luminance_prefilter_mip_indx;
	std140::float_							luminance_prefilter_roughness;
	std140::uint_							luminance_prefilter_base_samples;
	std140::uint_							source_cubemap_res;
	
	std140::float_							avg_log_luminance;
	
	std140::float_							shadow1_inv_far;
};
ASSERT_STD140_ALIGNMENT(std140_Global, cam_to_clip);
ASSERT_STD140_ALIGNMENT(std140_Global, target_res);
ASSERT_STD140_ALIGNMENT(std140_Global, mouse_cursor_pos);
ASSERT_STD140_ALIGNMENT(std140_Global, transforms);
ASSERT_STD140_ALIGNMENT(std140_Global, mat);
ASSERT_STD140_ALIGNMENT(std140_Global, lights_count);
ASSERT_STD140_ALIGNMENT(std140_Global, lightbulb_indx);
ASSERT_STD140_ALIGNMENT(std140_Global, lights);
ASSERT_STD140_ALIGNMENT(std140_Global, camera_to_world);
ASSERT_STD140_ALIGNMENT(std140_Global, luminance_prefilter_mip_count);
ASSERT_STD140_ALIGNMENT(std140_Global, luminance_prefilter_mip_indx);
ASSERT_STD140_ALIGNMENT(std140_Global, luminance_prefilter_roughness);
ASSERT_STD140_ALIGNMENT(std140_Global, luminance_prefilter_base_samples);
ASSERT_STD140_ALIGNMENT(std140_Global, source_cubemap_res);
ASSERT_STD140_ALIGNMENT(std140_Global, avg_log_luminance);
ASSERT_STD140_ALIGNMENT(std140_Global, shadow1_inv_far);

// For writing sub-sets of the global uniform block
struct std140_Transforms_Material {
	std140_Transforms						transforms;
	std140_Material							mat;
};
struct std140_Shading {
	std140::uint_							lights_count;
	std140::uint_							lightbulb_indx;
	std140::float_							pad_00[2];
	
	std140::array<std140_Light, MAX_LIGHTS_COUNT>	lights;
};

// For VBO writing
struct std140_Ground_Plane_Vertex {
	v3										pos_world;
	v3										normal_world;
	v2										uv;
};
////

#pragma pack (pop)

DECL void _ubo_write (void const* data, GLintptr offs, GLsizeiptr size) {
	glBufferSubData(GL_UNIFORM_BUFFER, offs, size, data);
}
template <typename STD140_T, typename T> DECL FORCEINLINE void ubo_write (T val, GLintptr offs) {
	STD140_T temp;
	temp.set(val);
	_ubo_write(&temp, offs, sizeof_t(GLsizeiptr, STD140_T));
}

#define GLOBAL_UBO_WRITE(global_ubo_member, pval) \
		_ubo_write( pval, offsetof_t(GLintptr, std140_Global, global_ubo_member), sizeof_t(GLsizeiptr, decltype(*pval)) )
#define GLOBAL_UBO_WRITE_VAL(std140_type, global_ubo_member, val) \
		ubo_write<std140_type>( val, offsetof_t(GLintptr, std140_Global, global_ubo_member) )

////
#pragma pack (pop)
