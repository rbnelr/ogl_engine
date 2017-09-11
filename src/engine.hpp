
#include "math.hpp"
using namespace math;

#include "stb/helper.hpp"

namespace engine {

//////
// Engine Stuff
//////

//// UBO
#include "ogl_ubo_interface.hpp"

//// special VBOs and VAOs
DECLD constexpr GLuint	GLOBAL_UNIFORM_BLOCK_BINDING =		0;

DECLD GLuint			global_uniform_buf;
DECLD GLuint			vbo_fullscreen_quad;
DECLD GLuint			vbo_render_cubemap;

DECLD GLuint			empty_vao;
DECLD GLuint			vao_fullscreen_quad;
DECLD GLuint			vao_render_cubemap;

#include "shaders.hpp"
#include "textures.hpp"
#include "meshes.hpp"

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
	MAT_ROUGH_MARBLE		,
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
DECLD bool					reloaded_vars;

DECL void set_vsync (s32 swap_interval) {
	
	print("Vsync % (%).\n", swap_interval ? "enabled":"disabled", swap_interval);
	
	auto ret = wglSwapIntervalEXT(swap_interval);
	assert(ret != FALSE);
}

DECLD GLuint				shaders[SHAD_COUNT];

DECLD Textures				tex;
DECLD Meshes				meshes;

DECLD std140_Material		materials[MAT_COUNT] =			{}; // init to zero except for default values

//////
// "Gameplay" - stuff
//////

typedef input::State		Inp;
typedef input::Sync_Input	SInp;

// try not to use these because you can easily make a 1-frame-latency error,
//  and explicit arguments are preferred
//  only use when always passing it into a function gets excessive, or for debug calculations
DECLD hm	_world_to_cam;
DECLD m4	_cam_to_clip;

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
	
	DECLM m4 calc_matrix (v2 vp window_aspect_ratio, v2* out_cam_frust_scale) const {
		
		v2 frust_scale;
		
		frust_scale.y = fp::tan(vfov * 0.5f);
		frust_scale.x = frust_scale.y * window_aspect_ratio.x;
		
		
		f32 temp = clip_near -clip_far;
		
		v2 frust_scale_inv = v2(1.0f) / frust_scale;
		
		f32 x = frust_scale_inv.x;
		f32 y = frust_scale_inv.y;
		f32 a = (clip_far +clip_near) / temp;
		f32 b = (2.0f * clip_far * clip_near) / temp;
		
		*out_cam_frust_scale = frust_scale;
		return m4::row(	x, 0, 0, 0,
						0, y, 0, 0,
						0, 0, a, b,
						0, 0, -1, 0 );
	}
	
};

