
#define WORKING_STK_CAP mebi<uptr>(128)
#include "startup.hpp"

DECLD Stack				out_stk;

typedef u32 rd_flags_t;
enum rd_flags_e : rd_flags_t {
	RD_FILENAME_ON_STK =		0x1,
	RD_NULL_TERMINATE_CHAR =	0x2,
};

DECL err_e read_file_contents_onto_stack (char const* filename, rd_flags_t flags, Stack* stk, byte** out_data, uptr* out_file_size) {
	
	assert((flags & ~(RD_FILENAME_ON_STK|RD_NULL_TERMINATE_CHAR)) == 0,
			"read_file_contents_onto_stack flags argument invalid!");
	
	HANDLE handle;
	auto err = win32::open_existing_file_rd(filename, &handle);
	if (err) { return err; }
	defer {
		win32::close_file(handle);
	};
	
	if (flags & RD_FILENAME_ON_STK) {
		stk->pop(filename);
	}
	
	u64 file_size = win32::get_file_size(handle);
	
	// 
	bool null_terminate = (flags & RD_NULL_TERMINATE_CHAR) != 0;
	
	auto data = stk->pushArrNoAlign<byte>(file_size +(null_terminate ? 1 : 0));
	
	err = win32::read_file(handle, data, file_size);
	if (err) { return err; }
	
	if (null_terminate) {
		reinterpret_cast<char*>(data)[file_size] = '\0';
	}
	
	*out_data = data;
	*out_file_size = file_size;
	return OK;
}

DECL err_e write_whole_file (char const* filename, void const* data, u64 size) {
	HANDLE handle;
	auto err = win32::overwrite_file_wr(filename, &handle);
	if (err) { return err; }
	
	defer {
		// Close file
		win32::close_file(handle);
	};
	
	err = win32::write_file(handle, data, size);
	if (err) { return err; }
	
	return OK;
}

#define TEXTURES_DIR						"assets_src\\textures\\"
#define OUT_FILENAME						"all.textures"

#include "gl/gl.h"
#include "gl/glext.h"
#include "textures_file.hpp"

namespace srgb {
	template <typename T> T srgb_to_linear (T srgb) {
		if (srgb <= (T)0.0404482362771082) {
			return srgb / (T)12.92;
		} else {
			return fp::pow( (srgb +(T)0.055) / (T)1.055, (T)2.4 );
		}
	}
	template <typename T> T linear_to_srgb (T linear) {
		if (linear <= (T)0.00313066844250063) {
			return linear * (T)12.92;
		} else {
			return ( (T)1.055 * fp::pow(linear, 1/(T)2.4) ) -(T)0.055;
		}
	}
	
	template <typename T> T srgb_to_linear_nodiv (T srgb) {
		if (srgb <= (T)0.0404482362771082) {
			return srgb * (T)(1/12.92);
		} else {
			return fp::pow( (srgb +(T)0.055) * (T)(1/1.055), (T)2.4 );
		}
	}
	template <typename T> T linear_to_srgb_nodiv (T linear) {
		if (linear <= (T)0.00313066844250063) {
			return linear * (T)12.92;
		} else {
			return ( (T)1.055 * fp::pow(linear, (T)(1/2.4)) ) -(T)0.055;
		}
	}
	
	f64 _srgb_to_linear_LUT_tbl[256];
	void init_srgb_to_linear_LUT_tbl () {
		for (u32 i=0; i<256; ++i) {
			_srgb_to_linear_LUT_tbl[i] = srgb_to_linear_nodiv((f64)(i) * (1.0/255));
		}
	}
	
	#if 0
	__m128 srgb_to_linear_LUT (u16 i0, u16 i1, u16 i2, u16 i3) {
		
		f32		a =		*(_srgb_to_linear_LUT_tbl +i0);
		f32		b =		*(_srgb_to_linear_LUT_tbl +i1);
		f32		c =		*(_srgb_to_linear_LUT_tbl +i2);
		f32		d =		*(_srgb_to_linear_LUT_tbl +i3);
		
		return SET_PS(a, b, c, d);
	}
	__m128 srgb_to_linear_LUT3 (u32 i0, u32 i1, u32 i2) {
		
		f32		a =		_srgb_to_linear_LUT_tbl[ i0 ];
		f32		b =		_srgb_to_linear_LUT_tbl[ i1 ];
		f32		c =		_srgb_to_linear_LUT_tbl[ i2 ];
		
		return SET_PS(a, b, c, 0.0f);
	}
	f32 srgb_to_linear_LUT (u32 i) {
		return _srgb_to_linear_LUT_tbl[i];
	}
	
