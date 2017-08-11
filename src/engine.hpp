	
	#include "stb/helper.hpp"
	
namespace engine {
	
	/*
		Coordinate system convetions:
		X - right/east,		red, right hand is primary
		Y - forward/north,	green
		Z - up,				blue, least important coordinate, discribes height in maps etc., x & y are like coordinates in a map
	*/
	
	#define PI				3.1415926535897932384626433832795f
	#define TWO_PI			6.283185307179586476925286766559f
	#define PId				3.1415926535897932384626433832795
	#define TWO_PId			6.283185307179586476925286766559
	
	#define QNAN			fp::qnan<f32>()
	#define QNANd			fp::qnan<f64>()
	
	#define DEG_TO_RADd		(TWO_PId / 360.0)
	#define DEG_TO_RAD		((f32)DEG_TO_RADd)
	#define RAD_TO_DEGd		(360.0 / TWO_PId)
	#define RAD_TO_DEG		((f32)RAD_TO_DEGd)
	
	// Units which always convert to the internal representation (which is radians)
	DECL constexpr  f32 deg (f32 deg) {
		return deg * DEG_TO_RAD;
	}
	DECL constexpr  f64 deg (f64 deg) {
		return deg * DEG_TO_RADd;
	}
	DECL constexpr  f32 rad (f32 rad) {
		return rad;
	}
	DECL constexpr  f64 rad (f64 rad) {
		return rad;
	}
	
	template <typename T>	DECL constexpr tv2<T> deg (tv2<T> vp deg) {		return deg * tv2<T>(T(DEG_TO_RADd)); }
	template <typename T>	DECL constexpr tv3<T> deg (tv3<T> vp deg) {		return deg * tv3<T>(T(DEG_TO_RADd)); }
	template <typename T>	DECL constexpr tv4<T> deg (tv4<T> vp deg) {		return deg * tv4<T>(T(DEG_TO_RADd)); }
	template <typename T>	DECL constexpr tv2<T> rad (tv2<T> vp rad) {		return rad; }
	template <typename T>	DECL constexpr tv3<T> rad (tv3<T> vp rad) {		return rad; }
	template <typename T>	DECL constexpr tv4<T> rad (tv4<T> vp rad) {		return rad; }
	
	// Conversions
	DECL constexpr  f64 to_deg (f64 rad) {
		return rad * RAD_TO_DEGd;
	}
	DECL constexpr  f32 to_deg (f32 rad) {
		return rad * RAD_TO_DEG;
	}
	DECL constexpr  f32 to_rad (f32 deg) {
		return deg * DEG_TO_RAD;
	}
	DECL constexpr  f64 to_rad (f64 deg) {
		return deg * DEG_TO_RADd;
	}
	
	template <typename T>	DECL constexpr tv2<T> to_deg (tv2<T> vp rad) {	return rad * tv2<T>(T(RAD_TO_DEGd)); }
	template <typename T>	DECL constexpr tv3<T> to_deg (tv3<T> vp rad) {	return rad * tv3<T>(T(RAD_TO_DEGd)); }
	template <typename T>	DECL constexpr tv4<T> to_deg (tv4<T> vp rad) {	return rad * tv4<T>(T(RAD_TO_DEGd)); }
	template <typename T>	DECL constexpr tv2<T> to_rad (tv2<T> vp deg) {	return deg * tv2<T>(T(DEG_TO_RADd)); }
	template <typename T>	DECL constexpr tv3<T> to_rad (tv3<T> vp deg) {	return deg * tv3<T>(T(DEG_TO_RADd)); }
	template <typename T>	DECL constexpr tv4<T> to_rad (tv4<T> vp deg) {	return deg * tv4<T>(T(DEG_TO_RADd)); }
	
	DECL f32 mod_pn_180deg (f32 ang) { // mod float into -/+ 180deg range
		return fp::proper_mod(ang +deg(180.0f), deg(360.0f)) -deg(180.0f);
	}
	
	template <typename T>	DECL T to_linear (T srgb) {
		if (srgb <= T(0.0404482362771082)) {
			return srgb * T(1/12.92);
		} else {
			return fp::pow( (srgb +T(0.055)) * T(1/1.055), T(2.4) );
		}
	}
	template <typename T>	DECL T to_srgb (T linear) {
		if (linear <= T(0.00313066844250063)) {
			return linear * T(12.92);
		} else {
			return ( T(1.055) * fp::pow(linear, T(1/2.4)) ) -T(0.055);
		}
	}
	
	template <typename T> DECL constexpr tv3<T> to_linear (tv3<T> srgb) {
		return tv3<T>( to_linear(srgb.x), to_linear(srgb.y), to_linear(srgb.z) );
	}
	template <typename T> DECL constexpr tv3<T> to_srgb (tv3<T> linear) {
		return tv3<T>( to_srgb(linear.x), to_srgb(linear.y), to_srgb(linear.z) );
	}
	
	template <typename T>	DECL constexpr tv3<T> col (T x, T y, T z) {		return tv3<T>(x,y,z); }
	template <typename T>	DECL constexpr tv3<T> srgb (T x, T y, T z) {	return to_linear(tv3<T>(x,y,z) * tv3<T>(T(1.0/255.0))); }
	template <typename T>	DECL constexpr tv3<T> col (T all) {				return col(all,all,all); }
	template <typename T>	DECL constexpr tv3<T> srgb (T all) {			return srgb(all,all,all); }
	
	DECL m3 scale_3 (v3 vp scale) {
		m3 ret = m3::zero();
		ret.row(0, 0, scale.x);
		ret.row(1, 1, scale.y);
		ret.row(2, 2, scale.z);
		return ret;
	}
	DECL m3 rotate_X3 (f32 angle) {
		auto t = fp::sincos(angle);
		m3 ret = m3::row(	1,		0,		0,
							0,		+t.c,	-t.s,
							0,		+t.s,	+t.c );
		return ret;
	}
	DECL m3 rotate_Y3 (f32 angle) {
		auto t = fp::sincos(angle);
		m3 ret = m3::row(	+t.c,	0,		+t.s,
							0,		1,		0,
							-t.s,	0,		+t.c );
		return ret;
	}
	DECL m3 rotate_Z3 (f32 angle) {
		auto t = fp::sincos(angle);
		m3 ret = m3::row(	+t.c,	-t.s,	0,
							+t.s,	+t.c,	0,
							0,		0,		1 );
		return ret;
	}
	
	DECL m4 translate_4 (v3 vp v) {
		m4 ret = m4::ident();
		ret.column(3, v4(v, 1));
		return ret;
	}
	DECL m4 scale_4 (v3 vp scale) {
		m4 ret = m4::zero();
		ret.row(0, 0, scale.x);
		ret.row(1, 1, scale.y);
		ret.row(2, 2, scale.z);
		ret.row(3, 3, 1);
		return ret;
	}
	DECL m4 rotate_X4 (f32 angle) {
		auto t = fp::sincos(angle);
		m4 ret = m4::row(	1,		0,		0,		0,
							0,		+t.c,	-t.s,	0,
							0,		+t.s,	+t.c,	0,
							0,		0,		0,		1 );
		return ret;
	}
	DECL m4 rotate_Y4 (f32 angle) {
		auto t = fp::sincos(angle);
		m4 ret = m4::row(	+t.c,	0,		+t.s,	0,
							0,		1,		0,		0,
							-t.s,	0,		+t.c,	0,
							0,		0,		0,		1 );
		return ret;
	}
	DECL m4 rotate_Z4 (f32 angle) {
		auto t = fp::sincos(angle);
		m4 ret = m4::row(	+t.c,	-t.s,	0,		0,
							+t.s,	+t.c,	0,		0,
							0,		0,		1,		0,
							0,		0,		0,		1 );
		return ret;
	}
	
	DECL hm translate_h (v3 vp v) {
		#if 0
		return hm::row(	1, 0, 0, v.x,
						0, 1, 0, v.y,
						0, 0, 1, v.z);
		#else // Better asm
		hm ret;
		ret.arr[0].x = 1;
		ret.arr[0].y = 0;
		ret.arr[0].z = 0;
		ret.arr[1].x = 0;
		ret.arr[1].y = 1;
		ret.arr[1].z = 0;
		ret.arr[2].x = 0;
		ret.arr[2].y = 0;
		ret.arr[2].z = 1;
		ret.arr[3].x = v.x;
		ret.arr[3].y = v.y;
		ret.arr[3].z = v.z;
		return ret;
		#endif
	}
	DECL hm scale_h (v3 vp scale) {
		hm ret = hm::ident();
		ret.row(0, 0, scale.x);
		ret.row(1, 1, scale.y);
		ret.row(2, 2, scale.z);
		return ret;
	}
	DECL hm rotate_x (f32 angle) {
		auto t = fp::sincos(angle);
		return hm::row(	1,		0,		0,		0,
						0,		+t.c,	-t.s,	0,
						0,		+t.s,	+t.c,	0 );
	}
	DECL hm rotate_y (f32 angle) {
		auto t = fp::sincos(angle);
		return hm::row(	+t.c,	0,		+t.s,	0,
						0,		1,		0,		0,
						-t.s,	0,		+t.c,	0 );
	}
	DECL hm rotate_z (f32 angle) {
		auto t = fp::sincos(angle);
		return hm::row(	+t.c,	-t.s,	0,		0,
						+t.s,	+t.c,	0,		0,
						0,		0,		1,		0 );
	}
	
	DECL quat rotate_quat_axis (v3 vp axis, f32 angle) {
		auto t = fp::sincos(angle / 2.0f);
		return quat( axis * v3(t.s), t.c );
	}
	DECL quat rotate_XQuat (f32 angle) {
		auto t = fp::sincos(angle / 2.0f);
		return quat( v3(t.s, 0, 0), t.c );
	}
	DECL quat rotate_YQuat (f32 angle) {
		auto t = fp::sincos(angle / 2.0f);
		return quat( v3(0, t.s, 0), t.c );
	}
	DECL quat rotate_ZQuat (f32 angle) {
		auto t = fp::sincos(angle / 2.0f);
		return quat( v3(0, 0, t.s), t.c );
	}
	
	constexpr f32 _90deg_arr[5] = { 0, +1, 0, -1, 0 };
	DECL fp::SC _90deg_getSC (si multiple) {
		fp::SC ret;
		
		multiple = multiple & 0b11;
		
		ret.s = _90deg_arr[multiple];
		ret.c = _90deg_arr[multiple +1];
		return ret;
	};
	DECL m3 rotate_X_90deg (si multiple) {
		auto g = _90deg_getSC(multiple);
		m3 ret = m3::row(	1,		0,		0,
							0,		+g.c,	-g.s,
							0,		+g.s,	+g.c );
		return ret;
	};
	DECL m3 rotate_Y_90deg (si multiple) {
		auto g = _90deg_getSC(multiple);
		m3 ret = m3::row(	+g.c,	0,		+g.s,
							0,		1,		0,
							-g.s,	0,		+g.c );
		return ret;
	};
	DECL m3 rotate_Z_90deg (si multiple) {
		auto g = _90deg_getSC(multiple);
		m3 ret = m3::row(	+g.c,	-g.s,	0,
							+g.s,	+g.c,	0,
							0,		0,		1 );
		return ret;
	};
	
//// UBO
#include "ogl_ubo_interface.hpp"
	
#pragma pack (push, 1)
	
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
		std140::vec3							power;
		std140::uint_							shad_i;
		std140::mat4							cam_to_light;
	};
	ASSERT_STD140_STRUCT_SIZE(std140_Light);
	ASSERT_STD140_ALIGNMENT(std140_Light, light_vec_cam);
	ASSERT_STD140_ALIGNMENT(std140_Light, power);
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
	
//// VBO
	DECLD constexpr GLuint	GLOBAL_UNIFORM_BLOCK_BINDING =		0;
	
	enum vbo_indx_e : u32 {
		GLOBAL_UNIFORM_BUF=0,
		NO_UV_ARR_BUF,
		NO_UV_INDX_BUF,
		UV_ARR_BUF,
		UV_INDX_BUF,
		UV_TANG_ARR_BUF,
		UV_TANG_INDX_BUF,
		//GROUND_PLANE_ARR_BUF,
		VBO_FULLSCREEN_QUAD,
		VBO_RENDER_CUBEMAP,
		
		VBO_COUNT
	};
	
//// VAO
	enum vao_indx_e : u32 {
		NO_UV_VAO=0,
		UV_VAO,
		UV_TANG_VAO,
		//GROUND_PLANE_VAO,
		VAO_FULLSCREEN_QUAD,
		VAO_RENDER_CUBEMAP,
		
		VAO_COUNT
	};
	
//// Shaders
	typedef GLint tex_unit_t;
	enum tex_unit_e : tex_unit_t {
		TEX_UNIT_ALBEDO =					6,//0,
		// Nanosuit
		DIFFUSE_EMISSIVE_TEX_UNIT =			6,//0,
		NORMAL_TEX_UNIT =					1,
		SPECULAR_ROUGHNESS_TEX_UNIT =		2,
		// Cerberus
		TEX_UNIT_CERB_ALBEDO =				6,//0,
		TEX_UNIT_CERB_NORMAL =				1,
		TEX_UNIT_CERB_METALLIC =			2,
		TEX_UNIT_CERB_ROUGHNESS =			3,
		// PBR global
		ENV_LUMINANCE_TEX_UNIT =			4,
		ENV_ILLUMINANCE_TEX_UNIT =			5,
		TEX_UNIT_PBR_BRDF_LUT =				0,//6,
		
		TEX_UNITS_SHADOW_FIRST =			7,
		
		
		TEX_UNIT_MAIN_PASS_LUMI =			0,
		TEX_UNIT_MAIN_PASS_BLOOM_0 =		1,
		TEX_UNIT_MAIN_PASS_BLOOM_1 =		2,
		TEX_UNIT_MAIN_PASS_BLOOM_2 =		3,
		TEX_UNIT_DBG_LINE_PASS =			4,
		TEX_UNIT_POST_DBG_TEX_0 =			5,
		TEX_UNIT_POST_DBG_TEX_1 =			6,
		
		
		TEX_UNIT_MAIN_PASS_DEPTH =			0,
		
		
		TEX_UNIT_BLOOM_GAUSS_LUT =			1,
	};
	DEFINE_ENUM_ITER_OPS(tex_unit_e, tex_unit_t)
	
	#define SHADERS_X \
		X0(	SHAD_FULLSCREEN_TEX ) \
		X(	SHAD_SHADOW_PASS_DIRECTIONAL ) \
		X(	SHAD_SHADOW_PASS_OMNIDIR ) \
		X(	SHAD_CONVOLVE_LUMINANCE ) \
		X(	SHAD_POST_BLOOM ) \
		X(	SHAD_POSTPROCESS ) \
		X(	SHAD_CAM_ALPHA_COL ) \
		X(	SHAD_TINT_AS_FRAG_COL ) \
		X(	SHAD_SKY ) \
		X(	SHAD_RENDER_IBL_ILLUMINANCE ) \
		X(	SHAD_RENDER_IBL_LUMINANCE_PREFILTER ) \
		X(	SHAD_RENDER_IBL_BRDF_LUT ) \
		X(	SHAD_RENDER_EQUIRECTANGULAR_TO_CUBEMAP ) \
		X(	SHAD_PBR_DEV_NOTEX ) \
		X(	SHAD_PBR_DEV_TEX ) \
		X(	SHAD_PBR_DEV_NORMAL_MAPPED ) \
		X(	SHAD_PBR_DEV_CERBERUS ) \
		X(	SHAD_PBR_DEV_UGLY ) \
		X(	SHAD_PBR_DEV_NANOSUIT )
	
	#define X0(id)	id=0,
	#define X(id)	id,
	enum shader_id_e: u32 {
		SHADERS_X
		SHAD_COUNT,
	};
	#undef X0
	#undef X
	DEFINE_ENUM_ITER_OPS(shader_id_e, u32)
	
	#define X0(id)	STRINGIFY(id),
	#define X(id)	STRINGIFY(id),
	DECLD constexpr char const* SHAD_IDS_NAMES[SHAD_COUNT] = {
		SHADERS_X
	};
	#undef X0
	#undef X
	
	#undef SHADERS_X
	
	DECLD constexpr shader_id_e			SHAD_FIRST =				(shader_id_e)0;
	DECLD constexpr shader_id_e			SHAD_END =					SHAD_COUNT;
	
namespace shaders_n {
	
	struct Source_Str_Shader { // A shader that is not a file, but an a constant string in the exe
		lstr str;
		lstr virtual_filename;
	};
	
	#define _SOURCE_SHADER(name, raw_string) { raw_string, name },
	
	constexpr Source_Str_Shader SOURCE_SHADERS[] = {
		#include "source_shaders.hpp"
	};
	
	struct Texture {
		tex_unit_t		tex_unit;
		char const*		unif_name;
	};
	
	DECLD constexpr u32 MAX_TEXTURES = 12;
	
	struct Program {
		lstr		shads[2];
		Texture		textures[MAX_TEXTURES];
	};
	
	DECLD constexpr GLenum				TYPES[2] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
	DECLD constexpr char const*			TYPES_STR[2] = { "vertex", "fragment" };
	
	DECLD Program						PROGRAMS[SHAD_COUNT] = {
		/* SHAD_FULLSCREEN_TEX */			{	{ "_fullscreen_quad.vert",		"_fullscreen_tex.frag" }, {
													{ 0,							"tex" } }
											},
		/* SHAD_SHADOW_PASS_DIRECTIONAL */	{	{ "_shadow_map_directional.vert",	"_shadow_map_directional.frag" }, {
													}
											},
		/* SHAD_SHADOW_PASS_OMNIDIR */		{	{ "_shadow_map_omnidir.vert",	"_shadow_map_omnidir.frag" }, {
													}
											},
		/* SHAD_CONVOLVE_LUMINANCE */		{	{ "_fullscreen_quad.vert",		"convolve_luminance.frag" }, {
													{ 0,							"tex" } }
											},
		/* SHAD_POST_BLOOM */				{	{ "_fullscreen_quad.vert",		"post_bloom.frag" }, {
													{ TEX_UNIT_MAIN_PASS_LUMI,		"main_pass_lumi_tex" },
													{ TEX_UNIT_BLOOM_GAUSS_LUT,		"gaussian_LUT_tex" } }
											},
		/* SHAD_POSTPROCESS */				{	{ "_fullscreen_quad.vert",		"postprocess.frag" }, {
													{ TEX_UNIT_MAIN_PASS_LUMI,		"main_pass_lumi_tex" },
													{ TEX_UNIT_MAIN_PASS_BLOOM_0,	"main_pass_bloom_0_tex" },
													{ TEX_UNIT_MAIN_PASS_BLOOM_1,	"main_pass_bloom_1_tex" },
													{ TEX_UNIT_MAIN_PASS_BLOOM_2,	"main_pass_bloom_2_tex" },
													{ TEX_UNIT_DBG_LINE_PASS,		"dbg_line_pass" },
													{ TEX_UNIT_POST_DBG_TEX_0,		"dbg_tex_0" },
													{ TEX_UNIT_POST_DBG_TEX_1,		"dbg_tex_1" } }
											},
		/* SHAD_CAM_ALPHA_COL */			{	{ "_camp_rgba.vert",			"rgba.frag" }, {
													{ TEX_UNIT_MAIN_PASS_DEPTH,		"main_pass_depth_tex" } }
											},
		/* SHAD_TINT_AS_FRAG_COL */			{	{ "_model_pos.vert",			"_solid_color.frag" }, {
													}
											},
		/* SHAD_SKY */						{	{ "_sky_transform.vert",		"_solid_color.frag" }, {
													}
											},
		/* SHAD_RENDER_IBL_ILLUMINANCE */	{	{ "_render_cubemap.vert",		"_render_ibl_illuminance.frag" }, {
													{ ENV_LUMINANCE_TEX_UNIT,		"env_luminance_tex" } }
											},
		/* SHAD_RENDER_IBL_LUMINANCE_PREFILTER */{ { "_render_cubemap.vert",		"_render_ibl_luminance_prefilter.frag" }, {
													{ ENV_LUMINANCE_TEX_UNIT,		"env_luminance_tex" } }
											},
		/* SHAD_RENDER_IBL_BRDF_LUT */		{	{ "_fullscreen_quad.vert",		"_render_ibl_brdf_lut.frag" }, {
													}
											},
		/* SHAD_RENDER_EQUIRECTANGULAR_TO_CUBEMAP */{ { "_render_cubemap.vert",		"_render_equirectangular_to_cubemap.frag" }, {
													{ ENV_LUMINANCE_TEX_UNIT,		"env_luminance_tex" } }
											},
		/* SHAD_PBR_DEV_NOTEX */			{	{ "_pbr_dev_notex.vert",		"_pbr_dev_notex.frag" }, {
													{ TEX_UNIT_ALBEDO,				"albedo_tex" },
													{ ENV_LUMINANCE_TEX_UNIT,		"env_luminance_tex" },
													{ ENV_ILLUMINANCE_TEX_UNIT,		"env_illuminance_tex" },
													{ TEX_UNIT_PBR_BRDF_LUT,		"brdf_lut_tex" },
													{ TEX_UNITS_SHADOW_FIRST,		(char const*)1 } } // Special case to generate all 
											},
		/* SHAD_PBR_DEV_TEX */				{	{ "_pbr_dev_tex.vert",			"_pbr_dev_albedo.frag" }, {
													{ TEX_UNIT_ALBEDO,				"albedo_tex" },
													{ ENV_LUMINANCE_TEX_UNIT,		"env_luminance_tex" },
													{ ENV_ILLUMINANCE_TEX_UNIT,		"env_illuminance_tex" },
													{ TEX_UNIT_PBR_BRDF_LUT,		"brdf_lut_tex" },
													{ TEX_UNITS_SHADOW_FIRST,		(char const*)1 } } // Special case to generate all 
											},
		/* SHAD_PBR_DEV_NORMAL_MAPPED */	{	{ "_pbr_dev_normal_mapped.vert", "_pbr_dev_normal_mapped.frag" }, {
													{ TEX_UNIT_ALBEDO,				"albedo_tex" },
													{ NORMAL_TEX_UNIT,				"normal_tex" },
													{ ENV_LUMINANCE_TEX_UNIT,		"env_luminance_tex" },
													{ ENV_ILLUMINANCE_TEX_UNIT,		"env_illuminance_tex" },
													{ TEX_UNIT_PBR_BRDF_LUT,		"brdf_lut_tex" },
													{ TEX_UNITS_SHADOW_FIRST,		(char const*)1 } } // Special case to generate all 
											},
		/* SHAD_PBR_DEV_CERBERUS */			{	{ "_pbr_dev_normal_mapped.vert", "_pbr_dev_cerberus.frag" }, {
													{ TEX_UNIT_CERB_ALBEDO,			"albedo_tex" },
													{ TEX_UNIT_CERB_NORMAL,			"normal_tex" },
													{ TEX_UNIT_CERB_METALLIC,		"metallic_tex" },
													{ TEX_UNIT_CERB_ROUGHNESS,		"roughness_tex" },
													{ ENV_LUMINANCE_TEX_UNIT,		"env_luminance_tex" },
													{ ENV_ILLUMINANCE_TEX_UNIT,		"env_illuminance_tex" },
													{ TEX_UNIT_PBR_BRDF_LUT,		"brdf_lut_tex" },
													{ TEX_UNITS_SHADOW_FIRST,		(char const*)1 } } // Special case to generate all 
											},
		/* SHAD_PBR_DEV_UGLY */				{	{ "_pbr_dev_tex.vert", 			"_pbr_dev_ugly.frag" }, {
													{ TEX_UNIT_CERB_ROUGHNESS,		"roughness_tex" },
													{ ENV_LUMINANCE_TEX_UNIT,		"env_luminance_tex" },
													{ ENV_ILLUMINANCE_TEX_UNIT,		"env_illuminance_tex" },
													{ TEX_UNIT_PBR_BRDF_LUT,		"brdf_lut_tex" },
													{ TEX_UNITS_SHADOW_FIRST,		(char const*)1 } } // Special case to generate all 
											},
		/* SHAD_PBR_DEV_NANOSUIT */			{	{ "_pbr_dev_normal_mapped.vert", "_pbr_dev_nanosuit.frag" }, {
													{ DIFFUSE_EMISSIVE_TEX_UNIT,	"diffusse_emmissive_tex" },
													{ NORMAL_TEX_UNIT,				"normal_tex" },
													{ SPECULAR_ROUGHNESS_TEX_UNIT,	"specular_roughness_tex" },
													{ ENV_LUMINANCE_TEX_UNIT,		"env_luminance_tex" },
													{ ENV_ILLUMINANCE_TEX_UNIT,		"env_illuminance_tex" },
													{ TEX_UNIT_PBR_BRDF_LUT,		"brdf_lut_tex" },
													{ TEX_UNITS_SHADOW_FIRST,		(char const*)1 } } // Special case to generate all 
											},
		
	};
	STATIC_ASSERT(arrlenof(PROGRAMS) == SHAD_COUNT);
	
	lstr read_shader (lstr cr virtual_filename) {
		
		for (uptr i=0; i<arrlenof(SOURCE_SHADERS); ++i) {
			if (str::comp(SOURCE_SHADERS[i].virtual_filename, virtual_filename)) {
				lstr str = SOURCE_SHADERS[i].str;
				str.str = working_stk.append(str.str, str.len);
				return str;
			}
		}
		
		lstr real_filename = str::append_term(&working_stk, SHADERS_DIR, virtual_filename);
		
		Mem_Block ret;
		if (platform::read_whole_file_onto(&working_stk, real_filename.str, RD_FILENAME_ON_STK, &ret)) {
			return {nullptr, 0};
		}
		if (!safe_cast(u32, ret.size)) {
			return {nullptr, 0};
		}
		return {(char*)ret.ptr, (u32)ret.size};
	}
	
	// TODO: make the $include relative to the dir of the file
	DECL lstr process_shader_file (lstr filename) {
		
		lstr file = read_shader(filename);
		if (!file) { return file; }
		
		for (;;) {
			char const* in_cur = file.str;
			char const* in_end = in_cur +file.len;
			
			file.str = working_stk.getTop<char>();
			
			bool included = false;
			
			bool in_line_comment = false;
			
			while (in_cur != in_end) {
				if (		*in_cur == '/' && in_cur[1] == '/' ) {
					in_line_comment = true;
				} else if (	*in_cur == '\r' || *in_cur == '\n' ) {
					in_line_comment = false;
				} else if (	!in_line_comment && *in_cur == '$' ) {
					++in_cur;
					
					included = true;
					
					auto whitespace = [&] () -> bool {
						if (in_cur != in_end && parse_n::is_whitespace_c(*in_cur)) {
							do {
								++in_cur;
							} while (in_cur != in_end && parse_n::is_whitespace_c(*in_cur));
							return true;
						} else {
							return false;
						}
					};
					
					auto is_newline = [&] () {
						auto cur = in_cur;
						char c = *cur;
						if (cur != in_end && parse_n::is_newline_c(c)) {
							++cur;
							assert(cur != in_end);
							if (*cur != c && parse_n::is_newline_c(*cur)) {
								++cur;
							}
						} else {
							return false;
						}
						
						return true;
					};
					
					// Optinal whitespace
					whitespace();
					
					{ // Command string
						char const* inc_str = "include";
						
						while (*inc_str != '\0') {
							assert(in_cur != in_end && *in_cur == *inc_str);
							++in_cur;
							++inc_str;
						}
					}
					
					// Optinal whitespace
					whitespace();
					
					assert(in_cur != in_end && *in_cur == '"');
					++in_cur;
					
					lstr inc_filename;
					inc_filename.str = in_cur;
					
					while (in_cur != in_end && *in_cur != '"') {
						++in_cur;
					}
					
					assert(in_cur != in_end && *in_cur == '"');
					
					inc_filename.len = safe_cast_assert(u32, ptr_sub(inc_filename.str, in_cur));
					
					++in_cur;
					
					// Optinal whitespace
					whitespace();
					
					// Newline or end of input
					assert(in_cur == in_end || is_newline(), "Shader source appender: No newline (or end of file) after '$include \"file\"' statement");
					
					{
						lstr str = read_shader(inc_filename);
						if (!str) { return str; }
					}
				}
				
				working_stk.push(*in_cur++);
			}
			
			file.len = safe_cast_assert(u32, ptr_sub(file.str, working_stk.getTop<char>()));
			
			if (!included) {
				return file;
			}
		}
	}
	DECL GLuint compile_shader (lstr filename, ui type_i) {
		
		auto shad = glCreateShader(TYPES[type_i]);
		assert(shad != 0);
		
		{
			DEFER_POP(&working_stk);
			lstr src_str = process_shader_file(filename);
			if (!src_str) { return 0; }
			
			GLint	len = safe_cast_assert(GLint, src_str.len);
			glShaderSource(shad, 1, &src_str.str, &len);
		}
		
		glCompileShader(shad);
		
		GLint status;
		glGetShaderiv(shad, GL_COMPILE_STATUS, &status);
		
		{
			DEFER_POP(&working_stk);
			
			GLsizei logLen;
			{
				GLint temp = 0;
				glGetShaderiv(shad, GL_INFO_LOG_LENGTH, &temp);
				logLen = safe_cast_assert(GLsizei, temp);
			}
			GLchar const* log_str;
			if (logLen <= 1) {
				log_str = "";
			} else {
				{
					DEFER_POP(&working_stk);
					
					GLsizei src_len;
					{
						GLint temp;
						glGetShaderiv(shad, GL_SHADER_SOURCE_LENGTH, &temp);
						assert(temp > 0);
						src_len = safe_cast_assert(GLsizei, temp);
					}
					
					auto buf = working_stk.pushArr<GLchar>(src_len); // GL_SHADER_SOURCE_LENGTH includes the null terminator
					
					GLsizei written_len;
					glGetShaderSource(shad, src_len, &written_len, buf);
					//assert(written_len <= (src_len -1));
					assert(written_len <= src_len); // BUG: on my AMD desktop written_len was actually the length with null terminator at some point
					
					lstr filepath = str::append_term(&working_stk, SHADERS_DIR, "zz_", filename, ".has_error.tmp");
					
					platform::write_whole_file(filepath.str, buf, src_len -1);
				}
				
				auto* log_str_ = working_stk.pushArr<GLchar>(logLen); // GL_INFO_LOG_LENGTH includes the null terminator
				
				GLsizei writtenLen = 0;
				glGetShaderInfoLog(shad, logLen, &writtenLen, log_str_);
				assert(writtenLen == (logLen -1));
				
				log_str = log_str_;
			}
			
			char const* shader_type_str = TYPES_STR[type_i];
			
			if (status != GL_FALSE) {
				if (logLen > 1) {
					warning("OpenGL % shader compilation log (%):\n>>>\n%\n<<<\n",
							shader_type_str, filename, log_str);
				}
			} else {
				warning("OpenGL error in % shader compilation (%)!\n>>>\n%\n<<<\n",
						shader_type_str, filename, log_str);
			}
		}
		
		return shad;
	}
	struct Program_Shad_Refs {
		GLuint			refs[2];
	};
	
