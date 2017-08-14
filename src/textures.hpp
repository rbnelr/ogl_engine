#define TEXTURES_X \
		X0(	TEX_MAT_ROUGHNESS ) \
		\
		X(	TEX_GRASS ) \
		X(	TEX_TERRAIN_CUBE_DIFFUSE ) \
		X(	TEX_TERRAIN_SPHERE_DIFFUSE ) \
		X(	TEX_TERRAIN_TREE_DIFFUSE ) \
		X(	TEX_TERRAIN_TREE_CUTS_DIFFUSE ) \
		X(	TEX_TERRAIN_OBELISK_DIFFUSE ) \
		\
		X(	TEX_STRUCTURE_WALLS ) \
		X(	TEX_STRUCTURE_GROUND ) \
		X(	TEX_METAL_GRIPPED_DIFFUSE ) \
		X(	TEX_METAL_RUSTY_02 ) \
		X(	TEX_MARBLE ) \
		X(	TEX_WOODEN_BEAM ) \
		\
		X(	TEX_TEX_DIF_BRICK_00 ) \
		X(	TEX_TEX_NRM_BRICK_00 ) \
		X(	TEX_TEX_DIF_BRICK_01 ) \
		X(	TEX_TEX_NRM_BRICK_01 ) \
		\
		X(	TEX_NANOSUIT_BODY_DIFF_EMISS ) \
		X(	TEX_NANOSUIT_BODY_SPEC_ROUGH ) \
		X(	TEX_NANOSUIT_BODY_NORM ) \
		X(	TEX_NANOSUIT_NECK_DIFF ) \
		X(	TEX_NANOSUIT_NECK_SPEC_ROUGH ) \
		X(	TEX_NANOSUIT_NECK_NORM ) \
		X(	TEX_NANOSUIT_HELMET_DIFF_EMISS ) \
		X(	TEX_NANOSUIT_HELMET_SPEC_ROUGH ) \
		X(	TEX_NANOSUIT_HELMET_NORM ) \
		\
		X(	TEX_NORM_TEST_00			) \
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

DECLD constexpr textures_e TEX_IDENT = (textures_e)-1;

DECLD GLuint tex_ident;
DECLD GLuint tex_ident_normal;

void init_tex_ident () {
	glGenTextures(1, &tex_ident);
	glGenTextures(1, &tex_ident_normal);
	
	{
		glBindTexture(GL_TEXTURE_2D, tex_ident);
		
		v4 col = v4(1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1,1, 0, GL_RGBA, GL_FLOAT, &col);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	}
	{
		glBindTexture(GL_TEXTURE_2D, tex_ident_normal);
		
		v3 up = v3(0.5f, 0.5f, 1.0f);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 1,1, 0, GL_RGB, GL_FLOAT, &up);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	}
	
	glBindTexture(GL_TEXTURE_2D, 0);
}

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
		
		glActiveTexture(GL_TEXTURE0);
		
		init_tex_ident();
		
		init_samplers(samplers, gl_max_aniso);
		
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
	
};