	FORCEINLINE __m128 _poly7 (__m128 x, __m128 x2,
			__m128 a, __m128 b, __m128 c, __m128 d, __m128 e, __m128 f, __m128 g, __m128 h) {
		
		__m128 ab =		sse::madd(a, x, b);
		__m128 cd =		sse::madd(c, x, d);
		__m128 abcd =	sse::madd(ab, x2, cd);
		
		__m128 ef =		sse::madd(e, x, f);
		__m128 gh =		sse::madd(g, x, h);
		__m128 efgh =	sse::madd(ef, x2, gh);
		
		__m128 x4 =		_mm_mul_ps(x2, x2);
		
		__m128 ret = 	sse::madd(abcd, x4, efgh);
		return ret;
	}
	
	__m128 srgb_to_linear_poly (__m128 x) {
		
		__m128	x2 =	_mm_mul_ps(x, x);
		
		__m128	l =		_mm_mul_ps(x, _mm_set_ps1((f32)(1.0/12.92)));
		
		__m128	e =		_poly7(x, x2,
				_mm_set_ps1(+0.15237971711927983387f),
				_mm_set_ps1(-0.57235993072870072762f),
				_mm_set_ps1(+0.92097986411523535821f),
				_mm_set_ps1(-0.90208229831912012386f),
				_mm_set_ps1(+0.88348956209696805075f),
				_mm_set_ps1(+0.48110797889132134175f),
				_mm_set_ps1(+0.03563925285274562038f),
				_mm_set_ps1(+0.00084585397227064120f) );
		
		__m128	cmp0 =	_mm_cmpgt_ps(x, _mm_set_ps1(0.04045f));
		__m128	ret =	sse::sel(l, e, cmp0);
		
		return ret;
	}
	__m128 linear_to_srgb_poly (__m128 x) {
		
		__m128	x2 =	_mm_mul_ps(x, x);
		__m128	x3 =	_mm_mul_ps(x, x2);
		
		__m128	l =		_mm_mul_ps(x, _mm_set_ps1(12.92f));
		
		__m128	e0 =	_poly7(x, x2,
				_mm_set_ps1(-6681.49576364495442248881f),
				_mm_set_ps1(+1224.97114922729451791383f),
				_mm_set_ps1( -100.23413743425112443219f),
				_mm_set_ps1(   +6.60361150127077944916f),
				_mm_set_ps1(   +0.06114808961060447245f),
				_mm_set_ps1(   -0.00022244138470139442f),
				_mm_set_ps1(   +0.00000041231840827815f),
				_mm_set_ps1(   -0.00000000035133685895f) );
			
		__m128	e1 =	_poly7(x, x2,
				_mm_set_ps1(-0.18730034115395793881f),
				_mm_set_ps1(+0.64677431008037400417f),
				_mm_set_ps1(-0.99032868647877825286f),
				_mm_set_ps1(+1.20939072663263713636f),
				_mm_set_ps1(+0.33433459165487383613f),
				_mm_set_ps1(-0.01345095746411287783f),
				_mm_set_ps1(+0.00044351684288719036f),
				_mm_set_ps1(-0.00000664263587520855f) );
		
		__m128	cmpl =	_mm_cmpgt_ps(x, _mm_set_ps1(0.0031308f));
		__m128	cmpe =	_mm_cmpgt_ps(x, _mm_set_ps1(0.0523f));
		
		__m128	sel0 =	sse::sel(e0, e1, cmpe);
		
		__m128	div0 =	_mm_div_ps(sel0, x3);
		
		__m128	ret =	sse::sel(l, div0, cmpl);
		return ret;
	}
	#endif
	
}

namespace bmp {
	constexpr union {
		char	str[2];
		WORD	w;
	} filetype_magic_number_u = { 'B', 'M' };
	