	DECL GLuint link_program (Program cr shad_info, Program_Shad_Refs cr shad_refs, shader_id_e id) {
		
		auto prog = glCreateProgram();
		
		for (ui i=0; i<2; ++i) {
			//print("  attach %\n", shad_info.shads[i]);
			glAttachShader(prog, shad_refs.refs[i]);
		}
		
		glLinkProgram(prog);
		
		GLint status;
		glGetProgramiv(prog, GL_LINK_STATUS, &status);
		
		{
			DEFER_POP(&working_stk);
			
			GLsizei logLen;
			{
				GLint temp = 0;
				glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &temp);
				logLen = safe_cast_assert(GLsizei, temp);
			}
			GLchar const* log_str;
			if (logLen <= 1) {
				log_str = "";
			} else {
				#if 0
				{
					DEFER_POP(&working_stk);
					
					char const* buf = working_stk.getTop<GLchar>();
					uptr len = 0;
					
					for (ui i=0; i<MAX_SHAD_PER_PROG; ++i) {
						if (shader_ids[i] == SHAD_NULL) {
							break;
						}
						auto shad = shaders[ shader_ids[i] ];
						
						GLsizei src_len;
						{
							GLint temp;
							glGetShaderiv(shad, GL_SHADER_SOURCE_LENGTH, &temp);
							assert(temp > 0);
							src_len = safe_cast_assert(GLsizei, temp);
						}
						
						//len += cstr::append(shader 'file'name);
						
						auto buf = working_stk.pushArr<GLchar>(src_len); // GL_SHADER_SOURCE_LENGTH includes the null terminator
						len += src_len;
						
						GLsizei written_len;
						glGetShaderSource(shad, src_len, &written_len, buf);
						
						assert(written_len == (src_len -1));
					}
					
					write_whole_file("shader_with_error.tmp", buf, len);
				}
				#endif
				
				print("For shader program with:\n");
				for (ui i=0; i<2; ++i) {
					print("  '%'\n", shad_info.shads[i]);
				}
				
				auto* log_str_ = working_stk.pushArr<GLchar>(logLen); // GL_INFO_LOG_LENGTH includes the null terminator
				
				GLsizei writtenLen = 0;
				glGetProgramInfoLog(prog, logLen, &writtenLen, log_str_);
				assert(writtenLen == (logLen -1));
				
				log_str = log_str_;
			}
			
			if (status != GL_FALSE) {
				if (logLen > 1) {
					warning("OpenGL shader linkage log:\n'%'\n", log_str);
				} else {
					//warning("OpenGL shader linkage ok, no log.", nl);
				}
			} else {
				warning("OpenGL error in shader linkage!\n'%'\n", log_str);
			}
		}
		
		for (ui i=0; i<2; ++i) {
			//print("  detach %\n", shad_info.shads[i]);
			glDetachShader(prog, shad_refs.refs[i]);
		}
		
		{
			auto block_indx = glGetUniformBlockIndex(prog, "Global");
			if (block_indx == GL_INVALID_INDEX) {
				//print("glGetUniformBlockIndex failed for UBO structure '%' maybe not used not shader?\n",
				//		"Global");
				//assert(false);
			} else {
				glUniformBlockBinding(prog, block_indx, GLOBAL_UNIFORM_BLOCK_BINDING);
			}
			
			glUseProgram(prog);
			
			for (u32 i=0; i<MAX_TEXTURES; ++i) {
				auto cr s = shad_info.textures[i];
				if (!s.unif_name) { break; }
				
				if (s.unif_name == (char const*)1) {
					
					for (u32 i=0; i<MAX_LIGHTS_COUNT; ++i) {
						DEFER_POP(&working_stk);
						cstr name = print_working_stk("shadow_2d_%_tex\\0", i).str;
						
						auto unif = glGetUniformLocation(prog, name);
						if (unif < 0) {
							//warning("link_program (for %):: glGetUniformLocation(, \"%\") failed!",
							//		SHAD_IDS_NAMES[id], buf);
						}
						
						glUniform1i(unif, TEX_UNITS_SHADOW_FIRST +i);
					}
					for (u32 i=0; i<MAX_LIGHTS_COUNT; ++i) {
						DEFER_POP(&working_stk);
						cstr name = print_working_stk("shadow_cube_%_tex\\0", i).str;
						
						auto unif = glGetUniformLocation(prog, name);
						if (unif < 0) {
							//warning("link_program (for %):: glGetUniformLocation(, \"%\") failed!",
							//		SHAD_IDS_NAMES[id], buf);
						}
						
						glUniform1i(unif, TEX_UNITS_SHADOW_FIRST +MAX_LIGHTS_COUNT +i);
					}
					
				} else {
					
					auto unif = glGetUniformLocation(prog, s.unif_name);
					if (unif < 0) {
						//warning("link_program (for %):: glGetUniformLocation(, \"%\") failed!",
						//		SHAD_IDS_NAMES[id], s.unif_name);
					}
					
					glUniform1i(unif, s.tex_unit);
				}
			}
		}
		
		return prog;
	};
	
	DECL bool load (GLuint* prog_refs) {
		
		bool success_accum = true;
		
		constexpr u32 MAX_SHAD_COUNT = SHAD_COUNT * 2;
		
		DEFER_POP(&working_stk);
		lstr*	shad_filenames = working_stk.pushArr<lstr>(MAX_SHAD_COUNT);
		auto*	shad_refs = working_stk.pushArr<GLuint>(MAX_SHAD_COUNT);
		u32		shad_count = 0;
		
		for (auto i=SHAD_FIRST; i<SHAD_END; ++i) {
			
			auto prog = PROGRAMS[i];
			
			Program_Shad_Refs prog_shad_refs;
			
			for (ui type_i=0; type_i<2; ++type_i) {
				
				lstr filename = prog.shads[type_i];
				
				GLuint* ref = nullptr;
				for (u32 i=0; i<shad_count; ++i) {
					if (str::comp(shad_filenames[i], filename)) {
						ref = &shad_refs[i];
						break;
					}
				}
				
				if (!ref) {
					
					shad_filenames[shad_count] = filename;
					ref = &shad_refs[shad_count++];
					
					*ref = compile_shader(filename, type_i);
					if (*ref == 0) { success_accum = false; }
				}
				
				prog_shad_refs.refs[type_i] = *ref;
			}
			
			prog_refs[i] = link_program(prog, prog_shad_refs, i);
			if (prog_refs[i] == 0) { success_accum = false; }
		}
		
		for (u32 i=0; i<shad_count; ++i) {
			glDeleteShader(shad_refs[i]);
		}
		
		return success_accum;
	}
	DECL void load_warn (GLuint* prog_refs) {
		if (!load(prog_refs)) {
			warning("Error in shader load!");
		}
	}
	
	DECL void reload (GLuint* prog_refs) {
		
		print("Reloading shaders.\n");
		
		GLuint new_progs[SHAD_COUNT] = {}; // zero all, so we can easily glDeleteProgram() all if error occurs
		
		if (!load(new_progs)) {
			print("Error in shader reload, shaders not reloaded.\n");
			
			for (u32 i=0; i<SHAD_COUNT; ++i) {
				glDeleteProgram(new_progs[i]);
			}
			
			return;
		}
		
		for (u32 i=0; i<SHAD_COUNT; ++i) {
			glDeleteProgram(prog_refs[i]);
			prog_refs[i] = new_progs[i];
		}
	}
	
}
	
//// Meshes
	#define MESHES \
		X0(	nouv_MSH_AXIS_CROSS_PX ) \
		X(	nouv_MSH_AXIS_CROSS_NX ) \
		X(	nouv_MSH_AXIS_CROSS_PY ) \
		X(	nouv_MSH_AXIS_CROSS_NY ) \
		X(	nouv_MSH_AXIS_CROSS_PZ ) \
		X(	nouv_MSH_AXIS_CROSS_NZ ) \
		X(	nouv_MSH_AXIS_CROSS_PLANE_XY ) \
		X(	nouv_MSH_3D_LINE_COMPONENT ) \
		X(	nouv_MSH_SPHERE ) \
		 \
		X(	nouv_MSH_LIGHT_BULB ) \
		X(	nouv_MSH_SUN_LAMP_MSH ) \
		 \
		X(	nouv_MSH_STRUCTURE_RING ) \
		X(	nouv_MSH_TERRAIN ) \
		X(	nouv_MSH_UTAHTEAPOT ) \
		 \
		X(	nouv_MSH_PLANETARIUM ) \
		X(	nouv_MSH_PLANETARIUM_PROJECTOR ) \
		 \
		X(	nouv_MSH_ICO_SPHERE ) \
		X(	nouv_MSH_STFD_BUNNY ) \
		X(	nouv_MSH_STFD_BUDDHA ) \
		X(	nouv_MSH_STFD_DRAGON ) \
		 \
		X(	nouv_MSH_WINDOW_PILLAR ) \
		X(	nouv_MSH_SHADOW_TEST_0 ) \
		 \
		X(	uv_MSH_UNIT_PLANE ) \
		 \
		X(	uv_SCENE_GROUND0 ) \
		X(	uv_MSH_UGLY ) \
		X(	uv_MSH_TERRAIN_TREE ) \
		X(	uv_MSH_TERRAIN_TREE_CUTS ) \
		X(	uv_MSH_TERRAIN_TREE_BLOSSOMS ) \
		X(	uv_MSH_TERRAIN_CUBE ) \
		X(	uv_MSH_TERRAIN_SPHERE ) \
		X(	uv_MSH_TERRAIN_OBELISK ) \
		X(	uv_MSH_STRUCTURE_WALLS ) \
		X(	uv_MSH_STRUCTURE_GROUND ) \
		X(	uv_MSH_STRUCTURE_BLOCK1 ) \
		X(	uv_MSH_STRUCTURE_BLOCK2 ) \
		X(	uv_MSH_STRUCTURE_BLOCK3 ) \
		X(	uv_MSH_STRUCTURE_BLOCK4 ) \
		X(	uv_MSH_STRUCTURE_BEAM ) \
		X(	uv_MSH_STRUCTURE_BEAM_CUTS ) \
		X(	uv_MSH_PLANETARIUM_STAND ) \
		 \
		X(	uv_tang_MSH_UNIT_PLANE ) \
		 \
		X(	uv_tang_NANOSUIT_TORSO ) \
		X(	uv_tang_NANOSUIT_LEGS ) \
		X(	uv_tang_NANOSUIT_NECK ) \
		X(	uv_tang_NANOSUIT_HELMET ) \
		 \
		X(	uv_tang_NORM_TEST_00 ) \
		\
		X(	uv_tang_CERBERUS )
	
	#define X0(name)	name=0,
	#define X(name)		name,
	enum mesh_id_e: u32 {
		MESHES
		MESHES_COUNT,
	};
	#undef X0
	#undef X
	DEFINE_ENUM_ITER_OPS(mesh_id_e, u32)
	
	#define X0(name)	TO_STR(name),
	#define X(name)		TO_STR(name),
	DECLD constexpr char const* MESH_ID_NAMES[MESHES_COUNT] {
		MESHES
	};
	#undef X0
	#undef X
	
	#undef MESHES
	
	DECLD constexpr mesh_id_e		NOUV_MSH_FIRST =				nouv_MSH_AXIS_CROSS_PX;
	DECLD constexpr mesh_id_e		NOUV_MSH_END =					(mesh_id_e)(nouv_MSH_SHADOW_TEST_0 +1);
	DECLD constexpr u32				NOUV_MSH_COUNT =				NOUV_MSH_END -NOUV_MSH_FIRST;
	
	DECLD constexpr mesh_id_e		UV_MSH_FIRST =					uv_MSH_UNIT_PLANE;
	DECLD constexpr mesh_id_e		UV_MSH_END =					(mesh_id_e)(uv_MSH_PLANETARIUM_STAND +1);
	DECLD constexpr u32				UV_MSH_COUNT =					UV_MSH_END -UV_MSH_FIRST;
	
	DECLD constexpr mesh_id_e		UV_TANG_MSH_FIRST =				uv_tang_MSH_UNIT_PLANE;
	DECLD constexpr mesh_id_e		UV_TANG_MSH_END =				(mesh_id_e)(uv_tang_CERBERUS +1);
	DECLD constexpr u32				UV_TANG_MSH_COUNT =				UV_TANG_MSH_END -UV_TANG_MSH_FIRST;
	
	DECLD char const*				mesh_names[MESHES_COUNT];
	
	#include "meshes_file.hpp"
	
	struct Mesh_Ref {
		GLvoid*				indx_offsets;
		GLsizei				indx_count;
		GLint				base_vertecies;
	};
	
namespace meshes_file_n {
	struct Meshes_File {
		
		Header*		header;
		uptr		file_size;
		
		DECLM void reload () {
			
			if (header) {
				free(header);
			}
			
			//stk->alignTop<FILE_ALIGN>();
			// Ignore FILE_ALIGN, not sure what to do about alignment for windows heap
			
			{
				Mem_Block file_data;
				assert(!platform::read_whole_file_onto_heap(MESHES_FILENAME, 0, &file_data));
				header = reinterpret_cast<Header*>(file_data.ptr);
				file_size = file_data.size;
			}
			
			assert(file_size >= sizeof(Header));
			assert(cmemcmp(header->magicString, magicString, sizeof(header->magicString)));
			
			assert(is_aligned(header->file_size, FILE_ALIGN));
		}
		
		DECLM Header_Mesh_Entry* query_mesh (const char* name) {
			
			if (!name) {
				return nullptr;
			}
			
			char* cur = (char*)ptr_add(header, sizeof(Header));
			char* end = (char*)ptr_add(header, file_size);
			
			for (u32 entry=0; entry<header->meshEntryCount; ++entry) {
				
				cur = align_up(cur, HEADER_MESH_ENTRY_ALIGN);
				
				auto ret = (Header_Mesh_Entry*)cur;
				
				cur += sizeof(Header_Mesh_Entry);
				
				assert(cur < end);
				
				bool name_matched = true;
				
				auto desiredName = name;
				do {
					assert(cur != end);
					if (name_matched && *cur != *desiredName++) {
						name_matched = false;
					}
				} while (*cur++ != '\0');
				
				if (name_matched) {
					return ret;
				}
			}
			return nullptr;
		}
		DECLM void reload_meshes (format_t format, mesh_id_e first_mesh, u32 mesh_count, Mesh_Ref* meshes) {
			
			uptr sizePerVertex = 0;
			uptr sizePerIndex;
			{
				{
					assert((format & INTERLEAVED_MASK) ==	INTERLEAVED);
					
					assert((format & POS_MASK) ==			POS_XYZ);
					sizePerVertex += 3;
					
					assert((format & NORM_MASK) ==			NORM_XYZ);
					sizePerVertex += 3;
					
					
					switch (format & TANG_MASK) {
						case TANG_NONE:	sizePerVertex += 0;	break;
						case TANG_XYZW:	sizePerVertex += 4;	break;
						default: assert(false);
					}
					
					switch (format & UV_MASK) {
						case UV_NONE:	sizePerVertex += 0;	break;
						case UV_UV:		sizePerVertex += 2;	break;
						default: assert(false);
					}
					
					assert((format & COL_MASK) ==			COL_NONE);
					
					sizePerVertex *= sizeof(f32);
				}
				{
					assert((format & INDEX_MASK) ==			INDEX_USHORT);
					sizePerIndex = sizeof(GLushort);
				}
			}
			
			u64	vertexCounter = 0;
			u64	indexCounter = 0;
			
			for (mesh_id_e i=first_mesh; i<(first_mesh +mesh_count); ++i) {
				
				meshes[i].indx_offsets =	0;
				meshes[i].indx_count =		0;
				meshes[i].base_vertecies =	0;
				
				auto mesh = query_mesh(mesh_names[i]);
				if (mesh == nullptr) {
					warning("queryMesh(\'%\') returned null (\"%\"), not loaded!\n",
							MESH_ID_NAMES[i], mesh_names[i] ? mesh_names[i] : "<null>" );
					continue;
				}
				
				assert(is_aligned(mesh->dataOffset, DATA_ALIGN));
				
				if (mesh->dataFormat != format) {
					auto actual = format_strs(mesh->dataFormat);
					auto desired = format_strs(format);
					warning("Wrong format of mesh % got %%%% (%) but wanted %%%% (%), not loaded!\n",
							mesh_names[i],
							actual.f, actual.tang, actual.uv, actual.col,		(format_t)mesh->dataFormat,
							desired.f, desired.tang, desired.uv, desired.col,	(format_t)format);
					continue;
				}
				
				meshes[i].indx_offsets =	(GLvoid*)(indexCounter * sizePerIndex);
				meshes[i].indx_count =		safe_cast_assert(GLsizei, mesh->indexCount);
				meshes[i].base_vertecies =	safe_cast_assert(GLint, vertexCounter);
				
				GLsizei vertexCount = mesh->vertexCount.glushort;
				
				vertexCounter +=	vertexCount;
				indexCounter +=		mesh->indexCount;
				
			}
			
			u64 totalVertexSize = vertexCounter * sizePerVertex;
			u64 totalIndexSize = indexCounter * sizePerIndex;
			
			glBufferData(GL_ARRAY_BUFFER,
					safe_cast_assert(GLsizeiptr, totalVertexSize), NULL, GL_STATIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER,
					safe_cast_assert(GLsizeiptr, totalIndexSize), NULL, GL_STATIC_DRAW);
			
			vertexCounter = 0;
			indexCounter = 0;
			
			byte* file_data = (byte*)header;
			
			for (mesh_id_e i=first_mesh; i<(first_mesh +mesh_count); ++i) {
				auto mesh = query_mesh(mesh_names[i]);
				if (mesh == nullptr) {
					continue;
				}
				
				GLsizei		vertexCount = mesh->vertexCount.glushort;
				
				u64			vertSize = vertexCount * sizePerVertex;
				u64			indexSize = mesh->indexCount * sizePerIndex;
				
				GLsizeiptr	gl_vertSize = safe_cast_assert(GLsizeiptr, vertSize);
				GLsizeiptr	gl_indexSize = safe_cast_assert(GLsizeiptr, indexSize);
				
				assert((mesh->dataOffset +vertSize) <= file_size);
				assert((mesh->dataOffset +vertSize +indexSize) <= file_size);
				
				auto vert_data = align_up(file_data +mesh->dataOffset, sizeof(GLfloat));
				auto indx_data = align_up(vert_data +vertSize, sizeof(GLushort));
				
				glBufferSubData(GL_ARRAY_BUFFER,
						safe_cast_assert(GLintptr, vertexCounter * sizePerVertex),
						gl_vertSize, vert_data);
				
				glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 
						safe_cast_assert(GLintptr, indexCounter * sizePerIndex),
						gl_indexSize, indx_data);
				
				vertexCounter +=	vertexCount;
				indexCounter +=		mesh->indexCount;
			}
		}
		
	};
}
	
	DECL std140_Transforms _set_transforms (hm mp model_to_cam, hm mp cam_to_model) {
		std140_Transforms temp;
		
		temp.model_to_cam.set(model_to_cam);
		temp.normal_model_to_cam.set(transpose(cam_to_model.m3())); // cancel out scaling and translation -> only keep rotaton
		
		hm test = cam_to_model * model_to_cam;
		
		return temp;
	}
	DECL void set_transforms (hm mp model_to_cam, hm mp cam_to_model) {
		std140_Transforms temp = _set_transforms(model_to_cam, cam_to_model);
		
		GLOBAL_UBO_WRITE(transforms, &temp);
	}
	DECL void set_transforms_and_mat (hm mp model_to_cam, hm mp cam_to_model, std140_Material const* mat) {
		
		std140_Transforms_Material temp;
		
		temp.transforms = _set_transforms(model_to_cam, cam_to_model);
		
		temp.mat = *mat;
		
		GLOBAL_UBO_WRITE(transforms, &temp);
	}
	DECL void set_transforms_and_solid_color (hm mp model_to_cam, hm mp cam_to_model, v3 vp col) {
		
		std140_Material mat;
		mat.albedo.set(col);
		mat.metallic.set(0);
		mat.roughness.set(0);
		mat.roughness.set(0);
		
		set_transforms_and_mat(model_to_cam, cam_to_model, &mat);
	}
	DECL void set_mat (std140_Material const* mat) {
		
		GLOBAL_UBO_WRITE(mat, mat);
	}
	
//// Textures
	#define TEXTURES_X \
			X0(	MAT_ROUGHNESS_TEX ) \
			\
			X(	GRASS_TEX ) \
			X(	TERRAIN_CUBE_DIFFUSE_TEX ) \
			X(	TERRAIN_SPHERE_DIFFUSE_TEX ) \
			X(	TERRAIN_TREE_DIFFUSE_TEX ) \
			X(	TERRAIN_TREE_CUTS_DIFFUSE_TEX ) \
			X(	TERRAIN_OBELISK_DIFFUSE_TEX ) \
			\
			X(	STRUCTURE_WALLS_TEX ) \
			X(	STRUCTURE_GROUND_TEX ) \
			X(	METAL_GRIPPED_DIFFUSE_TEX ) \
			X(	METAL_RUSTY_02_TEX ) \
			X(	MARBLE_TEX ) \
			X(	WOODEN_BEAM_TEX ) \
			\
			X(	TEX_DIF_BRICK_00 ) \
			X(	TEX_NRM_BRICK_00 ) \
			X(	TEX_DIF_BRICK_01 ) \
			X(	TEX_NRM_BRICK_01 ) \
			\
			X(	NANOSUIT_BODY_DIFF_EMISS_TEX ) \
			X(	NANOSUIT_BODY_SPEC_ROUGH_TEX ) \
			X(	NANOSUIT_BODY_NORM_TEX ) \
			X(	NANOSUIT_NECK_DIFF_TEX ) \
			X(	NANOSUIT_NECK_SPEC_ROUGH_TEX ) \
			X(	NANOSUIT_NECK_NORM_TEX ) \
			X(	NANOSUIT_HELMET_DIFF_EMISS_TEX ) \
			X(	NANOSUIT_HELMET_SPEC_ROUGH_TEX ) \
			X(	NANOSUIT_HELMET_NORM_TEX ) \
			\
			X(	NORM_TEST_00_TEX			) \
			\
			X(	TEX_CERBERUS_ALBEDO			) \
			X(	TEX_CERBERUS_NORMAL			) \
			X(	TEX_CERBERUS_METALLIC		) \
			X(	TEX_CERBERUS_ROUGHNESS		)
	
	#define X0(id)				id=0,
	#define X(id)				id,
	enum textures_e : u32 {
		TEXTURES_X
		
		TEXTURES_COUNT,
		NULL_TEX=(u32)-1,
	};
	DEFINE_ENUM_ITER_OPS(textures_e, u32)
	#undef X0
	#undef X
	
	DECLD constexpr textures_e	FILE_TEXURES_FIRST =		(textures_e)0;
	DECLD constexpr textures_e	FILE_TEXURES_LAST =			(textures_e)(TEXTURES_COUNT -1);
	DECLD constexpr textures_e	FILE_TEXURES_END =			(textures_e)(FILE_TEXURES_LAST +1);
	DECLD constexpr u32			FILE_TEXURES_COUNT =		FILE_TEXURES_END -FILE_TEXURES_FIRST;
	
	#define X0(id)				STRINGIFY(id),
	#define X(id)				STRINGIFY(id),
	DECLD constexpr const char* TEXTURES_ID_STR[TEXTURES_COUNT] = {
		TEXTURES_X
	};
	#undef X0
	#undef X
	
	#undef TEXTURES_X
	
	enum samplers_e : u32 {
		DEFAULT_SAMPLER=0,
		SAMPLER_SHADOW_2D,
		SAMPLER_SHADOW_CUBE,
		CUBEMAP_SAMPLER,
		SAMPLER_RESAMPLE,
		SAMPLER_1D_LUT,
		SAMPLER_2D_LUT,
		SAMPLER_BORDER,
		
		SAMPLERS_COUNT,
	};
	void init_samplers (GLuint* samplers, f32 gl_max_aniso) {
		glGenSamplers(SAMPLERS_COUNT, samplers);
		
		glSamplerParameteri(samplers[DEFAULT_SAMPLER],		GL_TEXTURE_WRAP_S,				GL_REPEAT);
		glSamplerParameteri(samplers[DEFAULT_SAMPLER],		GL_TEXTURE_WRAP_T,				GL_REPEAT);
		glSamplerParameteri(samplers[DEFAULT_SAMPLER],		GL_TEXTURE_MIN_FILTER,			GL_LINEAR_MIPMAP_LINEAR);
		glSamplerParameteri(samplers[DEFAULT_SAMPLER],		GL_TEXTURE_MAG_FILTER,			GL_LINEAR);
		glSamplerParameterf(samplers[DEFAULT_SAMPLER],		GL_TEXTURE_MAX_ANISOTROPY_EXT,	gl_max_aniso);
		
		v4 one = v4(1,0,0,1);
		glSamplerParameteri(samplers[SAMPLER_SHADOW_2D],	GL_TEXTURE_WRAP_S,				GL_CLAMP_TO_BORDER);
		glSamplerParameteri(samplers[SAMPLER_SHADOW_2D],	GL_TEXTURE_WRAP_T,				GL_CLAMP_TO_BORDER);
		glSamplerParameteri(samplers[SAMPLER_SHADOW_2D],	GL_TEXTURE_MIN_FILTER,			GL_LINEAR);
		glSamplerParameteri(samplers[SAMPLER_SHADOW_2D],	GL_TEXTURE_MAG_FILTER,			GL_LINEAR);
		glSamplerParameterf(samplers[SAMPLER_SHADOW_2D],	GL_TEXTURE_MAX_ANISOTROPY_EXT,	gl_max_aniso);
		glSamplerParameterfv(samplers[SAMPLER_SHADOW_2D],	GL_TEXTURE_BORDER_COLOR,		&one.x);
		glSamplerParameteri(samplers[SAMPLER_SHADOW_2D],	GL_TEXTURE_COMPARE_MODE,		GL_COMPARE_REF_TO_TEXTURE);
		glSamplerParameterf(samplers[SAMPLER_SHADOW_2D],	GL_TEXTURE_COMPARE_FUNC,		GL_LEQUAL);
		
		glSamplerParameteri(samplers[SAMPLER_SHADOW_CUBE],	GL_TEXTURE_WRAP_S,				GL_CLAMP_TO_EDGE);
		glSamplerParameteri(samplers[SAMPLER_SHADOW_CUBE],	GL_TEXTURE_WRAP_T,				GL_CLAMP_TO_EDGE);
		glSamplerParameteri(samplers[SAMPLER_SHADOW_CUBE],	GL_TEXTURE_WRAP_R,				GL_CLAMP_TO_EDGE);
		glSamplerParameteri(samplers[SAMPLER_SHADOW_CUBE],	GL_TEXTURE_MIN_FILTER,			GL_LINEAR);
		glSamplerParameteri(samplers[SAMPLER_SHADOW_CUBE],	GL_TEXTURE_MAG_FILTER,			GL_LINEAR);
		glSamplerParameteri(samplers[SAMPLER_SHADOW_CUBE],	GL_TEXTURE_COMPARE_MODE,		GL_COMPARE_REF_TO_TEXTURE);
		glSamplerParameterf(samplers[SAMPLER_SHADOW_CUBE],	GL_TEXTURE_COMPARE_FUNC,		GL_LEQUAL);
		
		glSamplerParameteri(samplers[CUBEMAP_SAMPLER],		GL_TEXTURE_WRAP_S,				GL_CLAMP_TO_EDGE);
		glSamplerParameteri(samplers[CUBEMAP_SAMPLER],		GL_TEXTURE_WRAP_T,				GL_CLAMP_TO_EDGE);
		glSamplerParameteri(samplers[CUBEMAP_SAMPLER],		GL_TEXTURE_WRAP_R,				GL_CLAMP_TO_EDGE);
		glSamplerParameteri(samplers[CUBEMAP_SAMPLER],		GL_TEXTURE_MIN_FILTER,			GL_LINEAR_MIPMAP_LINEAR);
		glSamplerParameteri(samplers[CUBEMAP_SAMPLER],		GL_TEXTURE_MAG_FILTER,			GL_LINEAR);
		glSamplerParameterf(samplers[CUBEMAP_SAMPLER],		GL_TEXTURE_MAX_ANISOTROPY_EXT,	gl_max_aniso);
		
		glSamplerParameteri(samplers[SAMPLER_RESAMPLE],		GL_TEXTURE_WRAP_S,				GL_CLAMP_TO_EDGE);
		glSamplerParameteri(samplers[SAMPLER_RESAMPLE],		GL_TEXTURE_WRAP_T,				GL_CLAMP_TO_EDGE);
		glSamplerParameteri(samplers[SAMPLER_RESAMPLE],		GL_TEXTURE_MIN_FILTER,			GL_LINEAR_MIPMAP_LINEAR);
		glSamplerParameteri(samplers[SAMPLER_RESAMPLE],		GL_TEXTURE_MAG_FILTER,			GL_LINEAR);
		
		glSamplerParameteri(samplers[SAMPLER_1D_LUT],		GL_TEXTURE_WRAP_S,				GL_CLAMP_TO_EDGE);
		glSamplerParameteri(samplers[SAMPLER_1D_LUT],		GL_TEXTURE_MIN_FILTER,			GL_LINEAR);
		glSamplerParameteri(samplers[SAMPLER_1D_LUT],		GL_TEXTURE_MAG_FILTER,			GL_LINEAR);
		
		glSamplerParameteri(samplers[SAMPLER_2D_LUT],		GL_TEXTURE_WRAP_S,				GL_CLAMP_TO_EDGE);
		glSamplerParameteri(samplers[SAMPLER_2D_LUT],		GL_TEXTURE_WRAP_T,				GL_CLAMP_TO_EDGE);
		glSamplerParameteri(samplers[SAMPLER_2D_LUT],		GL_TEXTURE_MIN_FILTER,			GL_LINEAR);
		glSamplerParameteri(samplers[SAMPLER_2D_LUT],		GL_TEXTURE_MAG_FILTER,			GL_LINEAR);
		
		v4 black = v4(0);
		glSamplerParameteri(samplers[SAMPLER_BORDER],		GL_TEXTURE_WRAP_S,				GL_CLAMP_TO_BORDER);
		glSamplerParameteri(samplers[SAMPLER_BORDER],		GL_TEXTURE_WRAP_T,				GL_CLAMP_TO_BORDER);
		glSamplerParameteri(samplers[SAMPLER_BORDER],		GL_TEXTURE_MIN_FILTER,			GL_LINEAR_MIPMAP_LINEAR);
		glSamplerParameteri(samplers[SAMPLER_BORDER],		GL_TEXTURE_MAG_FILTER,			GL_LINEAR);
		glSamplerParameterfv(samplers[SAMPLER_BORDER],		GL_TEXTURE_BORDER_COLOR,		&black.x);
		
	}
	