#if 0
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
				if (platform::read_file_onto(stk, filepath.str, &file)) {
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
#endif

#define DISABLE_ENV_ALL		0
#define DISABLE_ENV_RENDER	0

struct Env_Viewer {
	
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
		
		{
			using namespace list_of_files_in_n;
			humus_folders = list_of_files_in(ENV_MAPS_HUMUS_DIR, FOLDERS|NO_BASE_PATH);
		}
		{
			using namespace list_of_files_in_n;
			sibl_files = list_of_files_in(ENV_MAPS_SIBL_DIR, FILES|RECURSIVE|NO_BASE_PATH);
		}
		
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
		
		glBindVertexArray(vao_fullscreen_quad);
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
				lstr filename = print_working_stk("%%%\\0", ENV_MAPS_HUMUS_DIR, humus_name, FACES[*face]);
				
				u8*	img;
				
				int w, h;
				{
					Mem_Block file;
					assert(!platform::read_file_onto(&working_stk, filename.str, &file));
					
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
			lstr filename = print_working_stk("%%\\0", ENV_MAPS_SIBL_DIR, sibl_name);
			
			print("sIBL environment: [%] '%'\n", cur_sibl_indx, sibl_name);
			
			f32* img;
			
			int w, h;
			{
				Mem_Block file;
				assert(!platform::read_file_onto(&working_stk, filename.str, &file));
				
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
				
				glBindVertexArray(vao_render_cubemap);
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
		
		glBindVertexArray(vao_render_cubemap);
		
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
	
};

struct Enum_Member {
	lstr			identifier;
	union {
		struct {
			u8		bit_offs;
			u8		bit_len;
			//u16		sub_members;
		};
		//u32			
	};
};

#include "entities.hpp"

//////
// Global data
//////

DECLD Inp	inp;
DECLD SInp	sinp =	{}; // init engine_res to zero

DECLD Controls				controls;

DECLD Camera				camera;
DECLD Camera				saved_cameras[10];

DECLD Env_Viewer			env_viewer;
DECLD Entities				entities;

struct Editor {
	
	bool	highl_manipulator;
	
	eid		selected =					0;
	eid		highl =						0;
	
	bool	dragging =					false;
	v3		select_offs_paren =			v3(0);
	v3		selected_pos_paren =		v3(0);
	
	DECLM f32 do_entity_aabb_selection_recursive (f32 least_t, eid id, Ray cr ray_paren, eid* highl_) {
		
		v3 ray_dir_inv = v3(1.0f) / ray_paren.dir; 
		
		do {
			Entity* e = entities.get(id);
			
			if (id != selected) { // can't highlight selected entity to make selection smoother
				
				AABB aabb = e->select_aabb_paren;
				
				{ // AABB ray intersection test
					f32 tmin;
					f32 tmax;
					{ // X
						f32 tl = (aabb.xl -ray_paren.pos.x) * ray_dir_inv.x;
						f32 th = (aabb.xh -ray_paren.pos.x) * ray_dir_inv.x;
						
						tmin = fp::MIN(tl, th);
						tmax = fp::MAX(tl, th);
					}
					{ // Y
						f32 tl = (aabb.yl -ray_paren.pos.y) * ray_dir_inv.y;
						f32 th = (aabb.yh -ray_paren.pos.y) * ray_dir_inv.y;
						
						// fix for edge cases where SSE min/max behavior of NaNs matters
						tmin = fp::MAX( tmin, fp::MIN(fp::MIN(tl, th), tmax) );
						tmax = fp::MIN( tmax, fp::MAX(fp::MAX(tl, th), tmin) );
					}
					{ // Z
						f32 tl = (aabb.zl -ray_paren.pos.z) * ray_dir_inv.z;
						f32 th = (aabb.zh -ray_paren.pos.z) * ray_dir_inv.z;
						
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
							*highl_ = id;
						}
					}
				}
			}
			
			if (e->children) {
				Ray ray_me;
				ray_me.pos = (e->from_paren * hv(ray_paren.pos)).xyz();
				ray_me.dir = e->from_paren.m3() * ray_paren.dir;
				least_t = do_entity_aabb_selection_recursive(least_t, e->children, ray_me, highl_);
			}
			
			id = e->next;
			
		} while (id);
		
		return least_t;
	}
	
	DECLM void do_entities (hm mp world_to_cam, hm mp cam_to_world, Inp cr inp, SInp cr sinp,
			v2 vp cam_frust_scale, m4 mp cam_to_clip, Camera cr cam) {
		
		PROFILE_SCOPED(THR_ENGINE, "editor_do_entities");
		
		if (inp.mouselook & FPS_MOUSELOOK) {
			dragging = false; // This is kinda ungraceful
			highl_manipulator = false;
			highl = 0;
		} else {
			
			v2 f_mouse_cursor_pos = cast_v2<v2>(sinp.mouse_cursor_pos);
			v2 f_window_res = cast_v2<v2>(sinp.window_res);
			
			eid highl_ = 0;
			if (entities.first) {
				f32 least_t = fp::inf<f32>();
				
				auto ray_world = target_ray_from_screen_pos(f_mouse_cursor_pos, f_window_res,
						cam_frust_scale, cam.clip_near, cam_to_world);
				
				least_t = do_entity_aabb_selection_recursive(least_t, entities.first, ray_world, &highl_);
			}
			
			bool highl_manipulator_ = false;
			
			// Axis cross manipulator processing
			v3 selectOffset_paren_;
			v3 selected_pos_paren_;
			
			if (selected) {
				
				// Place manipulator at location of selected draggable
				
				Entity* sel_e = entities.get(selected);
				
				hm paren_to_cam = world_to_cam;
				hm cam_to_paren = cam_to_world;
				if (sel_e->parent) {
					Entity* paren = entities.get(sel_e->parent);
					paren_to_cam *= paren->to_world;
					cam_to_paren = paren->from_world * cam_to_paren;
				}
				
				v3 pos_cam = (paren_to_cam * hv(sel_e->pos)).xyz();
				f32 zDist_cam = -pos_cam.z;
				
				bool onscreen = zDist_cam > 0.0f;
				
				f32 fovScaleCorrect = fp::tan(cam.vfov / 2.0f) / 6.0f;
				f32 scaleCorrect = zDist_cam * fovScaleCorrect;
				
				constexpr f32 minHighlightDist = 8.0f;
				
				v2	axisBase = project_point_onto_screen(pos_cam, cam_to_clip, f_window_res);
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
								v3 temp = (paren_to_cam * hv(sel_e->pos +v)).xyz();
								axisP = project_point_onto_screen(temp, cam_to_clip, f_window_res) -axisBase;
								v = v3(0);
								v[axis] = -0.75f * scaleCorrect;
								temp = (paren_to_cam * hv(sel_e->pos +v)).xyz();
								axisN = project_point_onto_screen(temp, cam_to_clip, f_window_res) -axisBase;
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
								f_window_res, cam_frust_scale, cam.clip_near, cam_to_paren);
						
						v3 axisDir_paren = v3(0);
						axisDir_paren[highlAxis] = +1.0f;
						
						v3 intersect;
						bool intersectValid = ray_line_closest_intersect(
								targetRay_paren.pos, targetRay_paren.dir,
								sel_e->pos, axisDir_paren, &intersect);
						assert(intersectValid);
						if (!intersectValid) {
							highlAxis = NO_AXIS; // Ignore selection by pretending no axis was highlighted
						}
						selectOffset_paren_ = intersect;
						selected_pos_paren_ = sel_e->pos;
					}
					
					if (highlAxis != NO_AXIS) { // Override highlighting of other entities
						highl_ = highlAxis +1;
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
						v3 temp = (paren_to_cam * hv(sel_e->pos +v)).xyz();
						axisDir_screen = project_point_onto_screen(temp, cam_to_clip,
								f_window_res) -axisBase;
						axisDir_paren = v;
					}
					
					if (normalize_or_zero(&axisDir_screen)) {
						
						f32 alongAxis = dot(cursorRel, axisDir_screen);
						
						auto targetRay_paren = target_ray_from_screen_pos(
								axisBase +(axisDir_screen * v2(alongAxis)),
								f_window_res, cam_frust_scale, cam.clip_near, cam_to_paren);
						
						v3 intersect;
						bool intersectValid = ray_line_closest_intersect(
								targetRay_paren.pos, targetRay_paren.dir,
								select_offs_paren, axisDir_paren,
								&intersect);
						
						if (intersectValid) {
							sel_e->pos = selected_pos_paren +(intersect -select_offs_paren);
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
					highl = 0;
					highl_manipulator = false;
				} else {
					bool is_down =		inp.editor_click_select_state == true;
					bool went_down =	is_down && inp.editor_click_select_counter > 0;
					
					if (went_down) {
						
						if (highl_) {
							
							if (!highl_manipulator_) {
								selected = highl_;
								
								Entity* sel_e = entities.get(selected);
								print("Moveable '%' selected at paren pos %.\n", sel_e->name, sel_e->pos);
							} else {
								
								dragging = true;
								select_offs_paren = selectOffset_paren_;
								selected_pos_paren = selected_pos_paren_;
								
							}
							
						} else {
							// Unselect selection
							selected = 0;
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
					
					Entity* sel_e = entities.get(selected);
					print("Entity '%' moved to local pos %.\n", sel_e->name, sel_e->pos);
				}
			}
		}
	}
};

DECLD Editor				editor;

//////
// Rendering
//////
DECLD GLuint dbg_tex_0 = 0;
DECLD GLuint dbg_tex_1 = 0;

#define DISABLE_POST 0
#define DISABLE_CONVOLUTION_GET 0

struct Passes {
	
	v3						main_pass_clear_col;
	
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
	GLuint					convolved_pbo;
	
	static constexpr ui		BLOOM_GAUSS_LUT_RES =				128;
	
	GLuint					bloom_fbo;
	GLuint					bloom_gauss_LUT_tex;
	
	static constexpr ui		BLOOM_PASSES =						5;
	f32						bloom_intensity;
	
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
		glGenBuffers(1,					&convolved_pbo);
		
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
			glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, convolved_pbo);
			glBufferData(GL_PIXEL_PACK_BUFFER_ARB, sizeof(f32)*4, NULL, GL_DYNAMIC_READ);
			glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
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
	
	void window_resolution_change (resolution_v vp window_res) {
		
		render_res = window_res;
		
		{
			auto u = cast_v2<v2u32>(window_res); // this should be replaced by simply printing the signed int without a '+' in this case
			print("Resolution: %x%\n", u.x,u.y);
		}
		
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
				
				glTexImage2D(GL_TEXTURE_2D, mip, GL_RGBA16F, r.x,r.y,
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
		
		glBindVertexArray(vao_fullscreen_quad);
		
		#if !DISABLE_CONVOLUTION_GET
		{
			PROFILE_SCOPED(THR_ENGINE, "prev_frame_luminance_convolution_result_get");
			
			glBindTexture(GL_TEXTURE_2D, convolved_tex);
			
			f32 avg_log_luminance;
			
			if (frame_number == 0) {
				avg_log_luminance = 0.0f;
			} else {
				
				glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, convolved_pbo);
				
				v4* data = (v4*)glMapBuffer(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY);
				assert(data);
				
				v4 convolved;
				if (data) {
					
					convolved = *data;
					
					glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
					
				} else {
					assert(false);
					convolved = v4(0);
				}
				
				glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
				
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
				
				{
					PROFILE_SCOPED(THR_ENGINE, "setup", (u32)mip);
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
				}
				
				{
					PROFILE_SCOPED(THR_ENGINE, "glDrawArrays", (u32)mip);
					glDrawArrays(GL_TRIANGLES, 0, 6);
				}
				
				if (r.x == 1 && r.y == 1) {
					break;
				}
				r.x = r.x == 1 ? 1 : r.x / 2;
				r.y = r.y == 1 ? 1 : r.y / 2;
				
			}
			
			#if !DISABLE_CONVOLUTION_GET
			{
				PROFILE_SCOPED(THR_ENGINE, "read_luminance_convolve_for_next_frame", (u32)mip);
				
				glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, convolved_pbo); // dst pbo
				glReadBuffer(GL_COLOR_ATTACHMENT0); // src framebuffer color
				
				glReadPixels(0,0, 1,1, GL_RGBA, GL_FLOAT, (void*)0);
				
				glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
			}
			#endif
			
			glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAX_LEVEL, mip -1);
		}
		
		#if 1
		{ // Bloom pass
			PROFILE_SCOPED(THR_ENGINE, "bloom_passes");
			
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
				PROFILE_SCOPED(THR_ENGINE, "bloom_pass", pass);
				
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
					
					glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, res.x,res.y, // GL_RGB8 GL_RGB16F GL_RGB32F
							0, GL_RGB, GL_FLOAT, NULL);
					
					glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_BASE_LEVEL, 0);
					glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_MAX_LEVEL, 0);
				}
				glBindTexture(GL_TEXTURE_2D, convolved_tex);
				
				f32 size = bloom_size[pass];
				
				{
					PROFILE_SCOPED(THR_ENGINE, "A", pass);
					
					glUniform1ui(unif_res, res.x);
					
					glUniform1ui(unif_axis, 0);
					glUniform1f(unif_size, size * window_aspect_ratio.y);
					
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloom_tex[pass][0], 0);
					
					{
						PROFILE_SCOPED(THR_ENGINE, "glDrawArrays", 0);
						glDrawArrays(GL_TRIANGLES, 0, 6);
					}
				}
				
				glBindTexture(GL_TEXTURE_2D, bloom_tex[pass][0]);
				{
					PROFILE_SCOPED(THR_ENGINE, "B", pass);
					
					glUniform1ui(unif_res, res.y);
					
					glUniform1ui(unif_axis, 1);
					glUniform1f(unif_size, size);
					
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloom_tex[pass][1], 0);
					
					{
						PROFILE_SCOPED(THR_ENGINE, "glDrawArrays", 1);
						glDrawArrays(GL_TRIANGLES, 0, 6);
					}
				}
			}
		}
		#endif
		
		dbg_tex_0 = convolved_tex;
		
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
				
				glActiveTexture(GL_TEXTURE0 +TEX_UNIT_MAIN_PASS_BLOOM_3);
				glBindTexture(GL_TEXTURE_2D, bloom_tex[3][1]);
				glBindSampler(TEX_UNIT_MAIN_PASS_BLOOM_3,		tex.samplers[SAMPLER_RESAMPLE]);
				
				glActiveTexture(GL_TEXTURE0 +TEX_UNIT_MAIN_PASS_BLOOM_4);
				glBindTexture(GL_TEXTURE_2D, bloom_tex[4][1]);
				glBindSampler(TEX_UNIT_MAIN_PASS_BLOOM_4,		tex.samplers[SAMPLER_RESAMPLE]);
				
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
			
			GLuint unif_inten =	glGetUniformLocation(shaders[SHAD_POSTPROCESS], "bloom_intensity");
			glUniform1f(unif_inten, bloom_intensity);
			
			{
				PROFILE_SCOPED(THR_ENGINE, "glDrawArrays");
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
		}
		
	}
	
};

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
	
	void begin () {
		buf.clear(tri_per_line * 128);
	}
	
	void push_line_cam (hm mp to_cam, v3 vp a_space, v3 vp b_space, v3 vp col, f32 width=0.008f) {
		
		f32	len_space = distance(a_space, b_space);
		
		// everything in camera space
		v3	a_cam = (to_cam * hv(a_space)).xyz();
		v3	b_cam = (to_cam * hv(b_space)).xyz();
		
		f32	hw = width * 0.5f;
		
		v4	a_clip = _cam_to_clip * v4(a_cam, 1);
		v4	b_clip = _cam_to_clip * v4(b_cam, 1);
		f32	aw_clip = _cam_to_clip.row(0, 0) * hw;
		f32	bw_clip = _cam_to_clip.row(0, 0) * hw;
		
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
		
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
	}
};