	struct {
		u32						width;
		u32						height;
		img_color_channels_e	channels;
		u32						stride;
		
	} parse_bmp_header (HANDLE handle) {
		
		constexpr uptr BMP_HEADER_READ_SIZE =
				 sizeof(BITMAPFILEHEADER)
				+MAX( sizeof(BITMAPINFOHEADER), sizeof(BITMAPV4HEADER))
				+(256 * sizeof(RGBQUAD));
		
		DEFER_POP(&working_stk);
		
		BITMAPFILEHEADER*	header;
		uptr				actual_read_size;
		uptr				file_size;
		{
			{ // 
				LARGE_INTEGER size;
				auto ret = GetFileSizeEx(handle, &size);
				assert(ret != 0);
				file_size = size.QuadPart;
			}
			
			actual_read_size = MIN(file_size, BMP_HEADER_READ_SIZE);
			auto data = working_stk.pushArr<byte>(actual_read_size);
			
			{ // Read file contents onto stack
				DWORD bytesRead;
				auto ret = ReadFile(handle, data, safe_cast_assert(DWORD, actual_read_size), &bytesRead, NULL);
				assert(ret != 0);
				assert(bytesRead == actual_read_size);
			}
			
			header = reinterpret_cast<BITMAPFILEHEADER*>(data);
		}
		
		assert(sizeof(BITMAPFILEHEADER) <= actual_read_size);
		
		assert(header->bfType == filetype_magic_number_u.w);
		assert(header->bfSize == file_size);
		
		BITMAPINFOHEADER* info = reinterpret_cast<BITMAPINFOHEADER*>(header +1);
		assert((sizeof(BITMAPFILEHEADER) +sizeof(BITMAPINFOHEADER)) <= actual_read_size);
		
		assert(info->biSize == sizeof(BITMAPINFOHEADER) || info->biSize == sizeof(BITMAPV4HEADER));
		
		assert((sizeof(BITMAPFILEHEADER) +info->biSize) <= actual_read_size);
		
		assert(info->biWidth > 0);
		assert(info->biHeight > 0);
		assert(info->biPlanes == 1);
		assert(info->biCompression == BI_RGB);
		
		DWORD header_size;
		DWORD payloadByteCount = header->bfSize -header->bfOffBits;
		
		DWORD dw_width = safe_cast_assert(DWORD, info->biWidth);
		DWORD dw_height = safe_cast_assert(DWORD, info->biHeight);
		
		DWORD pix_size;
		
		img_color_channels_e	channels;
		DWORD					dw_stride;
		
		switch (info->biBitCount) {
			
			case 8: {
				channels = CH_RED;
				assert(info->biClrUsed == 256);
				
				{
					DWORD size = 256 * sizeof(RGBQUAD);
					
					header_size = sizeof(BITMAPFILEHEADER) +info->biSize +size;
					
					assert(header_size <= actual_read_size);
					
					auto* bmiColors = reinterpret_cast<RGBQUAD*>(
							reinterpret_cast<byte*>(header) +sizeof(BITMAPFILEHEADER) +info->biSize );
					for (u32 i=0; i<256; ++i) {
						auto col = bmiColors[i];
						assert(
							(col.rgbBlue == i) &&
							(col.rgbGreen == i) &&
							(col.rgbRed == i) );
					}
				}
				
				dw_stride = align_up<DWORD>(dw_width * (1 * sizeof(u8)), 4);
				pix_size = dw_height * dw_stride;
			} break;
			
			case 24: {
				channels = CH_BGR;
				assert(info->biClrUsed == 0);
				
				header_size = sizeof(BITMAPFILEHEADER) +info->biSize;
				
				dw_stride = align_up<DWORD>(dw_width * (3 * sizeof(u8)), 4);
				pix_size = dw_height * dw_stride;
			} break;
			
			case 32: {
				channels = CH_BGRA;
				assert(info->biClrUsed == 0);
				
				header_size = sizeof(BITMAPFILEHEADER) +info->biSize;
				
				dw_stride = dw_width * (4 * sizeof(u8));
				pix_size = dw_height * dw_stride;
			} break;
			
			default: assert(false);
		}
		
		assert(header->bfOffBits >= header_size);
		
		assert(pix_size <= payloadByteCount);
		
		{
			LONG high = 0;
			auto ret = SetFilePointer(handle, header->bfOffBits, &high, FILE_BEGIN);
			if (ret == INVALID_SET_FILE_POINTER) {
				auto err = GetLastError();
				assert(err == NO_ERROR);
			}
		}
		
		return {
			safe_cast_assert(u32, dw_width),
			safe_cast_assert(u32, dw_height),
			channels,
			safe_cast_assert(u32, dw_stride)
		};
	}
	
}