#include "textures_file.hpp"
	
//// Entities
	//////////////////// These will go away after completely switching to the new entity based editor features
	struct Generic_Movable {
		char const*	name;
		v3			pos_world;
		quat		ori_world;
	};
	
////
	bool load_humus_cubemap (lstr cr filename, Stack* stk) {
		
		u32	w, h;
		for (ui face=0; face<6; ++face) { 
			u8*	stb_img;
			{
				static constexpr lstr FACES[6] = {
					"/posx.jpg",
					"/negx.jpg",
					"/posz.jpg",
					"/negz.jpg",
					"/posy.jpg",
					"/negy.jpg",
				};
				
				DEFER_POP(stk);
				lstr filepath = str::append_term(stk, ENV_MAPS_HUMUS_DIR, filename, FACES[face]);
				
				{
					Mem_Block file;
					if (platform::read_whole_file_onto(stk, filepath.str, 0, &file)) {
						return false;
					}
					
					int sw, sh, comp=3;
					stb_img = stb::stbi_load_from_memory((uchar*)file.ptr, safe_cast_assert(int, file.size), &sw, &sh, &comp, comp);
					
					if (sw != sh) {
						warning("Warning: humus cubemap has non-square resolution!\n");
					}
					
					u32 uw = (u32)sw;
					u32 uh = (u32)sh;
					
					if (face != 0) {
						if (uw != w || uh != h) {
							warning("Warning: Resolution of humus cubemap face % does not match resolution of first face (face % is %x% px but face % is %x% px)!\n",
									FACES[face], FACES[face], uw,uh, FACES[0], w,h);
						}
					}
					
					w = uw;
					h = uh;
				}
			}
			
			u32 stride = align_up<u32>(w * 3, 4);
			u8* rotated = (u8*)malloc((uptr)h * stride);
			
			{ // Rotate humus cubemap faces to our z-up coordinates
				
				u8*	out_data = rotated;
				
				u8*	in_pix_data = stb_img;
				u8*	out_pix_data = out_data;
				
				enum rotate_e {
					DONT=0, CCW_90, _180, CW_90
				};
				static constexpr rotate_e HUMUS_FACE_ROTATION[6] = {
					CCW_90,
					CW_90,
					DONT,
					_180,
					DONT,
					_180,
				};
				
				auto rot = HUMUS_FACE_ROTATION[face];
				
				auto get_in = [&] (u32 x, u32 y) -> u8* {
					return in_pix_data +(y * w * 3) +(x * 3);
				};
				
				switch (rot) {
					
					case CCW_90: {
						for (u32 j=0; j<h; ++j) {
							
							u8* out_pix = out_pix_data;
							
							for (u32 i=0; i<w; ++i) {
								u8* in_pix = get_in( j, w -1 -i );
								
								out_pix[0] = in_pix[0];
								out_pix[1] = in_pix[1];
								out_pix[2] = in_pix[2];
								
								out_pix += 3;
							}
							
							out_pix_data += stride;
							
							while (out_pix != out_pix_data) {
								*out_pix++ = 0;
							}
						}
					} break;
					
					case CW_90: {
						for (u32 j=0; j<h; ++j) {
							
							u8* out_pix = out_pix_data;
							
							for (u32 i=0; i<w; ++i) {
								u8* in_pix = get_in( h -1 -j, i );
								
								out_pix[0] = in_pix[0];
								out_pix[1] = in_pix[1];
								out_pix[2] = in_pix[2];
								
								out_pix += 3;
							}
							
							out_pix_data += stride;
							
							while (out_pix != out_pix_data) {
								*out_pix++ = 0;
							}
						}
					} break;
					
					case _180: {
						for (u32 j=0; j<h; ++j) {
							
							u8* out_pix = out_pix_data;
							
							for (u32 i=0; i<w; ++i) {
								u8* in_pix = get_in( h -1 -i, w -1 -j );
								
								out_pix[0] = in_pix[0];
								out_pix[1] = in_pix[1];
								out_pix[2] = in_pix[2];
								
								out_pix += 3;
							}
							
							out_pix_data += stride;
							
							while (out_pix != out_pix_data) {
								*out_pix++ = 0;
							}
						}
					} break;
					
					case DONT: {
						for (u32 j=0; j<h; ++j) {
							
							u8* out_pix = out_pix_data;
							
							for (u32 i=0; i<w; ++i) {
								u8* in_pix = get_in( i, j );
								
								out_pix[0] = in_pix[0];
								out_pix[1] = in_pix[1];
								out_pix[2] = in_pix[2];
								
								out_pix += 3;
							}
							
							out_pix_data += stride;
							
							while (out_pix != out_pix_data) {
								*out_pix++ = 0;
							}
						}
					} break;
				}
				
			}
			
			free(stb_img);
			
			{
				GLsizei gl_w = safe_cast_assert(GLsizei, w);
				GLsizei gl_h = safe_cast_assert(GLsizei, h);
				
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X +face, 0, GL_SRGB8, gl_w,gl_h, 0,
						GL_RGB, GL_UNSIGNED_BYTE, rotated);
			}
			
			free(rotated);
			
		}
		
		return true;
	}
	
////
	DECL void set_vsync (s32 swap_interval) {
		
		print("Vsync % (%).\n", swap_interval ? "enabled":"disabled", swap_interval);
		
		auto ret = wglSwapIntervalEXT(swap_interval);
		assert(ret != FALSE);
	}
	
//// Materials
	enum materials_e : u32 {
		MAT_IDENTITY			=0,
		
		MAT_WHITENESS			,
		MAT_TERRAIN				,
		MAT_GLASS				,
		MAT_PLASTIC				,
		MAT_SHINY_PLATINUM		,
		MAT_GRASS				,
		MAT_RUSTY_METAL			,
		MAT_BLOTCHY_METAL		,
		MAT_GRIPPED_METAL		,
		MAT_DIRT				,
		MAT_TREE_BARK			,
		MAT_TREE_CUTS			,
		MAT_TREE_BLOSSOMS		,
		MAT_OBELISK				,
		MAT_MARBLE				,
		MAT_WOODEN_BEAM			,
		MAT_WOODEN_BEAM_CUTS	,
		MAT_LIGHTBULB			,
		
		MAT_SHOW_PLASTIC		,
		MAT_SHOW_GLASS			,
		MAT_SHOW_PLASIC_H		,
		MAT_SHOW_RUBY			,
		MAT_SHOW_DIAMOND		,
		MAT_SHOW_IRON			,
		MAT_SHOW_COPPER			,
		MAT_SHOW_GOLD			,
		MAT_SHOW_ALU			,
		MAT_SHOW_SILVER			,
		
		MAT_COUNT
	};
	DEFINE_ENUM_ITER_OPS(materials_e, u32)
	
	DECLD constexpr materials_e	MAT_SHOW_FIRST =			MAT_SHOW_PLASTIC;
	DECLD constexpr materials_e	MAT_SHOW_END =				(materials_e)(MAT_SHOW_SILVER +1);
	
////
	struct Line3 {
		v3	a, b;
	};
	
	union Box_Edges {
		struct {
			Line3	lll_hll;
			Line3	hll_hhl;
			Line3	hhl_lhl;
			Line3	lhl_lll;
			
			Line3	lll_llh;
			Line3	hll_hlh;
			Line3	hhl_hhh;
			Line3	lhl_lhh;
			
			Line3	llh_hlh;
			Line3	hlh_hhh;
			Line3	hhh_lhh;
			Line3	lhh_llh;
		};
		Line3		arr[12];
	};
	union Box_Corners { // l=low h=high
		struct {
			v3		lll;
			v3		hll;
			v3		hhl;
			v3		lhl;
			
			v3		llh;
			v3		hlh;
			v3		hhh;
			v3		lhh;
		};
		v3			arr[8];
		
		Box_Edges box_edges () const {
			Box_Edges ret;
			ret.lll_hll = {lll, hll};
			ret.hll_hhl = {hll, hhl};
			ret.hhl_lhl = {hhl, lhl};
			ret.lhl_lll = {lhl, lll};
			
			ret.lll_llh = {lll, llh};
			ret.hll_hlh = {hll, hlh};
			ret.hhl_hhh = {hhl, hhh};
			ret.lhl_lhh = {lhl, lhh};
			
			ret.llh_hlh = {llh, hlh};
			ret.hlh_hhh = {hlh, hhh};
			ret.hhh_lhh = {hhh, lhh};
			ret.lhh_llh = {lhh, llh};
			return ret;
		}
	};
	
}
	
	engine::Box_Corners operator* (m3 mp m, engine::Box_Corners cr b) {
		engine::Box_Corners ret;
		for (ui i=0; i<8; ++i) {
			ret.arr[i] = m * b.arr[i];
		}
		return ret;
	}
	engine::Box_Corners operator* (hm mp m, engine::Box_Corners cr b) {
		engine::Box_Corners ret;
		for (ui i=0; i<8; ++i) {
			ret.arr[i] = (m * hv(b.arr[i])).xyz();
		}
		return ret;
	}
	
	engine::Box_Corners operator+ (engine::Box_Corners cr b, v3 vp v) {
		engine::Box_Corners ret;
		for (ui i=0; i<8; ++i) {
			ret.arr[i] = b.arr[i] +v;
		}
		return ret;
	}
	
namespace engine {
	
	union AABB {
		// l -> low
		// h -> high
		// low <= high
		struct {
			f32	xl, xh;
			f32	yl, yh;
			f32	zl, zh;
		};
		f32	arr[6];
		
		static constexpr AABB inf () { // initialize to infinite values so that doing MIN() & MAX() in a loop will get us the AABB
			return { {	+fp::inf<f32>(), -fp::inf<f32>(),
						+fp::inf<f32>(), -fp::inf<f32>(),
						+fp::inf<f32>(), -fp::inf<f32>() } };
		}
		bool is_inf () const {
			bool ret = !std::isfinite(xl);
			#if DEBUG
			if (ret) {
				for (ui i=0; i<6; ++i) {
					assert(!std::isnan(arr[i]) && !std::isfinite(arr[i]));
				}
			}
			#endif
			return ret;
		}
		
		void minmax (v3 vp v) {
			xl = MIN(xl, v.x);
			xh = MAX(xh, v.x);
			yl = MIN(yl, v.y);
			yh = MAX(yh, v.y);
			zl = MIN(zl, v.z);
			zh = MAX(zh, v.z);
		}
		void minmax (Box_Corners cr box) {
			for (u32 i=0; i<8; ++i) {
				minmax(box.arr[i]);
			}
		}
		void minmax (AABB cr aabb) {
			minmax(aabb.box_corners());
		}
		
		static AABB from_obb (Box_Corners cr obb) {
			AABB ret = inf();
			ret.minmax(obb);
			return ret;
		}
		
		Box_Corners box_corners () const {
			Box_Corners ret;
			ret.lll =	v3(xl,yl,zl);
			ret.hll =	v3(xh,yl,zl);
			ret.hhl =	v3(xh,yh,zl);
			ret.lhl =	v3(xl,yh,zl);
			
			ret.llh	=	v3(xl,yl,zh);
			ret.hlh	=	v3(xh,yl,zh);
			ret.hhh	=	v3(xh,yh,zh);
			ret.lhh	=	v3(xl,yh,zh);
			return ret;
		}
		
		Box_Edges box_edges () const {
			return box_corners().box_edges();
		}
		
	};
	
	
////
	DECLD bool					reloaded_vars;
	
	
	DECLD GLuint				shaders[SHAD_COUNT];
	
	DECLD GLuint				VBOs[VBO_COUNT];
	DECLD GLuint				VAOs[VAO_COUNT];
	
	DECLD Mesh_Ref				meshes[MESHES_COUNT];
	DECLD AABB					meshes_aabb[MESHES_COUNT];
	
	DECL void draw_solid_color (mesh_id_e mesh_id, hm mp model_to_camera, hm mp camera_to_model, v3 col) {
		auto& mesh = meshes[mesh_id];
		
		set_transforms_and_solid_color(model_to_camera, camera_to_model, col);
		
		glDrawElementsBaseVertex(GL_TRIANGLES, mesh.indx_count, GL_UNSIGNED_SHORT,
				mesh.indx_offsets, mesh.base_vertecies);
	}
	DECL void draw (mesh_id_e mesh_id, hm mp model_to_camera, hm mp camera_to_model, std140_Material const* mat) {
		auto& mesh = meshes[mesh_id];
		
		set_transforms_and_mat(model_to_camera, camera_to_model, mat);
		
		glDrawElementsBaseVertex(GL_TRIANGLES, mesh.indx_count, GL_UNSIGNED_SHORT,
				mesh.indx_offsets, mesh.base_vertecies);
	}
	DECL void draw_keep_set (mesh_id_e mesh_id) {
		auto& mesh = meshes[mesh_id];
		
		glDrawElementsBaseVertex(GL_TRIANGLES, mesh.indx_count, GL_UNSIGNED_SHORT,
				mesh.indx_offsets, mesh.base_vertecies);
	}
	DECL void draw_keep_mat (mesh_id_e mesh_id, hm mp model_to_camera, hm mp camera_to_model) {
		auto& mesh = meshes[mesh_id];
		
		set_transforms(model_to_camera, camera_to_model);
		
		glDrawElementsBaseVertex(GL_TRIANGLES, mesh.indx_count, GL_UNSIGNED_SHORT,
				mesh.indx_offsets, mesh.base_vertecies);
	}
	
	hm transl_ori (Generic_Movable cr ent) {
		return translate_h(ent.pos_world) * hm::ident().m3( conv_to_m3(ent.ori_world) );
	}
	hm transl_ori  (hm mp mat, Generic_Movable cr ent) {
		return mat * transl_ori(ent);
	}
	hm transl_ori_scale (hm mp mat, Generic_Movable cr ent, v3 vp scale) {
		return transl_ori(mat, ent) * scale_h(scale);
	}
	
	hm inverse_transl_ori (Generic_Movable cr ent) { // direct inverse calculation from entity
		return hm::ident().m3( conv_to_m3(inverse(ent.ori_world)) ) * translate_h(-ent.pos_world);
	}
	
	meshes_file_n::Meshes_File	meshes_file = {}; // init to null
	
	AABB calc_AABB_model (mesh_id_e id) {
		
		// AABB returned is in the space between to_world & from_model
		// gets deplayed in world space
		
		byte*		vertices;
		GLushort*	indices;
		
		u32			vertex_stride;
		
		u32			vertex_count;
		u32			index_count;
		{
			byte* file_data = (byte*)meshes_file.header;
			
			auto mesh = meshes_file.query_mesh(mesh_names[id]);
			assert(mesh);
			
			vertex_count = safe_cast_assert(u32, mesh->vertexCount.glushort);
			assert(vertex_count > 0);
			
			index_count = safe_cast_assert(u32, mesh->indexCount);
			assert(index_count > 0);
			
			switch (mesh->dataFormat) {
				using namespace meshes_file_n;
				
				case (INTERLEAVED|POS_XYZ|NORM_XYZ|INDEX_USHORT): {
					struct Vertex {
						v3	pos;
						v3	norm;
					};
					vertex_stride = sizeof(Vertex);
				} break;
				
				case (INTERLEAVED|POS_XYZ|NORM_XYZ|UV_UV|INDEX_USHORT): {
					struct Vertex {
						v3	pos;
						v3	norm;
						v2	uv;
					};
					vertex_stride = sizeof(Vertex);
				} break;
				
				case (INTERLEAVED|POS_XYZ|NORM_XYZ|UV_UV|TANG_XYZW|INDEX_USHORT): {
					struct Vertex {
						v3	pos;
						v3	norm;
						v4	tang;
						v2	uv;
					};
					vertex_stride = sizeof(Vertex);
				} break; 
				
				default: assert(false); vertex_stride = 0; // shut up compiler
			}
			
			u32 vert_size = vertex_count * vertex_stride;
			u32 index_size = index_count * sizeof(GLushort);
			
			assert((mesh->dataOffset +vert_size) <= meshes_file.file_size);
			assert((mesh->dataOffset +vert_size +index_size) <= meshes_file.file_size);
			
			vertices = (byte*)align_up(file_data +mesh->dataOffset, sizeof(GLfloat));
			indices = (GLushort*)align_up(((byte*)vertices) +vert_size, sizeof(GLushort));
			
		}
		
		AABB ret = AABB::inf();
		
		for (u32 i=0; i<vertex_count; ++i) {
			v3 pos = *(v3*)( vertices +vertex_stride * i );
			ret.minmax(pos);
		}
		
		return ret;
	}
	
	DECL void reload_meshes () { // Loading meshes from disk to GPU driver
		using namespace meshes_file_n;
		
		PROFILE_SCOPED(THR_ENGINE, "reload_meshes");
		
		glBindVertexArray(0); // Unbind VAO so that we don't mess with its GL_ELEMENT_ARRAY_BUFFER binding
		
		meshes_file.reload();
		
		//
		glBindBuffer(GL_ARRAY_BUFFER,			VBOs[NO_UV_ARR_BUF]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,	VBOs[NO_UV_INDX_BUF]);
		
		meshes_file.reload_meshes(INTERLEAVED|POS_XYZ|NORM_XYZ|INDEX_USHORT, NOUV_MSH_FIRST, NOUV_MSH_COUNT, meshes);
		
		//
		glBindBuffer(GL_ARRAY_BUFFER,			VBOs[UV_ARR_BUF]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,	VBOs[UV_INDX_BUF]);
		
		meshes_file.reload_meshes(INTERLEAVED|POS_XYZ|NORM_XYZ|UV_UV|INDEX_USHORT, UV_MSH_FIRST, UV_MSH_COUNT, meshes);
		
		//
		glBindBuffer(GL_ARRAY_BUFFER,			VBOs[UV_TANG_ARR_BUF]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,	VBOs[UV_TANG_INDX_BUF]);
		
		meshes_file.reload_meshes(INTERLEAVED|POS_XYZ|NORM_XYZ|TANG_XYZW|UV_UV|INDEX_USHORT, UV_TANG_MSH_FIRST, UV_TANG_MSH_COUNT, meshes);
		
		for (mesh_id_e id=(mesh_id_e)0; id<MESHES_COUNT; ++id) {
			meshes_aabb[id] = calc_AABB_model(id);
		}
	}
	
////
	typedef input::State		Inp;
	typedef input::Sync_Input	SInp;
	
	DECLD Inp	inp;
	DECLD SInp	sinp =	{}; // init engine_res to zero
	
	DECLD hm	_world_to_cam;	// try not to use this because you can easily make a 1-frame-latency error,
								//  and explicit arguments are preferred
								//  only use when always passing it into a function gets excessive, or for debug calculations
	
	struct Controls {
		
		v2					cam_mouselook_sens;
		f32					cam_roll_vel;
		
		// positional_units/second
		v3					cam_translate_vel;
		v3					cam_translate_fast_mult;
		
		f32					cam_fov_control_vel;
		f32					cam_fov_control_mw_vel;
	};
	
	struct Camera {
		// 
		v3					pos_world;
		// These are like the parameters in an altazimuth camera mount
		//  Alimuth is applied first, usually by mouse x and determines the direction on the horizon the camera points to
		//  Elevation is applied second, usually by mouse y and can only rotate the camera towards the ground/sky.
		//   Usually capped to some maximum angle up and down to prevent the camera from flipping over, which feels very wierd when with mouselook
		//  Roll is applied last and can only roll the camera around the point it is already looking at (determined by azimuth and elevation)
		//   Usually unused, can be used lightly during camerashakes, if desired.
		// TODO: maybe document what the values mean (ex. azim 0 deg is +y, elev goes from 0 deg to 180 deg)
		v3					aer;
		// Camera base orientation: this determines how the camera 'tripod' is oriented, ie. 'up' and 'down' is
		//  Zero-g games for ex. would have to make us of this
		quat				base_ori;
		
		f32					vfov;
		f32					clip_near;
		f32					clip_far;
		
		m4					cam_to_clip;
		v2					frust_scale;
		
		DECLM void update_matrix (v2 vp window_aspect_ratio) {
			
			print("Camera: vertical FOV: % deg (h: % deg)\n", to_deg(vfov), to_deg(vfov * window_aspect_ratio.x));
			
			{
				frust_scale.y = fp::tan(vfov * 0.5f);
				frust_scale.x = frust_scale.y * window_aspect_ratio.x;
			}
			
			{
				f32 temp = clip_near -clip_far;
				
				v2 frust_scale_inv = v2(1.0f) / frust_scale;
				
				f32 x = frust_scale_inv.x;
				f32 y = frust_scale_inv.y;
				f32 a = (clip_far +clip_near) / temp;
				f32 b = (2.0f * clip_far * clip_near) / temp;
				
				cam_to_clip = m4::row(	x, 0, 0, 0,
										0, y, 0, 0,
										0, 0, a, b,
										0, 0, -1, 0 );
			}
			
		}
		
	};
	
	struct Textures {
		
		GLuint						gl_refs[TEXTURES_COUNT];
		
		GLuint						samplers[SAMPLERS_COUNT];
		
		f32							gl_max_aniso;
		
		HANDLE						h_tex_file;
		tex_file::Header const*		header;
		tex_file::Tex_Entry const*	texs;
		tex_file::Mip_Entry const*	mips;
		
		textures_e*					file_texs;	// source code texture ids for each texture entry in texture file
												//  to be able to map mipmap load entries to source code textures
		
		struct Mip_State {
			u32	cur_mip;
			u32	mip_row;
		};
		
		Mip_State					loading_s;
		
		// Instead of doing complicated async disk load and syncronize to main thread potentially using PBOs
		//  we simply 'allocate' some time each frame to load chunks from disk and upload them syncronously and hope we don't stall
		// You can control the budget each frame with these variables
		// TODO: experiment and document
		#if NOTEBOOK
		static constexpr f64		LOAD_FRAME_TIME_BUDGET =		milli(1.5); // Stuttering happens because GPU stalls which we can't capture with CPU timing, so this value probably has little use in this regard
		static constexpr u32		LOAD_FRAME_CHUNKS_BUDGET =		8;
		#else
		static constexpr f64		LOAD_FRAME_TIME_BUDGET =		milli(3.0);
		static constexpr f64		LOAD_FRAME_CHUNKS_BUDGET =		16;
		#endif
		static constexpr uptr		LOAD_CHUNK_SIZE =				kibi<u32>(32*8); // This needs to fit at least 1 line of the largest texture for get_next_blob() to work
		
		byte*						load_buf;
		
		u32							chunk_i =						0;
		bool						finished_loading =				false;
		
		char const*					names[TEXTURES_COUNT];
		
		DECLM void drop_os_file_cache (char const* filename) {
			
			PROFILE_SCOPED(THR_ENGINE, "drop_os_file_cache");
			
			auto handle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
					FILE_FLAG_NO_BUFFERING, NULL);
			assert(handle != INVALID_HANDLE_VALUE, "CreateFile() failed for % (err: %)\n",
					filename, GetLastError());
			
			win32::close_handle(handle);
		}
		
		DECLM void init () {
			
			glGenTextures(TEXTURES_COUNT, gl_refs);
			
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gl_max_aniso);
			
			init_samplers(samplers, gl_max_aniso);
			
			glActiveTexture(GL_TEXTURE0);
			
