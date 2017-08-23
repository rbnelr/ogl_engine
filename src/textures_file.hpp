
enum img_color_channels_e : u32 {
	CH_RED =	1,
	CH_BGR =	3,
	CH_BGRA =	4,
};

namespace tex_type {
	typedef u32 type_t;
	
	DECLD constexpr ui		CHANNELS_OFFS =			0;
	DECLD constexpr ui		CHANNELS_SIZE =			3;
	DECLD constexpr type_t	CHANNELS_MASK =			0b111 << CHANNELS_OFFS;
	
	DECLD constexpr ui		MIPS_OFFS =				CHANNELS_OFFS +CHANNELS_SIZE;
	DECLD constexpr ui		SRGB_OFFS =				MIPS_OFFS +1;
	DECLD constexpr ui		TARGET_OFFS =			SRGB_OFFS +1;
	
	DECLD constexpr ui		EXTRA_OFFS =			TARGET_OFFS +1;
	DECLD constexpr ui		CUBEMAP_TYPE_OFFS =		EXTRA_OFFS;
	DECLD constexpr ui		CUBEMAP_MASK =			0b1 << CUBEMAP_TYPE_OFFS;
	
	typedef img_color_channels_e channels_e;
	
	enum type_e : type_t {
		
		T_CH_BGR =			(type_t)CH_BGR <<	CHANNELS_OFFS,
		T_CH_BGRA =			(type_t)CH_BGRA <<	CHANNELS_OFFS,
		T_CH_RED =			(type_t)CH_RED <<	CHANNELS_OFFS,
		
		T_MIPS =			(type_t)0x0 <<		MIPS_OFFS,
		T_NO_MIPS =			(type_t)0x1 <<		MIPS_OFFS,
		
		T_SRGB =			(type_t)0x0 <<		SRGB_OFFS,
		T_NO_SRGB =			(type_t)0x1 <<		SRGB_OFFS,
		
		T_TEX_2D =			(type_t)0x0 <<		TARGET_OFFS,
		T_CUBEMAP =			(type_t)0x1 <<		TARGET_OFFS,
		
		T_MY_CUBEMAP =		(type_t)0x0 <<		EXTRA_OFFS,
		T_HUMUS_CUBEMAP =	(type_t)0x1 <<		EXTRA_OFFS,
		
		//
		STANDART =			T_TEX_2D|	T_CH_BGR|	T_MIPS|		T_SRGB, // SBGR
		NORMAL =			T_TEX_2D|	T_CH_BGR|	T_MIPS|		T_NO_SRGB,
		
		SBGR_A =			T_TEX_2D|	T_CH_BGRA|	T_MIPS|		T_SRGB,
		SBGR =				T_TEX_2D|	T_CH_BGR|	T_MIPS|		T_SRGB,
		
		CUBEMAP =			T_CUBEMAP|	T_CH_BGR|	T_MIPS|		T_SRGB|		T_MY_CUBEMAP,
		CUBEMAP_HUMUS =		T_CUBEMAP|	T_CH_BGR|	T_MIPS|		T_SRGB|		T_HUMUS_CUBEMAP,
		
		ANY_CUBEMAP =		T_CUBEMAP|	T_CH_BGR|	T_MIPS|		T_SRGB,
		
		LINEAR_2D =			T_TEX_2D|	T_CH_BGR|	T_MIPS|		T_NO_SRGB,
		LINEAR_R_2D =		T_TEX_2D|	T_CH_RED|	T_MIPS|		T_NO_SRGB,
	};
	
	DEFINE_ENUM_FLAG_OPS(type_e, type_t)
	
	DECLD constexpr GLenum TEX_TARG_LUT[] = {
		GL_TEXTURE_2D,
		GL_TEXTURE_CUBE_MAP,
	};
	
	GLenum to_gl_channels (channels_e channels) {
		switch (channels) {
			case CH_BGR:	return GL_BGR;
			case CH_BGRA:	return GL_BGRA;
			case CH_RED:	return GL_RED;
			default: assert(false);
							return 0;
		}
	}
	GLint to_gl_colors (channels_e channels, bool no_srgb) {
		if (no_srgb) {
			switch (channels) {
				case CH_BGR:	return GL_RGB8;
				case CH_BGRA:	return GL_RGBA8;
				case CH_RED:	return GL_R8;
				default: assert(false);
								return 0;
			}
		} else {
			switch (channels) {
				case CH_BGR:	return GL_SRGB8;
				case CH_BGRA:	return GL_SRGB8_ALPHA8;
				default: assert(false);
								return 0;
			}
		}
	}
	
	struct Type_Fields {
		channels_e	channels;
		bool		no_srgb;
		GLenum		gl_targ;
		GLenum		gl_channels;
		GLint		gl_colors;
	};
	
	FORCEINLINE Type_Fields get_all_fields (type_e t) {
		Type_Fields ret;
		ret.channels =		(channels_e)((t >> CHANNELS_OFFS) & CHANNELS_MASK);
		ret.no_srgb =		(t & T_NO_SRGB) != 0;
		ret.gl_targ =		TEX_TARG_LUT[ (t >> TARGET_OFFS) & 1 ];
		ret.gl_channels =	to_gl_channels(ret.channels);
		ret.gl_colors =		to_gl_colors(ret.channels, ret.no_srgb);
		return ret;
	}
	
	u32 calc_stride (channels_e channels, u32 width) {
		return align_up<u32>(width * ((u32)channels * sizeof(u8)), 4);
	}
	u64 calc_face_size (channels_e channels, u32 width, u32 height) {
		return (u64)calc_stride(channels, width) * (u64)height;
	}
	u64 calc_mip_size (ui faces, channels_e channels, u32 width, u32 height) {
		return (u64)faces * calc_face_size(channels, width, height);
	}
}

namespace tex_file {
#pragma pack (push, 1)
	
	DECLD constexpr file_id_8 FILE_ID = {{'R','A','Z','_','T','E','X','S'}};
	
	DECLD constexpr uptr TEX_ENTRY_ALIGN = sizeof(u32);
	struct Tex_Entry {
		u32					name_offs;
		tex_type::type_e	type;
		u32					width;
		u32					height;
		u32					mip_count;
	};
	
	DECLD constexpr uptr MIP_ENTRY_ALIGN = sizeof(u64);
	struct Mip_Entry {
		u64					offs;
		u32					tex_indx;
		u32					mip_level;
	};
	
	DECLD constexpr uptr FILE_ALIGN = 16;
	struct Header {
		file_id_8			id;
		u64					file_size;
		
		u32					data_offs; // =headers_size
		u32					tex_count;
		u32					mips_count;
		u32					last_1px_mip;	// when loading the list of mips in order from mips[]
											//  indx of last 1x1 mipmap (for inital loading purposes)
		
		//Tex_Entry			texs[tex_count];
		//Mip_Entry			mips[mips_count];
		
		//char				filename_str_blob[];
	};
	
#pragma pack (pop)
}