struct Mip_Entry;

struct File {
	HANDLE			handle;
	lstr			name;
	
	Mip_Entry*		mips;
	u32				mips_count;
	
	tex_type::type_e type;
	
	u32				indx;
};

struct Mip_Entry {
	Mip_Entry*		next;
	Mip_Entry*		next_mip;
	File*			file;
	
	u64				offs;
	u64				size;
	
	u32				mip_level;
	u32				width;
	u32				height;
	u32				stride;
};

Mip_Entry* do_sort (Mip_Entry* list, u32 count) {
	
	if (count >= 2) {
		
		// bubble sort
		u32 j=0;
		do {
			Mip_Entry* prev =	nullptr;
			Mip_Entry* cur =	list;
			Mip_Entry* next;
			
			u32 i=0;
			do {
				
				next = cur->next;
				
				if (cur->size > next->size) {
					//assert((i==0) == (cur==list));
					if (i == 0) {
						list = next;
					}
					
					if (prev) {
						prev->next = next;
					}
					
					cur->next = next->next;
					
					next->next = cur;
					
					prev = next;
				} else {
					prev = cur;
					cur = next;
				}
				
			} while (++i < (count -j -1));
		} while (++j < (count -1)); 
	}
	
	#if 0
	{
		auto* cur = list;
		
		for (u32 i=0; i<count; ++i) {
			
			print(cur->file->filename, ": ", cur->size, " - ",
					cur->mip_level, " ", cur->width, "x", cur->height, nl);
			
			if (i != (count -1)) {
				assert(cur != cur->next);
				assert(cur->size <= cur->next->size);
			} else {
				assert(cur->next == nullptr);
			}
			cur = cur->next;
		}
	}
	#endif
	
	return list;
}

constexpr char const* cube_faces_[6 * 2] = {
	"px.bmp",
	"nx.bmp",
	"py.bmp",
	"ny.bmp",
	"pz.bmp",
	"nz.bmp",
	
	"posx.bmp",
	"negx.bmp",
	"posz.bmp",
	"negz.bmp",
	"posy.bmp",
	"negy.bmp",
};

u32 process_filename (File* file) {
	using namespace tex_type;
	
	file->type = (type_e)0;
	
	char*	bmp_extension = nullptr;
	char*	cubemap_bmp;
	
	char*	cur = (char*)file->name.str;
	while (cur[0] != '\0') {
		if (*cur++ == '.') {
			
			mlstr str;
			str.str = cur;
			
			while (*cur != '.' && *cur != '/' && *cur != '\0') { ++cur; }
			
			str.len = (u32)ptr_sub(str.str, cur);
			
			if (			str::comp(str, "bmp") ) {
				assert(*cur == '\0', "'%'", file->name); // bmp only at the end
				bmp_extension = str.str -1;
			} else {
				assert(*cur != '\0', "'%'", file->name); // bmp should be at the end
				assert(file->type == 0);
				
				if (*cur == '.') {
					
					if (str[0] >= '0' && str[0] <= '9') {
						// explicit resolution
					} else if (	str::comp(str, "sbgr_a") ) {
						file->type = SBGR_A;
					} else if (	str::comp(str, "linear") ) {
						file->type = LINEAR_2D;
					} else if (	str::comp(str, "linear_r") ) {
						file->type = LINEAR_R_2D;
					} else if (	str::comp(str, "nrm") ) {
						file->type = NORMAL;
					} else if (	str::comp(str, "alb") ) {
						file->type = SBGR;
					} else {
						str.str[str.len] = '\0';
						assert(false, "unknown file type: '%' '%'", str, file->name);
					}
				} else {
					++cur; // skip '\\'
					
					if (		str::comp(str, "cubemap") ) {
						file->type = CUBEMAP;
						cubemap_bmp = cur;
					} else if (	str::comp(str, "cubemap_humus") ) {
						file->type = CUBEMAP_HUMUS;
						cubemap_bmp = cur;
					} else {
						str.str[str.len] = '\0';
						assert(false, "unknown folder type: '%' '%'", str, file->name);
					}
				}
			}
		}
	}
	
	u32 bmp_count = 1;
	
	if (file->type & T_CUBEMAP) {
		bmp_count = 6;
		
		{ // Assert that the next 5 files are also the corresponding cubemap ones
			
			// Check that the path until "\\nx.bmp" is identical
			uptr len = ptr_sub(file->name.str, cubemap_bmp);
			for (u32 j=1; j<6; ++j) {
				assert(str::comp_up_to_count(file->name, (file +j)->name, len));
			}
			
			char const* const* cube_faces = &cube_faces_[0];
			if (file->type == CUBEMAP_HUMUS) {
				cube_faces += 6;
			}
			
			u32 sort[6];
			// Make sure all faces appear exactly once
			for (u32 j=0; j<6; ++j) {
				
				for (u32 i=0;; ++i) {
					if ( str::comp((file +i)->name.str +len, cube_faces[j]) ) {
						sort[j] = i;
						break;
					}
					assert(i < 6);
				}
			}
			
			HANDLE handles[6];
			//char* filenames[6];
			
			for (u32 i=0; i<6; ++i) {
				
				handles[i] = (file +i)->handle;
				//filenames[i] = (file +i)->name;
			}
			
			// Reorder the faces
			for (u32 i=0; i<6; ++i) {
				
				(file +i)->handle = handles[sort[i]];
				//(file +i)->name = filenames[sort[i]];
			}
			
			*(cubemap_bmp -1) = '\0'; // path\\cubemap_layout.cubemap\\px -> path\\cubemap_layout.cubemap
		}
	}
	
	{
		constexpr uptr TEXTURES_DIR_LEN = lit_cstrlen(TEXTURES_DIR);
		
		assert( str::comp_up_to_count(file->name, TEXTURES_DIR, TEXTURES_DIR_LEN) );
		
		file->name.str += TEXTURES_DIR_LEN;
	}
	
	assert(bmp_extension, "%", file->name);
	*bmp_extension = '\0';
	
	if (file->type == 0) {
		file->type = STANDART;
	}
	
	return bmp_count;
}