			#if 0
			{
				glBindTexture(GL_TEXTURE_2D, WHITENESS_TEXTURE);
				
				u8 col[4] = { 255, 255, 255, 255 };
				glTexImage2D(GL_TEXTURE_2D, (GLint)0, GL_SRGB8_ALPHA8, 1, 1, (GLint)0, GL_RGBA, GL_UNSIGNED_BYTE, col);
				
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, (GLint)0);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, (GLint)0);
			}
			#endif
			
			{
				using namespace tex_file;
				
				load_buf = texture_load_stk.pushArrAlign<64, byte>(LOAD_CHUNK_SIZE);
				
				{
					#if 0
					{
						drop_os_file_cache(TEXTURES_FILENAME);
						
						print_records(THR_ENGINE);
					}
					#endif
					
					h_tex_file = win32::open_existing_file_rd_assert(TEXTURES_FILENAME);
					
					u64 file_size = win32::get_file_size(h_tex_file);
					
					u64 header_read_size = align_up<u64>(sizeof(Header), TEX_ENTRY_ALIGN);
					assert(file_size >= header_read_size);
					
					{
						auto* data = texture_load_stk.pushArrAlign<FILE_ALIGN, byte>(header_read_size);
						
						win32::read_file_assert(h_tex_file, data, header_read_size);
						header = (Header*)data;
					}
					
					assert(header->id.i == FILE_ID.i);
					assert(header->file_size == file_size);
					
					assert(header->data_offs >= header_read_size);
					{
						DWORD rest_read_size = safe_cast_assert(DWORD, header->data_offs -header_read_size);
						
						auto* data = texture_load_stk.pushArrNoAlign<byte>(rest_read_size);
						
						win32::read_file_assert(h_tex_file, data, rest_read_size);
						
						texs = (Tex_Entry*)data;
						
						data += header->tex_count * sizeof(Tex_Entry);
						data = align_up(data, MIP_ENTRY_ALIGN);
						
						mips = (Mip_Entry*)data;
						
						data += header->mips_count * sizeof(Mip_Entry);
						
						assert(header->data_offs >= ptr_sub((byte*)header, data),
								"% %", header->data_offs, ptr_sub((byte*)header, data));
					}
				}
				
				file_texs = texture_load_stk.pushArr<textures_e>(header->tex_count);
				for (u32 i=0; i<header->tex_count; ++i) {
					file_texs[i] = NULL_TEX;
				}
				
				for (textures_e tex=FILE_TEXURES_FIRST; tex<FILE_TEXURES_END; ++tex) {
					PROFILE_SCOPED(THR_ENGINE, "tex", (u32)tex);
					
					char const* name = names[tex];
					if (name == nullptr) {
						continue;
					}
					
					u32 found;
					for (u32 i=0;; ++i) {
						assert(i != header->tex_count, "% not found.", name);
						
						u32 offs = texs[i].name_offs;
						char const* i_name = (char const*)header +offs;
						
						if (str::comp(i_name, name)) {
							found = i;
							break;
						}
					}
					
					file_texs[found] = tex;
					auto& t = texs[found];
					
					//print(name, nl);
					
					//
					using namespace tex_type;
					
					auto res = get_all_fields(t.type);
					
					glBindTexture(res.gl_targ, gl_refs[tex]);
					
					GLint gl_mip_count =	safe_cast_assert(GLint, t.mip_count);
					GLsizei gl_width =		safe_cast_assert(GLsizei, t.width);
					GLsizei gl_height =		safe_cast_assert(GLsizei, t.height);
					
					GLenum gl_face = res.gl_targ;
					ui face_end = 1;
					
					if (gl_face == GL_TEXTURE_CUBE_MAP) {
						gl_face = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
						face_end = 6;
					}
					
					for (GLint gl_mip_level=0;;) {
						PROFILE_SCOPED(THR_ENGINE, "mip", (u32)gl_mip_level);
						
						{
							//PROFILE_SCOPED(THR_ENGINE, "glTexImage2D_faces", 0);
							
							for (ui face=0; face<face_end; ++face) {
								//PROFILE_SCOPED(THR_ENGINE, "glTexImage2D", 0);
								
								glTexImage2D(gl_face +face, gl_mip_level, res.gl_colors, gl_width, gl_height, 0,
										res.gl_channels, GL_UNSIGNED_BYTE, NULL);
							}
						}
						
						//print("  ", gl_mip_level, " ", gl_width, "x", gl_height, nl);
						
						if (++gl_mip_level == gl_mip_count) {
							break;
						}
						assert(gl_width > 1 || gl_height > 1);
						gl_width = gl_width == 1 ? 1 : gl_width / 2;
						gl_height = gl_height == 1 ? 1 : gl_height / 2;
					}
					
					glTexParameteri(res.gl_targ, GL_TEXTURE_MAX_LEVEL, gl_mip_count -1);
					
				}
				
				win32::set_filepointer(h_tex_file, header->data_offs);
				
			}
		}
		
		struct Blob_Info {
			u32						size;
			u32						tex_indx;
			u32						mip_level;
			u32						stride;
			u32						row_count;
			u32						y;
			u32						w;
			u32						h;
			tex_type::Type_Fields	type;
			bool					mip_done;
		};
		
		DECLM Blob_Info get_next_blob (Mip_State* mip_s, u32 buf_cur) {
			using namespace tex_type;
			using namespace tex_file;
			
			Blob_Info ret;
			
			auto&	mip =		mips[mip_s->cur_mip];
			auto&	tex =		texs[mip.tex_indx];
			auto*	tex_name =	(char const*)( (u8 const*)header +tex.name_offs );
			
			ret.tex_indx = mip.tex_indx;
			ret.mip_level = mip.mip_level;
			ret.y = mip_s->mip_row;
			
			ret.w = tex.width;
			ret.h = tex.height;
			{
				u32 temp = (u32)1 << ret.mip_level;
				ret.w = ret.w < temp ? 1 : ret.w >> ret.mip_level;
				ret.h = ret.h < temp ? 1 : ret.h >> ret.mip_level;
			}
			
			ret.type = get_all_fields(tex.type);
			
			ui faces = tex.type & T_CUBEMAP ? 6 : 1;
			
			ret.stride = calc_stride(ret.type.channels, ret.w);
			
			u32 effective_h = faces * ret.h;
			
			u32 effective_h_remain = effective_h -mip_s->mip_row;
			
			u32 buf_size_remain = LOAD_CHUNK_SIZE -buf_cur;
			
			u32 row_fit = buf_size_remain / ret.stride;
			
			ret.row_count = MIN(row_fit, effective_h_remain);
			u32 new_mip_row = mip_s->mip_row +ret.row_count;
			
			ret.size = ret.row_count * ret.stride;
			
			if (row_fit == 0) {
				if (ret.stride > LOAD_CHUNK_SIZE) {
					assert(false, "LOAD_CHUNK_SIZE too small for % %x%!", tex_name, ret.w, ret.h);
				}
				
				return ret;
			}
			
			#if 0
			print(
					"    %>% (%)    % %x%    %>% (%)\n",
					buf_cur, buf_cur +ret.size, ret.size,
					tex_name, ret.w, ret.h,
					mip_s->mip_row, new_mip_row, ret.row_count);
			#endif
			
			ret.mip_done = new_mip_row == effective_h;
			if (ret.mip_done) {
				new_mip_row = 0;
				++mip_s->cur_mip;
			}
			
			mip_s->mip_row = new_mip_row;
			
			return ret;
		}
		
		DECLM void incremental_texture_load () {
			using namespace tex_type;
			
			if (finished_loading) {
				return;
			}
			
			PROFILE_SCOPED(THR_ENGINE, "incremental_texture_load");
			
			if (loading_s.cur_mip == header->mips_count) {
				finished_loading = true;
				print("Finished loading textures.\n");
				return;
			}
			
			u64 qpc_begin = time::QPC::get_time();
			PROFILE_BEGIN_M(THR_ENGINE, qpc_begin, "chunk load", chunk_i);
			
			glActiveTexture(GL_TEXTURE0);
			
			//u64 dbg_cur = mips[loading_s.cur_mip].offs;
			
			u32 chunk_loop_i = 0;
			while (chunk_loop_i < LOAD_FRAME_CHUNKS_BUDGET && loading_s.cur_mip < header->mips_count) {
				
				Mip_State upload_s = loading_s;
				{
					u32 read_size = 0;
					do {
						
						Blob_Info blob = get_next_blob(&loading_s, read_size);
						if (blob.size == 0) {
							break;
						}
						
						read_size += blob.size;
						if (read_size == LOAD_CHUNK_SIZE) {
							break;
						}
					} while (loading_s.cur_mip < header->mips_count);
					
					PROFILE_SCOPED(THR_ENGINE, "read_file");
					win32::read_file(h_tex_file, load_buf, read_size);
				}
				
				{
					u32 buf_cur = 0;
					do {
						
						#if 0
						u64 offs = mips[upload_s.cur_mip].offs;
						{
							auto&	mip =		mips[upload_s.cur_mip];
							auto&	tex =		texs[mip.tex_indx];
							
							u32 w = tex.width;
							{
								u32 temp = (u32)1 << mip.mip_level;
								w = w < temp ? 1 : w >> mip.mip_level;
							}
							
							auto res = get_all_fields(tex.type);
							
							u32 stride = calc_stride(res.channels, w);
							
							offs += upload_s.mip_row * stride;
						}
						#endif
						
						Blob_Info blob = get_next_blob(&upload_s, buf_cur);
						if (blob.size == 0) {
							break;
						}
						
						#if 0
						assert(offs == dbg_cur, "offs % dbg_cur %", offs, dbg_cur);
						dbg_cur += blob.stride * blob.row_count;
						#endif
						
						auto tex = file_texs[blob.tex_indx];
						if (tex != NULL_TEX) {	// Skip textures that we don't have in our source code
												// not preventing loading textures from disk that we don't actually use makes the code simpler
							PROFILE_SCOPED(THR_ENGINE, "tex_upload");
							
							glBindTexture(blob.type.gl_targ, gl_refs[tex]);
							
							GLenum gl_face = blob.type.gl_targ;
							if (gl_face == GL_TEXTURE_CUBE_MAP) {
								gl_face = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
							}
							
							u64 face_offs = 0;
							
							u32 face = blob.y / blob.h;
							u32 y = blob.y % blob.h;
							u32 h;
							if (y +blob.row_count <= blob.h) {
								h = blob.row_count;
							} else {
								h = blob.h -y;
							}
							
							GLint gl_mip_level = (GLint)blob.mip_level;
							glTexParameteri(blob.type.gl_targ, GL_TEXTURE_BASE_LEVEL, 0);
							
							do { // Loop over faces in cubemap case
								PROFILE_SCOPED(THR_ENGINE, "glTexSubImage2D");
								
								GLint gl_y = (GLint)y;
								GLsizei gl_w = (GLsizei)blob.w;
								GLsizei gl_h = (GLsizei)h;
								
								glTexSubImage2D(gl_face +face, gl_mip_level,
										0, gl_y, gl_w, gl_h,
										blob.type.gl_channels, GL_UNSIGNED_BYTE, &load_buf[buf_cur +face_offs]);
								
								blob.row_count -=	h;
								face_offs +=		(u64)h * (u64)blob.stride;
								++face;
								y =					0;
								h = MIN(blob.row_count, blob.h);
								
							} while (blob.row_count > 0);
							
							if (blob.mip_done) {
								glTexParameteri(blob.type.gl_targ, GL_TEXTURE_BASE_LEVEL, gl_mip_level);
							} else {
								glTexParameteri(blob.type.gl_targ, GL_TEXTURE_BASE_LEVEL, gl_mip_level +1);
							}
						}
						
						buf_cur += blob.size;
						if (buf_cur == LOAD_CHUNK_SIZE) {
							break;
						}
					} while (upload_s.cur_mip < header->mips_count);
				}
				
				++chunk_i;
				
				u64 qpc_now = time::QPC::get_time();
				
				f64 fdiff = (f64)(qpc_now -qpc_begin) * time::QPC::inv_freq;
				if ( fdiff >= LOAD_FRAME_TIME_BUDGET && loading_s.cur_mip > header->last_1px_mip ) {
					break;
				}
				
				if (++chunk_loop_i == LOAD_FRAME_CHUNKS_BUDGET) {
					break;
				}
				if (loading_s.cur_mip == header->mips_count) {
					break;
				}
				
				PROFILE_STEP_M(THR_ENGINE, qpc_now, "chunk load", chunk_i);
				
			}
			
			PROFILE_END(THR_ENGINE, "chunk load", chunk_i -1);
			
		}
		
		DECLM void set_texture (GLenum gl_targ, textures_e texture) {
			
			glBindTexture(gl_targ, gl_refs[texture]);
		}
		
	};
	
	Textures tex;
	
	DECLD GLuint dbg_tex_0 = 0;
	DECLD GLuint dbg_tex_1 = 0;
	
	#define DISABLE_POST 0
	
	struct Passes {
		
		resolution_v			render_res;
		s32						render_res_mips;
		
		u32						shadow_res;
		
		GLuint					shadow_fbo;
		GLuint					shadow_lights_tex[MAX_LIGHTS_COUNT];
		
		GLuint					main_fbo;
		GLuint					main_lumi_tex;
		GLuint					main_depth_tex;
		
		GLuint					convolve_fbo;
		GLuint					convolved_tex;
		
		static constexpr ui		BLOOM_GAUSS_LUT_RES =				128;
		
		GLuint					bloom_fbo;
		GLuint					bloom_gauss_LUT_tex;
		
		static constexpr ui		BLOOM_PASSES =						3;
		GLuint					bloom_tex			[BLOOM_PASSES][2];
		f32						bloom_size			[BLOOM_PASSES];
		u32						bloom_res_scale		[BLOOM_PASSES];
		f32						bloom_coeff			[BLOOM_PASSES];
		
		GLuint					dbg_line_fbo;
		GLuint					dbg_line_tex;
		GLuint					dbg_line_depth_renderbuf;
		
		void init () {
			PROFILE_SCOPED(THR_ENGINE, "passes_init");
			
			glGenFramebuffers(1,			&shadow_fbo);
			glGenTextures(MAX_LIGHTS_COUNT, shadow_lights_tex);
			
			glGenFramebuffers(1,			&main_fbo);
			glGenTextures(1,				&main_lumi_tex);
			glGenTextures(1,				&main_depth_tex);
			
			glGenFramebuffers(1,			&convolve_fbo);
			glGenTextures(1,				&convolved_tex);
			
			glGenFramebuffers(1,			&bloom_fbo);
			glGenTextures(1,				&bloom_gauss_LUT_tex);
			glGenTextures(BLOOM_PASSES*2,	&bloom_tex[0][0]);
			
			glGenFramebuffers(1,			&dbg_line_fbo);
			glGenTextures(1,				&dbg_line_tex);
			glGenRenderbuffers(1,			&dbg_line_depth_renderbuf);
			
			{
				glBindFramebuffer(GL_FRAMEBUFFER, main_fbo);
				
				{
					glBindTexture(GL_TEXTURE_2D, main_lumi_tex);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, main_lumi_tex, 0);
				}
				
				{
					glBindTexture(GL_TEXTURE_2D, main_depth_tex);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, main_depth_tex, 0);
				}
				
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
			
			{
				STATIC_ASSERT(BLOOM_GAUSS_LUT_RES >= 2);
				
				glBindTexture(GL_TEXTURE_1D, bloom_gauss_LUT_tex);
				
				f32	lut[BLOOM_GAUSS_LUT_RES];
				
				f32	k = PI; // higher values get rid of the 
				f32	tmp = k / fp::sqrt( PI );
				f32	nk2 = -k*k;
				
				for (ui i=0; i<BLOOM_GAUSS_LUT_RES; ++i) {
					
					f32 x = (f32)i/(BLOOM_GAUSS_LUT_RES -1);
					
					lut[i] = tmp * fp::exp( nk2 * (x*x) );
				}
				
				glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, (s32)BLOOM_GAUSS_LUT_RES, 0, GL_RED, GL_FLOAT, lut);
				
				glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_BASE_LEVEL,	0);
				glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAX_LEVEL,	0);
			}
			
			{
				glBindFramebuffer(GL_FRAMEBUFFER, dbg_line_fbo);
				
				{
					glBindTexture(GL_TEXTURE_2D, dbg_line_tex);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dbg_line_tex, 0);
				}
				
				{
					glBindRenderbuffer(GL_RENDERBUFFER, dbg_line_depth_renderbuf);
					glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 0, 0); // Error if this is not called before
					
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, dbg_line_depth_renderbuf);
				}
				glBindRenderbuffer(GL_RENDERBUFFER, 0);
				
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
		}
		
		void window_resolution_change (resolution_v vp window_res, v2* window_aspect_ratio) {
			
			render_res = window_res;
			
			{
				auto u = cast_v2<v2u32>(window_res); // this should be replaced by simply printing the signed int without a '+' in this case
				print("Resolution: %x%\n", u.x,u.y);
			}
			
			auto fRes = cast_v2<v2>(window_res);
			*window_aspect_ratio = v2( fRes.x/fRes.y, fRes.y/fRes.x );
			
			{
				glBindTexture(GL_TEXTURE_2D, main_lumi_tex);
				
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, render_res.x,render_res.y,
						0, GL_RGB, GL_FLOAT, NULL);
				
				glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAX_LEVEL, 0);
			}
			
			{
				glBindTexture(GL_TEXTURE_2D, main_depth_tex);
				
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, render_res.x,render_res.y,
						0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
				
				glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAX_LEVEL, 0);
			}
			
			{
				glBindTexture(GL_TEXTURE_2D, convolved_tex);
				
				auto r = render_res;
				
				GLint mip=0;
				for (;; ++mip) {
					
					glTexImage2D(GL_TEXTURE_2D, mip, GL_RGBA32F, r.x,r.y,
							0, GL_RGBA, GL_FLOAT, NULL);
					
					if (r.x == 1 && r.y == 1) {
						break;
					}
					r.x = r.x == 1 ? 1 : r.x / 2;
					r.y = r.y == 1 ? 1 : r.y / 2;
				}
				
				render_res_mips = mip +1;
			}
			
			{
				glBindTexture(GL_TEXTURE_2D, dbg_line_tex);
				
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, render_res.x,render_res.y,
						0, GL_RGBA, GL_FLOAT, NULL);
				
				glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAX_LEVEL, 0);
			}
			
			{
				glBindRenderbuffer(GL_RENDERBUFFER, dbg_line_depth_renderbuf);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, render_res.x,render_res.y);
			}
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			
		}
		
		template <typename DRAW_SCENE>
		void shadow_pass_directional (GLuint tex, m4 mp projection, hm mp world_to_light, hm mp light_to_world,
				DRAW_SCENE draw_scene) {
			
			glUseProgram(shaders[SHAD_SHADOW_PASS_DIRECTIONAL]);
			
			// tex already bound
			
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_res,shadow_res,
					0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			
			glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_BASE_LEVEL,	0);
			glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAX_LEVEL,	0);
			
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex, 0);
			
			
			glViewport(0,0, shadow_res,shadow_res);
			
			glClear(GL_DEPTH_BUFFER_BIT);
			
			GLOBAL_UBO_WRITE_VAL( std140::mat4, cam_to_clip, projection );
			
			draw_scene(world_to_light, light_to_world);
			
		}
		
		template <typename DRAW_SCENE>
		void shadow_pass_point (GLuint tex, m4 mp projection, f32 far, hm mp world_to_light, hm mp light_to_world,
				DRAW_SCENE draw_scene) {
			
			glUseProgram(shaders[SHAD_SHADOW_PASS_OMNIDIR]);
			
			{
				GLuint unif_far =	glGetUniformLocation(shaders[SHAD_SHADOW_PASS_OMNIDIR], "unif_inv_far");
				f32 inv_far = 1.0f / far;
				
				glUniform1f(unif_far, inv_far);
				GLOBAL_UBO_WRITE_VAL( std140::float_, shadow1_inv_far, inv_far );
			}
			
			{
				// tex already bound
				
				for (ui face=0; face<6; ++face) {
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X +face, 0, GL_DEPTH_COMPONENT, shadow_res,shadow_res,
							0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
				}
				
				glTexParameteri(GL_TEXTURE_CUBE_MAP,	GL_TEXTURE_BASE_LEVEL,	0);
				glTexParameteri(GL_TEXTURE_CUBE_MAP,	GL_TEXTURE_MAX_LEVEL,	0);
			}
			
			glViewport(0,0, shadow_res,shadow_res);
			
			v4	X = v4(1,0,0,0);
			v4	Y = v4(0,1,0,0);
			v4	Z = v4(0,0,1,0);
			v4	W = v4(0,0,0,1);
			
			m4 face_mat[6] = {
				m4::row(+Z,+Y,-X,	W),
				m4::row(-Z,+Y,+X,	W),
				m4::row(-X,-Z,-Y,	W),
				m4::row(-X,+Z,+Y,	W),
				m4::row(-X,+Y,-Z,	W),
				m4::row(+X,+Y,+Z,	W),
			};
			
			for (ui face=0; face<6; ++face) {
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
						GL_TEXTURE_CUBE_MAP_POSITIVE_X +face, tex, 0);
				
				glClear(GL_DEPTH_BUFFER_BIT);
				
				GLOBAL_UBO_WRITE_VAL( std140::mat4, cam_to_clip, projection * face_mat[face] );
				
				draw_scene(world_to_light, light_to_world);
				
			}
			
		}
		
		void main_pass () {
			
			#if DISABLE_POST
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0,0, render_res.x,render_res.y);
			#else
			glBindFramebuffer(GL_FRAMEBUFFER, main_fbo);
			glViewport(0,0, render_res.x,render_res.y);
			#endif
			
		}
		
		void dbg_line_pass () {
			
			glBindFramebuffer(GL_FRAMEBUFFER, dbg_line_fbo);
			glViewport(0,0, render_res.x,render_res.y);
			
			glClearColor(0,0,0, 0);
			glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
			
			glActiveTexture(GL_TEXTURE0 +TEX_UNIT_MAIN_PASS_DEPTH);
			glBindTexture(GL_TEXTURE_2D, main_depth_tex);
			glBindSampler(TEX_UNIT_MAIN_PASS_DEPTH,			tex.samplers[SAMPLER_RESAMPLE]);
			
		}
		
		void post_passes (resolution_v vp window_res, v2 vp window_aspect_ratio) {
			
			#if DISABLE_POST
			return;
			#endif
			
			glBindVertexArray(VAOs[VAO_FULLSCREEN_QUAD]);
			
			#if 1
			{
				PROFILE_SCOPED(THR_ENGINE, "prev_frame_luminance_convolution_result_get");
				
				glBindTexture(GL_TEXTURE_2D, convolved_tex);
				
				f32 avg_log_luminance;
				
				if (frame_number == 0) {
					avg_log_luminance = 0.0f;
				} else {
					{
						GLint w, h;
						
						glGetTexLevelParameteriv(GL_TEXTURE_2D, render_res_mips -1, GL_TEXTURE_WIDTH, &w);
						glGetTexLevelParameteriv(GL_TEXTURE_2D, render_res_mips -1, GL_TEXTURE_HEIGHT, &h);
						
						assert(w == 1 && h == 1, "% % %", w, h, render_res_mips);
					}
					
					v4 convolved;
					glGetTexImage(GL_TEXTURE_2D, render_res_mips -1, GL_RGBA, GL_FLOAT, &convolved);
					
					auto luminance = [] (v3 vp linear_col) -> f32 {
						static constexpr v3 weights = v3(0.2126f, 0.7152f, 0.0722f);
						v3 tmp = linear_col * weights;
						return tmp.x +tmp.y +tmp.z;
					};
					
					avg_log_luminance = convolved.w;
				}
				
				//print(">> %\n", avg_log_luminance);
				GLOBAL_UBO_WRITE_VAL(std140::float_, avg_log_luminance, avg_log_luminance);
			}
			#else
			{
				GLOBAL_UBO_WRITE_VAL(std140::float_, avg_log_luminance, 1.0f);
			}
			#endif
			
			{ // Convolve pass
				PROFILE_SCOPED(THR_ENGINE, "luminance_convolve");
				
				glActiveTexture(GL_TEXTURE0 +0);
				glBindTexture(GL_TEXTURE_2D, main_lumi_tex);
				glBindSampler(0,							tex.samplers[SAMPLER_RESAMPLE]);
				
				glBindFramebuffer(GL_FRAMEBUFFER, convolve_fbo);
				glUseProgram(shaders[SHAD_CONVOLVE_LUMINANCE]);
				
				GLuint unif_bd = glGetUniformLocation(shaders[SHAD_CONVOLVE_LUMINANCE], "blur_dist");
				GLuint unif_pass = glGetUniformLocation(shaders[SHAD_CONVOLVE_LUMINANCE], "mip_pass");
				
				f32 blur_dist_factor = 1.0f; // Blur dist (3x3 kernel sample distance) in output pixel coords
				
				auto r = render_res;
				
				GLint mip = 0;
				for (;; ++mip) {
					PROFILE_SCOPED(THR_ENGINE, "mip", (u32)mip);
					
					auto prev_res = r;
					
					if (mip == 1) {
						glBindTexture(GL_TEXTURE_2D, convolved_tex);
					}
					if (mip >= 1) {
						glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_BASE_LEVEL, mip -1);	// read from prev mip level
						glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAX_LEVEL, mip -1);	// 
					}
					
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, convolved_tex, mip);
					glViewport(0,0, r.x,r.y);
					
					v2 blur_dist = v2(blur_dist_factor) / cast_v2<v2>(r);
					glUniform1ui(unif_pass, mip);
					glUniform2fv(unif_bd, 1, &blur_dist.x);
					
					glDrawArrays(GL_TRIANGLES, 0, 6);
					
					if (r.x == 1 && r.y == 1) {
						break;
					}
					r.x = r.x == 1 ? 1 : r.x / 2;
					r.y = r.y == 1 ? 1 : r.y / 2;
					
				}
				
				glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAX_LEVEL, mip -1);
			}
			
			#if 0
			{ // Bloom pass
				PROFILE_SCOPED(THR_ENGINE, "bloom_pass");
				
				glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo);
				
				glUseProgram(shaders[SHAD_POST_BLOOM]);
				
				glActiveTexture(GL_TEXTURE0 +TEX_UNIT_BLOOM_GAUSS_LUT);
				glBindTexture(GL_TEXTURE_1D, bloom_gauss_LUT_tex);
				glBindSampler(TEX_UNIT_BLOOM_GAUSS_LUT,		tex.samplers[SAMPLER_1D_LUT]);
				
				glActiveTexture(GL_TEXTURE0 +TEX_UNIT_MAIN_PASS_LUMI);
				glBindSampler(TEX_UNIT_MAIN_PASS_LUMI,		tex.samplers[SAMPLER_BORDER]);	// Prevent small bright dots
																							//  from becoming way brighter when leaving the screen because of GL_CLAMP_TO_EDGE
																							//  GL_CLAMP_TO_BORDER (black border) instead
				
				GLuint unif_res =	glGetUniformLocation(shaders[SHAD_POST_BLOOM], "gauss_res");
				GLuint unif_axis =	glGetUniformLocation(shaders[SHAD_POST_BLOOM], "gauss_axis");
				GLuint unif_size =	glGetUniformLocation(shaders[SHAD_POST_BLOOM], "gauss_kernel_size");
				GLuint unif_coeff =	glGetUniformLocation(shaders[SHAD_POST_BLOOM], "gauss_coeff");
				
				for (u32 pass=0; pass<BLOOM_PASSES; ++pass) {
					
					auto res = render_res;
					res.x >>= bloom_res_scale[pass];
					res.y >>= bloom_res_scale[pass];
					res.x = res.x >= 1 ? res.x : 1;
					res.y = res.y >= 1 ? res.y : 1;
					
					glUniform1f(unif_coeff, bloom_coeff[pass]);
					
					glViewport(0,0, res.x,res.y);
					GLOBAL_UBO_WRITE_VAL( std140::ivec2, target_res, res );
					
					for (ui i=0; i<2; ++i) {
						glBindTexture(GL_TEXTURE_2D, bloom_tex[pass][i]);
						
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, res.x,res.y,
								0, GL_RGB, GL_FLOAT, NULL);
						
						glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_BASE_LEVEL, 0);
						glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAX_LEVEL, 0);
					}
					glBindTexture(GL_TEXTURE_2D, convolved_tex);
					
					f32 size = bloom_size[pass];
					
					{
						glUniform1f(unif_res, cast<f32>(res.x));
						
						glUniform1ui(unif_axis, 0);
						glUniform1f(unif_size, size * window_aspect_ratio.y);
						
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloom_tex[pass][0], 0);
						
						glDrawArrays(GL_TRIANGLES, 0, 6);
					}
					
					glBindTexture(GL_TEXTURE_2D, bloom_tex[pass][0]);
					{
						glUniform1f(unif_res, cast<f32>(res.y));
						
						glUniform1ui(unif_axis, 1);
						glUniform1f(unif_size, size);
						
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloom_tex[pass][1], 0);
						
						glDrawArrays(GL_TRIANGLES, 0, 6);
					}
				}
			}
			#endif
			
			{
				PROFILE_SCOPED(THR_ENGINE, "post_combine");
				
				{
					PROFILE_SCOPED(THR_ENGINE, "gl_binding");
					
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					glViewport(0, 0, window_res.x, window_res.y);
					GLOBAL_UBO_WRITE_VAL( std140::ivec2, target_res, window_res );
					
					glActiveTexture(GL_TEXTURE0 +TEX_UNIT_MAIN_PASS_LUMI);
					glBindTexture(GL_TEXTURE_2D, main_lumi_tex);
					glBindSampler(TEX_UNIT_MAIN_PASS_LUMI,			tex.samplers[SAMPLER_RESAMPLE]);
					
					glActiveTexture(GL_TEXTURE0 +TEX_UNIT_MAIN_PASS_BLOOM_0);
					glBindTexture(GL_TEXTURE_2D, bloom_tex[0][1]);
					glBindSampler(TEX_UNIT_MAIN_PASS_BLOOM_0,		tex.samplers[SAMPLER_RESAMPLE]);
					
					glActiveTexture(GL_TEXTURE0 +TEX_UNIT_MAIN_PASS_BLOOM_1);
					glBindTexture(GL_TEXTURE_2D, bloom_tex[1][1]);
					glBindSampler(TEX_UNIT_MAIN_PASS_BLOOM_1,		tex.samplers[SAMPLER_RESAMPLE]);
					
					glActiveTexture(GL_TEXTURE0 +TEX_UNIT_MAIN_PASS_BLOOM_2);
					glBindTexture(GL_TEXTURE_2D, bloom_tex[2][1]);
					glBindSampler(TEX_UNIT_MAIN_PASS_BLOOM_2,		tex.samplers[SAMPLER_RESAMPLE]);
					
					glActiveTexture(GL_TEXTURE0 +TEX_UNIT_DBG_LINE_PASS);
					glBindTexture(GL_TEXTURE_2D, dbg_line_tex);
					glBindSampler(TEX_UNIT_DBG_LINE_PASS,			tex.samplers[SAMPLER_RESAMPLE]);
					
					glActiveTexture(GL_TEXTURE0 +TEX_UNIT_POST_DBG_TEX_0);
					glBindTexture(GL_TEXTURE_2D, dbg_tex_0);
					glBindSampler(TEX_UNIT_POST_DBG_TEX_0,			tex.samplers[SAMPLER_RESAMPLE]);
					
					glActiveTexture(GL_TEXTURE0 +TEX_UNIT_POST_DBG_TEX_1);
					glBindTexture(GL_TEXTURE_CUBE_MAP, dbg_tex_1);
					glBindSampler(TEX_UNIT_POST_DBG_TEX_1,			tex.samplers[CUBEMAP_SAMPLER]);
					
					glUseProgram(shaders[SHAD_POSTPROCESS]);
				}
				
				{
					PROFILE_SCOPED(THR_ENGINE, "glDrawArrays");
					glDrawArrays(GL_TRIANGLES, 0, 6);
				}
			}
			
		}
		
	};
	
	DECLD v2					window_aspect_ratio; // x=w/h y=h/w
	
	DECLD Controls				controls;
	DECLD Camera				camera;
	DECLD Passes				passes;
	
	struct Dbg_Lines {
		
		struct Vert {
			v4	pos_clip;
			v4	col;
			f32	line_pos; // pos along line
		};
		struct Tri {
			Vert	a, b, c;
		};
		
		u32	tri_per_line = 6;
		
		GLuint VAO;
		GLuint VBO;
		
		dynarr<Tri>	buf;
		
		void init () {
			PROFILE_SCOPED(THR_ENGINE, "dbg_lines_init");
			
			glGenBuffers(1, &VBO);
			glGenVertexArrays(1, &VAO);
			
			buf.alloc(0, tri_per_line * 128);
			
			{
				glBindVertexArray(VAO);
				
				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
				glEnableVertexAttribArray(2);
				
				glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, (GLsizei)sizeof(Vert), (void*)offsetof(Vert, pos_clip));
				glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, (GLsizei)sizeof(Vert), (void*)offsetof(Vert, col));
				glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, (GLsizei)sizeof(Vert), (void*)offsetof(Vert, line_pos));
				
				glBindVertexArray(0);
			}
			
		}
		
		void push_line_cam (hm mp to_cam, v3 vp a_space, v3 vp b_space, v3 vp col, f32 width=0.008f) {
			
			f32	len_space = distance(a_space, b_space);
			
			// everything in camera space
			v3	a_cam = (to_cam * hv(a_space)).xyz();
			v3	b_cam = (to_cam * hv(b_space)).xyz();
			
			f32	hw = width * 0.5f;
			
			v4	a_clip = camera.cam_to_clip * v4(a_cam, 1);
			v4	b_clip = camera.cam_to_clip * v4(b_cam, 1);
			f32	aw_clip = camera.cam_to_clip.row(0, 0) * hw;
			f32	bw_clip = camera.cam_to_clip.row(0, 0) * hw;
			
			v3	a_ndc = a_clip.xyz() / a_clip.www();
			v3	b_ndc = b_clip.xyz() / b_clip.www();
			f32	aw_ndc = aw_clip / a_clip.w;
			f32	bw_ndc = bw_clip / b_clip.w;
			
			v2	res = cast_v2<v2>(passes.render_res);
			v2	hres = res * v2(0.5f);
			v2	a_scr = a_ndc.xy() * hres +hres;
			v2	b_scr = b_ndc.xy() * hres +hres;
			f32	aw_scr = aw_ndc * hres.x;
			f32	bw_scr = bw_ndc * hres.x;
			
			// in screen space
			v2	line = b_scr -a_scr;
			f32	len = length(line);
			v2	dir = line / v2(len);
			
			v2	perp = v2(dir.y, -dir.x);
			
			v2	ai_scr = v2(aw_scr) * perp;
			v2	ao_scr = ai_scr +perp;	// one pixel antialias (with alpha gradient) around line
										// also prevents line from becoming invisible because too thin,
										//  because it will alwas be at least two pixel wide 0-1-0 alpha gradient (which results in a 1px antilaliased line)
			
			v2	bi_scr = v2(bw_scr) * perp;
			v2	bo_scr = bi_scr +perp;	//
			
			// in clip space
			v4	a = a_clip;
			v4	b = b_clip;
			
			v4	ai = v4(ai_scr / hres * a_clip.ww(), 0, 0);
			v4	ao = v4(ao_scr / hres * a_clip.ww(), 0, 0);
			
			v4	bi = v4(bi_scr / hres * b_clip.ww(), 0, 0);
			v4	bo = v4(bo_scr / hres * b_clip.ww(), 0, 0);
			
			Vert A0 =	{a -ao,		v4(col, 0),		0 };
			Vert A1 =	{a -ai,		v4(col, 1),		0 };
			Vert A2 =	{a +ai,		v4(col, 1),		0 };
			Vert A3 =	{a +ao,		v4(col, 0),		0 };
			                                              
			Vert B0 =	{b -bo,		v4(col, 0),		len_space };
			Vert B1 =	{b -bi,		v4(col, 1),		len_space };
			Vert B2 =	{b +bi,		v4(col, 1),		len_space };
			Vert B3 =	{b +bo,		v4(col, 0),		len_space };
			
			Tri* tri = buf.grow_by(tri_per_line);
			
			tri[ 0] = {A0, A1, B0};
			tri[ 1] = {B0, A1, B1};
			
			tri[ 2] = {A1, A2, B1};
			tri[ 3] = {B1, A2, B2};
			
			tri[ 4] = {A2, A3, B2};
			tri[ 5] = {B2, A3, B3};
			
		}
		void push_line_world (hm mp to_world, v3 vp a, v3 vp b, v3 vp col, f32 width=0.008f) {
			push_line_cam(_world_to_cam * to_world, a, b, col, width);
		}
		
		void push_box_world (hm mp to_world, Box_Edges cr box, v3 vp col) {
			for (ui i=0; i<12; ++i) {
				push_line_cam(_world_to_cam * to_world, box.arr[i].a, box.arr[i].b, col);
			}
		}
		void push_box_cam (hm mp to_cam, Box_Edges cr box, v3 vp col) {
			for (ui i=0; i<12; ++i) {
				push_line_cam(to_cam, box.arr[i].a, box.arr[i].b, col);
			}
		}
		
		void push_box_world (hm mp to_world, AABB cr aabb, v3 vp col) {
			push_box_world(to_world, aabb.box_edges(), col);
		}
		void push_box_cam (hm mp to_cam, AABB cr aabb, v3 vp col) {
			push_box_cam(to_cam, aabb.box_edges(), col);
		}
		
		void push_cross_world (hm mp to_world, v3 pos, v3 vp col, f32 r=0.25f) {
			push_line_world(to_world, pos -v3(r,0,0), pos +v3(r,0,0), col);
			push_line_world(to_world, pos -v3(0,r,0), pos +v3(0,r,0), col);
			push_line_world(to_world, pos -v3(0,0,r), pos +v3(0,0,r), col);
		}
		
		void draw () {
			if (buf.len == 0) {
				return;
			}
			
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			
			glUseProgram(shaders[SHAD_CAM_ALPHA_COL]);
			
			glBindVertexArray(VAO);
			
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, (u32)sizeof(Tri)*buf.len, &buf[0], GL_STREAM_DRAW);
			
			glDrawArrays(GL_TRIANGLES, 0, (GLsizei)buf.len*3);
			
			buf.clear(0, tri_per_line * 128);
			
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glDisable(GL_BLEND);
		}
	};
	DECLD Dbg_Lines				dbg_lines;
	
	enum light_type_e : u32 {
		LT_DIRECTIONAL=0,
		LT_POINT,
		
		LIGHT_TYPES
	};
	enum light_flags_e : u32 {
		LF_DISABLED=	0b00000001,
		LF_NO_SHADOW=	0b00000010,
	};
	DEFINE_ENUM_FLAG_OPS(light_flags_e, u32)
	struct Light : public Generic_Movable {
		light_type_e	type;
		light_flags_e	flags;
		v3				power;
	};
	
	struct Lights {
		dynarr<Light>		lights;
		f32						select_radius[LIGHT_TYPES];
	};
	
	#define DISABLE_ENV_ALL		NOTEBOOK
	#define DISABLE_ENV_RENDER	NOTEBOOK
	
	DECLD struct Env_Viewer {
		
		u32						illuminance_res;
		u32						luminance_res;
		u32						luminance_prefilter_levels;
		u32						luminance_prefilter_base_samples;
		
		GLuint					env_source_cubemap;
		GLuint					env_source_equirectangular;
		GLuint					env_luminance;
		GLuint					env_illuminance;
		GLuint					pbr_brdf_LUT;
		
		GLuint					ibl_render_fbo;
		static constexpr GLsizei	PBR_BRDF_LUT_RES = 512;
		
		u32						cur_humus_indx;
		u32						cur_sibl_indx;
		bool					sibl =						true;
		
		Filenames				humus_folders;
		Filenames				sibl_files;
		
		u32 count_mip_levels (u32 res) {
			for (ui mip=0;; ++mip) {
				res /= 2;
				if (res == 0) {
					return mip +1;
				}
			}
		}
		
		u32 resolution () {
			u32 illuminance_levels = count_mip_levels(illuminance_res);
			
			if ( ((u32)1 << (luminance_prefilter_levels -1)) > luminance_res ) {
				warning(" ((u32)1 << (luminance_prefilter_levels -1)) > luminance_res");
				luminance_prefilter_levels = count_mip_levels(luminance_res);
			}
			
			{
				glBindTexture(GL_TEXTURE_CUBE_MAP, env_luminance);
				{
					GLsizei dim = luminance_res;
					for (u32 mip=0; mip<luminance_prefilter_levels; ++mip) {
						for (ui face=0; face<6; ++face) {
							glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X +face, (GLint)mip, GL_RGB32F, dim,dim,
									0, GL_RGB, GL_FLOAT, NULL);
						}
						dim /= 2;
					}
				}
				
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, luminance_prefilter_levels -1);
				
				//
				glBindTexture(GL_TEXTURE_CUBE_MAP, env_illuminance);
				for (ui face=0; face<6; ++face) {
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X +face, 0, GL_RGB32F, illuminance_res,illuminance_res,
							0, GL_RGB, GL_FLOAT, NULL);
				}
				
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, illuminance_levels -1);
			}
			
			GLOBAL_UBO_WRITE_VAL(std140::uint_, luminance_prefilter_base_samples, luminance_prefilter_base_samples);
			
			return illuminance_levels;
		}
		
		void init () {
			PROFILE_SCOPED(THR_ENGINE, "env_viewer_init");
			
			glGenTextures(1, &env_source_cubemap);			// srgb8 (humus) or rgb32f (sibl)
			glGenTextures(1, &env_source_equirectangular);	// rgb32f
			glGenTextures(1, &env_luminance);
			glGenTextures(1, &env_illuminance);
			glGenTextures(1, &pbr_brdf_LUT);
			
			humus_folders = list_of_folders_in(ENV_MAPS_HUMUS_DIR);
			sibl_files = recursive_list_of_files_in(ENV_MAPS_SIBL_DIR);
			//assert(humus_folders.arr.len > 0,
			//		"No cubemaps found in % (a cubemap should be a folder with the cubemap faces as images named posx/posy/posz/negx/negy/negz +.file_ext)!\n",
			//		ENV_MAPS_HUMUS_DIR);
			safe_cast_assert(u32, humus_folders.arr.len);
			safe_cast_assert(u32, sibl_files.arr.len);
			
			//
			glGenFramebuffers(1, &ibl_render_fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, ibl_render_fbo);
			
			render_brdf_LUT();
		}
		
		void render_brdf_LUT () {
			glActiveTexture(GL_TEXTURE0 +TEX_UNIT_PBR_BRDF_LUT);
			glBindTexture(GL_TEXTURE_2D, pbr_brdf_LUT);
			
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, PBR_BRDF_LUT_RES,PBR_BRDF_LUT_RES, 0,
					GL_RG, GL_FLOAT, NULL);
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			
			glBindFramebuffer(GL_FRAMEBUFFER, ibl_render_fbo);
			
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_2D, pbr_brdf_LUT, 0);
			
			{
				auto ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
				assert(ret == GL_FRAMEBUFFER_COMPLETE);
			}
			
			#if !DISABLE_ENV_RENDER
			glViewport(0,0, PBR_BRDF_LUT_RES,PBR_BRDF_LUT_RES);
			
			glBindVertexArray(VAOs[VAO_FULLSCREEN_QUAD]);
			glUseProgram(shaders[SHAD_RENDER_IBL_BRDF_LUT]);
			
			glDrawArrays(GL_TRIANGLES, 0, 6);
			#endif
		}
		
		void update (Inp cr inp) {
			
			bool update =	//reloaded_vars ||
							//inp.misc_reload_shaders ||
							frame_number == 0;
			
			if (inp.graphics_toggle_env_counter > 0) {
				sibl ^= inp.graphics_toggle_env_counter & 1;
				
				update = true;
			}
			
			if (!sibl) {
				if (inp.graphics_switch_env_accum != 0) {
					cur_humus_indx += inp.graphics_switch_env_accum;
					cur_humus_indx = (s32)cur_humus_indx % (s32)humus_folders.arr.len;
					if ((s32)cur_humus_indx < 0) { cur_humus_indx += (s32)humus_folders.arr.len; }
					
					update = true;
				}
				
			} else {
				if (inp.graphics_switch_env_accum != 0) {
					cur_sibl_indx += inp.graphics_switch_env_accum;
					cur_sibl_indx = (s32)cur_sibl_indx % (s32)sibl_files.arr.len;
					if ((s32)cur_sibl_indx < 0) { cur_sibl_indx += (s32)sibl_files.arr.len; }
					
					update = true;
				}
			}
			
			#if DISABLE_ENV_ALL
			return;
			#endif
			
			if (!update) {
				return;
			}
			
			glBindFramebuffer(GL_FRAMEBUFFER, ibl_render_fbo);
			
			u32 illuminance_levels = resolution();
			
			glActiveTexture(GL_TEXTURE0 +ENV_LUMINANCE_TEX_UNIT);
			
			if (!sibl) {
				glBindTexture(GL_TEXTURE_CUBE_MAP, env_source_cubemap);
				
				auto humus_name = humus_folders.get_filename(cur_humus_indx);
				
				print("Humus environment: [%] '%'\n", cur_humus_indx, humus_name);
				
				static constexpr lstr FACES[6] = {
					"/posx.jpg",
					"/negx.jpg",
					"/posz.jpg",
					"/negz.jpg",
					"/posy.jpg",
					"/negy.jpg",
				};
				enum rotate_e {
					DONT=0, CCW_90, _180, CW_90
				};
				static constexpr rotate_e HUMUS_FACE_ROTATION[6] = {
					CCW_90,
					CW_90,
					DONT,
					_180,
					DONT,
					_180,
				};
				
				static constexpr ui FACE_ORDER[6] = {
					0, // pos x
					2, // pos y
					1, // neg x
					3, // neg y // do the four horizontal directions in one circular 'motion'
					4, // pos z
					5, // neg z
				};
				
				u32	dim;
				
				for (ui const* face=FACE_ORDER; face < &FACE_ORDER[6]; ++face) {
					bool first_face = face == FACE_ORDER;
					
					DEFER_POP(&working_stk);
					lstr filename = str::append_term(&working_stk, ENV_MAPS_HUMUS_DIR, humus_name, FACES[*face]);
					
					u8*	img;
					
					int w, h;
					{
						Mem_Block file;
						assert(!platform::read_whole_file_onto(&working_stk, filename.str, 0, &file));
						
						int comp=3;
						img = stb::stbi_load_from_memory((uchar*)file.ptr, safe_cast_assert(int, file.size), &w, &h, &comp, comp);
						if (!img) {
							warning("stbi error %\n", stb::stbi_failure_reason());
							
						}
					}
					defer {
						free(img);
					};
					
					{
						if (w != h) {
							warning("Humus cubemap has non-square resolution!");
							return;
						}
						
						u32 img_dim = (u32)w;
						
						if (first_face) {
							dim = img_dim;
						} else {
							if (img_dim != dim) {
								warning("Warning: humus cubemap face % resolution does not match resolution of previous faces!",
										FACES[*face]);
								return;
							}
						}
						dim = img_dim;
					}
					
					u32 stride = align_up<u32>(dim * 3, 4);
					u8* rotated = (u8*)malloc((uptr)dim * stride);
					defer {
						free(rotated);
					};
					
					{ // Rotate humus cubemap faces to our z-up coordinates
						
						u8*	in_pix_data = img;
						u8*	out_pix_data = rotated;
						
						auto rot = HUMUS_FACE_ROTATION[*face];
						
						auto get_in = [&] (u32 x, u32 y) -> u8* {
							return in_pix_data +(y * dim * 3) +(x * 3);
						};
						
						switch (rot) {
							
							case CCW_90: {
								for (u32 j=0; j<dim; ++j) {
									
									u8* out_pix = out_pix_data;
									
									for (u32 i=0; i<dim; ++i) {
										u8* in_pix = get_in( j, dim -1 -i );
										
										out_pix[0] = in_pix[0];
										out_pix[1] = in_pix[1];
										out_pix[2] = in_pix[2];
										
										out_pix += 3;
									}
									
									out_pix_data += stride;
									
									while (out_pix != out_pix_data) {
										*out_pix++ = 0;
									}
								}
							} break;
							
							case CW_90: {
								for (u32 j=0; j<dim; ++j) {
									
									u8* out_pix = out_pix_data;
									
									for (u32 i=0; i<dim; ++i) {
										u8* in_pix = get_in( dim -1 -j, i );
										
										out_pix[0] = in_pix[0];
										out_pix[1] = in_pix[1];
										out_pix[2] = in_pix[2];
										
										out_pix += 3;
									}
									
									out_pix_data += stride;
									
									while (out_pix != out_pix_data) {
										*out_pix++ = 0;
									}
								}
							} break;
							
							case _180: {
								for (u32 j=0; j<dim; ++j) {
									
									u8* out_pix = out_pix_data;
									
									for (u32 i=0; i<dim; ++i) {
										u8* in_pix = get_in( dim -1 -i, dim -1 -j );
										
										out_pix[0] = in_pix[0];
										out_pix[1] = in_pix[1];
										out_pix[2] = in_pix[2];
										
										out_pix += 3;
									}
									
									out_pix_data += stride;
									
									while (out_pix != out_pix_data) {
										*out_pix++ = 0;
									}
								}
							} break;
							
							case DONT: {
								for (u32 j=0; j<dim; ++j) {
									
									u8* out_pix = out_pix_data;
									
									for (u32 i=0; i<dim; ++i) {
										u8* in_pix = get_in( i, j );
										
										out_pix[0] = in_pix[0];
										out_pix[1] = in_pix[1];
										out_pix[2] = in_pix[2];
										
										out_pix += 3;
									}
									
									out_pix_data += stride;
									
									while (out_pix != out_pix_data) {
										*out_pix++ = 0;
									}
								}
							} break;
						}
						
					}
					
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X +*face, 0, GL_SRGB8, (GLsizei)dim,(GLsizei)dim, 0,
							GL_RGB, GL_UNSIGNED_BYTE, rotated);
					
				}
				
				glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
				
			} else {
				glBindTexture(GL_TEXTURE_2D, env_source_equirectangular);
				
				auto sibl_name = sibl_files.get_filename(cur_sibl_indx);
				
				DEFER_POP(&working_stk);
				lstr filename = str::append_term(&working_stk, ENV_MAPS_SIBL_DIR, sibl_name);
				
				print("sIBL environment: [%] '%'\n", cur_sibl_indx, sibl_name);
				
				f32* img;
				
				int w, h;
				{
					Mem_Block file;
					assert(!platform::read_whole_file_onto(&working_stk, filename.str, 0, &file));
					
					int comp=3;
					img = stb::stbi_loadf_from_memory((uchar*)file.ptr, safe_cast_assert(int, file.size), &w, &h, &comp, comp);
					assert(img, "stbi error %\n", stb::stbi_failure_reason());
				}
				defer {
					free(img);
				};
				
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, (GLsizei)w,(GLsizei)h, 0,
						GL_RGB, GL_FLOAT, img);
				
				glGenerateMipmap(GL_TEXTURE_2D);
				
				{
					glBindTexture(GL_TEXTURE_CUBE_MAP, env_source_cubemap);
					for (ui face=0; face<6; ++face) {
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X +face, 0, GL_RGB32F, luminance_res,luminance_res, 0,
								GL_RGB, GL_FLOAT, NULL);
					}
					
					#if !DISABLE_ENV_RENDER
					glViewport(0,0, luminance_res,luminance_res);
					
					glBindVertexArray(VAOs[VAO_RENDER_CUBEMAP]);
					glUseProgram(shaders[SHAD_RENDER_EQUIRECTANGULAR_TO_CUBEMAP]);
					
					// ENV_LUMINANCE_TEX_UNIT still bound
					glBindTexture(GL_TEXTURE_2D, env_source_equirectangular);
					
					for (ui face=0; face<6; ++face) {
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
								GL_TEXTURE_CUBE_MAP_POSITIVE_X +face, env_source_cubemap, 0);
						
						glDrawArrays(GL_TRIANGLES, face*6, 6);
					}
					#endif
				}
				
				glBindTexture(GL_TEXTURE_CUBE_MAP, env_source_cubemap);
				
				glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
				
			}
			
			glBindVertexArray(VAOs[VAO_RENDER_CUBEMAP]);
			
			#if !DISABLE_ENV_RENDER
			// Source: env_source_cubemap still bound
			
			glUseProgram(shaders[SHAD_RENDER_IBL_ILLUMINANCE]);
			{
				
				glViewport(0,0, illuminance_res,illuminance_res);
				
				for (ui face=0; face<6; ++face) {
					// Dest:
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
							GL_TEXTURE_CUBE_MAP_POSITIVE_X +face, env_illuminance, 0);
					
					glDrawArrays(GL_TRIANGLES, face*6, 6);
					
				}
				
				glActiveTexture(GL_TEXTURE0 +ENV_ILLUMINANCE_TEX_UNIT);
				glBindTexture(GL_TEXTURE_CUBE_MAP, env_illuminance);
				
				glGenerateMipmap(GL_TEXTURE_CUBE_MAP); // gen mipmaps for illuminance map
				
			}
			#endif
			
			#if !DISABLE_ENV_RENDER
			GLOBAL_UBO_WRITE_VAL(std140::uint_, source_cubemap_res, luminance_res);
			GLOBAL_UBO_WRITE_VAL(std140::float_, luminance_prefilter_mip_count, (f32)luminance_prefilter_levels );
			
			glUseProgram(shaders[SHAD_RENDER_IBL_LUMINANCE_PREFILTER]);
			{
				GLsizei dim = luminance_res;
				for (u32 mip=0; mip<luminance_prefilter_levels; ++mip) {
					
					glViewport(0,0, dim,dim);
					
					f32 roughness = (f32)mip / (f32)(luminance_prefilter_levels -1);
					GLOBAL_UBO_WRITE_VAL(std140::uint_, luminance_prefilter_mip_indx, mip);
					GLOBAL_UBO_WRITE_VAL(std140::float_, luminance_prefilter_roughness, roughness);
					
					for (ui face=0; face<6; ++face) {
						// Dest:
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
								GL_TEXTURE_CUBE_MAP_POSITIVE_X +face, env_luminance, (GLint)mip);
						
						glDrawArrays(GL_TRIANGLES, face * 6, 6);
						
					}
					
					dim /= 2;
				}
				
			}
			#endif
			
		}
		
		void set_env_map () {
			
			#if DISABLE_ENV_ALL
			glActiveTexture(GL_TEXTURE0 +ENV_LUMINANCE_TEX_UNIT);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			
			glActiveTexture(GL_TEXTURE0 +ENV_ILLUMINANCE_TEX_UNIT);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			
			glActiveTexture(GL_TEXTURE0 +TEX_UNIT_PBR_BRDF_LUT);
			glBindTexture(GL_TEXTURE_2D, 0);
			#else
			glActiveTexture(GL_TEXTURE0 +ENV_LUMINANCE_TEX_UNIT);
			glBindTexture(GL_TEXTURE_CUBE_MAP, env_luminance);
			
			glActiveTexture(GL_TEXTURE0 +ENV_ILLUMINANCE_TEX_UNIT);
			glBindTexture(GL_TEXTURE_CUBE_MAP, env_illuminance);
			
			glActiveTexture(GL_TEXTURE0 +TEX_UNIT_PBR_BRDF_LUT);
			glBindTexture(GL_TEXTURE_2D, pbr_brdf_LUT);
			#endif
			
		}
		
	} env_viewer;
	
	DECLD struct Showcase {
		
		u32				grid_steps;
		
		struct Grid_Obj : public Generic_Movable {
			f32				select_radius;
			f32				grid_offs;
		};
		
		Grid_Obj		sphere;
		Grid_Obj		bunny;
		Grid_Obj		buddha;
		Grid_Obj		dragon;
		Grid_Obj		teapot;
		Grid_Obj		materials;
		
		struct Cerberus : public Generic_Movable {
			f32				select_radius;
		}				cerberus;
		
	} showcase;
	