DECLD Dbg_Lines				dbg_lines;

//// Var file
#include "var_file.hpp"

DECL void init () {
	
	PROFILE_SCOPED(THR_ENGINE, "engine_init");
	
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
				
				var::init();
				
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
	var::entities_n::init();
	
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
	
	{ // VAO VBO config
		PROFILE_SCOPED(THR_ENGINE, "ogl_vao_vbo_setup");
		
		glGenBuffers(1, &global_uniform_buf);
		glGenBuffers(1, &vbo_fullscreen_quad);
		glGenBuffers(1, &vbo_render_cubemap);
		
		glGenVertexArrays(1, &empty_vao);
		glGenVertexArrays(1, &vao_fullscreen_quad);
		glGenVertexArrays(1, &vao_render_cubemap);
		
		{
			glBindBuffer(GL_UNIFORM_BUFFER, global_uniform_buf);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(std140_Global), NULL, GL_STREAM_DRAW);
			
			glBindBufferRange(GL_UNIFORM_BUFFER, GLOBAL_UNIFORM_BLOCK_BINDING, global_uniform_buf,
					0, sizeof(std140_Global));
		}
		
		{ // VAO_FULLSCREEN_QUAD
			glBindVertexArray(vao_fullscreen_quad);
			
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			
			glBindBuffer(GL_ARRAY_BUFFER, vbo_fullscreen_quad);
			
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
		{ // VAO_RENDER_CUBEMAP
			glBindVertexArray(vao_render_cubemap);
			
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			
			glBindBuffer(GL_ARRAY_BUFFER, vbo_render_cubemap);
			
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
		
		meshes.setup_vaos();
		
		glBindVertexArray(0);
	}
	
	{ // Create shader programs
		PROFILE_SCOPED(THR_ENGINE, "shaders_inital_load");
		
		shaders_n::load_warn(shaders);
	}
	
	{
		PROFILE_SCOPED(THR_ENGINE, "meshes_init");
		meshes.init();
	}
	
	#if 0
	{
		PROFILE_SCOPED(THR_ENGINE, "test_entities");
		entities.test_entities();
	}
	#endif
	
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
		
		var::test_load_entitites();
		
		entities.calc_entities_and_aabb_transforms(); // calculate inital entity data
	}
	
	input::print_button_bindings::print();
}