DECL s32 app_main () {
	
	out_stk = makeStack(0, mebi<uptr>(16));
	
	srgb::init_srgb_to_linear_LUT_tbl();
	
	{
		
		Filenames filenames;
		{
			using namespace list_of_files_in_n;
			lstr inc[1] = { "bmp" };
			filenames = list_of_files_in(TEXTURES_DIR, FILES|RECURSIVE|FILTER_FILES|FILTER_INCL, 1, inc);
		}
		
		auto*	files = out_stk.pushArr<File>(filenames.arr.len);
		
		for (u32 i=0; i<filenames.arr.len; ++i) {
			
			files[i].name = filenames.get_filename(i);
			{
				auto ret = CreateFile(files[i].name.str, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
						FILE_FLAG_SEQUENTIAL_SCAN, NULL);
				assert(ret != INVALID_HANDLE_VALUE, "%", files[i].name);
				files[i].handle = ret;
			}
		}
		
		auto*	mip_list = out_stk.getTop<Mip_Entry>();
		
		u32		tex_count = 0;
		u32		mip_count = 0;
		{ // Open files and build linked list of their mipmaps
			Mip_Entry*	prev =	nullptr;
			
			for (u32 i=0; i<filenames.arr.len;) {
				using namespace tex_type;
				
				auto file = &files[i];
				
				u32 bmp_count = process_filename(file);
				
				//print("Parsing '", file->name, "'", nl);
				auto ret = bmp::parse_bmp_header(file->handle);
				assert(is_power_of_two(ret.width) && is_power_of_two(ret.height));
				assert(ret.channels == (channels_e)(file->type & CHANNELS_MASK));
				
				for (u32 i=1; i<bmp_count; ++i) {
					auto ret_ = bmp::parse_bmp_header((file +i)->handle);
					assert(	ret_.width == ret.width && ret_.height == ret.height &&
							ret_.channels == ret.channels && ret_.stride == ret.stride );
				}
				
				auto*		mip = out_stk.push<Mip_Entry>();
				if (prev) {
					prev->next = mip;
				}
				++mip_count;
				
				file->mips = mip;
				
				ui faces = file->type & T_CUBEMAP ? 6 : 1;
				
				u32			mip_level =	0;
				u32			width =		ret.width;
				u32			height =	ret.height;
				u32			stride =	ret.stride;
				
				for (;;) {
					
					mip->next =			nullptr; // gets overwritten with prev->next
					mip->file =			file;
					
					mip->size =			(u64)stride * (u64)height * (u64)faces;
					
					mip->mip_level =	mip_level;
					
					mip->width =		width;
					mip->height =		height;
					mip->stride =		stride;
					
					++mip_level;
					
					prev =		mip;
					
					if ((width == 1 && height == 1) || (file->type & T_NO_MIPS)) {
						mip->next_mip = nullptr;
						break;
					}
					
					width =		width == 1 ? 1 : width / 2;
					height =	height == 1 ? 1 : height / 2;
					
					stride =	calc_stride(ret.channels, width);
					
					mip = out_stk.push<Mip_Entry>();
					prev->next = mip;
					++mip_count;
					
					prev->next_mip = mip;
				}
				
				file->mips_count = mip_level;
				
				++tex_count;
				
				i += bmp_count; 
			}
		}
		
		// Sort the mipmaps by their size
		mip_list = do_sort(mip_list, mip_count);
		
		out_stk.zeropad_to_align<tex_file::FILE_ALIGN>();
		auto*	header =		out_stk.pushNoAlign<tex_file::Header>();
		
		out_stk.zeropad_to_align<tex_file::TEX_ENTRY_ALIGN>();
		auto*	out_texs =		out_stk.pushArrNoAlign<tex_file::Tex_Entry>(tex_count);
		
		out_stk.zeropad_to_align<tex_file::MIP_ENTRY_ALIGN>();
		auto*	out_mips =		out_stk.pushArrNoAlign<tex_file::Mip_Entry>(mip_count);
		
		u32		header_size;
		{
			using namespace tex_type;
			
			u32 indx = 0;
			
			for (u32 i=0; i<filenames.arr.len;) {
				
				auto* file = &files[i];
				bool is_cubemap = (file->type & T_CUBEMAP) != 0;
				u32 bmp_count = is_cubemap ? 6 : 1;
				
				out_texs[indx].name_offs = safe_cast_assert(u32,
						ptr_sub(reinterpret_cast<char*>(header), out_stk.getTop<char>()) );
				out_texs[indx].type =		file->type;
				out_texs[indx].width =		file->mips->width;
				out_texs[indx].height =		file->mips->height;
				out_texs[indx].mip_count =	file->mips_count;
				
				file->indx = indx;
				
				str::append_term(&out_stk, file->name);
				
				++indx;
				i += bmp_count;
			}
			
			out_stk.zeropad_to_align<16>();
			
			header_size = safe_cast_assert(u32, ptr_sub(reinterpret_cast<byte*>(header), out_stk.getTop()) );
		}
		
		uptr data_size;
		{ // Calculate the position of each mipmap in the data block
			uptr offs = 0;
			
			auto* cur = mip_list;
			
			while (cur) {
				
				assert(is_aligned<u64>(offs, 4));
				
				cur->offs = offs;
				offs += cur->size;
				
				cur = cur->next;
			}
			
			data_size = offs;
		}
		
		HANDLE out_file_handle;
		{
			{
				header->id.i =				tex_file::FILE_ID.i;
				header->file_size =			header_size +data_size;
				header->data_offs =			header_size;
				header->tex_count =			tex_count;
				header->mips_count =		mip_count;
				header->last_1px_mip =		0;
			}
			
			{
				auto* cur = mip_list;
				
				for (u32 i=0; i<mip_count; ++i) {
					auto* mip = cur;
					
					out_mips[i].offs =			cur->offs;
					out_mips[i].tex_indx =		cur->file->indx;
					out_mips[i].mip_level =		cur->mip_level;
					
					if (cur->width == 1 && cur->height == 1) {
						header->last_1px_mip = MAX( header->last_1px_mip, i );
					}
					
					cur = cur->next;
				}
				
				assert(cur == nullptr);
			}
			
			{
				auto ret = CreateFile(OUT_FILENAME, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
						FILE_ATTRIBUTE_NORMAL, NULL);
				assert(ret != INVALID_HANDLE_VALUE);
				out_file_handle = ret;
			}
			
			win32::set_filepointer_and_eof(out_file_handle, header->file_size);
		}
		
		{
			using namespace tex_type;
			
			auto output_mip = [&] (u8* data, Mip_Entry const* mip) -> void {
				
				win32::set_filepointer(out_file_handle, mip->offs +(u64)header_size);
				
				win32::write_file(out_file_handle, data, mip->size);
			};
			
			u64 largest = 0;
			lstr largest_name = lstr::null();
			
			for (u32 i=0; i<filenames.arr.len;) {
				
				auto const* file = &files[i];
				auto const* mip = file->mips;
				
				channels_e channels =	(channels_e)((file->type >> CHANNELS_OFFS) & CHANNELS_MASK);
				bool is_cubemap =		(file->type & T_CUBEMAP) != 0;
				bool no_srgb =			(file->type & T_NO_SRGB) != 0;
				bool is_humus_cubem =	(file->type & (T_CUBEMAP|T_HUMUS_CUBEMAP)) == (T_CUBEMAP|T_HUMUS_CUBEMAP);
				
				u32 bmp_count = is_cubemap ? 6 : 1;
				
				DEFER_POP(&working_stk);
				u8*	data = working_stk.pushArrAlign<16, u8>(mip->size);
				
				u64 face_size = (u64)mip->height * (u64)mip->stride;
				
				{
					u64 size = (file->type & T_CUBEMAP ? 6 : 1) * face_size;
					u64 total_size = data_size;
					
					if (size >= largest) {
						largest = size;
						largest_name = file->name;
					}
					
					f64 perc = ((f64)size / (f64)total_size) * 100.0;
					
					print("% %  %\\% of total file\n", repeat("*", (u32)(perc * 2.0)), file->name, perc);
				}
				
				DWORD dw_size = safe_cast_assert(DWORD, face_size);
				for (ui face=0; face<bmp_count; ++face) {
					DWORD bytes_read;
					auto ret = ReadFile((file +face)->handle, data +(face * dw_size), dw_size, &bytes_read, NULL);
					assert(ret != 0);
					assert(bytes_read == dw_size);
				}
				
				if (is_humus_cubem) {
					enum rotate_e {
						DONT=0, CCW_90, _180, CW_90
					};
					
					static constexpr rotate_e humus_cube_face_rotation[6] = {
						CCW_90,
						CW_90,
						DONT,
						_180,
						DONT,
						_180,
					};
					
					u8*	in_data = data;
					u8*	out_data = working_stk.pushArrAlign<16, u8>(mip->size);
					
					data = out_data;
					
					u8*	in_pix_data = in_data;
					u8*	out_pix_data = out_data;
					
					for (ui face=0; face<6; ++face) {
						
						auto rot = humus_cube_face_rotation[face];
						
						auto in = [&] (u32 x, u32 y) -> u8* {
							return in_pix_data +(y * mip->stride) +(x * 3);
						};
						
						switch (rot) {
							
							case CCW_90: {
								for (u32 j=0; j<mip->height; ++j) {
									
									u8* out_pix = out_pix_data;
									
									for (u32 i=0; i<mip->width; ++i) {
										u8* in_pix = in( j, mip->height -1 -i );
										
										out_pix[0] = in_pix[0];
										out_pix[1] = in_pix[1];
										out_pix[2] = in_pix[2];
										
										out_pix += 3;
									}
									
									out_pix_data += mip->stride;
									
									while (out_pix != out_pix_data) {
										*out_pix++ = 0;
									}
								}
							} break;
							
							case CW_90: {
								for (u32 j=0; j<mip->height; ++j) {
									
									u8* out_pix = out_pix_data;
									
									for (u32 i=0; i<mip->width; ++i) {
										u8* in_pix = in( mip->width -1 -j, i );
										
										out_pix[0] = in_pix[0];
										out_pix[1] = in_pix[1];
										out_pix[2] = in_pix[2];
										
										out_pix += 3;
									}
									
									out_pix_data += mip->stride;
									
									while (out_pix != out_pix_data) {
										*out_pix++ = 0;
									}
								}
							} break;
							
							case _180: {
								for (u32 j=0; j<mip->height; ++j) {
									
									u8* out_pix = out_pix_data;
									
									for (u32 i=0; i<mip->width; ++i) {
										u8* in_pix = in( mip->width -1 -i, mip->height -1 -j );
										
										out_pix[0] = in_pix[0];
										out_pix[1] = in_pix[1];
										out_pix[2] = in_pix[2];
										
										out_pix += 3;
									}
									
									out_pix_data += mip->stride;
									
									while (out_pix != out_pix_data) {
										*out_pix++ = 0;
									}
								}
							} break;
							
							case DONT: {
								for (u32 j=0; j<mip->height; ++j) {
									
									u8* out_pix = out_pix_data;
									
									for (u32 i=0; i<mip->width; ++i) {
										u8* in_pix = in( i, j );
										
										out_pix[0] = in_pix[0];
										out_pix[1] = in_pix[1];
										out_pix[2] = in_pix[2];
										
										out_pix += 3;
									}
									
									out_pix_data += mip->stride;
									
									while (out_pix != out_pix_data) {
										*out_pix++ = 0;
									}
								}
							} break;
						}
						
						in_pix_data += face_size;
					}
				}
				
				{
					u8* prev_data;
					
					for (;;) {
						
						//print("  ", mip->mip_level, " ", mip->offs, " ", mip->size, nl);
						
						output_mip(data, mip);
						
						auto* prev =	mip;
						auto* cur =		mip->next_mip;
						
						mip = cur;
						
						if (!mip) {
							break;
						}
						
						prev_data = data;
						data = working_stk.pushArrAlign<16, u8>(cur->size);
						
						{
							// Handle non-square textures 
							u8* prev_rowA = prev_data;
							u8* prev_rowB = prev_data;
							
							uptr prev_effective_stride = prev->stride;
							
							if (prev->height > 1) {
								prev_rowB +=				prev->stride;
								prev_effective_stride *=	2;
							}
							
							u32 column_offset = prev->width > 1 ? channels : 0;
							
							u8* row = data;
							u8* end = data +cur->size;
							do {
								
								u8* prev_pixA = prev_rowA;
								u8* prev_pixB = prev_rowB;
								
								u8* pix = row;
								u8* row_end = row +(cur->width * channels);
								do {
									
									u8* a = prev_pixA;
									u8* b = prev_pixA +column_offset;
									u8* c = prev_pixB;
									u8* d = prev_pixB +column_offset;
									
									if (no_srgb) {
										for (u32 col=0; col<channels; ++col) {
											u32 ab = (u32)a[col] +(u32)b[col];
											u32 cd = (u32)c[col] +(u32)d[col];
											pix[col] = (u8)((ab +cd) / 4);
										}
									} else {
										for (u32 col=0; col<channels; ++col) {
											
											#if 0
											f64 fa = cast_flt<f64>(a[col]);
											f64 fb = cast_flt<f64>(b[col]);
											f64 fc = cast_flt<f64>(c[col]);
											f64 fd = cast_flt<f64>(d[col]);
											
											fa = srgb::srgb_to_linear_nodiv(fa);
											fb = srgb::srgb_to_linear_nodiv(fb);
											fc = srgb::srgb_to_linear_nodiv(fc);
											fd = srgb::srgb_to_linear_nodiv(fd);
											#else
											f64 fa = srgb::_srgb_to_linear_LUT_tbl[ a[col] ];
											f64 fb = srgb::_srgb_to_linear_LUT_tbl[ b[col] ];
											f64 fc = srgb::_srgb_to_linear_LUT_tbl[ c[col] ];
											f64 fd = srgb::_srgb_to_linear_LUT_tbl[ d[col] ];
											#endif
											
											f64 ab = fa +fb;
											f64 cd = fc +fd;
											
											f64 res = (ab +cd) * 0.25;
											
											res = srgb::linear_to_srgb_nodiv(res);
											
											pix[col] = (u8)( (res * 255.0) +0.5 );
										}
									}
									
									pix += channels;
									prev_pixA += 2 * channels;
									prev_pixB += 2 * channels;
									
								} while (pix != row_end);
								
								row += cur->stride;
								while (pix != row) {
									*pix++ = 0; // dont have garbage data in row padding
								}
								
								prev_rowA += prev_effective_stride;
								prev_rowB += prev_effective_stride;
								
							} while (row != end);
						}
					}
				}
				
				working_stk.purge();
				
				i += bmp_count;
			}
			
			{
				u64 total_size = data_size;
				f64 fract = (f64)largest / (f64)total_size;
				
				print("> largest texture: %  %\\% of total file\n", largest_name ? largest_name : lstr("<null>"), fract * 100.0);
			}
		}
		
		win32::set_filepointer(out_file_handle, 0);
		win32::write_file(out_file_handle, header, header_size);
		
	}
	
	print("Generated Textures file (%)\n", OUT_FILENAME);
	
	return 0;
}