//// Entities
	enum entity_tag : u32 {
		ET_ROOT=0,
		ET_MESH,
		ET_MATERIAL_SHOWCASE_GRID,
		ET_GROUP,
		ET_LIGHT,
		ET_SCENE,
	};
	enum entity_flag : u32 {
		EF_HAS_MESHES=	0b01,
	};
	DEFINE_ENUM_FLAG_OPS(entity_flag, u32)
	
	struct Entity {
		char const*		name;
		Entity*			next;
		Entity*			parent;
		Entity*			children;
		
		// Relative to parent space / children are in the space defined by these placement variables
		//  scale always first, orientation second and position last
		v3				pos;
		quat			ori;
		v3				scale;
		
		AABB			aabb_mesh_paren; // only valid if (eflags & EF_HAS_MESHES)
		AABB			aabb_mesh_world; // only valid if (eflags & EF_HAS_MESHES)
		
		entity_flag		eflags;
		entity_tag		tag;
	};
	
	struct Mesh : public Entity {
		mesh_id_e		mesh_id;
	};
	
	DECLD v2u32		material_showcase_grid_steps;
	DECLD m2		material_showcase_grid_mat;
	
	struct Material_Showcase_Grid : public Entity {
		mesh_id_e		mesh_id;
		v2				grid_offs;
	};
	struct Group : public Entity {
		
	};
	struct Light_ : public Entity {
		light_type_e	type;
		light_flags_e	flags;
		v3				power;
	};
	struct Scene : public Entity {
		bool			draw;
		dynarr<Light_*>	lights;
	};
	struct Root : public Entity {
		constexpr Root (): Entity{
				"<root>", nullptr, nullptr, nullptr,
			v3(QNAN), quat(v3(QNAN), QNAN), v3(QNAN),
			{{QNAN,QNAN, QNAN,QNAN, QNAN,QNAN}},	{{QNAN,QNAN, QNAN,QNAN, QNAN,QNAN}},
			(entity_flag)0, ET_ROOT,
		} {}
	};
	
	hm me_to_parent (Entity const* e) {
		return translate_h(e->pos) * hm::ident().m3(conv_to_m3(e->ori)) * scale_h(e->scale);
	}
	hm parent_to_me (Entity const* e) {
		return scale_h(v3(1) / e->scale) * hm::ident().m3(conv_to_m3(inverse(e->ori))) * translate_h(-e->pos);
	}
	hm calc_me_to_world (Entity const* e) {
		hm ret = me_to_parent(e);
		e = e->parent;
		while (e->tag != ET_ROOT) {
			ret = me_to_parent(e) * ret;
			e = e->parent;
		}
		return ret;
	}
	hm calc_mat_me_world (Entity const* e, hm* out_world_to_me) {
		hm ret = me_to_parent(e);
		hm inv = parent_to_me(e);
		e = e->parent;
		while (e->tag != ET_ROOT) {
			ret = me_to_parent(e) * ret;
			inv = inv * parent_to_me(e);
			e = e->parent;
		}
		*out_world_to_me = inv;
		return ret;
	}
	hm calc_parent_to_world (Entity const* e) {
		hm ret;
		if (e->parent->tag != ET_ROOT) {
			e = e->parent;
			ret = me_to_parent(e);
			
			e = e->parent;
			while (e->tag != ET_ROOT) {
				ret = me_to_parent(e) * ret;
				e = e->parent;
			}
		} else {
			ret = hm::ident();
		}
		return ret;
	}
	hm calc_mat_parent_world (Entity const* e, hm* out_world_to_parent) {
		hm ret;
		hm inv;
		if (e->parent->tag != ET_ROOT) {
			e = e->parent;
			ret = me_to_parent(e);
			inv = parent_to_me(e);
			
			e = e->parent;
			while (e->tag != ET_ROOT) {
				ret = me_to_parent(e) * ret;
				inv = inv * parent_to_me(e);
				e = e->parent;
			}
		} else {
			ret = hm::ident();
			inv = hm::ident();
		}
		*out_world_to_parent = inv;
		return ret;
	}
	
	ui calc_depth (Entity const* e) {
		ui ret = 0;
		e = e->parent;
		while (e) { // root == level 0, this function can also be called on the root node
			++ret;
			e = e->parent;
		}
		return ret;
	}
	
	template <typename F>	void for_entity_subtree (Entity* e, F func) {
		func(e);
		
		e = e->children;
		while (e) {
			for_entity_subtree(e, func);
			
			e = e->next;
		}
	}
	template <typename F>	void for_entity_subtree_mat (Entity* e, hm mp parent_to_cam, hm mp cam_to_parent, F func) {
		hm mat = parent_to_cam * me_to_parent(e);
		hm inv = parent_to_me(e) * cam_to_parent;
		
		func(e, mat, inv, parent_to_cam);
		
		e = e->children;
		while (e) {
			for_entity_subtree_mat(e, mat, inv, func);
			
			e = e->next;
		}
	}
	
	DECL AABB calc_material_showcase_grid_aabb (Material_Showcase_Grid const* m) {
		
		assert(material_showcase_grid_steps.x >= 1 && material_showcase_grid_steps.y >= 1);
		
		mesh_id_e mesh_id = m->mesh_id;
		AABB aabb_mesh = meshes_aabb[mesh_id];
		
		aabb_mesh.xh += (f32)(material_showcase_grid_steps.x -1) * m->grid_offs.x;
		aabb_mesh.yh += (f32)(material_showcase_grid_steps.y -1) * m->grid_offs.y;
		
		return aabb_mesh;
	}
	
struct Entities {
	
	Root					root; // init to root node data
	
	void init () {
		
	}
	
	template <typename T, entity_tag TAG>
	T* make_entity (char const* name, v3 vp pos, quat vp ori, v3 vp scale=v3(1)) const {
		T* ret = working_stk.push<T>();
		ret->name =		name;
		
		ret->next =		nullptr;
		ret->parent =	nullptr;
		ret->children =	nullptr;
		
		ret->pos =		pos;
		ret->ori =		ori;
		ret->scale =	scale;
		
		ret->eflags =	(entity_flag)0;
		ret->tag =		TAG;
		
		//print(">>> Entity '%' at %\n", name, ret);
		return ret;
	}
	
	Mesh* mesh (char const* name, v3 vp pos, quat vp ori, mesh_id_e mesh_id) {
		auto* ret = make_entity<Mesh, ET_MESH>(name, pos, ori);
		ret->mesh_id = mesh_id;
		return ret;
	}
	Mesh* mesh (char const* name, v3 vp pos, quat vp ori, v3 vp scale, mesh_id_e mesh_id) {
		auto* ret = make_entity<Mesh, ET_MESH>(name, pos, ori, scale);
		ret->mesh_id = mesh_id;
		return ret;
	}
	
	Material_Showcase_Grid* material_showcase_grid (char const* name, v3 vp pos, quat vp ori, v3 vp scale,
			mesh_id_e mesh_id, v2 vp grid_offs) {
		auto* ret = make_entity<Material_Showcase_Grid, ET_MATERIAL_SHOWCASE_GRID>(name, pos, ori, scale);
		ret->mesh_id = mesh_id;
		ret->grid_offs = grid_offs;
		return ret;
	}
	
	Group* group (char const* name, v3 vp pos, quat vp ori) {
		auto* ret = make_entity<Group, ET_GROUP>(name, pos, ori);
		return ret;
	}
	
	Light_* dir_light (char const* name, v3 vp pos, quat vp ori, light_flags_e flags, v3 vp power) {
		auto* ret = make_entity<Light_, ET_LIGHT>(name, pos, ori);
		ret->type = LT_DIRECTIONAL;
		ret->flags = flags;
		ret->power = power;
		return ret;
	}
	Light_* point_light (char const* name, v3 vp pos, light_flags_e flags, v3 vp power) {
		auto* ret = make_entity<Light_, ET_LIGHT>(name, pos, quat::ident());
		ret->type = LT_POINT;
		ret->flags = flags;
		ret->power = power;
		return ret;
	}
	
