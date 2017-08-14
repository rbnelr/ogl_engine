
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
	TEX_UNIT_CERB_ROUGHNESS =			2,
	TEX_UNIT_CERB_METALLIC =			3,
	// PBR global
	ENV_LUMINANCE_TEX_UNIT =			4,
	ENV_ILLUMINANCE_TEX_UNIT =			5,
	TEX_UNIT_PBR_BRDF_LUT =				0,//6,
	
	TEX_UNITS_SHADOW_FIRST =			7,
	
	
	TEX_UNIT_MAIN_PASS_LUMI =			0,
	TEX_UNIT_MAIN_PASS_BLOOM_0 =		1,
	TEX_UNIT_MAIN_PASS_BLOOM_1 =		2,
	TEX_UNIT_MAIN_PASS_BLOOM_2 =		3,
	TEX_UNIT_MAIN_PASS_BLOOM_3 =		4,
	TEX_UNIT_MAIN_PASS_BLOOM_4 =		5,
	TEX_UNIT_DBG_LINE_PASS =			6,
	TEX_UNIT_POST_DBG_TEX_0 =			7,
	TEX_UNIT_POST_DBG_TEX_1 =			8,
	
	
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
	X(	SHAD_PBR_DEV_NOTEX_INST ) \
	X(	SHAD_PBR_DEV_LIGHTBULB ) \
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
													{ TEX_UNIT_MAIN_PASS_BLOOM_3,	"main_pass_bloom_3_tex" },
													{ TEX_UNIT_MAIN_PASS_BLOOM_4,	"main_pass_bloom_4_tex" },
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
		/* SHAD_PBR_DEV_NOTEX_INST */		{	{ "_pbr_dev_notex_inst.vert",		"_pbr_dev_notex_inst.frag" }, {
													{ TEX_UNIT_ALBEDO,				"albedo_tex" },
													{ ENV_LUMINANCE_TEX_UNIT,		"env_luminance_tex" },
													{ ENV_ILLUMINANCE_TEX_UNIT,		"env_illuminance_tex" },
													{ TEX_UNIT_PBR_BRDF_LUT,		"brdf_lut_tex" },
													{ TEX_UNITS_SHADOW_FIRST,		(char const*)1 } } // Special case to generate all 
											},
		/* SHAD_PBR_DEV_LIGHTBULB */		{	{ "_pbr_dev_lightbulb.vert",		"_pbr_dev_lightbulb.frag" }, {
													{ TEX_UNIT_ALBEDO,				"albedo_tex" },
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