DECL void frame () {
	PROFILE_SCOPED(THR_ENGINE, "engine_frame");
	_world_to_cam = hm::all(QNAN); // to not make any 1-frame-latency errors
	_cam_to_clip = m4::all(QNAN);
	
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
	var::test_reload_entitites();
	
	dbg_lines.begin();
	
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
		
		//entities.test_enumerate_all();
	}
	
	if (inp.misc_reload_shaders) {
		PROFILE_SCOPED(THR_ENGINE, "reload_shaders");
		
		shaders_n::reload(shaders);
	}
	
	v2	window_aspect_ratio; // x=w/h y=h/w
	v2	cam_frust_scale;
	m4	cam_to_clip;
	{ // Window res and camera fov
		PROFILE_SCOPED(THR_ENGINE, "projection_setup");
		
		{
			auto fRes = cast_v2<v2>(sinp.window_res);
			window_aspect_ratio = v2( fRes.x/fRes.y, fRes.y/fRes.x );
		}
		
		bool print_fov = false;
		
		if (sinp.window_res_changed || reloaded_vars) {
			passes.window_resolution_change(sinp.window_res);
			print_fov = true;
		}
		
		if (inp.fov_control_mw || inp.fov_control_dir != 0 || reloaded_vars) {
			
			if (inp.fov_control_mw) {
				camera.vfov += (f32)inp.mouse_wheel_accum * -controls.cam_fov_control_mw_vel;
			} else {
				camera.vfov += (f32)inp.fov_control_dir * controls.cam_fov_control_vel * dt;
			}
			
			camera.vfov = clamp(camera.vfov, deg(1.0f / 1024), deg(180.0f -(1.0f / 1024)));
			
			print_fov = true;
		}
		
		if (inp.camsave_num_counter > 0 && inp.camsave_num_state) {
			u32 i = inp.camsave_slot;
			if (inp.camsave_restore_save) {
				saved_cameras[i] = camera;
				print("Saved current camera view to saved_cameras[%].\n", i);
			} else {
				camera = saved_cameras[i];
				print("Restored camera view from saved_cameras[%].\n", i);
			}
			
			print_fov = true;
		}
		
		if (print_fov) {
			print("Camera: vertical FOV: % deg (h: % deg)\n",
					to_deg(camera.vfov), to_deg(camera.vfov * window_aspect_ratio.x));
		}
		
		cam_to_clip = camera.calc_matrix(window_aspect_ratio, &cam_frust_scale);
	}
	
	hm	world_to_cam;
	hm	cam_to_world;
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
					
					auto* sel_e = entities.get(editor.selected);
					
					v3 aer = aer_horizontal_q_to_euler(sel_e->ori);
					
					aer.x -= temp.x;
					aer.y -= temp.y;
					
					aer.x = mod_pn_180deg(aer.x);
					aer.y = clamp(aer.y, deg(-89.95f), deg(+89.95f)); // elev close to +/-90 causes azim to lose precision, with azim being corrupted completely at the singularity (azim=atan2(0,0))
					aer.z = 0.0f;
					
					sel_e->ori = aer_horizontal_q_from_euler(aer);
					
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
	_cam_to_clip = cam_to_clip;
	
	// select entities based on their <current> position and AABB (<current> = like rendered prev frame)
	editor.do_entities(world_to_cam, cam_to_world, inp, sinp,
			cam_frust_scale, cam_to_clip, camera);
	
	entities.calc_entities_and_aabb_transforms(); // update entity data since entities could have moved in do_entities
	
	{
		PROFILE_SCOPED(THR_ENGINE, "bind_samplers");
		
		glBindSampler(TEX_UNIT_ALBEDO,					tex.samplers[DEFAULT_SAMPLER]);
		glBindSampler(TEX_UNIT_NORMAL,					tex.samplers[DEFAULT_SAMPLER]);
		glBindSampler(TEX_UNIT_METALLIC,				tex.samplers[DEFAULT_SAMPLER]);
		glBindSampler(TEX_UNIT_ROUGHNESS,				tex.samplers[DEFAULT_SAMPLER]);
		
		#if !DISABLE_ENV_ALL
		glBindSampler(ENV_LUMINANCE_TEX_UNIT,			tex.samplers[CUBEMAP_SAMPLER]);
		glBindSampler(ENV_ILLUMINANCE_TEX_UNIT,			tex.samplers[CUBEMAP_SAMPLER]);
		glBindSampler(TEX_UNIT_PBR_BRDF_LUT,			tex.samplers[SAMPLER_2D_LUT]);
		#endif
	}
	
	env_viewer.update(inp);
	
	{
		PROFILE_SCOPED(THR_ENGINE, "main_pass_clear");
		
		passes.main_pass();
		
		if (0) {
			PROFILE_SCOPED(THR_ENGINE, "color|depth");
			
			auto c = to_srgb(passes.main_pass_clear_col);
			glClearColor(c.x, c.y, c.z, 1.0f);
			glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
		} else {
			PROFILE_SCOPED(THR_ENGINE, "depth");
			
			glClear(GL_DEPTH_BUFFER_BIT);
		}
	}
	
	{ ////// Seperate shadow and main passes for all scenes, (it will become clear if excessive passes will kill performance or not, and if you sould try to reduce passes even during development or not)
		PROFILE_SCOPED(THR_ENGINE, "scenes_main_passes");
		
		u32 scn_i = 0;
		auto id = entities.storage.len > 1 ? 1 : 0;
		while (id) {
			auto* scn_e = entities.get(id);
			
			Scene* scn;
			{
				assert(scn_e->tag == ET_SCENE);
				scn = (Scene*)scn_e;
			}
			
			if (scn->draw) {
				
				//print("Drawing scene #%\n", scn_i);
				//entities.test_enumerate(p_scn);
				
				auto draw_mesh = [] (eMesh const* msh, hm mp world_to_cam, hm mp cam_to_world) {
					PROFILE_SCOPED(THR_ENGINE, "mesh");
					
					glUseProgram(shaders[SHAD_PBR_DEV_COMMON]);
					
					meshes.bind_vao(msh->mesh);
					
					//print(">>> draw '%' at %\n", msh->name, (to_world * hv(0)).xyz());
					
					GLOBAL_UBO_WRITE_VAL( std140::uint_, lightbulb_indx, -1);
					
					{
						std140_Transforms_Material temp;
						
						temp.transforms.model_to_cam.set(world_to_cam * msh->to_world);
						temp.transforms.normal_model_to_cam.set(transpose((msh->from_world * cam_to_world).m3()));
						
						temp.mat = materials[msh->material];
						
						GLOBAL_UBO_WRITE(transforms, &temp);
					}
				
					glActiveTexture(GL_TEXTURE0 +TEX_UNIT_ALBEDO);
					glBindTexture(GL_TEXTURE_2D, msh->tex.albedo == TEX_IDENT ? tex_ident : tex.gl_refs[msh->tex.albedo]);
					
					glActiveTexture(GL_TEXTURE0 +TEX_UNIT_NORMAL);
					glBindTexture(GL_TEXTURE_2D, msh->tex.normal == TEX_IDENT ? tex_ident_normal : tex.gl_refs[msh->tex.normal]);
					
					glActiveTexture(GL_TEXTURE0 +TEX_UNIT_ROUGHNESS);
					glBindTexture(GL_TEXTURE_2D, msh->tex.roughness == TEX_IDENT ? tex_ident : tex.gl_refs[msh->tex.roughness]);
					
					glActiveTexture(GL_TEXTURE0 +TEX_UNIT_METALLIC);
					glBindTexture(GL_TEXTURE_2D, msh->tex.metallic == TEX_IDENT ? tex_ident : tex.gl_refs[msh->tex.metallic]);
					
					glDrawElementsBaseVertex(GL_TRIANGLES, msh->mesh->vbo_data.indx_count, GL_UNSIGNED_SHORT,
							msh->mesh->vbo_data.indx_offset, msh->mesh->vbo_data.base_vertex);
					
				};
				auto draw_mesh_shadow = [] (eMesh_Base const* msh, hm mp world_to_light, hm mp light_to_world) {
					PROFILE_SCOPED(THR_ENGINE, "mesh");
					
					meshes.bind_vao(msh->mesh);
					
					{
						std140_Transforms temp;
						temp.model_to_cam.set(world_to_light * msh->to_world);
						temp.normal_model_to_cam.set(transpose((msh->from_world * light_to_world).m3()));
						GLOBAL_UBO_WRITE(transforms, &temp);
					}
					
					glDrawElementsBaseVertex(GL_TRIANGLES, msh->mesh->vbo_data.indx_count, GL_UNSIGNED_SHORT,
							msh->mesh->vbo_data.indx_offset, msh->mesh->vbo_data.base_vertex);
				};
				
				auto draw_mesh_nanosuit = [] (eMesh_Nanosuit const* msh, hm mp world_to_cam, hm mp cam_to_world) {
					PROFILE_SCOPED(THR_ENGINE, "mesh");
					
					glUseProgram(shaders[SHAD_PBR_DEV_NANOSUIT]);
					
					meshes.bind_vao(msh->mesh);
					
					GLOBAL_UBO_WRITE_VAL( std140::uint_, lightbulb_indx, -1);
					
					{
						std140_Transforms_Material temp;
						
						temp.transforms.model_to_cam.set(world_to_cam * msh->to_world);
						temp.transforms.normal_model_to_cam.set(transpose((msh->from_world * cam_to_world).m3()));
						
						temp.mat = materials[msh->material];
						
						GLOBAL_UBO_WRITE(transforms, &temp);
					}
					
					glActiveTexture(GL_TEXTURE0 +TEX_UNIT_NANO_DIFFUSE_EMISSIVE);
					glBindTexture(GL_TEXTURE_2D, msh->tex.diffuse_emissive == TEX_IDENT ? tex_ident : tex.gl_refs[msh->tex.diffuse_emissive]);
					
					glActiveTexture(GL_TEXTURE0 +TEX_UNIT_NANO_NORMAL);
					glBindTexture(GL_TEXTURE_2D, msh->tex.normal == TEX_IDENT ? tex_ident_normal : tex.gl_refs[msh->tex.normal]);
					
					glActiveTexture(GL_TEXTURE0 +TEX_UNIT_NANO_SPECULAR_ROUGHNESS);
					glBindTexture(GL_TEXTURE_2D, msh->tex.specular_roughness == TEX_IDENT ? tex_ident : tex.gl_refs[msh->tex.specular_roughness]);
					
					
					glDrawElementsBaseVertex(GL_TRIANGLES, msh->mesh->vbo_data.indx_count, GL_UNSIGNED_SHORT,
							msh->mesh->vbo_data.indx_offset, msh->mesh->vbo_data.base_vertex);
					
				};
				
				auto unif_steps = glGetUniformLocation(shaders[SHAD_PBR_DEV_NOTEX_INST], "unif_grid_steps");
				auto unif_offs = glGetUniformLocation(shaders[SHAD_PBR_DEV_NOTEX_INST], "unif_grid_offs");
				auto unif_offs_mat = glGetUniformLocation(shaders[SHAD_PBR_DEV_NOTEX_INST], "unif_grid_offs_mat");
				
				auto draw_material_showcase_grid = [&] (Material_Showcase_Grid const* e, hm mp world_to_cam, hm mp cam_to_world) {
					PROFILE_SCOPED(THR_ENGINE, "material_showcase_grid");
					
					auto steps = material_showcase_grid_steps;
					if (steps.x < 2 || steps.y < 2) {
						warning("material_showcase_grid_steps xy needs to be at least 2 each");
						steps = MIN_v(steps, v2u32(2));
					}
					
					std140_Material mat;
					mat.albedo.set(v3(0.91f, 0.92f, 0.92f)); // aluminium
					
					glUseProgram(shaders[SHAD_PBR_DEV_NOTEX_INST]);
					
					meshes.bind_vao(e->mesh);
					
					glUniform2uiv(unif_steps, 1, &steps.x);
					glUniform2fv(unif_offs, 1, &e->grid_offs.x);
					glUniformMatrix2fv(unif_offs_mat, 1, GL_FALSE, &material_showcase_grid_mat.arr[0].x);
					
					GLOBAL_UBO_WRITE_VAL( std140::uint_, lightbulb_indx, -1);
					{
						std140_Transforms_Material temp;
						
						temp.transforms.model_to_cam.set(world_to_cam * e->to_world);
						temp.transforms.normal_model_to_cam.set(transpose((e->from_world * cam_to_world).m3()));
						
						temp.mat = mat;
						
						GLOBAL_UBO_WRITE(transforms, &temp);
					}
					
					auto inst_count = safe_cast_assert(GLsizei, steps.x*steps.y);
					
					glDrawElementsInstancedBaseVertex(GL_TRIANGLES, e->mesh->vbo_data.indx_count, GL_UNSIGNED_SHORT,
							e->mesh->vbo_data.indx_offset, inst_count, e->mesh->vbo_data.base_vertex);
					
				};
				auto draw_material_showcase_grid_shadow = [&] (Material_Showcase_Grid const* e, hm mp world_to_light, hm mp light_to_world) {
					PROFILE_SCOPED(THR_ENGINE, "material_showcase_grid");
					
					auto steps = material_showcase_grid_steps;
					if (steps.x < 2 || steps.y < 2) {
						warning("material_showcase_grid_steps xy needs to be at least 2 each");
						steps = MIN_v(steps, v2u32(2));
					}
					
					meshes.bind_vao(e->mesh);
					
					for (u32 j=0; j<steps.y; ++j) {
						for (u32 i=0; i<steps.x; ++i) {
							
							v2 offs = material_showcase_grid_mat * (v2(e->grid_offs) * cast_v2<v2>(v2u32(i, j)));
							
							{
								std140_Transforms temp;
								temp.model_to_cam.set(world_to_light * e->to_world * translate_h(v3(offs, 0)));
								temp.normal_model_to_cam.set(transpose((translate_h(v3(-offs, 0)) * e->from_world * light_to_world).m3())); // cancel out scaling and translation -> only keep rotaton
								GLOBAL_UBO_WRITE(transforms, &temp);
							}
							
							glDrawElementsBaseVertex(GL_TRIANGLES, e->mesh->vbo_data.indx_count, GL_UNSIGNED_SHORT,
									e->mesh->vbo_data.indx_offset, e->mesh->vbo_data.base_vertex);
						}
					}
					
				};
				
				
				{ ////// Shadow pass
					PROFILE_SCOPED(THR_ENGINE, "scene_shadow_lights", scn_i);
					
					glBindFramebuffer(GL_FRAMEBUFFER, passes.shadow_fbo);
					
					glCullFace(GL_FRONT);
					
					auto draw_scene = [&] (hm mp world_to_light, hm mp light_to_world) -> void {
						PROFILE_SCOPED(THR_ENGINE, "draw_scene_shadow");
						
						for (eid id=1; id<entities.storage.len; ++id) {
							Entity* e=&entities.storage.arr[id].base;
							switch (e->tag) {
								
								case ET_MESH:
								case ET_MESH_NANOSUIT:
									draw_mesh_shadow((eMesh_Base*)e, world_to_light, light_to_world);
									break;
									
								case ET_LIGHT:
									break;
									
								case ET_MATERIAL_SHOWCASE_GRID:
									draw_material_showcase_grid_shadow((Material_Showcase_Grid*)e, world_to_light, light_to_world);
									break;
									
								case ET_GROUP:
								case ET_SCENE:
									break;
								
								case ET_ROOT:
								default: assert(false);
							}
						}
					};
					
					std140_Shading temp = {};
					
					u32	shadow_2d_indx =		0;
					u32	shadow_cube_indx =		0;
					u32 enabled_light_indx =	0;
					
					auto l_id = scn->children;
					for (u32 light_i=0; l_id;) {
						
						auto* e = entities.get(l_id);
						if (e->tag == ET_LIGHT) {
							auto* l = (Light*)e;
							
							if (!(l->flags & LF_DISABLED)) {
								
								if (enabled_light_indx == MAX_LIGHTS_COUNT) {
									warning("scene '%' has more lights (at least %) than MAX_LIGHTS_COUNT(%), additional lights won't have effect.",
										scn->name, enabled_light_indx, MAX_LIGHTS_COUNT);
								}
								
								bool casts_shadow = (l->flags & LF_SHADOW);
								
								auto shadow_tex = passes.shadow_lights_tex[light_i];
								
								switch (l->type) {
								case LT_DIRECTIONAL: {
									
									v3 light_dir_cam = world_to_cam.m3() * l->to_world.m3() * v3(0,0,-1);
									
									auto& out_l = temp.lights[enabled_light_indx];
									
									out_l.light_vec_cam.set(	v4( -light_dir_cam, 0) );
									out_l.luminance.set(			l->luminance );
									out_l.shad_i.set(			casts_shadow ? shadow_2d_indx : -1);
									
									if (casts_shadow) {
										PROFILE_SCOPED(THR_ENGINE, "scene_shadow_dir_light", light_i);
										
										auto aabb = AABB::from_obb(l->from_world * scn->shadow_aabb_paren.box_corners());
										
										//dbg_lines.push_box_world(l->to_world, aabb.box_edges(), v3(0.99,1,0)); // TODO: BUG: using v3(1,1,0) as color causes the box to be yellow blue striped instead of yellow ???
										
										v3	dim =	v3(aabb.xh -aabb.xl, aabb.yh -aabb.yl, aabb.zh -aabb.zl);
										v3	offs =	v3(aabb.xh +aabb.xl, aabb.yh +aabb.yl, aabb.zh +aabb.zl);
										
										v3	d =		v3(2) / dim;
											offs /=	dim;
										
										m4 light_projection = m4::row(	d.x,	0,		0,		-offs.x,
																		0,		d.y,	0,		-offs.y,
																		0,		0,		-d.z,	+offs.z,
																		0,		0,		0,		1 );
										
										m4 cam_to_light =	light_projection;
										cam_to_light *=		l->from_world.m4();
										cam_to_light *=		cam_to_world.m4();
										
										out_l.cam_to_light.set(		cam_to_light );
										
										{
											u32 tex_unit_indx = TEX_UNITS_SHADOW_FIRST +shadow_2d_indx;
											glBindSampler(tex_unit_indx,	tex.samplers[SAMPLER_SHADOW_2D]);
											glActiveTexture(GL_TEXTURE0 +tex_unit_indx);
											glBindTexture(GL_TEXTURE_2D, shadow_tex);
											
											++shadow_2d_indx;
										}
										
										passes.shadow_pass_directional(shadow_tex, light_projection, l->from_world, l->to_world, draw_scene);
										
										//dbg_tex_0 = shadow_tex;
									} else {
										dbg_tex_0 = 0;
									}
									
								} break;
								
								case LT_POINT: {
									
									v3 light_pos_cam = (world_to_cam * l->to_world * hv(0) ).xyz();
									
									auto& out_l = temp.lights[enabled_light_indx];
									
									out_l.light_vec_cam.set(	v4( light_pos_cam, 1) );
									out_l.luminance.set(			l->luminance );
									out_l.shad_i.set(			casts_shadow ? shadow_cube_indx : -1 );
									
									if (casts_shadow) {
										PROFILE_SCOPED(THR_ENGINE, "scene_shadow_omnidir_light", light_i);
										
										f32	radius =		25.0f;
										
										m4 light_projection = m4::row(	-1,	0,	0,	0,
																		0,	-1,	0,	0,
																		0,	0,	0,	0,	// z does not matter since i override it with the actual eucilidan distance in the fragment shader by writing to gl_FragDepth (it can't be inf or nan, though)
																		0,	0,	-1,	0 );
										
										m4 cam_to_light =	l->to_world.m4();
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
										
										passes.shadow_pass_point(shadow_tex, light_projection, radius, l->from_world, l->to_world, draw_scene);
										
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
							++light_i;
						}
						
						l_id = e->next;
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
					{
						PROFILE_SCOPED(THR_ENGINE, "draw_scene_forw_setup", scn_i);
						
						passes.main_pass();
						
						GLOBAL_UBO_WRITE_VAL( std140::mat4, cam_to_clip, cam_to_clip );
						
						env_viewer.set_env_map();
					}
					{
						PROFILE_SCOPED(THR_ENGINE, "draw_scene_forw");
						
						u32 enabled_light_indx =	0;
						
						auto draw_light_bulb = [&] (Light const* l, hm mp world_to_cam, hm mp cam_to_world) {
							PROFILE_SCOPED(THR_ENGINE, "light_bulb");
							
							glUseProgram(shaders[SHAD_PBR_DEV_LIGHTBULB]);
							
							if (!(inp.mouselook & FPS_MOUSELOOK)) {
								
								meshes.bind_vao(l->mesh);
								
								GLOBAL_UBO_WRITE_VAL( std140::uint_, lightbulb_indx,
										(l->flags & LF_DISABLED) ? -1 : enabled_light_indx );
								
								{
									std140_Transforms_Material temp;
									
									temp.transforms.model_to_cam.set(world_to_cam * l->to_world);
									temp.transforms.normal_model_to_cam.set(transpose((l->from_world * cam_to_world).m3()));
									
									temp.mat = materials[MAT_LIGHTBULB];
									
									GLOBAL_UBO_WRITE(transforms, &temp);
								}
								
								glDrawElementsBaseVertex(GL_TRIANGLES, l->mesh->vbo_data.indx_count, GL_UNSIGNED_SHORT,
										l->mesh->vbo_data.indx_offset, l->mesh->vbo_data.base_vertex);
								
								if (!(l->flags & LF_DISABLED)) {
									++enabled_light_indx;
								}
								
							}
						};
						
						for (eid id=1; id<entities.storage.len; ++id) {
							Entity* e = entities.get(id);
							
							switch (e->tag) {
								
								case ET_MESH:
									draw_mesh((eMesh*)e, world_to_cam, cam_to_world);
									break;
									
								case ET_MESH_NANOSUIT:
									draw_mesh_nanosuit((eMesh_Nanosuit*)e, world_to_cam, cam_to_world);
									break;
									
								case ET_LIGHT:
									draw_light_bulb((Light*)e, world_to_cam, cam_to_world);
									break;
									
								case ET_MATERIAL_SHOWCASE_GRID:
									draw_material_showcase_grid((Material_Showcase_Grid*)e, world_to_cam, cam_to_world);
									break;
									
								case ET_GROUP:
									if (0) {
										auto* paren = entities.get_null_valid(e->parent);
										dbg_lines.push_box_world(paren->to_world, e->select_aabb_paren, v3(0.5f));
									}
									break;
								case ET_SCENE:
									if (0) {
										auto* paren = entities.get_null_valid(e->parent);
										dbg_lines.push_box_world(paren->to_world, e->select_aabb_paren, v3(0.85f));
									}
									break;
								
								case ET_ROOT:
								default: assert(false);
							}
						}
					}
				}
				
			}
			
			id = scn->next;
			++scn_i;
		}
	}
	
	if (1) { // Global 'skybox' draw
		PROFILE_SCOPED(THR_ENGINE, "skybox_draw");
		
		glBindVertexArray(empty_vao);
		glUseProgram(shaders[SHAD_SKY_DRAW_COL]);
		
		glDrawArrays(GL_TRIANGLES, 0, 6); // draw attributeless by using gl_VertexID
	}
	
	{ // Entity highlighting by drawing bounding box with as dbg_lines
		PROFILE_SCOPED(THR_ENGINE, "entity_highlighting");
		
		if (!editor.highl_manipulator && editor.highl && editor.highl != editor.selected) {
			v3 highl_col = col(0.25f, 0.25f, 1.0f);
			auto* e = entities.get(editor.highl);
			auto* paren = entities.get_null_valid(e->parent);
			dbg_lines.push_box_world(paren->to_world, e->select_aabb_paren, highl_col);
			
			//print(">> %\n", e->select_aabb_paren.xl);
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
		
		glBindVertexArray(meshes.formats[MF_NOUV].vao);
		glUseProgram(shaders[SHAD_TINT_AS_FRAG_COL]);
		
		{ // Draw 3d manipulator (axis cross)
			PROFILE_SCOPED(THR_ENGINE, "draw_3d_manipulator");
			
			{
				bool draw =				!(inp.mouselook & FPS_MOUSELOOK) &&
										(editor.dragging || editor.selected);
				bool drawAxisLines =	editor.dragging;
				if (draw) {
					
					if (editor.dragging) {
						assert(editor.selected);
					}
					
					auto* sel_e = entities.get(editor.selected);
					
					hm paren_to_cam = world_to_cam;
					hm cam_to_paren = cam_to_world;
					if (sel_e->parent) {
						Entity* paren = entities.get(sel_e->parent);
						paren_to_cam *= paren->to_world;
						cam_to_paren = paren->from_world * cam_to_paren;
					}
					
					v3 scaleCorrect;
					{
						
						hv pos_cam = paren_to_cam * hv(sel_e->pos);
						f32 zDist_cam = -pos_cam.z;
						
						f32 fovScaleCorrect = fp::tan(camera.vfov / 2.0f) / 6.0f;
						scaleCorrect = v3(zDist_cam * fovScaleCorrect);
					}
					
					u32 highl_axis = u32(-1);
					if (editor.highl_manipulator) {
						highl_axis = ((u32)(uptr)editor.highl) -1;
						
						assert(highl_axis >= 0 && highl_axis < 3);
					}
					
					auto draw_solid_color = [] (Mesh* mesh, hm mp model_to_cam, hm mp cam_to_model, v3 col) {
						
						assert(mesh->format == MF_NOUV);
						
						std140_Material mat;
						mat.albedo.set(col);
						mat.metallic.set(0);
						mat.roughness.set(0);
						mat.roughness.set(0);
						
						{
							std140_Transforms_Material temp;
							
							temp.transforms.model_to_cam.set(model_to_cam);
							temp.transforms.normal_model_to_cam.set(transpose(cam_to_model.m3())); // cancel out scaling and translation -> only keep rotaton
							
							temp.mat = mat;
							
							GLOBAL_UBO_WRITE(transforms, &temp);
						}
						
						glDrawElementsBaseVertex(GL_TRIANGLES, mesh->vbo_data.indx_count, GL_UNSIGNED_SHORT,
								 mesh->vbo_data.indx_offset, mesh->vbo_data.base_vertex);
					};
					if (drawAxisLines) {
						assert(highl_axis != u32(-1));
						
						v3 axisScale = v3(0.25f);
						axisScale[highl_axis] = 8.0f;
						
						v3 p_col = v3(0.0f);
						v3 n_col = v3(0.75f);
						p_col[highl_axis] = 0.75f;
						n_col[highl_axis] = 0.0f;
						
						hm mat = paren_to_cam * translate_h(sel_e->pos) * scale_h(scaleCorrect * axisScale);
						
						draw_solid_color( meshes.non_entity_meshes[AXIS_CROSS_POS_X +highl_axis],
								mat, hm::ident(), p_col); // TODO: maybe pass actual inverse matric here, if needed
						draw_solid_color( meshes.non_entity_meshes[AXIS_CROSS_NEG_X +highl_axis],
								mat, hm::ident(), n_col); // TODO: maybe pass actual inverse matric here, if needed
						
					}
					for (u32 axis=0; axis<3; ++axis) {
						f32 temp = axis == highl_axis ? 1.0f : 0.75f;
						
						v3 p_col = v3(0.0f);
						v3 n_col = v3(temp);
						p_col[axis] = temp;
						n_col[axis] = 0.0f;
						
						hm mat = paren_to_cam * translate_h(sel_e->pos) * scale_h(scaleCorrect);
						
						draw_solid_color( meshes.non_entity_meshes[AXIS_CROSS_POS_X +axis],
								mat, hm::ident(), p_col); // TODO: maybe pass actual inverse matric here, if needed
						draw_solid_color( meshes.non_entity_meshes[AXIS_CROSS_NEG_X +axis],
								mat, hm::ident(), n_col); // TODO: maybe pass actual inverse matric here, if needed
						
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
						draw_solid_color(meshes.non_entity_meshes[AXIS_CROSS_PLANE],
								paren_to_cam * translate_h(sel_e->pos)
								* hm::ident().m3(rot) * scale_h(scaleCorrect), hm::ident(), // TODO: maybe pass actual inverse matric here, if needed
								v3(1.0f, 0.2f, 0.2f));
					};
					
					glDisable(GL_CULL_FACE);
					
					v3 dir = -(cam_to_paren * hv(sel_e->pos)).xyz();
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

} // namespace engine