	Scene* scene (char const* name, v3 vp pos, quat vp ori, bool draw=false) {
		auto* ret = make_entity<Scene, ET_SCENE>(name, pos, ori);
		
		ret->draw = draw;
		ret->lights.alloc(0, 8);
		
		return ret;
	}
	
	template <typename P, typename E, typename... Es>
	P* tree (P* parent, E* child0, Es... children) {
		Entity*	child[] = { child0, children... };
		
		parent->children = child[0];
		
		for (uptr i=0;;) {
			child[i]->parent = parent;
			
			if (++i == arrlenof(child)) break;
			
			child[i -1]->next = child[i];
		}
		
		assert(child[arrlenof(child) -1]->next == nullptr);
		
		return parent;
	}
	
	template <typename P, typename E, typename... Es>
	Scene* scene_tree (P* scn, E* child0, Es... children) {
		
		tree(scn, child0, children...);
		
		for_entity_subtree(scn,
			[&] (Entity const* e) {
				switch (e->tag) {
					case ET_LIGHT:	scn->lights.append((Light_*)e);		break;
					default: {}
				}
			}
		);
		
		return scn;
	}
	
	void test_enumerate (Entity* e) {
		for_entity_subtree(e,
			[&] (Entity const* e) {
				if (e == &root) {
					print("%'%'\n", repeat("  ", calc_depth(e)), e->name);
				} else {
					print("%'%' % %\n", repeat("  ", calc_depth(e)), e->name, e->pos, e->ori);
				}
			}
		);
	}
	
	void test_enumerate_all () {
		print(">>> Entity enumeration:\n");
		test_enumerate(&root);
	}
	
	void test_entities () {
		
		auto ON = (light_flags_e)0;
		auto OFF = LF_DISABLED;
		auto NOSHAD = LF_NO_SHADOW;
		
		auto shadow_test_pl = 1 ? (light_flags_e)0 : LF_NO_SHADOW;
		auto shadow_test_pl2 = 0 ? (light_flags_e)0 : LF_NO_SHADOW;
		auto shadow_test_dl = LF_DISABLED;
		
		auto* scn = scene("shadow_test_0",
				v3(-6.19f, +12.07f, +1.00f), quat::ident(), true);
			
			auto* lgh0 = dir_light("Test dir light",
					v3(+1.21f, -2.92f, +3.51f), quat(v3(+0.61f, +0.01f, +0.01f), +0.79f),
					ON,
					srgb(244.0f,217.0f,171.0f) * col(2000.0f));
			
			auto* msh0 = mesh("shadow_test_0",
					v3(+2.67f, +2.47f, +0.00f), quat(v3(+0.00f, -0.00f, -0.04), +1.00f),
					nouv_MSH_SHADOW_TEST_0);
				
				auto* lgh1 = point_light("Test point light 1",
						v3(+0.9410f, +1.2415f, +1.1063f),
						NOSHAD, srgb(200.0f,48.0f,79.0f) * col(100.0f));
				auto* lgh2 = point_light("Test point light 2",
						v3(+1.0914f, +0.5582f, +1.3377f),
						NOSHAD, srgb(48.0f,200.0f,79.0f) * col(100.0f));
				auto* lgh3 = point_light("Test point light 3",
						v3(+0.3245f, +0.7575f, +1.0226f),
						NOSHAD, srgb(48.0f,7.0f,200.0f) * col(100.0f));
				
			auto* msh1 = mesh("Window_Pillar",
					v3(-3.21f, +0.00f, +0.00f), quat(v3(-0.00f, +0.00f, -1.00f), +0.08f),
					nouv_MSH_WINDOW_PILLAR);
				
				auto* lgh4 = point_light("Torch light L",
						v3(-0.91969f, +0.610f, +1.880f),
						NOSHAD, srgb(240.0f,142.0f,77.0f) * col(60.0f));
				auto* lgh5 = point_light("Torch light R",
						v3(+0.91969f, +0.610f, +1.880f),
						NOSHAD, srgb(240.0f,142.0f,77.0f) * col(60.0f));
			
			auto* nano = group("Nanosuit",
					v3(+1.64f, +3.23f, +0.54f), quat(v3(+0.00f, +0.02f, +1.00f), +0.01f));
				
				auto* nano0 = mesh("Torso",		v3(0), quat::ident(), uv_tang_NANOSUIT_TORSO);
				auto* nano1 = mesh("Legs",		v3(0), quat::ident(), uv_tang_NANOSUIT_LEGS);
				auto* nano2 = mesh("Neck",		v3(0), quat::ident(), uv_tang_NANOSUIT_NECK);
				auto* nano3 = mesh("Helmet",	v3(0), quat::ident(), uv_tang_NANOSUIT_HELMET);
			
		auto* scn1 = scene("tree_scene",
				v3(-5.13f, -14.30f, +0.94f), quat(v3(-0.00f, -0.00f, +0.00f), +1.00f), true);
			
			auto* lgh10 = dir_light("Sun",
					v3(+0.06f, -2.76f, +4.53f), quat(v3(+0.31f, +0.01f, +0.04f), +0.95f),
					(light_flags_e)0,
					//LF_NO_SHADOW,
					srgb(244.0f,217.0f,171.0f) * col(2000.0f));
			auto* msh10 = mesh("terrain",
					v3(0), quat::ident(),
					nouv_MSH_TERRAIN);
				auto* msh11 = mesh("tree",
						v3(0), quat::ident(),
						uv_MSH_TERRAIN_TREE);
				auto* msh12 = mesh("tree_cuts",
						v3(0), quat::ident(),
						uv_MSH_TERRAIN_TREE_CUTS);
				auto* msh13 = mesh("tree_blossoms",
						v3(0), quat::ident(),
						uv_MSH_TERRAIN_TREE_BLOSSOMS);
				auto* msh14 = mesh("cube",
						v3(+0.97f, +1.54f, +0.90f),
						quat(v3(+0.305f, +0.344f, +0.054f), +0.886f),
						v3(0.226f),
						uv_MSH_TERRAIN_CUBE);
				auto* msh15 = mesh("sphere",
						v3(-1.49f, +1.45f, +0.86f),
						quat(v3(0.0f, 0.0f, -0.089f), +0.996f),
						v3(0.342f),
						uv_MSH_TERRAIN_SPHERE);
				auto* msh16 = mesh("obelisk",
						v3(-1.49f, -1.53f, +0.83f),
						quat(v3(+0.005f, +0.01f, -0.089f), +0.996f),
						uv_MSH_TERRAIN_OBELISK);
				auto* msh17 = mesh("teapot",
						v3(+1.49f, -1.445f, +0.284f),
						quat(v3(-0.025f, -0.055f, -0.561f), +0.826f),
						nouv_MSH_UTAHTEAPOT);
			
		auto* scn2 = scene("ugly_scene",
				v3(+3.70f, -10.70f, +0.94f), quat(v3(-0.00f, -0.00f, +0.00f), +1.00f), true);
			
			auto* lgh20 = dir_light("Sun",
					v3(-1.83f, +1.37f, +2.05f), quat(v3(+0.21f, -0.45f, -0.78f), +0.37f),
					(light_flags_e)0, srgb(244.0f,217.0f,171.0f) * col(2000.0f));
			auto* msh20 = mesh("ugly",
					v3(0), quat::ident(),
					uv_MSH_UGLY);
			
		auto* scn3 = scene("structure_scene",
				v3(0), quat::ident(), true);
			
			auto* lgh30 = dir_light("Sun",
					v3(-2.84f, +4.70f, +4.90f), quat(v3(+0.15f, -0.12f, -0.62f), +0.76f),
					(light_flags_e)0, srgb(244.0f,217.0f,171.0f) * col(2000.0f));
			auto* msh30 = mesh("ring",
					v3(0), quat::ident(),
					nouv_MSH_STRUCTURE_RING);
			auto* msh31 = mesh("walls",
					v3(0), quat::ident(),
					uv_MSH_STRUCTURE_WALLS);
			auto* msh32 = mesh("ground",
					v3(0), quat::ident(),
					uv_MSH_STRUCTURE_GROUND);
			auto* msh33 = mesh("block 1",
					v3(+1.52488f, +0.41832f, -3.31113f), quat(v3(-0.012f, +0.009f, -0.136f), +0.991f),
					uv_MSH_STRUCTURE_BLOCK1);
			auto* msh34 = mesh("block 2",
					v3(+3.05563f, +5.89111f, +0.53238f), quat(v3(-0.038f, -0.001f, +0.076f), +0.996f),
					uv_MSH_STRUCTURE_BLOCK2);
			auto* msh35 = mesh("block 3",
					v3(-2.84165f, +4.51917f, -2.67442f), quat(v3(-0.059f, -0.002f, +0.056f), +0.997f),
					uv_MSH_STRUCTURE_BLOCK3);
			auto* msh36 = mesh("block 4",
					v3(+0.69161f, +1.3302f, -2.57026f), quat(v3(-0.013f, +0.009f, -0.253f), +0.967f),
					uv_MSH_STRUCTURE_BLOCK4);
					
			auto* msh37 = group("beam",
					v3(-3.4297f, +1.47318f, -1.26951f), quat(v3(-0.088f, +0.017f, +0.996f), +0.0008585f));
				
				auto* msh37_0 = mesh("beam",
						v3(0), quat::ident(),
						uv_MSH_STRUCTURE_BEAM);
				auto* msh37_1 = mesh("cuts",
						v3(0), quat::ident(),
						uv_MSH_STRUCTURE_BEAM_CUTS);
			
		auto* scn4 = scene("normals",
				v3(-8.62f, +1.19f, +0.48f), quat(v3(-0.00f, -0.00f, +0.00f), +1.00f), false);
			
			auto* lgh40 = dir_light("Sun",
					v3(+2.29f, -0.11f, +2.77f), quat(v3(+0.17f, +0.27f, +0.80f), +0.51f),
					(light_flags_e)0, srgb(244.0f,217.0f,171.0f) * col(2000.0f));
			auto* msh40 = mesh("brick_wall",
					v3(+0.00f, +1.15f, +0.77f), quat(v3(+0.61f, +0.36f, +0.36f), +0.61f),
					uv_tang_MSH_UNIT_PLANE);
			auto* msh41 = mesh("weird plane",
					v3(-0.73f, -1.07f, +0.00f), quat(v3(+0.06f, +0.08f, +0.80f), +0.59f),
					uv_tang_NORM_TEST_00);
			
		auto* scn5 = scene("showcase",
				v3(+8.19f, +3.45f, +0.00f), quat(v3(-0.00f, -0.00f, +0.71f), +0.71f), false);
			
			auto* lgh50 = dir_light("Sun",
					v3(+2.86f, +0.17f, +3.35f), quat(v3(+0.06f, +0.49f, +0.86f), +0.11f),
					(light_flags_e)0, srgb(244.0f,217.0f,171.0f) * col(2000.0f));
			auto* msh50 = material_showcase_grid("ico_sphere",
					v3(-4.84f, -1.52f, +0.00f), quat::ident(), v3(0.3f),
					nouv_MSH_ICO_SPHERE, v2(2.5f));
			auto* msh51 = material_showcase_grid("bunny",
					v3(+2.39f, +0.00f, +0.00f), quat::ident(), v3(2.0f),
					nouv_MSH_STFD_BUNNY, v2(0.375f));
			auto* msh52 = material_showcase_grid("buddha",
					v3(+5.00f, +0.00f, +0.00f), quat::ident(), v3(1),
					nouv_MSH_STFD_BUDDHA, v2(0.85f));
			auto* msh53 = material_showcase_grid("dragon",
					v3(+0.04f, +7.95f, +0.00f), quat::ident(), v3(1),
					nouv_MSH_STFD_DRAGON, v2(0.65f));
			auto* msh54 = material_showcase_grid("teapot",
					v3(+2.09f, +7.92f, +0.00f), quat::ident(), v3(2.5f),
					nouv_MSH_UTAHTEAPOT, v2(0.3f));
			
		tree(&root,
			scene_tree(scn,
				lgh0,
				tree(msh0,
					lgh1,
					lgh2,
					lgh3
				),
				tree(msh1,
					lgh4,
					lgh5
				),
				tree(nano,
					nano0,
					nano1,
					nano2,
					nano3
				)
			),
			scene_tree(scn1,
				lgh10,
				tree(msh10,
					msh11,
					msh12,
					msh13,
					msh14,
					msh15,
					msh16,
					msh17
				)
			),
			scene_tree(scn2,
				lgh20,
				msh20
			),
			scene_tree(scn3,
				lgh30,
				msh30,
				msh31,
				msh32,
				msh33,
				msh34,
				msh35,
				msh36,
				tree(msh37,
					msh37_0,
					msh37_1
				)
			),
			scene_tree(scn4,
				lgh40,
				msh40,
				msh41
			),
			scene_tree(scn5,
				lgh50,
				msh50,
				msh51,
				msh52,
				msh53,
				msh54
			)
		);
		
		#if 0
		{
			{
				
				for (auto i=MAT_SHOW_FIRST; i<MAT_SHOW_END; ++i) {
					
					f32 offs = (f32)(i -MAT_SHOW_FIRST) * showcase.materials.grid_offs;
					
					draw(nouv_MSH_ICO_SPHERE,
							m * translate_h(showcase.materials.pos_world) * hm::ident().m3( conv_to_m3(showcase.materials.ori_world) ) *
							translate_h(v3(offs, 0, 0)) * scale_h(v3(0.3f)),
							&materials[i]);
				}
			}
			
			glBindVertexArray(VAOs[UV_TANG_VAO]);
			glUseProgram(shaders[SHAD_PBR_DEV_CERBERUS]);
			
			glActiveTexture(GL_TEXTURE0 +TEX_UNIT_CERB_ALBEDO);
			tex.set_texture(GL_TEXTURE_2D, TEX_CERBERUS_ALBEDO);
			
			glActiveTexture(GL_TEXTURE0 +TEX_UNIT_CERB_NORMAL);
			tex.set_texture(GL_TEXTURE_2D, TEX_CERBERUS_NORMAL);
			
			glActiveTexture(GL_TEXTURE0 +TEX_UNIT_CERB_METALLIC);
			tex.set_texture(GL_TEXTURE_2D, TEX_CERBERUS_METALLIC);
			
			glActiveTexture(GL_TEXTURE0 +TEX_UNIT_CERB_ROUGHNESS);
			tex.set_texture(GL_TEXTURE_2D, TEX_CERBERUS_ROUGHNESS);
			
			draw(uv_tang_CERBERUS,
					m * translate_h(showcase.cerberus.pos_world) * hm::ident().m3( conv_to_m3(showcase.cerberus.ori_world) )
					//* scale_h(v3(1.0f)),
					,
					&materials[MAT_IDENTITY]);
		}
		#endif
		
		//test_enumerate();
		//
		//print(">>> Meshes:\n");
		//scenes[0]->meshes.templ_forall( [] (Mesh* e) { print("  %\n", e->name); } );
		//
		//print(">>> Lights:\n");
		//scenes[0]->lights.templ_forall( [] (Light_* e) { print("  %\n", e->name); } );
		
	}
	
	entity_flag _calc_mesh_aabb (Entity* e, hm mp parent_to_world) {
		
		entity_flag ret = (entity_flag)0;
		
		AABB aabb_mesh;
		
		switch (e->tag) {
			case ET_MESH: {
				auto* m = (Mesh*)e;
				mesh_id_e mesh_id = m->mesh_id;
				aabb_mesh = meshes_aabb[mesh_id];
				ret = EF_HAS_MESHES;
			} break;
			
			case ET_MATERIAL_SHOWCASE_GRID: {
				aabb_mesh = calc_material_showcase_grid_aabb((Material_Showcase_Grid*)e);
				ret = EF_HAS_MESHES;
			} break;
			
			case ET_LIGHT: {
				auto* l = (Light_*)e;
				mesh_id_e mesh_id;
				switch (l->type) {
					case LT_DIRECTIONAL:	mesh_id = nouv_MSH_SUN_LAMP_MSH;	break;
					case LT_POINT:			mesh_id = nouv_MSH_LIGHT_BULB;		break;
					default: assert(false); mesh_id = (mesh_id_e)0;
				}
				aabb_mesh = meshes_aabb[mesh_id];
				ret = EF_HAS_MESHES;
			} break;
			
			case ET_GROUP: {
			case ET_SCENE:
				aabb_mesh = AABB::inf();
			} break;
				
			default: assert(false);
		}
		
		hm to_parent = me_to_parent(e);
		hm to_world = parent_to_world * to_parent;
		
		if (ret) {
			auto aabb_mesh_corners = aabb_mesh.box_corners();
			
			e->aabb_mesh_paren = AABB::from_obb(to_parent * aabb_mesh_corners);
			e->aabb_mesh_world = AABB::from_obb(to_world * aabb_mesh_corners);
			
			//dbg_lines.push_box_world(parent_to_world * to_parent, aabb_mesh.box_edges(), v3(0.2f));
		} else {
			e->aabb_mesh_paren = AABB::inf();
			e->aabb_mesh_world = AABB::inf();
		}
		
		auto* c = e->children;
		while (c) {
			auto res = _calc_mesh_aabb(c, to_world);
			if (res) {
				ret |= res;
				
				e->aabb_mesh_paren.minmax( to_parent * c->aabb_mesh_paren.box_corners() );
				e->aabb_mesh_world.minmax( c->aabb_mesh_world );
			}
			
			c = c->next;
		}
		
		if (ret) {
		//	dbg_lines.push_box_world(parent_to_world, e->aabb_mesh_paren.box_edges(), v3(1.0f));
		//	dbg_lines.push_box_world(hm::ident(), e->aabb_mesh_world.box_edges(), v3(0.25f));
		}
		
		e->eflags &= ~EF_HAS_MESHES;
		e->eflags |= ret;
		return ret;
	}
	void calc_mesh_aabb () {
		PROFILE_SCOPED(THR_ENGINE, "entities_calc_mesh_aabb");
		
		auto* e = root.children;
		while (e) {
			_calc_mesh_aabb(e, hm::ident());
			
			e = e->next;
		}
	}
	
};
	
	AABB calc_shadow_cast_aabb (Entity const* e, hm mp parent_to_light) {
		
		AABB aabb_me;
		
		switch (e->tag) {
			case ET_MESH: {
				auto* m = (Mesh*)e;
				mesh_id_e mesh_id = m->mesh_id;
				aabb_me = meshes_aabb[mesh_id];
			} break;
			case ET_MATERIAL_SHOWCASE_GRID: {
				aabb_me = calc_material_showcase_grid_aabb((Material_Showcase_Grid*)e);
			} break;
			
			case ET_GROUP:
			case ET_LIGHT:
			case ET_SCENE: {
				aabb_me = AABB::inf();
			} break;
				
			default: assert(false);
		}
		
		hm me_to_light = parent_to_light * me_to_parent(e);
		
		AABB aabb_light;
		if (!aabb_me.is_inf()) {
			aabb_light = AABB::from_obb(me_to_light * aabb_me.box_corners());
		} else {
			aabb_light = aabb_me; // inf
		}
		
		auto* c = e->children;
		while (c) {
			auto child_aabb_light = calc_shadow_cast_aabb(c, me_to_light);
			if (!child_aabb_light.is_inf()) {
				aabb_light.minmax( child_aabb_light ); // child_aabb_light might be inf
			}
			
			c = c->next;
		}
		
		return aabb_light;
	}
	
////
	DECLD Entities				entities;
	
	DECLD Lights				lights;
	
	hm inverse_transl_ori (hm mp m) { // faster inverse for rotate->translate matricies
		return hm::ident().m3(transpose(m.m3())) * translate_h(-m.column(3).xyz());
	}
	
	struct Ray {
		v3	pos;
		v3	dir; // not normalized
	};
	Ray target_ray_from_screen_pos (v2 vp window_pos, v2 vp window_res, Camera cr cam) { // result in cam space
		v2 temp = window_pos / window_res;
		temp = (temp * v2(2.0f)) -v2(1.0f);
		temp *= cam.frust_scale;
		Ray ret;
		ret.dir = v3(temp, -1.0f);
		ret.pos = ret.dir * v3(cam.clip_near); // ray.pos is on near clip plane
		return ret;
	}
	Ray target_ray_from_screen_pos (v2 vp window_pos, v2 vp window_res, Camera cr cam, hm cam_to_space) {
		Ray ret = target_ray_from_screen_pos(window_pos, window_res, cam);
		ret.pos = (cam_to_space * hv(ret.pos)).xyz();
		ret.dir = cam_to_space.m3() * ret.dir;
		return ret;
	}
	v2 project_point_onto_screen (v3 vp cam, m4 mp camera_to_clip, v2 res) { 
		v4 clip = camera_to_clip * v4(cam, 1.0f);
		v2 ndc = clip.xy() / v2(clip.w);
		v2 screen = ((ndc +v2(1.0f)) * v2(0.5f)) * res;
		return screen;
	}
	f32 sphere_ray_intersect (v3 vp sphere_pos, f32 sphere_radius, v3 vp ray_pos, v3 vp ray_dir) {
		v3 sphere_pos_rel = sphere_pos -ray_pos;
		f32 dot_sphere_onto_ray = dot(sphere_pos_rel, ray_dir);
		if (dot_sphere_onto_ray >= 0.0f) {
			
			v3 closest_pos_rel = v3(dot_sphere_onto_ray) * ray_dir;
			v3 v_to_sphere = sphere_pos_rel -closest_pos_rel;
			f32 dist_to_sphere = length(v_to_sphere);
			if (dist_to_sphere <= sphere_radius) {
				
				f32 dist_into_sphere = fp::sqrt(SQUARED(sphere_radius) -SQUARED(dist_to_sphere));
				f32 dist_to_first_intersect = dot_sphere_onto_ray -dist_into_sphere;
				return dist_to_first_intersect;
			}
		}
		return fp::inf<f32>();
	}
	bool ray_line_closest_intersect (v3 vp posRay, v3 vp dirRay, v3 vp posLine, v3 vp dirLine,
			v3* outIntersect) {
		
		v3 crossDir = cross(dirRay, dirLine);
		f32 divisor = length_sqr(crossDir);
		
		if (divisor == 0.0f) {
			// target ray and axis line are exactly parallel
			return false;
		}
		
		f32 invDivisor = 1.0f / divisor;
		v3 diffPos = posLine -posRay;
		
		m3 m = m3::column( diffPos, dirLine, crossDir );
		f32 tRay = determinant(m);
		tRay *= invDivisor;
		
		if (tRay < 0.0f) {
			// target ray points away from axis line (intersection would be backwards on the ray (behind the camera))
			return false;
		}
		
		m.column(1, dirRay);
		f32 tLine = determinant(m);
		tLine *= invDivisor;
		
		*outIntersect = posLine +(dirLine * v3(tLine));
		return true;
	}
	
	struct Intersect {
		f32	dist;
		v3	pos;
	};
	
	Intersect ray_triangle_intersect (v3 vp tri_a, v3 vp tri_b, v3 vp tri_c, v3 vp ray_pos, v3 vp ray_dir) {
		// Copied from http://geomalgorithms.com/a06-_intersect-2.html
		// ray_dir can be any length vec
		
		v3 u = tri_b -tri_a; // triangle edge 1
		v3 v = tri_c -tri_a; // triangle edge 2
		v3 n = cross(u, v); // triangle normal
		
		f32 proj_div = dot(n, ray_dir);
		if (proj_div == 0.0f) { // TODO: EPSILON?
			return { fp::inf<f32>(), v3(fp::qnan<f32>()) }; // ray parallel to triangle
		}
		
		f32 proj_num = dot(n, tri_a -ray_pos);
		f32 ri = proj_num / proj_div;
		if (ri < 0.0f) {
			return { fp::inf<f32>(), v3(fp::qnan<f32>()) }; // intersection on backwards part of ray
		}
		
		v3 p = ray_pos +ray_dir * v3(ri); // intersection point on triangle plane
		
		v3 w = p -tri_a;
		
		f32 uv = dot(u, v);
		f32 uu = dot(u, u);
		f32 vv = dot(v, v);
		f32 wu = dot(w, u);
		f32 wv = dot(w, v);
		
		f32 inv_div = 1.0f / (uv*uv -uu*vv); // should never be zero for non-degenerate triangles
		
		f32 s = (uv*wv -vv*wu) * inv_div;
		f32 t = (uv*wu -uu*wv) * inv_div;
		
		if (s < 0.0f || t < 0.0f || (s +t) > 1.0f) {
			return { fp::inf<f32>(), v3(fp::qnan<f32>()) }; // Intersection outside of triangle
		} 
		
		return { ri, p };
	}
	
	v3 triangle_calc_barycentric_coords (v3 vp p, v3 vp tri_a, v3 vp tri_b, v3 vp tri_c) {
		
		v3 u = tri_b -tri_a; // triangle edge 1
		v3 v = tri_c -tri_a; // triangle edge 2
		
		v3 w = p -tri_a;
		
		f32 uv = dot(u, v);
		f32 uu = dot(u, u);
		f32 vv = dot(v, v);
		f32 wu = dot(w, u);
		f32 wv = dot(w, v);
		
		f32 inv_div = 1.0f / (uv*uv -uu*vv); // should never be zero for non-degenerate triangles
		
		v3 ret;
		ret.y = (uv*wv -vv*wu) * inv_div;
		ret.z = (uv*wu -uu*wv) * inv_div;
		ret.x = 1.0f -ret.y -ret.z;
		return ret;
	}
	
	DECLD struct Editor {
		
		bool				highl_manipulator;
		
		Entity*				selected =					nullptr;
		Entity*				highl =						nullptr;
		
		bool				dragging =					false;
		v3					select_offs_paren =			v3(0);
		v3					selected_pos_paren =		v3(0);
		
		DECLM void do_entities (hm mp world_to_cam, hm mp cam_to_world, Inp cr inp, SInp cr sinp,
				Camera cr cam, Lights* lights, Entities* entities) {
			PROFILE_SCOPED(THR_ENGINE, "editor_do_entities");
			
			if (inp.mouselook & FPS_MOUSELOOK) {
				dragging = false; // This is kinda ungraceful
				highl_manipulator = false;
				highl = nullptr;
			} else {
				
				v2 f_mouse_cursor_pos = cast_v2<v2>(sinp.mouse_cursor_pos);
				v2 f_window_res = cast_v2<v2>(sinp.window_res);
				
				Entity* highl_ = nullptr;
				
				{ // Test entities for cursor highlight
					auto ray_world = target_ray_from_screen_pos(f_mouse_cursor_pos, f_window_res, cam, cam_to_world);
					
					f32 least_t = fp::inf<f32>();
					
					v3 ray_dir_inv = v3(1.0f) / ray_world.dir; 
					
					auto for_entities = [&] (Entity* e) {
						
						//print("%'%' %\n", repeat("  ", calc_depth(e)), e->name, (u32)e->eflags);
						
						if (!(e->eflags & EF_HAS_MESHES)) return;
						if (e == selected) return; // can't highlight selected entity to make selection smoother
						
						AABB aabb = e->aabb_mesh_world;
						
						{ // AABB ray intersection test
							f32 tmin;
							f32 tmax;
							{ // X
								f32 tl = (aabb.xl -ray_world.pos.x) * ray_dir_inv.x;
								f32 th = (aabb.xh -ray_world.pos.x) * ray_dir_inv.x;
								
								tmin = fp::MIN(tl, th);
								tmax = fp::MAX(tl, th);
							}
							{ // Y
								f32 tl = (aabb.yl -ray_world.pos.y) * ray_dir_inv.y;
								f32 th = (aabb.yh -ray_world.pos.y) * ray_dir_inv.y;
								
								// fix for edge cases where SSE min/max behavior of NaNs matters
								tmin = fp::MAX( tmin, fp::MIN(fp::MIN(tl, th), tmax) );
								tmax = fp::MIN( tmax, fp::MAX(fp::MAX(tl, th), tmin) );
							}
							{ // Z
								f32 tl = (aabb.zl -ray_world.pos.z) * ray_dir_inv.z;
								f32 th = (aabb.zh -ray_world.pos.z) * ray_dir_inv.z;
								
								// fix for edge cases where SSE min/max behavior of NaNs matters
								tmin = fp::MAX( tmin, fp::MIN(fp::MIN(tl, th), tmax) );
								tmax = fp::MIN( tmax, fp::MAX(fp::MAX(tl, th), tmin) );
							}
							
							bool intersect = tmax > fp::MAX(tmin, 0.0f);
							if (intersect) {
								if (tmin < least_t) {
									if (tmin < 0.0f) {
										least_t = tmax;
									} else {
										least_t = tmin;
									}
									highl_ = e;
								}
							}
						}
						
					};
					for_entity_subtree(&entities->root, for_entities);
					
					#if 0
					if (highl_) {
						print("highl: '%' least_t: %\n", highl_->name, least_t);
						
						dbg_lines.push_cross(v3(least_t) * ray.dir +ray.pos, v3(1));
					}
					#endif
					
				}
				
				bool highl_manipulator_ = false;
				
				// Axis cross manipulator processing
				v3 selectOffset_paren_;
				v3 selected_pos_paren_;
				
				if (selected) {
					
					// Place manipulator at location of selected draggable
					
					hm cam_to_paren;
					hm paren_to_cam;
					{
						hm world_to_paren;
						hm paren_to_world = calc_mat_parent_world(selected, &world_to_paren);
						
						paren_to_cam = world_to_cam * paren_to_world;
						cam_to_paren = world_to_paren * cam_to_world;
					}
					
					v3 pos_cam = (paren_to_cam * hv(selected->pos)).xyz();
					f32 zDist_cam = -pos_cam.z;
					
					bool onscreen = zDist_cam > 0.0f;
					
					f32 fovScaleCorrect = fp::tan(cam.vfov / 2.0f) / 6.0f;
					f32 scaleCorrect = zDist_cam * fovScaleCorrect;
					
					constexpr f32 minHighlightDist = 8.0f;
					
					v2	axisBase = project_point_onto_screen(pos_cam, cam.cam_to_clip, f_window_res);
					v2	cursorRel = f_mouse_cursor_pos -axisBase;
					
					if (onscreen) {
						
						constexpr u32 NO_AXIS = u32(-1);
						u32 highlAxis = NO_AXIS;
						
						f32	highlDist;
						v2	highlAxisDir;
						f32	highlAlongAxis;
						
						for (u32 axis=0; axis<3; ++axis) {
							
							v2	axisDir;
							f32	axisPLen;
							f32	axisNLen;
							{
								v2	axisP;
								v2	axisN;
								{ // axis cross arrow tips (postive and negative for xyz)
									v3 v = v3(0);
									v[axis] = +1.0625f * scaleCorrect;
									v3 temp = (paren_to_cam * hv(selected->pos +v)).xyz();
									axisP = project_point_onto_screen(temp, cam.cam_to_clip, f_window_res) -axisBase;
									v = v3(0);
									v[axis] = -0.75f * scaleCorrect;
									temp = (paren_to_cam * hv(selected->pos +v)).xyz();
									axisN = project_point_onto_screen(temp, cam.cam_to_clip, f_window_res) -axisBase;
								}
								axisNLen = length(axisN);
								axisDir = axisP;
							}
							
							bool straightIntoCam = !normalize_or_zero(&axisDir, &axisPLen);
							
							assert(straightIntoCam == (axisNLen == 0.0f), "% %", straightIntoCam, axisNLen); // This asserted once when setting cam pos equal to a entity pos and then moving
							
							if (!straightIntoCam) {
								
								v2 axisOrth = v2(-axisDir.y, +axisDir.x);
								
								f32 alongAxis = dot(cursorRel, axisDir);
								f32 orthToAxis = dot(cursorRel, axisOrth);
								
								// Shortest 2d pixelspace distance to projected axis line segment
								f32 dist;
								if (alongAxis < -axisNLen) {
									dist = length( v2(alongAxis +axisNLen, orthToAxis) );
								} else if (alongAxis > axisPLen) {
									dist = length( v2(alongAxis -axisPLen, orthToAxis) );
								} else {
									dist = fp::ABS(orthToAxis);
								}
								
								if (dist <= minHighlightDist && (highlAxis == NO_AXIS || dist < highlDist)) {
									highlAxis = axis;
									highlDist = dist;
									highlAxisDir = axisDir;
									highlAlongAxis = alongAxis;
								} else {
									// Dont try to highlight current axis
								}
							}
						}
						
						if (highlAxis != NO_AXIS) {
							
							Ray targetRay_paren = target_ray_from_screen_pos(
									axisBase +(highlAxisDir * v2(highlAlongAxis)),
									f_window_res, cam, cam_to_paren);
							
							v3 axisDir_paren = v3(0);
							axisDir_paren[highlAxis] = +1.0f;
							
							v3 intersect;
							bool intersectValid = ray_line_closest_intersect(
									targetRay_paren.pos, targetRay_paren.dir,
									selected->pos, axisDir_paren, &intersect);
							assert(intersectValid);
							if (!intersectValid) {
								highlAxis = NO_AXIS; // Ignore selection by pretending no axis was highlighted
							}
							selectOffset_paren_ = intersect;
							selected_pos_paren_ = selected->pos;
						}
						
						if (highlAxis != NO_AXIS) { // Override highlighting of other entities
							highl_ = (Entity*)(uptr)(highlAxis +1);
							highl_manipulator_ = true;
						}
					}
					
					if (dragging) {
						
						assert(highl_manipulator);
						u32 axis = ((u32)(uptr)highl) -1;
						
						v3	axisDir_paren;
						v2	axisDir_screen;
						{
							v3 v = v3(0);
							v[axis] = +1.0f;
							v3 temp = (paren_to_cam * hv(selected->pos +v)).xyz();
							axisDir_screen = project_point_onto_screen(temp, cam.cam_to_clip,
									f_window_res) -axisBase;
							axisDir_paren = v;
						}
						
						if (normalize_or_zero(&axisDir_screen)) {
							
							f32 alongAxis = dot(cursorRel, axisDir_screen);
							
							auto targetRay_paren = target_ray_from_screen_pos(
									axisBase +(axisDir_screen * v2(alongAxis)),
									f_window_res, cam, cam_to_paren);
							
							v3 intersect;
							bool intersectValid = ray_line_closest_intersect(
									targetRay_paren.pos, targetRay_paren.dir,
									select_offs_paren, axisDir_paren,
									&intersect);
							
							if (intersectValid) {
								selected->pos = selected_pos_paren +(intersect -select_offs_paren);
							} else {
								// Dont move axis cross
							}
						} else {
							// axis points directly into camera
						}
					}
					
				}
				
				if (!dragging) {
					if (!sinp.mouse_cursor_in_window) {
						highl = nullptr;
						highl_manipulator = false;
					} else {
						bool is_down =		inp.editor_click_select_state == true;
						bool went_down =	is_down && inp.editor_click_select_counter > 0;
						
						if (went_down) {
							
							if (highl_) {
								
								if (!highl_manipulator_) {
									selected = highl_;
									
									print("Moveable '%' selected at local pos %.\n",
											selected->name, selected->pos);
								} else {
									
									dragging = true;
									select_offs_paren = selectOffset_paren_;
									selected_pos_paren = selected_pos_paren_;
									
								}
								
							} else {
								// Unselect selection
								selected = nullptr;
							}
						}
						
						if (is_down && !went_down) {
							// Keep highlighting while button is held down
						} else {
							highl = highl_;
							highl_manipulator = highl_manipulator_;
						}
					}
				} else {
					assert(highl_manipulator);
					
					bool up = inp.editor_click_select_counter > 0 &&
							inp.editor_click_select_state == false;
					if (up) {
						dragging = false;
						
						print("Entity '%' moved to local pos %.\n", selected->name, selected->pos);
					}
				}
			}
		}
	} editor;
	
	DECLD std140_Material		materials[MAT_COUNT] =			{}; // init to zero except for default values
	
////
	
	DECL m3 aer_world_to_cam_m (v3 vp aer) {
		// 0,0,0 points down in world (this is how camera space works, since we have z-up instead of y-up),
		//  pitch range: [0,180]
		// note the reverse order of rotations (still zxz because zxz mirrored is zxz)
		#if 0
		m3 ret;
		ret =	rotate_Z3(aer.z);
		ret *=	rotate_X3(aer.y);
		ret *=	rotate_Z3(aer.x);
		return ret;
		#else
		v3 aer_ = aer;
		//aer_.z = deg(30.0f);
		
		//m3 test;
		//test =	rotationZ3(aer_.z);
		//test *=	rotationX3(aer_.y);
		//test *=	rotationZ3(aer_.x);
		
		m3 ret = mat_op::compute_euler_angle_zxz_m3(v3(aer_.z, aer_.y, aer_.x));
		
		//print("test:\n%\n%\n", test, ret);
		
		return ret;
		#endif
	}
	DECL m3 aer_horizontal_m (v3 vp aer) { // 0,0,0 points +y in world, pitch range: [-90,+90]
		#if 0
		m3 ret;
		ret =	rotationZ3(aer.x);
		ret *=	rotationX3(aer.y);
		ret *=	rotationY3(aer.z);
		return ret;
		#else
		
		//m3 test;
		//test =	rotationZ3(aer.x);
		//test *=  rotationX3(aer.y);
		//test *=	rotationY3(aer.z);
		
		m3 ret = mat_op::compute_euler_angle_zxy_m3(aer);
		
		//print("test:\n%\n%\n", test, ret);
		
		return ret;
		#endif
	}
	DECL quat aer_camera_q (v3 vp aer) {
		#if 0
		quat ret;
		ret =	rotationZQuat(aer.z);
		ret *=	rotationXQuat(aer.y);
		ret *=	rotationZQuat(aer.x);
		return ret;
		#else // easily optimized by removing zero products
		return quat_op::compute_euler_angle_zxz_q(v3(aer.z, aer.y, aer.x));
		#endif
	}
	DECL quat aer_horizontal_q_from_euler (v3 vp aer) {
		quat ret;
		ret =	rotate_ZQuat(aer.x);
		ret *=	rotate_XQuat(aer.y);
		ret *=	rotate_YQuat(aer.z);
		return ret;
	}
	DECL v3 aer_horizontal_q_to_euler (quat vp q) {
		
		// Implementation derived with help of this paper: https://www.geometrictools.com/Documentation/EulerAngles.pdf
		//  and the modified/optimized result matches the form found at https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
		// This does have drift when elev is close to +/- 90deg (noticable >89.5deg) (some angle value drives over the course of quat->euler->quat)
		// And obviously with euler angles there are singularities where angles lose all precision (and so their information)
		//  and at the singularity can possibly also become NaN if not accounted for (in this impl asin() has it's imput clamped and atan2 does not seem to return NaN (it seems like it returns 0 ie. garbage but non-NaN data))
		
	#if 0
		m3 m = conv_to_m3(q);
		
		v3 ret;
		#if 0	// These cases seem to be completely unnecessary,
				//  since they only seem to prevent out of range errors (nan) in the asin function, which can simply be handled with a clamp since values should only ever be epsilon over/under +/-1
				//  atan2 seems to always behave properly
		if (		m.arr[1].z <= -1.0f ) {
			print("  if 1");
			ret.x = -fp::atan2(m.arr[2].x, m.arr[0].x);
			ret.y = -deg(90.0f);
			ret.z = 0.0f;
		} else if (	m.arr[1].z >= -1.0f ) {
			print("  if 2");
			ret.x = +fp::atan2(m.arr[2].x, m.arr[0].x);
			ret.y = +deg(90.0f);
			ret.z = 0.0f;
		} else {
			print("  else");
		#endif
		
			ret.x = fp::atan2(-m.arr[1].x, m.arr[1].y);
			ret.y = fp::asin(m.arr[1].z);
			ret.z = fp::atan2(-m.arr[0].z, m.arr[2].z);
		
		#if 0
		}
		#endif
		return ret;
	#else
		
		f32 zw = q.z * q.w;
		f32 xy = q.x * q.y;
		f32 yz = q.y * q.z;
		f32 xw = q.x * q.w;
		f32 yw = q.y * q.w;
		f32 xz = q.x * q.z;
		f32 xx = q.x * q.x;
		f32 yy = q.y * q.y;
		f32 zz = q.z * q.z;
		
		v3 ret;
		ret.x = fp::atan2(2.0f * (zw -xy), 1.0f -( 2.0f * (xx +zz) ));
		ret.y = fp::asin(fp::clamp(2.0f * (yz +xw), -1.0f, +1.0f));
		ret.z = fp::atan2(2.0f * (yw -xz), 1.0f -( 2.0f * (xx +yy) ));
		return ret;
		
	#endif
	}
	DECL v3 aer_horizontal_q_to_euler_geom (quat vp q) {
		
		// Implementation derived with help of this paper: http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/quat_2_euler_paper_ver2-1.pdf
		// This geometric conversions seems a bit more stable when approaching the singularities (gibal lock)
		//   (It does still drift when elev is close to +/- 90deg (noticable >89.5deg))
		//  possibly because every the second angle is calculated from the first and the third from the first two
		//  although in that case this still has the problem of precision loss (possibly even nan?) in on the first angle (atan2)
		
		#if 0
		quat Qg = q;
		
		v3 V3 = v3(0,1,0);
		
		v3 V3rot = Qg * V3;
		
		f32 ang1 = fp::atan2(-V3rot.x, +V3rot.y); // does not ever seen to return nan, so just let it return noisy/garbage angle for now
		f32 ang2 = fp::asin(clamp(+V3rot.z, -1.0f, +1.0f));
		
		quat Q1 = rotate_ZQuat(ang1);
		quat Q2 = rotate_XQuat(ang2);
		
		quat Q12 = Q1 * Q2;
		
		v3 V3n = v3(0,0,1);
		
		v3 V3n12 = Q12 * V3n;
		v3 V3nG = Qg * V3n;
		
		f32 cos_ang = dot(V3n12, V3nG);
		f32 mag = fp::acos(sse::clamp(cos_ang, -1.0f, +1.0f));
		
		v3 Vc = cross(V3n12, V3nG);
		f32 m = dot(Vc, V3rot);
		
		f32 ang3 = fp::set_sign(mag, fp::get_sign(m));
		
		return v3(ang1, ang2, ang3);
		
		#else
		// ret.x = (l.y * r.z) -(l.z * r.y);
		// ret.y = (l.z * r.x) -(l.x * r.z);
		// ret.z = (l.x * r.y) -(l.y * r.x);
		// 
		// 
		// tv3<T> qv = tv3<T>(q.x, q.y, q.z);
		// tv3<T> uv = vec_op::cross(qv, v);
		// tv3<T> uuv = vec_op::cross(qv, uv);
		// 
		// return v +::operator*( (::operator*(uv, tv3<T>(q.w)) +uuv), tv3<T>(2) );
		
		v3 V3rot;
		V3rot.x = ( ((q.y * q.x) -(q.z * q.w)) *  2.0f );
		V3rot.y = ( ((q.z * q.z) +(q.x * q.x)) * -2.0f ) +1.0f;
		V3rot.z = ( ((q.x * q.w) +(q.y * q.z)) *  2.0f );
		
		v3 aer;
		aer.x = fp::atan2(-V3rot.x, +V3rot.y);
		aer.y = fp::asin(clamp(+V3rot.z, -1.0f, +1.0f));
		
		auto t1 = fp::sincos(aer.x / 2.0f);
		auto t2 = fp::sincos(aer.y / 2.0f);
		
		v3 V3n12;
		v3 V3nG;
		{
			
			f32 t1s1c = t1.s * t1.c;
			f32 t1s1s = t1.s * t1.s;
			f32 t1c1c = t1.c * t1.c;
			
			f32 t2s2c = t2.s * t2.c;
			f32 t2s2s = t2.s * t2.s;
			f32 t2c2s = t2.c * t2.s;
			
			V3n12.x = ( ((t1s1c * t2s2c) +(t1s1c * t2c2s)) *  2.0f );
			V3n12.y = ( ((t1s1s * t2s2c) -(t1c1c * t2c2s)) *  2.0f );
			V3n12.z = ( ((t1c1c * t2s2s) +(t1s1s * t2s2s)) * -2.0f ) +1.0f;
			
			
			V3nG.x = ( ((q.z * q.x) +(q.y * q.w)) *  2.0f );
			V3nG.y = ( ((q.z * q.y) -(q.x * q.w)) *  2.0f );
			V3nG.z = ( ((q.x * q.x) +(q.y * q.y)) * -2.0f ) +1.0f;
			
		}
		
		f32 cos_ang = dot(V3n12, V3nG);
		f32 mag = fp::acos(fp::clamp(cos_ang, -1.0f, +1.0f));
		
		v3 Vc = cross(V3n12, V3nG);
		f32 m = dot(Vc, V3rot);
		
		aer.z = fp::set_sign(mag, fp::get_sign(m));
		
		return aer;
		#endif
	}
	
//// Var file
#include "var_file.hpp"
	
DECL void init () {
	PROFILE_SCOPED(THR_ENGINE, "engine_init");
	
	{
		PROFILE_SCOPED(THR_ENGINE, "test_entities");
		entities.test_entities();
	}
	
	{
		PROFILE_SCOPED(THR_ENGINE, "var_file_setup");
		{
			PROFILE_SCOPED(THR_ENGINE, "var_file_cmd_pipe_setup");
			{
				auto ret = CreateNamedPipe(CMD_PIPE_NAME, PIPE_ACCESS_INBOUND|FILE_FLAG_OVERLAPPED,
						PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_WAIT, 1, kibi(1), kibi(1), 0, NULL);
				if (ret == INVALID_HANDLE_VALUE) {
					warning("CreateNamedPipe(\"%\", ...) failed [%], "
							"possibly because another instance is already running, pipe commands will not work.\n",
							CMD_PIPE_NAME, GetLastError());
				}
				cmd_pipe_handle = ret;
			}
			
			{
				auto ret = CreateEventW(NULL, FALSE, TRUE, NULL);
				assert(ret != NULL);
				cmd_pipe_ov.hEvent = ret;
			}
			
			if (cmd_pipe_handle != INVALID_HANDLE_VALUE) {
				auto ret = ConnectNamedPipe(cmd_pipe_handle, &cmd_pipe_ov);
				
				if (ret != 0) {
					print("inital ConnectNamedPipe() completed instantly\n");
				} else {
					auto err = GetLastError();
					assert(err == ERROR_IO_PENDING, "%", err);
					//print("inital ConnectNamedPipe() pending\n");
				}
			}
		}
		
		{
			using namespace var;
			{
				PROFILE_SCOPED(THR_ENGINE, "var_file_open");
				
				init_vars();
				
				assert(!win32::open_existing_file(ALL_VAR_FILENAME, &var_file_h,
						GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE));
				
				assert(GetFileTime(var_file_h, NULL, NULL, &var_file_filetime) != 0);
			}
			{
				PROFILE_SCOPED(THR_ENGINE, "var_file_inital_parse");
				
				if (!parse_var_file(var::PF_INITIAL_LOAD)) {
					warning(".var file failed initial load!\n");
					assert(false);
				}
				
				parse_prev_file_data( var::PF_ONLY_SYNTAX_CHECK|var::PF_WRITE_CURRENT ); // Write inital values (ie. $i) to $
			}
		}
	}
	
	{
		PROFILE_SCOPED(THR_ENGINE, "engine_ogl_state_init");
		
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
		
		{ // OpenGL state
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CCW);
			
			glClearDepth(1.0f);
			glDepthFunc(GL_LEQUAL);
			glDepthRange(0.0f, 1.0f);
			
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);
			glDisable(GL_DEPTH_CLAMP);
			
			glEnable(GL_FRAMEBUFFER_SRGB);
			
			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		}
	}
	
	{ // GL buffer setup
		PROFILE_SCOPED(THR_ENGINE, "ogl_buffer_alloc");
		
		glGenBuffers(VBO_COUNT, VBOs);
		
		glBindBuffer(GL_UNIFORM_BUFFER, VBOs[GLOBAL_UNIFORM_BUF]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(std140_Global), NULL, GL_STREAM_DRAW);
		
		glBindBufferRange(GL_UNIFORM_BUFFER, GLOBAL_UNIFORM_BLOCK_BINDING, VBOs[GLOBAL_UNIFORM_BUF],
				0, sizeof(std140_Global));
		
	}
	{ // VAO config
		PROFILE_SCOPED(THR_ENGINE, "ogl_vao_setup");
		
		glGenVertexArrays(VAO_COUNT, VAOs);
		
		{ // Default mesh buffer
			glBindVertexArray(VAOs[NO_UV_VAO]);
			
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			
			glBindBuffer(GL_ARRAY_BUFFER, VBOs[NO_UV_ARR_BUF]);
			
			auto vert_size = safe_cast_assert(GLsizei, (3 +3) * sizeof(f32));
			
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)((0) * sizeof(f32)));				// pos xyz
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)((3 +0) * sizeof(f32)));			// normal xyz
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOs[NO_UV_INDX_BUF]);
		}
		{ // UV mesh buffer
			glBindVertexArray(VAOs[UV_VAO]);
			
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(3);
			
			glBindBuffer(GL_ARRAY_BUFFER, VBOs[UV_ARR_BUF]);
			
			auto vert_size = safe_cast_assert(GLsizei, (3 +3 +2) * sizeof(f32));
			
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)((0) * sizeof(f32)));				// pos xyz
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)((3 +0) * sizeof(f32)));			// normal xyz
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, vert_size, (void*)((3 +3 +0) * sizeof(f32)));	// uv
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOs[UV_INDX_BUF]);
		}
		{ // UV TANGENT mesh buffer
			glBindVertexArray(VAOs[UV_TANG_VAO]);
			
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);
			
			glBindBuffer(GL_ARRAY_BUFFER, VBOs[UV_TANG_ARR_BUF]);
			
			auto vert_size = safe_cast_assert(GLsizei, (3 +3 +4 +2) * sizeof(f32));
			
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)((0) * sizeof(f32)));				// pos xyz
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)((3 +0) * sizeof(f32)));			// normal xyz
			glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, vert_size, (void*)((3 +3 +0) * sizeof(f32)));		// tangent xyzw
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, vert_size, (void*)((3 +3 +4 +0) * sizeof(f32)));	// uv
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOs[UV_TANG_INDX_BUF]);
		}
		#if 0
		{ //
			glBindVertexArray(VAOs[GROUND_PLANE_VAO]);
			
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			
			glBindBuffer(GL_ARRAY_BUFFER, VBOs[GROUND_PLANE_ARR_BUF]);
			
			glBufferData(GL_ARRAY_BUFFER, GROUND_PLANE_BUF_SIZE, NULL, GL_STATIC_DRAW);
			
			auto vert_size = GROUND_PLANE_VERT_SIZE;
			
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)((0) * sizeof(f32)));
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)((3 +0) * sizeof(f32)));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vert_size, (void*)((3 +3 +0) * sizeof(f32)));
		}
		#endif
		{ //
			glBindVertexArray(VAOs[VAO_FULLSCREEN_QUAD]);
			
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			
			glBindBuffer(GL_ARRAY_BUFFER, VBOs[VBO_FULLSCREEN_QUAD]);
			
			struct Vert {
				v2	pos_ndc;
				v2	uv;
			};
			Vert data[6] = {
				{ v2(+1,-1), v2(1,0) },
				{ v2(+1,+1), v2(1,1) },
				{ v2(-1,-1), v2(0,0) },
				
				{ v2(-1,-1), v2(0,0) },
				{ v2(+1,+1), v2(1,1) },
				{ v2(-1,+1), v2(0,1) },
			};
			
			glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
			
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)((0) * sizeof(f32)));
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)((2 +0) * sizeof(f32)));
		}
		{ //
			glBindVertexArray(VAOs[VAO_RENDER_CUBEMAP]);
			
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			
			glBindBuffer(GL_ARRAY_BUFFER, VBOs[VBO_RENDER_CUBEMAP]);
			
			struct Vert {
				v2	pos_ndc;
				v3	render_cubemap_dir;
			};
			Vert data[6 * 6] = {
				// face +X
				{ v2(+1,-1), v3(+1, +1,-1) },
				{ v2(+1,+1), v3(+1, -1,-1) },
				{ v2(-1,-1), v3(+1, +1,+1) },
				{ v2(-1,-1), v3(+1, +1,+1) },
				{ v2(+1,+1), v3(+1, -1,-1) },
				{ v2(-1,+1), v3(+1, -1,+1) },
				// face -X
				{ v2(+1,-1), v3(-1, +1,+1) },
				{ v2(+1,+1), v3(-1, -1,+1) },
				{ v2(-1,-1), v3(-1, +1,-1) },
				{ v2(-1,-1), v3(-1, +1,-1) },
				{ v2(+1,+1), v3(-1, -1,+1) },
				{ v2(-1,+1), v3(-1, -1,-1) },
				// face +Y
				{ v2(+1,-1), v3(+1, +1, -1) },
				{ v2(+1,+1), v3(+1, +1, +1) },
				{ v2(-1,-1), v3(-1, +1, -1) },
				{ v2(-1,-1), v3(-1, +1, -1) },
				{ v2(+1,+1), v3(+1, +1, +1) },
				{ v2(-1,+1), v3(-1, +1, +1) },
				// face -Y
				{ v2(+1,-1), v3(+1, -1, +1) },
				{ v2(+1,+1), v3(+1, -1, -1) },
				{ v2(-1,-1), v3(-1, -1, +1) },
				{ v2(-1,-1), v3(-1, -1, +1) },
				{ v2(+1,+1), v3(+1, -1, -1) },
				{ v2(-1,+1), v3(-1, -1, -1) },
				// face +Z
				{ v2(+1,-1), v3(+1,+1, +1) },
				{ v2(+1,+1), v3(+1,-1, +1) },
				{ v2(-1,-1), v3(-1,+1, +1) },
				{ v2(-1,-1), v3(-1,+1, +1) },
				{ v2(+1,+1), v3(+1,-1, +1) },
				{ v2(-1,+1), v3(-1,-1, +1) },
				// face -Z
				{ v2(+1,-1), v3(-1,+1, -1) },
				{ v2(+1,+1), v3(-1,-1, -1) },
				{ v2(-1,-1), v3(+1,+1, -1) },
				{ v2(-1,-1), v3(+1,+1, -1) },
				{ v2(+1,+1), v3(-1,-1, -1) },
				{ v2(-1,+1), v3(+1,-1, -1) },
			};
			
			glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
			
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)((0) * sizeof(f32)));
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vert), (void*)((2 +0) * sizeof(f32)));
		}
		glBindVertexArray(0);
	}
	
	{ // Create shader programs
		PROFILE_SCOPED(THR_ENGINE, "shaders_inital_load");
		
		shaders_n::load_warn(shaders);
	}
	
	{
		PROFILE_SCOPED(THR_ENGINE, "texture_loader_init");
		
		tex.init();
		tex.incremental_texture_load(); // To ensure at least the 1x1 textures are loaded
	}
	
	{
		PROFILE_SCOPED(THR_ENGINE, "init_other_init");
		
		passes.init();
		dbg_lines.init();
		env_viewer.init();
		
		entities.init();
		entities.calc_mesh_aabb(); // calculate inital AABB
	}
	
	input::print_button_bindings::print();
}
	
DECL void frame () {
	PROFILE_SCOPED(THR_ENGINE, "engine_frame");
	
	reloaded_vars = frame_number == 0; // reloaded vars in init function before frame 0
	{
		PROFILE_SCOPED(THR_ENGINE, "frame_var_file_reloading");
		{
			FILETIME now;
			
			{
				PROFILE_SCOPED(THR_ENGINE, "frame_var_file_GetFileTime");
				assert(GetFileTime(var_file_h, NULL, NULL, &now) != 0);
			}
			
			auto ret = CompareFileTime(&now, &var_file_filetime);
			
			var_file_filetime = now;
			
			if (ret == -1) {
				warning("var file % changed, but new time is before prev time!", ALL_VAR_FILENAME);
			}
			if (ret == 1) {
				print("var file % changed, reparsing\n", ALL_VAR_FILENAME);
				parse_var_file( (var::parse_flags_e)0 );
				reloaded_vars = true;
			}
		}
		
		bool write_current = pipe_test();
		if (write_current) { // This seems to stop the once-a-second loop of autonomously reloading
			parse_prev_file_data( var::PF_ONLY_SYNTAX_CHECK|var::PF_WRITE_CURRENT );
		}
	}
	
	tex.incremental_texture_load();
	
	{ // Input query
		PROFILE_SCOPED(THR_ENGINE, "input_query");
		input::query(&inp, &sinp);
	}
	
	GLOBAL_UBO_WRITE_VAL( std140::vec2, mouse_cursor_pos,
			cast_v2<v2>(sinp.mouse_cursor_pos) / cast_v2<v2>(sinp.window_res) );
	
	if (inp.misc_save_state) {
		PROFILE_SCOPED(THR_ENGINE, "save_state");
		
		print("Saving current state to $i in variable file %.\n", ALL_VAR_FILENAME);
		
		parse_prev_file_data( (var::parse_flags_e)(var::PF_ONLY_SYNTAX_CHECK|var::PF_WRITE_SAVE) );
		
		entities.test_enumerate_all();
	}
	
	if (inp.misc_reload_shaders) {
		PROFILE_SCOPED(THR_ENGINE, "reload_shaders");
		
		shaders_n::reload(shaders);
	}
	
	if (reloaded_vars) {
		print(frame_number == 0 ? "Loading meshes.\n":"Reloading meshes.\n");
		reload_meshes();
	}
	
	{ // Window res and camera fov
		PROFILE_SCOPED(THR_ENGINE, "projection_setup");
		
		bool res_changed = sinp.window_res_changed || reloaded_vars;
		if (res_changed) {
			passes.window_resolution_change(sinp.window_res, &window_aspect_ratio);
		}
		
		bool proj_changed = inp.fov_control_mw || inp.fov_control_dir != 0 || reloaded_vars;
		if (proj_changed) {
			
			if (inp.fov_control_mw) {
				camera.vfov += (f32)inp.mouse_wheel_accum * -controls.cam_fov_control_mw_vel;
			} else {
				camera.vfov += (f32)inp.fov_control_dir * controls.cam_fov_control_vel * dt;
			}
			
			camera.vfov = clamp(camera.vfov, deg(1.0f / 1024), deg(180.0f -(1.0f / 1024)));
		}
		
		if (proj_changed || res_changed) {
			camera.update_matrix(window_aspect_ratio);
		}
		
	}
	
	hm world_to_cam;
	hm cam_to_world;
	{ // Camera placement
		PROFILE_SCOPED(THR_ENGINE, "camera_placement");
		
		m3 world_to_cam_m3;
		m3 world_to_cam_m3_inv;
		{ // Camera orientation
			auto temp = cast_v2<v2>(inp.mouselook_accum) * controls.cam_mouselook_sens;
			
			if ((inp.mouselook & FPS_MOUSELOOK) || !inp.editor_entity_control) { // Always control camera if in fps mode or if in gui mode control camera or entity depending on editor_entity_control
				
				camera.aer.x -= temp.x;
				camera.aer.y -= temp.y;
				
				camera.aer.x = fp::proper_mod(camera.aer.x, deg(360.0f));
				camera.aer.y = clamp(camera.aer.y, deg(0.0f), deg(+180.0f));
				
				//print("cam.aer: %\n", to_deg(camera.aer));
			} else {
				
				if ( !identical(temp, v2(0)) && editor.selected) {
					
					v3 aer = aer_horizontal_q_to_euler(editor.selected->ori);
					
					aer.x -= temp.x;
					aer.y -= temp.y;
					
					aer.x = mod_pn_180deg(aer.x);
					aer.y = clamp(aer.y, deg(-89.95f), deg(+89.95f)); // elev close to +/-90 causes azim to lose precision, with azim being corrupted completely at the singularity (azim=atan2(0,0))
					aer.z = 0.0f;
					
					editor.selected->ori = aer_horizontal_q_from_euler(aer);
					
					print("Entity rotate: azim %  elev %\n", to_deg(aer.x), to_deg(aer.y));
					
				}
				
			}
			
			world_to_cam_m3 = aer_world_to_cam_m(-camera.aer) * conv_to_m3(camera.base_ori);
			world_to_cam_m3_inv = transpose(world_to_cam_m3); // inverse
		}
		
		GLOBAL_UBO_WRITE_VAL(std140::mat3, camera_to_world, world_to_cam_m3_inv);
		
		{ // Camera position
			v3 fdir = cast_v3<v3>(inp.freecam_dir);
			fdir = normalize_or_zero(fdir);
			
			v3 camVel_cam = fdir * controls.cam_translate_vel;
			if (inp.freecam_fast) {
				camVel_cam *= controls.cam_translate_fast_mult;
			}
			
			camera.pos_world += world_to_cam_m3_inv * (camVel_cam * v3(dt));
			//print("cam.pos_world: %\n", camera.pos_world);
		}
		
		world_to_cam =	hm::ident().m3(world_to_cam_m3) * translate_h(-camera.pos_world);
		cam_to_world =	translate_h(camera.pos_world) * hm::ident().m3(world_to_cam_m3_inv);
	}
	
	_world_to_cam = world_to_cam;
	
	// select entities based on their <current> position and AABB (<current> = like rendered prev frame)
	editor.do_entities(world_to_cam, cam_to_world, inp, sinp, camera, &lights, &entities);
	
	entities.calc_mesh_aabb(); // update AABB since entities could have moved in do_entities
	
	{
		PROFILE_SCOPED(THR_ENGINE, "bind_samplers");
		
		glBindSampler(TEX_UNIT_CERB_ALBEDO,				tex.samplers[DEFAULT_SAMPLER]);
		glBindSampler(TEX_UNIT_CERB_NORMAL,				tex.samplers[DEFAULT_SAMPLER]);
		glBindSampler(TEX_UNIT_CERB_METALLIC,			tex.samplers[DEFAULT_SAMPLER]);
		glBindSampler(TEX_UNIT_CERB_ROUGHNESS,			tex.samplers[DEFAULT_SAMPLER]);
		
		#if !DISABLE_ENV_ALL
		glBindSampler(ENV_LUMINANCE_TEX_UNIT,			tex.samplers[CUBEMAP_SAMPLER]);
		glBindSampler(ENV_ILLUMINANCE_TEX_UNIT,			tex.samplers[CUBEMAP_SAMPLER]);
		glBindSampler(TEX_UNIT_PBR_BRDF_LUT,			tex.samplers[SAMPLER_2D_LUT]);
		#endif
	}
	
	env_viewer.update(inp);
	
	{
		PROFILE_SCOPED(THR_ENGINE, "main_pass_clear");
		
		v4 main_pass_clear_col = v4(v3(0.2f), 1);
		
		passes.main_pass();
		
		auto c = main_pass_clear_col;
		glClearColor(c.x, c.y, c.z, c.w);
		glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	}
	
	{ ////// Seperate shadow and main passes for all scenes, (it will become clear if excessive passes will kill performance or not, and if you sould try to reduce passes even during development or not)
		PROFILE_SCOPED(THR_ENGINE, "scenes_main_passes");
		
		u32 scn_i = 0;
		auto* e = entities.root.children;
		while (e) {
			
			Scene* scn;
			{
				assert(e->tag == ET_SCENE);
				scn = (Scene*)e;
			}
			
			if (scn->draw) {
				
				//print("Drawing scene #%\n", scn_i);
				//entities.test_enumerate(p_scn);
				
				{ ////// Shadow pass
					PROFILE_SCOPED(THR_ENGINE, "scene_shadow_lights", scn_i);
					
					glBindFramebuffer(GL_FRAMEBUFFER, passes.shadow_fbo);
					
					glCullFace(GL_FRONT);
					
					auto draw_scene = [&] (hm mp world_to_light, hm mp light_to_world) -> void {
						
						auto draw_mesh = [] (Entity const* e, hm mp to_light, hm mp from_light, hm mp paren_to_cam) -> void {
							
							if (e->tag != ET_MESH) return;
							auto* msh = (Mesh*)e;
							
							auto id = msh->mesh_id;
							if (		id >= NOUV_MSH_FIRST && id < NOUV_MSH_END ) {
								glBindVertexArray(VAOs[NO_UV_VAO]);
							} else if (	id >= UV_MSH_FIRST && id < UV_MSH_END ) {
								glBindVertexArray(VAOs[UV_VAO]);
							} else if (	id >= UV_TANG_MSH_FIRST && id < UV_TANG_MSH_END ) {
								glBindVertexArray(VAOs[UV_TANG_VAO]);
							} else {
								assert(false, "unknown mesh type (id: %)", (u32)id);
							}
							
							draw_keep_mat(id, to_light, from_light);
							
						};
						
						for_entity_subtree_mat(scn, world_to_light, light_to_world, draw_mesh);
					};
					
					std140_Shading temp = {};
					
					u32	shadow_2d_indx =		0;
					u32	shadow_cube_indx =		0;
					u32 enabled_light_indx =	0;
					
					if (scn->lights.len > MAX_LIGHTS_COUNT) {
						warning("scene '%' has more lights (%) than MAX_LIGHTS_COUNT(%), additional lights won't have effect.",
							scn->name, scn->lights.len, MAX_LIGHTS_COUNT);
					}
					u32 lights_count = MIN(scn->lights.len, MAX_LIGHTS_COUNT);
					
					for (u32 light_i=0; light_i<lights_count; ++light_i) {
						
						auto const* l = scn->lights[light_i];
						
						auto shadow_tex = passes.shadow_lights_tex[light_i];
						
						hm world_to_light;
						hm light_to_world = calc_mat_me_world(l, &world_to_light);
						
						switch (l->type) {
						case LT_DIRECTIONAL: {
							
							if (l->flags & LF_DISABLED) {
								continue;
							}
							bool casts_shadow = !(l->flags & LF_NO_SHADOW);
							
							v3 light_dir_cam = world_to_cam.m3() * light_to_world.m3() * v3(0,0,-1);
							
							auto& out_l = temp.lights[enabled_light_indx];
							
							out_l.light_vec_cam.set(	v4( -light_dir_cam, 0) );
							out_l.power.set(			l->power );
							out_l.shad_i.set(			casts_shadow ? shadow_2d_indx : -1);
							
							if (casts_shadow) {
								PROFILE_SCOPED(THR_ENGINE, "scene_shadow_dir_light", light_i);
								
								auto aabb = calc_shadow_cast_aabb(scn, world_to_light);
								//dbg_lines.push_box_world(light_to_world, aabb.box_edges(), v3(1,1,0));
								
								v3	dim =	v3(aabb.xh -aabb.xl, aabb.yh -aabb.yl, aabb.zh -aabb.zl);
								v3	offs =	v3(aabb.xh +aabb.xl, aabb.yh +aabb.yl, aabb.zh +aabb.zl);
								
								v3	d =		v3(2) / dim;
									offs /=	dim;
								
								m4 light_projection = m4::row(	d.x,	0,		0,		-offs.x,
																0,		d.y,	0,		-offs.y,
																0,		0,		-d.z,	+offs.z,
																0,		0,		0,		1 );
								
								m4 cam_to_light =	light_projection;
								cam_to_light *=		world_to_light.m4();
								cam_to_light *=		cam_to_world.m4();
								
								out_l.cam_to_light.set(		cam_to_light );
								
								{
									u32 tex_unit_indx = TEX_UNITS_SHADOW_FIRST +shadow_2d_indx;
									glBindSampler(tex_unit_indx,	tex.samplers[SAMPLER_SHADOW_2D]);
									glActiveTexture(GL_TEXTURE0 +tex_unit_indx);
									glBindTexture(GL_TEXTURE_2D, shadow_tex);
									
									++shadow_2d_indx;
								}
								
								passes.shadow_pass_directional(shadow_tex, light_projection, world_to_light, light_to_world, draw_scene);
								
								dbg_tex_0 = shadow_tex;
							} else {
								dbg_tex_0 = 0;
							}
							
						} break;
						
						case LT_POINT: {
							if (l->flags & LF_DISABLED) {
								continue;
							}
							bool casts_shadow = !(l->flags & LF_NO_SHADOW);
							
							v3 light_pos_cam = (world_to_cam * light_to_world * hv(0) ).xyz();
							
							auto& out_l = temp.lights[enabled_light_indx];
							
							out_l.light_vec_cam.set(	v4( light_pos_cam, 1) );
							out_l.power.set(			l->power );
							out_l.shad_i.set(			casts_shadow ? shadow_cube_indx : -1 );
							
							if (casts_shadow) {
								PROFILE_SCOPED(THR_ENGINE, "scene_shadow_omnidir_light", light_i);
								
								f32	radius =		25.0f;
								
								m4 light_projection = m4::row(	-1,	0,	0,	0,
																0,	-1,	0,	0,
																0,	0,	0,	0,	// z does not matter since i override it with the actual eucilidan distance in the fragment shader by writing to gl_FragDepth (it can't be inf or nan, though)
																0,	0,	-1,	0 );
								
								m4 cam_to_light =	world_to_light.m4();
								cam_to_light *=		cam_to_world.m4();
								
								out_l.cam_to_light.set(		cam_to_light );
								
								{
									u32 tex_unit_indx = TEX_UNITS_SHADOW_FIRST +MAX_LIGHTS_COUNT +shadow_cube_indx;
									glBindSampler(tex_unit_indx,	tex.samplers[SAMPLER_SHADOW_CUBE]);
									glActiveTexture(GL_TEXTURE0 +tex_unit_indx);
									glBindTexture(GL_TEXTURE_CUBE_MAP, shadow_tex);
									
									++shadow_cube_indx;
								}
								
								glEnable(GL_DEPTH_CLAMP);
								
								passes.shadow_pass_point(shadow_tex, light_projection, radius, world_to_light, light_to_world, draw_scene);
								
								glDisable(GL_DEPTH_CLAMP);
								
								dbg_tex_1 = shadow_tex;
							} else {
								dbg_tex_1 = 0;
							}
							
						} break;
						
						default: assert(false);
						}
						
						++enabled_light_indx;
					}
					
					{
						for (u32 i=shadow_2d_indx; i<MAX_LIGHTS_COUNT; ++i) {
							glActiveTexture(GL_TEXTURE0 +TEX_UNITS_SHADOW_FIRST +i);
							glBindTexture(GL_TEXTURE_2D, 0);
						}
						for (u32 i=shadow_cube_indx; i<MAX_LIGHTS_COUNT; ++i) {
							glActiveTexture(GL_TEXTURE0 +TEX_UNITS_SHADOW_FIRST +MAX_LIGHTS_COUNT +i);
							glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
						}
					}
					
					temp.lights_count.set(enabled_light_indx);
					
					GLOBAL_UBO_WRITE(lights_count, &temp);
					GLOBAL_UBO_WRITE_VAL( std140::uint_, lightbulb_indx, -1 );
					
					glCullFace(GL_BACK);
				}
				
				{ ////// Main pass
					PROFILE_SCOPED(THR_ENGINE, "scene_forw_lighting_pass", scn_i);
					
					passes.main_pass();
					
					GLOBAL_UBO_WRITE_VAL( std140::mat4, cam_to_clip, camera.cam_to_clip );
					
					env_viewer.set_env_map();
					
					{
						glUseProgram(shaders[SHAD_PBR_DEV_NOTEX]);
						
						auto draw_mesh = [] (Mesh const* msh, hm mp to_cam, hm mp from_cam) {
							
							auto id = msh->mesh_id;
							if (		id >= NOUV_MSH_FIRST && id < NOUV_MSH_END ) {
								glBindVertexArray(VAOs[NO_UV_VAO]);
							} else if (	id >= UV_MSH_FIRST && id < UV_MSH_END ) {
								glBindVertexArray(VAOs[UV_VAO]);
							} else if (	id >= UV_TANG_MSH_FIRST && id < UV_TANG_MSH_END ) {
								glBindVertexArray(VAOs[UV_TANG_VAO]);
							} else {
								assert(false, "unknown mesh type (id: %)", (u32)id);
							}
							
							//print(">>> draw '%' at %\n", msh->name, (to_world * hv(0)).xyz());
							
							GLOBAL_UBO_WRITE_VAL( std140::uint_, lightbulb_indx, -1);
							
							draw(id, to_cam, from_cam, &materials[MAT_WHITENESS]);
							
						};
						
						u32 enabled_light_indx =	0;
						
						auto draw_light_bulb = [&] (Light_ const* l, hm mp to_cam, hm mp from_cam) {
							
							if (!(inp.mouselook & FPS_MOUSELOOK)) {
								
								glBindVertexArray(VAOs[NO_UV_VAO]);
								
								GLOBAL_UBO_WRITE_VAL( std140::uint_, lightbulb_indx,
										(l->flags & LF_DISABLED) ? -1 : enabled_light_indx );
								
								draw(l->type == LT_POINT ? nouv_MSH_LIGHT_BULB : nouv_MSH_SUN_LAMP_MSH,
										to_cam, from_cam,
										&materials[MAT_LIGHTBULB] );
								
								if (!(l->flags & LF_DISABLED)) {
									++enabled_light_indx;
								}
								
							}
						};
						
						//print(">>>> % \n", material_showcase_grid_mat);
						
						auto draw_material_showcase_grid = [] (Material_Showcase_Grid const* e, hm mp to_cam, hm mp from_cam) {
							
							auto steps = material_showcase_grid_steps;
							assert(steps.x >= 2 && steps.y >= 2);
							
							v2 inv_range = v2(1) / cast_v2<v2>(steps -v2u32(1));
							
							std140_Material mat;
							mat.albedo.set(v3(0.91f, 0.92f, 0.92f)); // aluminium
							
							glBindVertexArray(VAOs[NO_UV_VAO]);
							
							for (u32 j=0; j<steps.y; ++j) {
								
								mat.metallic.set( fp::lerp(0.0f, 1.0f, (f32)j * inv_range.y) );
								
								for (u32 i=0; i<steps.x; ++i) {
									
									mat.roughness.set( fp::lerp(0.01f, 1.0f, (f32)i * inv_range.x) );
									
									v2 offs = v2(e->grid_offs) * cast_v2<v2>(v2u32(i, j));
									
									draw(e->mesh_id,
											to_cam * translate_h(v3(offs, 0)), translate_h(v3(-offs, 0)) * from_cam,
											//&mat
											&materials[MAT_LIGHTBULB] 
											);
								}
							}
							
						};
						
						auto draw_entity = [&] (Entity const* e, hm mp to_cam, hm mp from_cam, hm mp paren_to_cam) -> void {
							
							switch (e->tag) {
								
								case ET_MESH:
									draw_mesh((Mesh*)e, to_cam, from_cam);
									break;
									
								case ET_LIGHT:
									draw_light_bulb((Light_*)e, to_cam, from_cam);
									break;
									
								case ET_MATERIAL_SHOWCASE_GRID:
									draw_material_showcase_grid((Material_Showcase_Grid*)e, to_cam, from_cam);
									break;
									
								case ET_GROUP:
									if (!e->aabb_mesh_paren.is_inf()) {
									//	dbg_lines.push_box_cam(paren_to_cam, e->aabb_mesh_paren, v3(0.5f));
									}
									break;
								case ET_SCENE:
									if (!e->aabb_mesh_paren.is_inf()) {
										dbg_lines.push_box_cam(paren_to_cam, e->aabb_mesh_paren, v3(0.85f));
									}
									break;
								
								case ET_ROOT:
								default: assert(false);
							}
						};
						
						for_entity_subtree_mat(scn, world_to_cam, cam_to_world, draw_entity);
						
					}
				}
				
			}
			
			e = scn->next;
			++scn_i;
		}
	}
	
	{ ////// Old main pass
		PROFILE_SCOPED(THR_ENGINE, "old_main_pass");
		
		hm m = world_to_cam;
		
		#if 1
		{ // Light processing
			std140_Shading temp = {};
			
			{
				for (u32 i=0; i<lights.lights.len; ++i) {
					auto& l = temp.lights[i];
					
					l.light_vec_cam.set(	v4( (world_to_cam * hv(lights.lights[i].pos_world) ).xyz(), 1) );
					l.power.set(			lights.lights[i].power );
					l.shad_i.set(			-1 );
					l.cam_to_light.set(		m4::zero() );
				}
				
				temp.lights_count.set(lights.lights.len);
			}
			
			GLOBAL_UBO_WRITE(lights_count, &temp);
		}
		GLOBAL_UBO_WRITE_VAL( std140::uint_, lightbulb_indx, -1 );
		#endif
		
		glUseProgram(shaders[SHAD_PBR_DEV_NORMAL_MAPPED]);
		
		{
			glBindVertexArray(VAOs[NO_UV_VAO]);
			glUseProgram(shaders[SHAD_PBR_DEV_NOTEX]);
			
			assert(showcase.grid_steps >= 2);
			
			f32 inv_range = 1.0f / (f32)(showcase.grid_steps -1);
			
			std140_Material mat;
			mat.albedo.set(v3(0.91f, 0.92f, 0.92f)); // aluminium
			
			auto draw_grid = [&] (mesh_id_e mesh, Showcase::Grid_Obj cr obj, v3 vp scale) {
				
				for (u32 j=0; j<3; ++j) {
					
					mat.metallic.set( (f32)j / 2.0f );
					
					for (u32 i=0; i<showcase.grid_steps; ++i) {
						
						mat.roughness.set( fp::lerp(0.01f, 1.0f, (f32)i * inv_range) );
						
						v2 offs = cast_v2<v2>(v2u32(i, j)) * v2(1,-1) * v2(obj.grid_offs);
						
						draw(mesh,
								m * translate_h(obj.pos_world) * hm::ident().m3( conv_to_m3(obj.ori_world) ) *
								translate_h(v3(offs, 0.0f)) * scale_h(scale), hm::ident(),
								&mat);
					}
				}
			};
			
			//draw_grid(nouv_MSH_ICO_SPHERE, showcase.sphere, v3(0.3f));
			//draw_grid(nouv_MSH_STFD_BUNNY, showcase.bunny, v3(2.0f));
			//draw_grid(nouv_MSH_STFD_BUDDHA, showcase.buddha, v3(1));
			//draw_grid(nouv_MSH_STFD_DRAGON, showcase.dragon, v3(1));
			//draw_grid(nouv_MSH_UTAHTEAPOT, showcase.teapot, v3(2.5f));
			
			{
				
				for (auto i=MAT_SHOW_FIRST; i<MAT_SHOW_END; ++i) {
					
					f32 offs = (f32)(i -MAT_SHOW_FIRST) * showcase.materials.grid_offs;
					
					draw(nouv_MSH_ICO_SPHERE,
							m * translate_h(showcase.materials.pos_world) * hm::ident().m3( conv_to_m3(showcase.materials.ori_world) ) *
							translate_h(v3(offs, 0, 0)) * scale_h(v3(0.3f)), hm::ident(),
							&materials[i]);
				}
			}
			
			glBindVertexArray(VAOs[UV_TANG_VAO]);
			glUseProgram(shaders[SHAD_PBR_DEV_CERBERUS]);
			
			glActiveTexture(GL_TEXTURE0 +TEX_UNIT_CERB_ALBEDO);
			tex.set_texture(GL_TEXTURE_2D, TEX_CERBERUS_ALBEDO);
			
			glActiveTexture(GL_TEXTURE0 +TEX_UNIT_CERB_NORMAL);
			tex.set_texture(GL_TEXTURE_2D, TEX_CERBERUS_NORMAL);
			
			glActiveTexture(GL_TEXTURE0 +TEX_UNIT_CERB_METALLIC);
			tex.set_texture(GL_TEXTURE_2D, TEX_CERBERUS_METALLIC);
			
			glActiveTexture(GL_TEXTURE0 +TEX_UNIT_CERB_ROUGHNESS);
			tex.set_texture(GL_TEXTURE_2D, TEX_CERBERUS_ROUGHNESS);
			
			draw(uv_tang_CERBERUS,
					m * translate_h(showcase.cerberus.pos_world) * hm::ident().m3( conv_to_m3(showcase.cerberus.ori_world) )
					//* scale_h(v3(1.0f)),
					, hm::ident(),
					&materials[MAT_IDENTITY]);
		}
		
		glBindVertexArray(VAOs[NO_UV_VAO]);
		glUseProgram(shaders[SHAD_PBR_DEV_NOTEX]);
		
		if (!(inp.mouselook & FPS_MOUSELOOK)) { // Light bulbs
			
			for (u32 light_i=0; light_i<lights.lights.len; ++light_i) {
				
				GLOBAL_UBO_WRITE_VAL( std140::uint_, lightbulb_indx, light_i );
				
				auto& l = lights.lights[light_i];
				{
					draw(l.type == LT_POINT ? nouv_MSH_LIGHT_BULB : nouv_MSH_SUN_LAMP_MSH,
							m * translate_h(l.pos_world) *  hm::ident().m3( conv_to_m3(l.ori_world) )
							* scale_h(v3(1.0f / 1)), hm::ident(),
							&materials[MAT_LIGHTBULB] );
				}
			}
			
			GLOBAL_UBO_WRITE_VAL( std140::uint_, lightbulb_indx, -1 );
			
		}
		
		glUseProgram(shaders[SHAD_TINT_AS_FRAG_COL]);
		
		#if 0
		glDisable(GL_CULL_FACE);
		glUseProgram(shaders[SHAD_PBR_DEV_TEX]);
		
		{ // Ground
			glActiveTexture(GL_TEXTURE0 +TEX_UNIT_ALBEDO);
			tex.set_texture(GL_TEXTURE_2D, GRASS_TEX);
			
			set_transforms_and_mat(m, &materials[MAT_GRASS]);
			
			glBindVertexArray(VAOs[UV_VAO]);
			draw_keep_set(uv_SCENE_GROUND0);
			
			glBindVertexArray(VAOs[GROUND_PLANE_VAO]);
			glDrawArrays(GL_TRIANGLES, 0, GROUND_PLANE_VERT_COUNT);
		}
		
		glEnable(GL_CULL_FACE);
		
		glBindVertexArray(VAOs[NO_UV_VAO]);
		//// Draw skybox
		glDepthMask(GL_FALSE);
		glEnable(GL_DEPTH_CLAMP);
		
		glUseProgram(shaders[SHAD_SKY]);
		
		{ // Sky
		  // Scene is drawn at the far nrm_wall (by having the vertex shader produce the correct z value)
		  // Depth clamping is enabled in case of precision problems so that the sky does not get clipped
		  // All pixels drawn before should have an early z rejection and only the rest should get drawn
			draw_solid_color(nouv_MSH_SKYSPHERE2_SUN,
					hm::ident().m3(m.m3() * sky_sphere_to_world),
					(scattered_sun_power * v3(4)) / v3(max_power) );
		}
		
		glDisable(GL_DEPTH_CLAMP);
		glDepthMask(GL_TRUE);
		#endif
		
	}
	
	{ // Entity highlighting by draing bounding box with as dbg_lines
		PROFILE_SCOPED(THR_ENGINE, "entity_highlighting");
		
		if (!editor.highl_manipulator && editor.highl && editor.highl != editor.selected) {
			v3 highl_col = col(0.25f, 0.25f, 1.0f);
			dbg_lines.push_box_world(hm::ident(), editor.highl->aabb_mesh_world.box_edges(), highl_col);
		}
	}
	
	#if !DISABLE_POST
	{	////// Overlay pass
		PROFILE_SCOPED(THR_ENGINE, "overlay_pass");
		
		{
			PROFILE_SCOPED(THR_ENGINE, "dbg_lines_pass");
			
			passes.dbg_line_pass();
			
			dbg_lines.draw();
		}
		
		hm m = world_to_cam;
		
		glBindVertexArray(VAOs[NO_UV_VAO]);
		glUseProgram(shaders[SHAD_TINT_AS_FRAG_COL]);
		
		{ // Draw 3d manipulator (axis cross)
			PROFILE_SCOPED(THR_ENGINE, "draw_3d_manipulator");
			
			{
				constexpr mesh_id_e p_msh[3] = { nouv_MSH_AXIS_CROSS_PX, nouv_MSH_AXIS_CROSS_PY, nouv_MSH_AXIS_CROSS_PZ };
				constexpr mesh_id_e n_msh[3] = { nouv_MSH_AXIS_CROSS_NX, nouv_MSH_AXIS_CROSS_NY, nouv_MSH_AXIS_CROSS_NZ };
				
				bool draw =				!(inp.mouselook & FPS_MOUSELOOK) &&
										(editor.dragging || editor.selected);
				bool drawAxisLines =	editor.dragging;
				if (draw) {
					
					if (editor.dragging) {
						assert(editor.selected);
					}
					
					hm cam_to_paren;
					hm paren_to_cam = calc_mat_parent_world(editor.selected, &cam_to_paren);
					
					paren_to_cam = world_to_cam * paren_to_cam;
					cam_to_paren = cam_to_paren * cam_to_world;
					
					v3 scaleCorrect;
					{
						
						hv pos_cam = paren_to_cam * hv(editor.selected->pos);
						f32 zDist_cam = -pos_cam.z;
						
						f32 fovScaleCorrect = fp::tan(camera.vfov / 2.0f) / 6.0f;
						scaleCorrect = v3(zDist_cam * fovScaleCorrect);
					}
					
					u32 highl_axis = u32(-1);
					if (editor.highl_manipulator) {
						highl_axis = ((u32)(uptr)editor.highl) -1;
						
						assert(highl_axis >= 0 && highl_axis < 3);
					}
					
					if (drawAxisLines) {
						assert(highl_axis != u32(-1));
						
						v3 axisScale = v3(0.25f);
						axisScale[highl_axis] = 8.0f;
						
						v3 p_col = v3(0.0f);
						v3 n_col = v3(0.75f);
						p_col[highl_axis] = 0.75f;
						n_col[highl_axis] = 0.0f;
						
						hm mat = paren_to_cam * translate_h(editor.selected->pos) * scale_h(scaleCorrect * axisScale);
						
						draw_solid_color(p_msh[highl_axis], mat, hm::ident(), p_col); // TODO: maybe pass actual inverse matric here, if needed
						draw_solid_color(n_msh[highl_axis], mat, hm::ident(), n_col); // TODO: maybe pass actual inverse matric here, if needed
						
					}
					for (u32 axis=0; axis<3; ++axis) {
						f32 temp = axis == highl_axis ? 1.0f : 0.75f;
						
						v3 p_col = v3(0.0f);
						v3 n_col = v3(temp);
						p_col[axis] = temp;
						n_col[axis] = 0.0f;
						
						hm mat = paren_to_cam * translate_h(editor.selected->pos) * scale_h(scaleCorrect);
						
						draw_solid_color(p_msh[axis], mat, hm::ident(), p_col); // TODO: maybe pass actual inverse matric here, if needed
						draw_solid_color(n_msh[axis], mat, hm::ident(), n_col); // TODO: maybe pass actual inverse matric here, if needed
						
					}
					
					auto getRotMultiple = [] (f32 a, f32 b) -> si {
						
						si ret;
						if (a < 0.0f) {
							if (b < 0.0f) {
								ret = +0;
							} else {
								ret = +3;
							}
						} else {
							if (b < 0.0f) {
								ret = +1;
							} else {
								ret = +2;
							}
						}
						return ret;
					};
					auto draw_plane = [&] (m3 rot) {
						draw_solid_color(nouv_MSH_AXIS_CROSS_PLANE_XY,
								paren_to_cam * translate_h(editor.selected->pos)
								* hm::ident().m3(rot) * scale_h(scaleCorrect), hm::ident(), // TODO: maybe pass actual inverse matric here, if needed
								v3(1.0f, 0.2f, 0.2f));
					};
					
					glDisable(GL_CULL_FACE);
					
					v3 dir = -(cam_to_paren * hv(editor.selected->pos)).xyz();
					{ // XY Nrm_Wall
						si rotMultiple = getRotMultiple(dir.x, dir.y);
						m3 rot = rotate_Z_90deg(rotMultiple);
						draw_plane(rot);
					}
					{ // YZ Nrm_Wall
						si rotMultiple = getRotMultiple(dir.y, dir.z);
						m3 rot = rotate_X_90deg(rotMultiple) * rotate_Y_90deg(-1);
						draw_plane(rot);
					}
					{ // ZX Nrm_Wall
						si rotMultiple = getRotMultiple(dir.z, dir.x);
						m3 rot = rotate_Y_90deg(rotMultiple) * rotate_X_90deg(+1);
						draw_plane(rot);
					}
					
					glEnable(GL_CULL_FACE);
				}
			}
		}
		
		//assert(false, "blah %", 5);
		
	}
	
	{ ////// Postprocess pass
		PROFILE_SCOPED(THR_ENGINE, "postprocess");
		
		glDisable(GL_CULL_FACE);	// No cull face
		glDepthMask(GL_FALSE);		// don't write to depth buffer
		glDisable(GL_DEPTH_TEST);	// don't read from depth buffer
		
		passes.post_passes(sinp.window_res, window_aspect_ratio);
		
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
	}
	#endif
	
}
	
////
}
