
#if 0
DECL FORCEINLINE s32 comp_str (lstr cr l, lstr cr r) { // (same as strcmp) -: l<r  0: l==r  +: l>r
	u32 cnt = MIN(l.len, r.len);
	for (u32 i=0; i<cnt; ++i) {
		if (l[i] != r[i])		return (si)(u8)l[i] -(si)(u8)r[i];
	}
	if (		l.len == r.len)	return 0;
	else if	(	l.len < r.len)	return 0 -(si)(u8)r[l.len]; // if l/r is a prefix of r/l then the longer string counts as higher (as if the next character is compared to the null terminator)
	else						return (si)(u8)l[r.len] -0;
}
#else
// this one optimizes better, since i don't care about the char differece
DECL s32 comp_str (lstr cr l, lstr cr r) { // (same as strcmp) -: l<r  0: l==r  +: l>r
	u32 cnt = MIN(l.len, r.len);
	for (u32 i=0; i<cnt; ++i) {
		if (	 (u8)l[i] > (u8)r[i])	return +1;
		else if ((u8)l[i] < (u8)r[i])	return -1;
	}
	if (		l.len == r.len)			return 0;
	else if	(	l.len < r.len)			return -1; // if l/r is a prefix of r/l then the longer string counts as higher (as if the next character is compared to the null terminator)
	else								return +1;
}
#endif

template <typename T>
DECL void inplace_sort (T* arr, u32 len) { // bubble sort
	
	u32 n = len;
	for (;;) {
		u32 new_n = 0;
		for (u32 i=1; i<n; ++i) {
			if (comp(arr[i-1], arr[i]) > 0) {
				{ // swap
					auto tmp = arr[i-1];
					arr[i-1] = arr[i];
					arr[i] = tmp;
				}
				new_n = i;
			}
		}
		
		n = new_n;
		if (n == 0) break;
	}
	
}

template <typename T>
DECL T* binary_search (lstr cr str, T* arr, u32 len) { // not sure which element this returns if there are equal elements
	assert(len != 0);
	if (len != 0) {
		
		u32 lo = 0;
		u32 hi = len -1;
		
		do {
			u32 mid = lo +((hi -lo) / 2);
			
			auto c = comp(arr[mid], str);
			if (		c < 0 ) {
				lo = mid +1;
			} else if ( c > 0 ) {
				if (mid == 0) break;
				hi = mid -1;
			} else {
				return &arr[mid];
			}
			
		} while (lo <= hi);
		
	}
	return nullptr;
}

namespace var {
	
	struct Str_Enum {
		lstr	str;
		u32		e;
	};
	
	DECL s32 comp (Str_Enum cr l, Str_Enum cr r) {
		return comp_str(l.str, r.str);
	}
	DECL s32 comp (Str_Enum cr l, lstr cr r) {
		return comp_str(l.str, r);
	}
	
	struct Token;
}

DECLD dynarr<char>			out_file_data;
DECLD char*					write_copy_cur;
DECLD print_n::Printer		print_copy;

DECL void write_copy (lstr cr str) {
	out_file_data.pushn(str.str, str.len);
}
DECL void write_copy_until (char* until) {
	uptr len = until -write_copy_cur;
	write_copy(lstr(write_copy_cur, safe_cast_assert(u32, until -write_copy_cur)));
	write_copy_cur += len;
}
DECL void write_skip_until (char* until) {
	write_copy_cur = until;
}


DECLD char		string_buf[kibi(2)]; // TODO: HACK: use this until i intruduce a temporary variable buffer that gets copied into the in memory variables after parsing succeeds, which will allow putting str data onto a dynarr and patching the pointers on sucessful parse
DECLD char*		string_buf_cur;

namespace var {
	using namespace parse_n;
	
	enum var_types_e : u32 {
		// These put first so that MSVC shows VT_FLT for value 0
		VT_FLT =									0b0000000000000,
		VT_SINT =									0b0000000100000,
		VT_UINT =									0b0000001100000,
		VT_EXTRAT =									0b0000001000000,
		
		VT_SCALAR =									0b0000000000000,
		VT_VEC2 =									0b0000000000001,
		VT_VEC3 =									0b0000000000010,
		VT_VEC4 =									0b0000000000011,
		VT_QUAT =									0b0000000000100,
		VT_MAT2 =									0b0000000000101,
		VT_MAT3 =									0b0000000000110,
		VT_MAT4 =									0b0000000000111,
		
		VT_SIZE1 =									0b0000000000000,
		VT_SIZE2 =									0b0000000001000,
		VT_SIZE4 =									0b0000000010000,
		VT_SIZE8 =									0b0000000011000,
		
		// If angle unit is set, actual storage in radians (conversion happens on unit being set)
		VT_ANGLE =									0b0000010000000, // Only for floats, actual storage in radians
		VT_COLOR =									0b0000100000000, // Only for floats, actual storage in linear
		
		VT_FPTR_VAR =								0b0001000000000, // pdata is not a variable in memory, but instead a pointer to a 'setter' function
		
		VT_SPECIAL =								0x80000000, // Special values, -> these follow the normal enum scheme (as opposed to the bitfields we use normally)
		
	};
	DEFINE_ENUM_FLAG_OPS(var_types_e, u32)
	namespace var_types_e_n {
		#define T DECLD constexpr var_types_e
		#define TI DECLD constexpr u32
		
		// 
		T VT_FV2 =									VT_FLT|VT_VEC2;
		T VT_FV3 =									VT_FLT|VT_VEC3;
		T VT_FV4 =									VT_FLT|VT_VEC4;
		T VT_FQUAT =								VT_FLT|VT_QUAT;
		
		T VT_SV2 =									VT_SINT|VT_VEC2;
		T VT_SV3 =									VT_SINT|VT_VEC3;
		T VT_SV4 =									VT_SINT|VT_VEC4;
		
		
		T VT_F32 =									VT_FLT|VT_SIZE4;
		T VT_F64 =									VT_FLT|VT_SIZE8;
		
		T VT_S8 =									VT_SINT|VT_SIZE1;
		T VT_S16 =									VT_SINT|VT_SIZE2;
		T VT_S32 =									VT_SINT|VT_SIZE4;
		T VT_S64 =									VT_SINT|VT_SIZE8;
		
		T VT_U8 =									VT_UINT|VT_SIZE1;
		T VT_U16 =									VT_UINT|VT_SIZE2;
		T VT_U32 =									VT_UINT|VT_SIZE4;
		T VT_U64 =									VT_UINT|VT_SIZE8;
		
		T VT_BOOL =									VT_EXTRAT|VT_SIZE1;
		T VT_CSTR =									VT_EXTRAT|VT_SIZE8;
		T VT_LSTR =									VT_EXTRAT|VT_SIZE8|VT_VEC2;
		
		
		T VT_F32V2 =								VT_F32|VT_VEC2;
		T VT_F32V3 =								VT_F32|VT_VEC3;
		T VT_F32V4 =								VT_F32|VT_VEC4;
		T VT_F32QUAT =								VT_F32|VT_QUAT;
		
		T VT_S32V2 =								VT_S32|VT_VEC2;
		T VT_S32V3 =								VT_S32|VT_VEC3;
		T VT_S32V4 =								VT_S32|VT_VEC4;
		
		T VT_U32V2 =								VT_U32|VT_VEC2;
		T VT_U32V3 =								VT_U32|VT_VEC3;
		T VT_U32V4 =								VT_U32|VT_VEC4;
		
		T VT_F32COL =								VT_F32V3|VT_COLOR;
		
		T VT_F32M2 =								VT_F32|VT_MAT2;
		T VT_U32M3 =								VT_F32|VT_MAT3;
		T VT_U32M4 =								VT_F32|VT_MAT4;
		
		// Special values for registered vars data structure
		T VT_KEYWORD =								VT_SPECIAL|(var_types_e)0; // Only for call expr arguments, to allow keywords as special function arguments
		
		T VT_ARRAY =								VT_SPECIAL|(var_types_e)1;
		T VT_DYNARR =							VT_SPECIAL|(var_types_e)2;
		
		T VT_STRUCT =								VT_SPECIAL|(var_types_e)3;
		
		T VT_ENTITY_TREE =							VT_SPECIAL|(var_types_e)4;
		T VT_CHILDREN_LIST =						VT_SPECIAL|(var_types_e)5;
		
		
		TI VT_VEC_OFFS =							0;
		T VT_VEC_BIT =				(var_types_e)	0b000000000111;
		TI VT_SCALAR_SIZE_OFFS =					3;
		T VT_SCALAR_SIZE_BIT =		(var_types_e)	0b000000011000;
		TI VT_SCALAR_TYPE_OFFS =					5;
		T VT_SCALAR_TYPE_BIT =		(var_types_e)	0b000001100000;
		TI VT_UNIT_OFFS =							7;
		T VT_UNIT_BIT =				(var_types_e)	0b000110000000;
		
		DECL constexpr bool is_flt (var_types_e t) {						return (t & VT_SCALAR_TYPE_BIT) == VT_FLT; }
		DECL constexpr bool is_sint (var_types_e t) {						return (t & VT_SCALAR_TYPE_BIT) == VT_SINT; }
		DECL constexpr bool is_flt_or_sint (var_types_e t) {				return (t & VT_EXTRAT) == 0; }
		DECL constexpr bool is_scalar (var_types_e t) {						return (t & VT_VEC_BIT) == VT_SCALAR; }
		DECL constexpr bool is_vec (var_types_e t) {						return (t & VT_VEC_BIT) >= VT_VEC2 && (t & VT_VEC_BIT) <= VT_VEC4; }
		DECL constexpr bool is_quat (var_types_e t) {						return (t & VT_VEC_BIT) == VT_QUAT; }
		DECL constexpr bool is_scalar_or_vec (var_types_e t) {				return (t & VT_QUAT) == 0; }
		DECL constexpr bool is_vec_or_quat (var_types_e t) {				return (t & VT_VEC_BIT) >= VT_VEC2 && (t & VT_VEC_BIT) <= VT_QUAT; }
		DECL constexpr bool is_scalar_or_vec_or_quat (var_types_e t) {		return (t & VT_VEC_BIT) <= VT_QUAT; }
		
		DECL constexpr bool is_str (var_types_e t) {						return t == VT_CSTR || t == VT_LSTR; }
		
		DECL constexpr bool is_size_4_or_8 (var_types_e t) {				return t & VT_SIZE4; }
		
		DECL constexpr u32			get_scalar_size_indx (var_types_e t) {	return (t & VT_SCALAR_SIZE_BIT) >> VT_SCALAR_SIZE_OFFS; }
		DECL constexpr u32			get_scalar_size_bits (var_types_e t) {	return (u32)8 << get_scalar_size_indx(t); }
		
		// Does not work for quat
		DECL constexpr u32			get_vec_dimensions (var_types_e t) {	return (t & VT_VEC_BIT) -VT_SCALAR +1; }
		DECL constexpr var_types_e	set_vec_dimensions (u32 i) {			return (var_types_e)(VT_SCALAR +(i -1)); }
		
		DECL u32					get_elements (var_types_e t) {
			t &= VT_VEC_BIT;
			switch (t) {
				case VT_SCALAR:
				case VT_VEC2:
				case VT_VEC3:
				case VT_VEC4:
					return t -VT_SCALAR +1;
				case VT_QUAT:
					return t -VT_SCALAR; // 4
				case VT_MAT2:
				case VT_MAT3:
				case VT_MAT4:
					return SQUARED(t -VT_SCALAR -3);
					
				default: assert(false); return 1; // can never happen
			}
		}
		DECL constexpr var_types_e	set_mat_dim (u32 i) {					return (var_types_e)(VT_MAT2 +(i -2)); }
		
		DECLD constexpr char const* VEC_TYPE_NAME[8] = {
			"scalar",
			"v2",
			"v3",
			"v4",
			"quat",
			"m2",
			"m3",
			"m4",
		};
		DECL constexpr char const* get_vec_name (var_types_e t) {			return VEC_TYPE_NAME[t & VT_VEC_BIT]; }
		
		DECLD constexpr char const* SCALAR_TYPE_NAME[2] = {
			"flt",
			"int",
		};
		DECL constexpr char const* get_scalar_type_name (var_types_e t) {	return SCALAR_TYPE_NAME[(t & VT_SCALAR_TYPE_BIT) >> VT_SCALAR_TYPE_OFFS]; }
		
		DECL bool is_array (var_types_e t) {
			switch (t) {
				case VT_ENTITY_TREE:
				case VT_ARRAY:
				case VT_DYNARR:
					return true;
				default:
					return false;
			}
		}
		
		DECL bool basic_type (var_types_e t) {
			return !(t & VT_SPECIAL);
		}
		
		#undef T
		#undef TI
	}
	using namespace var_types_e_n;
	
	typedef tm4<s64> expr_val_im;
	typedef tm4<f64> expr_val_fm;
	
	enum keyword_e : u32;
	
	struct Expr_Val {
		var_types_e		type;
		union {
			expr_val_fm fm;
			expr_val_im im;
			keyword_e	kw;
			lstr		lstr;
		};
		
		// MSVC now suddenly gives 'attempting to reference a deleted function' without this??
		Expr_Val () {}
	};
	
	union Val_Union { // For temporary storage in print_basic_type()
		expr_val_fm fm;
		expr_val_im im;
		char const* cstr;
	};
	
	DECLD constexpr ui	MAX_VAR_SIZE = 16 * 8;
	
	#define SETTER_FUNC_SIG(name)	void (name)(Expr_Val cr val)
	typedef SETTER_FUNC_SIG(* setter_fp);
	
	DECL SETTER_FUNC_SIG(setter_vsync_swap_interval) {
		set_vsync((s32)val.im.row(0,0));
	}
	
	union Var;
	
	struct Var_Single {
		Var*				next;
		uptr				pdata;
		lstr				name;
		var_types_e			type;
	};
	struct Var_Struct {
		Var*				next;
		uptr				pdata;
		lstr				name;
		var_types_e			type;
		
		Var*				members;
	};
	struct Var_Array {
		Var*				next;
		uptr				pdata;
		lstr				name;
		var_types_e			type;
		
		Var*				members;
		uptr				arr_len;
		uptr				arr_stride;
	};
	
	union Var {
		struct {
			Var*			next;
			uptr			pdata;
			lstr			name;
			var_types_e		type;
		};
		Var_Struct			strct;
		Var_Array			arr;
	};
	
	DECLV Var* _struct (u32 count, ...) {
		
		va_list vl;
		va_start(vl, count);
		
		Var* ret = va_arg(vl, Var*);
		Var* cur = ret;
		
		for (u32 i=1; i<count; ++i) {
			Var* next = va_arg(vl, Var*);
			cur->next = next;
			cur = next;
		}
		
		va_end(vl);
		
		return ret;
	}
	template <typename... ARGS>
	DECL FORCEINLINE Var* STRUCT (ARGS... args) {
		return _struct(packlenof<u32, ARGS...>(), args...);
	}
	
	DECL Var* _var (uptr pdata, lstr name, var_types_e type) {
		return (Var*)working_stk.push<Var_Single>({nullptr, pdata, name, type});
	}
	DECL Var* _var (uptr pdata, lstr name, Var* members) { // var is struct
		return (Var*)working_stk.push<Var_Struct>({nullptr, pdata, name, VT_STRUCT, members});
	}
	
	DECL Var* _array (uptr pdata, lstr name, var_types_e type, uptr len, uptr stride, var_types_e memb_type) {
		return (Var*)working_stk.push<Var_Array>({nullptr, pdata, name, type, _var(0,"<array_member>",memb_type), len, stride});
	}
	DECL Var* _array (uptr pdata, lstr name, var_types_e type, uptr len, uptr stride, Var* members) { // arr member is struct
		return (Var*)working_stk.push<Var_Array>({nullptr, pdata, name, type, _var(0,"<array_member>",members), len, stride});
	}
	
	#define GBL_VAR(inst, type)					_var(		(uptr)&inst,			STRINGIFY(inst),	type)
	#define GBL_VAR_NAME(name, inst, type)		_var(		(uptr)&inst,			STRINGIFY(name),	type)
	#define GBL_ARRAY(inst, stride, memb)		_array(		(uptr)&inst,			STRINGIFY(inst),	VT_ARRAY,		arrlenof(inst),			stride, memb)
	#define GBL_DYNARR(inst, stride, memb)		_array(		(uptr)&inst,			STRINGIFY(inst),	VT_DYNARR,	0,						stride, memb)
	
	#define VAR(s,m, type)						_var(		offsetof(s,m),			STRINGIFY(m),		type)
	#define VAR_SETTR(inst, type)				_var(		(uptr)&setter_##inst,	STRINGIFY(inst),	VT_FPTR_VAR|type)
	#define ARRAY(s,m, stride, memb)			_array(		offsetof(s,m),			STRINGIFY(m),		VT_ARRAY,		arrlenof(((s*)0)->m),	stride, memb)
	#define DYNARR(s,m, stride, memb)			_array(		offsetof(s,m),			STRINGIFY(m),		VT_DYNARR,	0,						stride, memb)
	
	DECL Var* _var_entities (uptr pdata, lstr name, Var* members) { // var is struct
		return (Var*)working_stk.push<Var_Struct>({nullptr, pdata, name, VT_ENTITY_TREE, members});
	}
	
	DECLD Var*		root_var;
	
	void init_vars () {
		
		auto strct_camera = STRUCT(
			VAR(		Camera,				pos_world,							VT_F32V3),
			VAR(		Camera,				aer,								VT_F32V3|VT_ANGLE),
			VAR(		Camera,				base_ori,							VT_F32QUAT),
			VAR(		Camera,				vfov,								VT_F32|VT_ANGLE),
			VAR(		Camera,				clip_near,							VT_F32),
			VAR(		Camera,				clip_far,							VT_F32)
		);
		
		#if 0
		auto entity = STRUCT(
			VAR(		Entity,				name,							VT_CSTR),
			VAR(		Entity,				children,						VT_CHILDREN_LIST),
			VAR(		Entity,				pos,							VT_F32V3),
			VAR(		Entity,				ori,							VT_F32QUAT),
			VAR(		Entity,				scale,							VT_F32V3),
			VAR(		Entity,				tag,							VT_U32),
			VAR(		Entity,				Mesh,							STRUCT(
				VAR(		Entity_Mesh,		mesh_name,						VT_CSTR)
			)),
			VAR(		Entity,				Light,							STRUCT(
				VAR(		Entity_Light,		type,							VT_U32),
				VAR(		Entity_Light,		flags,							VT_U32),
				VAR(		Entity_Light,		power,							VT_F32V3)
			))
		);
		#endif
		
		root_var = _var(0, "<global>",
		STRUCT(
			VAR_SETTR(						vsync_swap_interval,				VT_S32),
			GBL_VAR(						controls,							STRUCT(
				VAR(		Controls,			cam_mouselook_sens,					VT_F32V2),
				VAR(		Controls,			cam_roll_vel,						VT_F32|VT_ANGLE),
				VAR(		Controls,			cam_translate_vel,					VT_F32V3),
				VAR(		Controls,			cam_translate_fast_mult,			VT_F32V3),
				VAR(		Controls,			cam_fov_control_vel,				VT_F32|VT_ANGLE),
				VAR(		Controls,			cam_fov_control_mw_vel,				VT_F32|VT_ANGLE)
			)),
			GBL_VAR(						passes,								STRUCT(
				VAR(		Passes,				main_pass_clear_col,				VT_F32V3|VT_COLOR),
				VAR(		Passes,				bloom_intensity,					VT_F32),
				ARRAY(		Passes,				bloom_size,			sizeof(f32),	VT_F32),
				ARRAY(		Passes,				bloom_res_scale,	sizeof(f32),	VT_U32),
				ARRAY(		Passes,				bloom_coeff,		sizeof(f32),	VT_F32),
				VAR(		Passes,				shadow_res,							VT_U32)
			)),
			GBL_VAR(						env_viewer,							STRUCT(
				VAR(		Env_Viewer,			illuminance_res,					VT_U32),
				VAR(		Env_Viewer,			luminance_res,						VT_U32),
				VAR(		Env_Viewer,			luminance_prefilter_levels,			VT_U32),
				VAR(		Env_Viewer,			luminance_prefilter_base_samples,	VT_U32),
				VAR(		Env_Viewer,			cur_humus_indx,						VT_U32),
				VAR(		Env_Viewer,			cur_sibl_indx,						VT_U32),
				VAR(		Env_Viewer,			sibl,								VT_BOOL)
			)),
			GBL_VAR(						camera,								strct_camera),
			GBL_ARRAY(						saved_cameras,		sizeof(Camera),	strct_camera),
			GBL_ARRAY(						materials,				sizeof(std140_Material), STRUCT(
				VAR(		std140_Material,	albedo,								VT_F32COL),
				VAR(		std140_Material,	metallic,							VT_F32),
				VAR(		std140_Material,	roughness,							VT_F32),
				VAR(		std140_Material,	IOR,								VT_F32)
			)),
			GBL_VAR(						tex,								STRUCT(
				ARRAY(		Textures,			names,					sizeof(char const*), VT_CSTR)
			)),
			GBL_VAR(						material_showcase_grid_steps,			VT_U32V2),
			GBL_VAR(						material_showcase_grid_mat,				VT_F32M2)
			
			#if 1
			#else
			_var_entities(		(uptr)&scenes,		STRINGIFY(scenes),			entity)
			#endif
			
		));
		
	}
	
	#undef GBL_VAR
	#undef GBL_ARRAY
	#undef GBL_ARR_S
	#undef GBL_DYNARR
	#undef GBL_DYNARR_S
	
	#undef VAR
	#undef VAR_SETTR
	#undef ARRAY
	#undef ARR_S
	#undef DYNARR
	#undef DYNARR_S
	
	DECL bool comp_iden (lstr cr iden, lstr cr str) {
		return str::comp(iden, str);
	}
	DECL u32 match_iden (lstr cr iden, Str_Enum* tbl, u32 tbl_size) {
		auto res = binary_search(iden, tbl, tbl_size);
		return res ? res->e : tbl_size;
	}
	
	////
	enum keyword_e : u32 {
		UNIT_RAD=0,
		UNIT_DEG,
		UNIT_COL,
		UNIT_SRGB,
		TYPE_V2,
		TYPE_V3,
		TYPE_V4,
		TYPE_SV2,
		TYPE_SV3,
		TYPE_SV4,
		TYPE_UV2,
		TYPE_UV3,
		TYPE_UV4,
		TYPE_QUAT,
		TYPE_M2,
		TYPE_M3,
		TYPE_M4,
		LIT_PI,
		LIT_FALSE,
		LIT_TRUE,
		LIT_NULL,
		LIT_IDENTITY,
		FUNC_NORMALIZE,
		KEYWORDS_COUNT
	};
	DEFINE_ENUM_ITER_OPS(keyword_e, u32)
	
	DECLD lstr KEYWORDS[KEYWORDS_COUNT] = {
		"rad",		
		"deg",		
		"col",		
		"srgb",		
		"v2",		
		"v3",		
		"v4",		
		"sv2",		
		"sv3",		
		"sv4",		
		"uv2",		
		"uv3",		
		"uv4",		
		"quat",		
		"m2",		
		"m3",		
		"m4",		
		"PI",		
		"false",	
		"true",		
		"null",		
		"ident",	
		"normalize",
	};
	
	DECLD Str_Enum keywords[KEYWORDS_COUNT] = {
		{"rad",			UNIT_RAD		},
		{"deg",			UNIT_DEG		},
		{"col",			UNIT_COL		},
		{"srgb",		UNIT_SRGB		},
		{"v2",			TYPE_V2			},
		{"v3",			TYPE_V3			},
		{"v4",			TYPE_V4			},
		{"sv2",			TYPE_SV2		},
		{"sv3",			TYPE_SV3		},
		{"sv4",			TYPE_SV4		},
		{"uv2",			TYPE_UV2		},
		{"uv3",			TYPE_UV3		},
		{"uv4",			TYPE_UV4		},
		{"quat",		TYPE_QUAT		},
		{"m2",			TYPE_M2			},
		{"m3",			TYPE_M3			},
		{"m4",			TYPE_M4			},
		{"PI",			LIT_PI			},
		{"false",		LIT_FALSE		},
		{"true",		LIT_TRUE		},
		{"null",		LIT_NULL		},
		{"ident",		LIT_IDENTITY	},
		{"normalize",	FUNC_NORMALIZE	},
	};
	
	DECL constexpr bool is_angle_unit (keyword_e kw) {			return kw >= UNIT_RAD && kw <= UNIT_DEG; }
	DECL constexpr bool is_color_unit (keyword_e kw) {			return kw >= UNIT_COL && kw <= UNIT_SRGB; }
	DECL constexpr bool is_vec (keyword_e kw) {					return kw >= TYPE_V2 && kw <= TYPE_UV4; }
	DECL constexpr bool is_quat (keyword_e kw) {				return kw == TYPE_QUAT; }
	DECL constexpr bool is_mat (keyword_e kw) {					return kw >= TYPE_M2 && kw <= TYPE_M4; }
	DECL constexpr bool is_keyword_literal (keyword_e kw) {		return kw >= LIT_PI && kw <= LIT_IDENTITY; }
	
	DECL u32			get_kw_vec_dim (keyword_e kw) {
		assert(kw >= UNIT_COL && kw <= TYPE_UV4, "%", u32(kw));
		return kw < TYPE_V2 ? 3 : ((kw -TYPE_V2) % 3) +2;
	}
	DECL u32			get_kw_mat_dim (keyword_e kw) {
		assert(kw >= TYPE_M2 && kw <= TYPE_M4, "%", u32(kw));
		return (kw -TYPE_M2) +2;
	}
	
	#define KW_ANGLE_UNIT_CASES				UNIT_RAD: case UNIT_DEG
	#define KW_COLOR_UNIT_CASES				UNIT_COL: case UNIT_SRGB
	#define KW_FVEC_CASES					TYPE_V2: case TYPE_V3: case TYPE_V4
	#define KW_VEC_CASES					KW_FVEC_CASES: \
											case TYPE_SV2: case TYPE_SV3: case TYPE_SV4: \
											case TYPE_UV2: case TYPE_UV3: case TYPE_UV4
	#define KW_FMAT_CASES					TYPE_M2: case TYPE_M3: case TYPE_M4
	#define KW_KEYWORD_LITERAL_CASES		LIT_PI: case LIT_FALSE: case LIT_TRUE: case LIT_NULL: case LIT_IDENTITY
	
	#define KW_UNIT_CASES					KW_ANGLE_UNIT_CASES: case KW_COLOR_UNIT_CASES
	#define KW_CONSTRUCTOR_CASES			KW_VEC_CASES: case TYPE_QUAT: case KW_FMAT_CASES
	
	#define KW_FUNC_CALL_CASES				KW_UNIT_CASES: case KW_CONSTRUCTOR_CASES: case FUNC_NORMALIZE
	
	DECL keyword_e match_keyword (lstr cr iden) {
		return (keyword_e)match_iden(iden, keywords, KEYWORDS_COUNT);
	}
	
	enum dollar_command_e : u32 {
		DC_CURRENT=0,
		DC_INITIAL,
		DC_SAVE,
		DOLLAR_COMMAND_COUNT
	};
	
	DECLD Str_Enum dollar_commands[] = {
		{"",	DC_CURRENT	},
		{"i",	DC_INITIAL	},
		{"s",	DC_SAVE		},
	};
	
	DECL dollar_command_e match_dollar_command (lstr cr iden) {
		return (dollar_command_e)match_iden(iden, dollar_commands, DOLLAR_COMMAND_COUNT);
	}
	
	enum parse_flags_e : u32 {
		PF_INFER_TYPE=							0b00000001,
		PF_STRING_APPEND=						0b00000010,
		PF_MEMBER_OF_ARRAY=						0b00000100,
		PF_IN_NNARY=							0b00001000,
		PF_IN_DOLLAR_CMD_VAL=					0b00010000,
		PF_ONLY_SYNTAX_CHECK=					0b00100000,
		PF_INITIAL_LOAD=						0b01000000,
		PF_WRITE_CURRENT=						0b10000000,
	};
	DECLD constexpr u32		PF_WRITE_SAVE=		0b11000000;
	DECLD constexpr u32		PF_WRITE_BIT=		0b11000000;
	DEFINE_ENUM_FLAG_OPS(parse_flags_e, u32)
	
	enum dollar_cmd_set_e : u32 {
		SET=		0b001,
		CURRENT=	0b010,
		INITIAL=	0b100,
	};
	DEFINE_ENUM_FLAG_OPS(dollar_cmd_set_e, u32)
	
	#define _DECL DECL
	#define _DECLV DECLV
	
	#define require(cond) if (!(cond)) { return 0; }
	#define requiret(cond) if (!(cond)) { return array<Token>{nullptr,0}; }
	
	enum token_e : u32 {
		EOF_=				0,
		EOF_MARKER,
		IDENTIFIER,
		KEYWORD,
		NUMBER,
		STRING,
		DOLLAR_CMD,
		PERCENT,
		MEMBER_DOT,
		EQUALS,
		SEMICOLON,
		COMMA,
		NNARY_COND,
		NNARY_SEPER,
		PAREN_OPEN,
		PAREN_CLOSE,
		BRACKET_OPEN,
		BRACKET_CLOSE,
		CURLY_OPEN,
		CURLY_CLOSE,
		ANGULAR_OPEN,
		ANGULAR_CLOSE,
		MINUS,
		PLUS,
		MULTIPLY,
		DIVIDE,
		OR,
		AND,
		
		ERROR_REP_NEWLINE,
		ERROR_REP_IN_COMMENT,
		ERROR_REP_TOKENIZER,
	};
	char const* TOKEN_NAME[] = {
		"EOF_",
		"EOF_MARKER",
		"IDENTIFIER",
		"KEYWORD",
		"NUMBER",
		"STRING",
		"DOLLAR_CMD",
		"PERCENT",
		"MEMBER_DOT",
		"EQUALS",
		"SEMICOLON",
		"COMMA",
		"NNARY_COND",
		"NNARY_SEPER",
		"PAREN_OPEN",
		"PAREN_CLOSE",
		"BRACKET_OPEN",
		"BRACKET_CLOSE",
		"CURLY_OPEN",
		"CURLY_CLOSE",
		"ANGULAR_OPEN",
		"ANGULAR_CLOSE",
		"MINUS",
		"PLUS",
		"MULTIPLY",
		"DIVIDE",
		"OR",
		"AND",
		"<newline>",
		"<in_comment>",
		"<in_tokenizer>",
	};
	
	struct Token {
		token_e					tok;
		union {
			keyword_e			kw;
			dollar_command_e	dollar_cmd;
		};
		// string data
		char*					begin;
		u32						len;
		// Source pos for error reporting
		u32						line_num;
		char*					line_begin;
		
		constexpr lstr get_lstr () const {
			return { begin, len };
		}
		constexpr uptr get_char_pos () const {
			return ptr_sub(line_begin, begin);
		}
	};
	
	#define VAR_PARSE_ERROR_REPORTING 1
	
	#if VAR_PARSE_ERROR_REPORTING
	
	_DECL void print_error_lines (Token const* first, Token const* last) {
		
		//	Just the first token for now
		//	TODO:
		//		add smarter handing of lines longer than the console width
		//		add printing of multiple lines (when there is a range of tokens)
		//		 where you print at max 2 lines (for more than 2 lines print the first and last)
		
		assert(first->begin <= last->begin);
		
		char* line[2];
		char* line_end[2];
		
		char* toks_begin[2];
		char* toks_end[2];
		
		line[0] = first->line_begin;
		line[1] = last->line_begin;
		
		assert(first->begin >= line[0]);
		assert(last->begin >= line[1]);
		
		assert(first->line_num <= last->line_num);
		
		u32 multiple_lines = last->line_num -first->line_num;
		ui lines_to_print = multiple_lines ? 2 : 1;
		
		assert((multiple_lines != 0) == (line[0] != line[1]));
		
		for (ui i=0; i<lines_to_print; ++i) {
			char* cur = line[i];
			while (!is_newline_c(*cur) && *cur != '\0') { ++cur; }
			
			if (cur == last->begin) { // Token is set to a newline pseudo-token or EOF_ token, include them in the line print
				if (*cur == '\0') {
					++cur;
				} else {
					cur = newline(cur);
				}
			}
			
			line_end[i] = cur;
		}
		
		if (multiple_lines) {
			toks_begin[0] = first->begin;
			toks_end[0] = line_end[0];
			toks_begin[1] = line[1];
			toks_end[1] = last->begin +last->len;
		} else {
			toks_begin[0] = first->begin;
			toks_end[0] = last->begin +last->len;
		}
		
		constexpr uptr MAX_LINE_LEN = kibi(4);
		
		DEFER_POP(&working_stk);
		char* line_esc = working_stk.pushArr<char>(MAX_LINE_LEN);
		u32 line_esc_len;
		
		for (ui i=0; i<lines_to_print; ++i) {
			assert(first->len > 0);
			uptr pad = 0;
			uptr tail = 0;
			
			{
				char* out = line_esc;
				char* out_end = line_esc +MAX_LINE_LEN;
				
				char* in = line[i];
				char* in_end = line_end[i];
				
				while (in != in_end) {
					assert(ptr_sub(out, out_end) >= 4); // 4 because print_escaped_ascii_len() needs 4 and tab is 4 spaces
					uptr len;
					char c = *in++;
					switch (c) {
						case '\t': {
							out[0] = ' ';
							out[1] = ' ';
							out[2] = ' ';
							out[3] = ' ';
							len = 4;
						} break;
						default: {
							len = print_escaped_ascii_len(c, out);
						}
					}
					out += len;
					
					if (in <= toks_begin[i]) {
						pad += len;
					} else if (in <= toks_end[i]) {
						tail += len;
					}
				}
				
				line_esc_len = safe_cast_assert(u32, ptr_sub(line_esc, out));
			}
			
			print_err("	 %\n", lstr(line_esc, line_esc_len));
			
			char const* format;
			if (i == (lines_to_print -1)) {
				format = "	%%^\n";
				tail = tail == 0 ? 0 : tail -1;
			} else {
				format = "	%%\n";
			}
			print_err(format, repeat(" ", pad), repeat("~", tail));
			
			if (i == 0 && multiple_lines > 2) {
				print_err("	 ...\n");
			}
		}
		
		print_err("\n");
		
	}
	
	_DECLV void _2_syntax_error (char const* str, print_type_e const* types, Token const* first, Token const* last, ...) {
		DEFER_POP(&working_stk);
		
		va_list vl;
		va_start(vl, last);
		
		lstr msg;
		vl = print_n::_print_base_ret_vl(&print_working_stk, str, types, vl, &msg);
		
		va_end(vl);
		
		assert(first && last);
		
		print_err("\nSyntax error on % at %:%!\n  %\n",
				TOKEN_NAME[last->tok], last->line_num, last->get_char_pos(), msg);
		
		print_error_lines(first, last);
		
		DBGBREAK_IF_DEBUGGER_PRESENT;
	}
	_DECLV void _2_warning (char const* str, print_type_e const* types, Token const* first, Token const* last, ...) {
		DEFER_POP(&working_stk);
		
		va_list vl;
		va_start(vl, last);
		
		lstr msg;
		vl = print_n::_print_base_ret_vl(&print_working_stk, str, types, vl, &msg);
		
		va_end(vl);
		
		assert(first && last);
		
		print_err("\nWarning on % at %:%!\n	 %\n",
				TOKEN_NAME[last->tok], last->line_num, last->get_char_pos(), msg);
		
		print_error_lines(first, last);
		
		DBGBREAK_IF_DEBUGGER_PRESENT;
	}
	struct Syntax_Error_Msg_Functor {
		Token const* first;
		Token const* last;
		
		template <typename... Ts>
		DECLM FORCEINLINE void operator() (print_n::Base_Printer* this_, char const* str, print_type_e const* type_arr, Ts... args) {
			_2_syntax_error(str, type_arr, first, last, args...);
		}
	};
	struct Warning_Msg_Functor {
		Token const* first;
		Token const* last;
		
		template <typename... Ts>
		DECLM FORCEINLINE void operator() (print_n::Base_Printer* this_, char const* str, print_type_e const* type_arr, Ts... args) {
			_2_warning(str, type_arr, first, last, args...);
		}
	};
	#endif
	
	#define VAR_PARSE_DEV_ERROR_REPORTING 1
	
	#if VAR_PARSE_DEV_ERROR_REPORTING
	_DECLV void _2_dev_syntax_error (char const* str, print_type_e const* types, ...) {
		DEFER_POP(&working_stk);
		
		va_list vl;
		va_start(vl, types);
		
		lstr msg;
		vl = print_n::_print_base_ret_vl(&print_working_stk, str, types, vl, &msg);
		
		va_end(vl);
		
		print_err("	   %\n", msg);
	}
	struct Dev_Syntax_Error_Msg_Functor {
		template <typename... Ts>
		DECLM FORCEINLINE void operator() (print_n::Base_Printer* this_, char const* str, print_type_e const* type_arr, Ts... args) {
			_2_dev_syntax_error(str, type_arr, args...);
		}
	};
	#endif
	
	template<typename... Ts> DECL FORCEINLINE void _1_syntax_error (Token const* first, Token const* last, Ts... msg) {
	#if VAR_PARSE_ERROR_REPORTING
		Syntax_Error_Msg_Functor f{first, last};
		print_n::_print_wrapper(f, nullptr, msg...);
	#endif
	}
	template<typename... Ts> DECL FORCEINLINE void _1_warning (Token const* first, Token const* last, Ts... msg) {
	#if VAR_PARSE_ERROR_REPORTING
		Warning_Msg_Functor f{first, last};
		print_n::_print_wrapper(f, nullptr, msg...);
	#endif
	}
	template<typename... Ts> DECL FORCEINLINE void _1_dev_syntax_error (Ts... msg) {
	#if VAR_PARSE_DEV_ERROR_REPORTING
		Dev_Syntax_Error_Msg_Functor f{};
		print_n::_print_wrapper(f, nullptr, msg...);
	#endif
	}
	
	template<typename... Ts> DECL FORCEINLINE bool _syntax (bool cond, Token const* first, Token const* last, Ts... msg) {
		if (!cond) {
			_1_syntax_error(first, last, msg...);
		}
		return cond;
	}
	template<typename... Ts> DECL FORCEINLINE bool warning_ (bool cond, Token const* first, Token const* last, Ts... msg) {
		if (!cond) {
			_1_warning(first, last, msg...);
		}
		return cond;
	}
	template<typename... Ts> DECL FORCEINLINE bool _dev_syntax_error (bool cond, Ts... msg) {
		if (!cond) {
			_1_dev_syntax_error(msg...);
		}
		return cond;
	}
	
	
	#define syntax(cond, first, last, ...)	require(_syntax(cond, first, last, __VA_ARGS__))
	#define syntaxdev(cond, ...)			require(_dev_syntax_error(cond, __VA_ARGS__))
	
	#define syntaxt(cond, first, last, ...)	requiret(_syntax(cond, first, last, __VA_ARGS__))
	#define syntaxdevt(cond, ...)			requiret(_dev_syntax_error(cond, __VA_ARGS__))
	
	_DECL char* line_comment (char* cur, u32* line_num, char** line_begin) {
		require(*cur == '#');
		
		do {
			++cur;
		} while (!(*cur == '\r' || *cur == '\n'));
		
		cur = newline(cur, line_num, line_begin);
		
		return cur;
	}
	_DECL NOINLINE char* comment (char* cur, u32* line_num, char** line_begin) {
		
		assert(*cur == '{' && cur[1] == '#');
		Token fake_ftok = { ERROR_REP_IN_COMMENT, {(keyword_e)0}, cur, 2, *line_num, *line_begin };
		
		cur += 2;
		
		ui depth = 0;
		for (;;) {
			switch (*cur) {
				case '\0': {
					Token fake_ltok = { ERROR_REP_IN_COMMENT, {(keyword_e)0}, cur, 1, *line_num, *line_begin };
					syntax(false, &fake_ftok, &fake_ltok, "comment:: comment not closed before end of file");
				} break;
				
				case '#':
					if (cur[1] == '}') {
						cur += 2;
						if (depth == 0) {
							return cur;
						}
						--depth;
					} else {
						cur = line_comment(cur, line_num, line_begin);
					}
					break;
				
				case '\r': case '\n':
					cur = newline(cur, line_num, line_begin);
					break;
				
				case '{':
					if (cur[1] == '#') {
						cur += 2;
						++depth;
						break;
					}
					// Fallthrough
				default:
					++cur;
					break;
			}
		}
	}
	
	#define DEC_DIGIT_CASES \
				 '0': case '1': case '2': case '3': case '4': \
			case '5': case '6': case '7': case '8': case '9'
	#define NUMBER_START_CASES DEC_DIGIT_CASES
	
	#define IDENTIFIER_START_CASES \
				 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': \
			case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': \
			case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z': \
			case '_': \
			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': \
			case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': \
			case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z'
	#define IDENTIFIER_CASES	NUMBER_START_CASES: case IDENTIFIER_START_CASES
	
	DECL bool FORCEINLINE is_identifier_start_c (char c) {
		switch (c) {
			case IDENTIFIER_START_CASES:	return true;
			default:				return false;
		}
	}
	DECL bool FORCEINLINE is_identifier_c (char c) {
		switch (c) {
			case IDENTIFIER_CASES:	return true;
			default:				return false;
		}
	}
	DECL bool FORCEINLINE is_dec_digit_c (char c) {
		switch (c) {
			case DEC_DIGIT_CASES:	return true;
			default:				return false;
		}
	}
	
	#define BIN_DIGIT_CASES '0': case '1'
	#define HEX_LOWER_CASES 'a': case 'b': case 'c': case 'd': case 'e': case 'f'
	#define HEX_UPPER_CASES 'A': case 'B': case 'C': case 'D': case 'E': case 'F'
	
	#define HEX_DIGIT_CASES DEC_DIGIT_CASES: case HEX_LOWER_CASES: case HEX_UPPER_CASES
	
	DECL bool FORCEINLINE is_bin_digit_c (char c) {
		switch (c) {
			case BIN_DIGIT_CASES:	return true;
			default:				return false;
		}
	}
	DECL bool FORCEINLINE is_hex_digit_c (char c) {
		switch (c) {
			case HEX_DIGIT_CASES:	return true;
			default:				return false;
		}
	}
	DECL bool FORCEINLINE is_hex_lower_c (char c) {
		switch (c) {
			case HEX_LOWER_CASES:	return true;
			default:				return false;
		}
	}
	DECL bool FORCEINLINE is_hex_upper_c (char c) {
		switch (c) {
			case HEX_UPPER_CASES:	return true;
			default:				return false;
		}
	}
	
	_DECL array<Token> tokenize (array<char> cr source_data) {
		PROFILE_SCOPED(THR_ENGINE, "tokenize");
		
		auto	tokens = dynarr<Token>::alloc(kibi(4));
		
		char*	cur = (char*)&source_data[0];
		
		u32		line_num = 1;
		char*	line_begin = cur;
		
		for (;;) {
			
			for (;;) {
				switch (*cur) {
					case ' ': case '\t':
						cur = whitespace(cur);
						break;
					
					case '{':
						if (*cur == '{' && cur[1] == '#') {
							syntaxdevt(cur = comment(cur, &line_num, &line_begin), "tokenize:: comment()");
							//print("comment\n");
							break;
						} else {
							goto end_l;
						}
					
					case '#':
						cur = line_comment(cur, &line_num, &line_begin);
						//print("line_comment\n");
						break;
					
					case '\r': case '\n':
						cur = newline(cur, &line_num, &line_begin);
						//print("newline\n");
						break;
					
					default: {
						goto end_l;
					}
				}
			} end_l:;
			
			auto& tok = tokens.push();
			
			#if DBG_INTERNAL
			tok.kw = (keyword_e)-1;
			#endif
			
			tok.line_num = line_num;
			tok.line_begin = line_begin;
			
			tok.begin = cur;
			char* tok_end = cur +1; // default 1 char token
			
			lstr str;
			
			switch (*cur) {
				case '\0':						tok.tok = EOF_;
					break;
				
				case '^':						tok.tok = EOF_MARKER; {
					
					++cur; // consume inital '^'
					
					str.str = cur;
					
					while (*cur != '^' && *cur != '\0' && !is_newline_c(*cur)) {
						++cur;
					}
					
					switch(*cur) {
						case '\0': {
							tok.len = (u32)ptr_sub(tok.begin, cur);
							Token tokl = { EOF_, {(keyword_e)0}, cur, 1, line_num, line_begin };
							syntaxt(false, &tok, &tokl, "eof_marker:: EOF_ encountered inside eof maker (^EOF_MARKER^).");
						}
						case NEWLINE_C_CASES: {
							tok.len = (u32)ptr_sub(tok.begin, cur);
							Token tokl = { ERROR_REP_NEWLINE, {(keyword_e)0},
									cur, (u32)ptr_sub(cur, newline(cur)), line_num, line_begin };
							syntaxt(false, &tok, &tokl, "eof_marker:: Newline encountered inside eof maker (^EOF_MARKER^).");
						}
						default: {}
					}
					
					str.len = safe_cast_assert(u32, ptr_sub(str.str, cur));
					
					++cur; // consume terminating '^'
					
					if (!str::comp(str, "EOF_MARKER")) {
						tok.len = (u32)ptr_sub(tok.begin, cur);
						syntaxt(false, &tok,&tok, "eof_marker:: expected ^EOF_MARKER^.");
					}
					
				tok_end = cur;
				} break;
				
				case IDENTIFIER_START_CASES: {
					
					str.str = cur;
					
					do {
						++cur;
					} while (is_identifier_c(*cur));
					
					str.len = safe_cast_assert(u32, ptr_sub(str.str, cur));
					
					// I could i theory allow for variables to be called the same as language keywords
					//	because it is never ambiguous which one is which, but it makes our
					//	this approach simpler if we can clearly say: this is a var-identifier and this a keyword
					
					auto kw = match_keyword(str);
					if (kw != KEYWORDS_COUNT) {
						tok.tok = KEYWORD;
						tok.kw = kw;
						
					} else { // No keyword matched
						tok.tok = IDENTIFIER;
						
					}
					
				tok_end = cur;
				} break;
				
				case NUMBER_START_CASES:		tok.tok = NUMBER; {
					
					bool hex_or_bin = *cur == '0';
					if (hex_or_bin) {
						switch (cur[1]) {
							case 'x': {
								++cur;
								do {
									++cur;
								} while (is_hex_digit_c(*cur));
							} break;
							
							case 'b': {
								++cur;
								do {
									++cur;
								} while (is_bin_digit_c(*cur));
							} break;
							
							default:
								hex_or_bin = false;
						}
					}
					
					if (!hex_or_bin) {
						do {
							++cur;
						} while (is_dec_digit_c(*cur));
						
						if (*cur == '.') {
							if (cur[1] == '@') { // Special float values like 1.@IND
								++cur;
								do {
									++cur;
								} while (is_identifier_c(*cur));
							} else {
								do {
									++cur;
								} while (is_dec_digit_c(*cur));
							}
						}
					}
					
					if (*cur == '_') {
						do {
							++cur;
						} while (is_identifier_c(*cur));
					}
					
				tok_end = cur;
				} break;
				
				case '"':						tok.tok = STRING; {
					
					++cur; // consume inital '"'
					
					while (*cur != '"' && *cur != '\0' && !is_newline_c(*cur)) {
						
						if (*cur == '`') {
							if (cur[1] == '`' || cur[1] == '"') {
								++cur; // escaped '"' or '`'
							} else {
								tok.len = (u32)ptr_sub(tok.begin, cur +1);
								syntaxt(false, &tok, &tok, "Escape character '`' has to always be escaped (``) for consistency.");
							}
						}
						
						++cur;
					}
					
					switch(*cur) {
						case '\0': {
							tok.len = (u32)ptr_sub(tok.begin, cur);
							Token tokl = { EOF_, {(keyword_e)0}, cur, 1, line_num, line_begin };
							syntaxt(false, &tok, &tokl, "quoted_string:: EOF_ encountered inside string literal.");
						}
						case NEWLINE_C_CASES: {
							tok.len = (u32)ptr_sub(tok.begin, cur);
							Token tokl = { ERROR_REP_NEWLINE, {(keyword_e)0},
									cur, (u32)ptr_sub(cur, newline(cur)), line_num, line_begin };
							syntaxt(false, &tok, &tokl, "quoted_string:: Newline encountered inside string literal.");
						}
						default: {}
					}
					
					++cur; // consume terminating '"'
					
				tok_end = cur;
				} break;
				
				case '$':						tok.tok = DOLLAR_CMD; {
					
					str.str = cur +1;
					
					do {
						++cur;
					} while (is_identifier_c(*cur));
					
					str.len = safe_cast_assert(u32, ptr_sub(str.str, cur));
					
					tok.dollar_cmd = match_dollar_command(str);
					
				tok_end = cur;
				} break;
				
				case '%':						tok.tok = PERCENT; {
					assert(false);
				} break;
				
				case '.':						tok.tok = MEMBER_DOT;		break;
				case '=':						tok.tok = EQUALS;			break;
				case ';':						tok.tok = SEMICOLON;		break;
				case ',':						tok.tok = COMMA;			break;
				case '?':						tok.tok = NNARY_COND;		break;
				case ':':						tok.tok = NNARY_SEPER;		break;
				case '(':						tok.tok = PAREN_OPEN;		break;
				case ')':						tok.tok = PAREN_CLOSE;		break;
				case '[':						tok.tok = BRACKET_OPEN;		break;
				case ']':						tok.tok = BRACKET_CLOSE;	break;
				case '{':						tok.tok = CURLY_OPEN;		break;
				case '}':						tok.tok = CURLY_CLOSE;		break;
				case '<':						tok.tok = ANGULAR_OPEN;		break;
				case '>':						tok.tok = ANGULAR_CLOSE;	break;
				case '-':						tok.tok = MINUS;			break;
				case '+':						tok.tok = PLUS;				break;
				case '*':						tok.tok = MULTIPLY;			break;
				case '/':						tok.tok = DIVIDE;			break;
				case '|':						tok.tok = OR;				break;
				case '&':						tok.tok = AND;				break;
				default: {
					Token fake_tok = { ERROR_REP_TOKENIZER, {(keyword_e)0}, cur, 1, line_num, line_begin };
					syntaxt(false, &fake_tok, &fake_tok, "Unexpected character '%' in tokenizer!", *cur);
				}
			}
			
			cur = tok_end;
			
			tok.len = safe_cast_assert(u32, ptr_sub(tok.begin, tok_end));
			
			#if 0
			{
				lstr str = "<unknown>";
				switch (tok.tok) {
					case EOF_:					str = "\\0";							break;
					case EOF_MARKER:			str = tok.get_lstr();					break;
					case IDENTIFIER:		str = tok.get_lstr();					break;
					case KEYWORD:				str = KEYWORDS[tok.kw];				break;
					case NUMBER:				str = tok.get_lstr();					break;
					case STRING:				str = tok.get_lstr();					break;
					case DOLLAR_CMD:			str = DOLLAR_COMMANDS[tok.dollar_cmd]; break;
					case PERCENT:				str = "%";							break;
					case MEMBER_DOT:			str = ".";							break;
					case EQUALS:				str = "=";							break;
					case SEMICOLON:				str = ";";							break;
					case COMMA:					str = ",";							break;
					case NNARY_COND:			str = "?";							break;
					case NNARY_SEPER:			str = ":";							break;
					case PAREN_OPEN:			str = "(";							break;
					case PAREN_CLOSE:			str = "";							break;
					case BRACKET_OPEN:			str = "[";							break;
					case BRACKET_CLOSE:			str = "]";							break;
					case CURLY_OPEN:			str = "{";							break;
					case CURLY_CLOSE:			str = "}";							break;
					case MINUS:					str = "-";							break;
					case PLUS:					str = "+";							break;
					case MULTIPLY:				str = "*";							break;
					case DIVIDE:				str = "/";							break;
					default: {}
				}
				
				print("%: %:% % '%'\n", safe_cast_assert(u32, tok -tokens),
						tok.line_num, tok.get_char_pos(), TOKEN_NAME[tok.tok], str);
			}
			#endif
			
			if (tok.tok == EOF_) break;
		}
		
		return tokens;
	}
	
	#define syntax_token(ptok, type, ...) { \
				syntax(type == (*(ptok))->tok, *(ptok), *(ptok), __VA_ARGS__); \
				++*(ptok); \
			}
	
	_DECL char* identifier (char* cur, lstr* out) {
		
		out->str = cur;
		
		while (is_identifier_c(*cur)) {
			++cur;
		}
		
		out->len = safe_cast_assert(u32, ptr_sub(out->str, cur));
		return cur;
	}
	
	#define UNARY_OPERATOR_CASES	PLUS: case MINUS
	#define BINARY_OPERATOR_CASES	MULTIPLY: case DIVIDE: case PLUS: case MINUS
	
	DECL bool is_unary_operator (token_e tok) {
		switch (tok) {
			case UNARY_OPERATOR_CASES:	return true;
			default:					return false;
		}
	}
	DECL bool is_binary_operator (token_e tok) {
		switch (tok) {
			case BINARY_OPERATOR_CASES: return true;
			default:					return false;
		}
	}
	
	_DECL void convert_unit (Expr_Val* val, keyword_e kw) {
		
		assert(is_angle_unit(kw) || is_color_unit(kw));
		assert(is_flt(val->type));
		switch (kw) {
			
			case UNIT_DEG: {
				__m128d xy0 = _mm_loadu_pd(&val->fm.arr[0].x);
				__m128d zw0 = _mm_loadu_pd(&val->fm.arr[0].z);
				__m128d xy1 = _mm_loadu_pd(&val->fm.arr[1].x);
				__m128d zw1 = _mm_loadu_pd(&val->fm.arr[1].z);
				__m128d xy2 = _mm_loadu_pd(&val->fm.arr[2].x);
				__m128d zw2 = _mm_loadu_pd(&val->fm.arr[2].z);
				__m128d xy3 = _mm_loadu_pd(&val->fm.arr[3].x);
				__m128d zw3 = _mm_loadu_pd(&val->fm.arr[3].z);
				
				__m128d conv = _mm_set1_pd(DEG_TO_RADd);
						xy0 = _mm_mul_pd(xy0, conv);
						zw0 = _mm_mul_pd(zw0, conv);
						xy1 = _mm_mul_pd(xy1, conv);
						zw1 = _mm_mul_pd(zw1, conv);
						xy2 = _mm_mul_pd(xy2, conv);
						zw2 = _mm_mul_pd(zw2, conv);
						xy3 = _mm_mul_pd(xy3, conv);
						zw3 = _mm_mul_pd(zw3, conv);
				
				_mm_storeu_pd(&val->fm.arr[0].x, xy0);
				_mm_storeu_pd(&val->fm.arr[0].z, zw0);
				_mm_storeu_pd(&val->fm.arr[1].x, xy1);
				_mm_storeu_pd(&val->fm.arr[1].z, zw1);
				_mm_storeu_pd(&val->fm.arr[2].x, xy2);
				_mm_storeu_pd(&val->fm.arr[2].z, zw2);
				_mm_storeu_pd(&val->fm.arr[3].x, xy3);
				_mm_storeu_pd(&val->fm.arr[3].z, zw3);
			} // Fallthrough
			case UNIT_RAD: {
				// Already in radians
				val->type |= VT_ANGLE;
			} break;
			
			case UNIT_SRGB: {
				// no color matrix for now
				val->fm.arr[0] = tv4<f64>(to_linear(val->fm.arr[0].xyz() * tv3<f64>(1.0/255.0)), 0);
			} // Fallthrough
			case UNIT_COL: {
				// Already in linear color
				val->type |= VT_COLOR;
			} break;
			
			default: assert(false);
		}
	}
	
	_DECL Token* number_literal (Token* tok, Expr_Val* val, parse_flags_e flags) {
		//PROFILE_SCOPED(THR_ENGINE, "number_literal");
		
		assert(tok->tok == NUMBER);
		
		char* cur = tok->begin;
		assert( is_dec_digit_c(*cur) );
		
		bool flt = false;
		if (flags & PF_INFER_TYPE) {
			if (val->type == VT_FLT) {
				flt = true;
			} else {
				warning_(false, tok, tok, "Encountered a number literal in a context where a type other than float would be inferred");
			}
		}
		
		bool has_unit;
		
		uptr int_ = 0;
		
		bool	hex_or_bin = cur[0] == '0' && (cur[1] == 'x' || cur[1] == 'b');
		if (hex_or_bin) {
			++cur;
			
			if (*cur++ == 'x') {
				// Hex number
				for (ui i=0;; ++i) {
					assert(i < (sizeof(uptr) * 2));
					
					u8 digit = *cur;
					if (		is_dec_digit_c(*cur) ) {
						digit -= '0';
					} else if ( is_hex_lower_c(*cur) ) {
						digit -= 'a' -10;
					} else if ( is_hex_upper_c(*cur) ) {
						digit -= 'A' -10;
					} else {
						break;
					}
					++cur;
					
					int_ <<= 4;
					int_ |= digit;
				}
				
			} else {
				// Binary number
				for (ui i=0;; ++i) {
					assert(i < (sizeof(uptr) * 8));
					
					if (!is_bin_digit_c(*cur)) { break; }
					u8 digit = *cur++;
					
					int_ <<= 1;
					int_ |= digit -'0';
				}
			}
			
			has_unit = *cur == '_';
			if (has_unit) {
				flt = true;
			}
			
			if (flt) {
				val->fm.arr[0].x = reint_int_as_flt(int_);
			}
			
		} else {
			
			ui indx = 0;
			
			do {
				++indx;
			} while (parse_n::is_dec_digit_c(cur[indx]));
			
			if (cur[indx] == '.') {
				// Number with fractional part
				flt = true;
				
				if (cur[indx +1] == '@') { // Special float values like 1.@IND
					++indx;
					syntax(flags & PF_ONLY_SYNTAX_CHECK, tok,tok, "Special float values like infiniy or NaN only allowed with PF_ONLY_SYNTAX_CHECK set (inactive $ values)");
					do {
						++indx;
					} while (parse_n::is_identifier_c(cur[indx]));
				} else {
					do {
						++indx;
					} while (parse_n::is_dec_digit_c(cur[indx]));
				}
			} else {
				// Number without fractional part
			}
			
			has_unit = cur[indx] == '_';
			if (has_unit) {
				flt = true;
			}
			
			if (flt) {
				
				if (!(flags & PF_ONLY_SYNTAX_CHECK)) { // strtof can't read special float codes like 1.#IND
					char* end;
					val->fm.arr[0].x = strtod(cur, &end);
					
					assert(end == (cur +indx));
				}
				
				cur += indx;
				
			} else {
				
				do {
					
					uptr temp = (int_ * 10) +(*cur -'0');
					if (temp < int_) {
						assert(false); // overflow
					}
					int_ = temp;
					
					++cur;
				} while (--indx);
				
			}
			
		}
		
		if (flt) {
			val->type = VT_FLT;
		} else {
			val->type = VT_SINT;
			val->im.arr[0].x = static_cast<sptr>(int_);
		}
		
		if (has_unit) {
			++cur;
			
			lstr iden;
			
			syntax(is_identifier_start_c(*cur), tok, tok, "number_literal:: unit identifier");
			cur = identifier(cur, &iden);
			
			auto kw = match_keyword(iden);
			
			assert(is_angle_unit(kw));
			convert_unit(val, kw);
		}
		
		assert(ptr_sub(tok->begin, cur) == tok->len);
		
		return ++tok;
	}
	_DECL Token* keyword (Token* tok, Expr_Val* val, parse_flags_e flags) {
		
		assert(tok->tok == KEYWORD);
		
		keyword_e kw = tok->kw;
		switch (kw) {
			case LIT_PI:
				val->type = VT_FLT;
				val->fm.arr[0].x = PId;
				break;
			case LIT_FALSE:
			case LIT_TRUE:
				val->type = VT_BOOL;
				val->im.arr[0].x = kw == LIT_TRUE ? 1 : 0;
				break;
			case LIT_NULL:
			case LIT_IDENTITY: {
				val->type = VT_KEYWORD;
				val->kw = kw;
			} break;
			default: assert(false);
		}
		
		return ++tok;
	}
	
	#define EXPRESSION_START_CASES \
			NUMBER_CASES: case UNARY_OPERATOR_CASES: case IDENTIFIER_START_CASES: case '('
	
	_DECL Token* expression (Token* tok, Expr_Val* val, parse_flags_e flags, ui prec);
	
	_DECL ui unary_precedence (token_e tok) {
		switch (tok) {
			case PLUS:
			case MINUS:
				return 3;
			default:
				assert(false);
				return 0;
		}
	}
	_DECL ui binary_precedence (token_e tok) {
		switch (tok) {
			case MULTIPLY:
			case DIVIDE:
				return 2;
			case PLUS:
			case MINUS:
				return 1;
			default:
				assert(false);
				return 0;
		}
	}
	
	_DECL Token* unary_expression (Token* tok, Expr_Val* val, parse_flags_e flags) {
		//PROFILE_SCOPED(THR_ENGINE, "unary_expression");
		
		Token* op_tok = tok++;
		token_e op = op_tok->tok;
		assert(is_unary_operator(op));
		
		ui prec = unary_precedence(op);
		
		syntaxdev(tok = expression(tok, val, flags, prec), "unary_expression:: expression()");
		
		assert((val->type & ~(VT_VEC_BIT|VT_SCALAR_TYPE_BIT|VT_UNIT_BIT)) == 0);
		
		switch (op) { // unary operators not defined for matrices
			case MINUS: {
				using namespace sse_op;
				auto v = load(&val->fm.arr[0]);
				
				switch (val->type & (VT_VEC_BIT|VT_SCALAR_TYPE_BIT)) {
					case VT_FLT: case VT_FV2: case VT_FV3: case VT_FV4: // only 
						v = neg(v);
						break;
					case VT_SINT: case VT_S32V2: case VT_S32V3: case VT_S32V4:
						v = cast<DV4>(neg( cast<S64V4>(v) )); // cast ist just to have one load instruction for both integer and float (just for code size and simplicity)
						break;
					default: assert(false);
				}
				
				store(&val->fm.arr[0], v);
			} break;
			case PLUS: {
				// Do nothing
			} break;
			default: assert(false);
		}
		return tok;
	}
	_DECL Token* binary_expression (Token* tok, Expr_Val* val, parse_flags_e flags, ui prec) {
		//PROFILE_SCOPED(THR_ENGINE, "binary_expression");
		
		Token* op_tok = tok++;
		token_e op = op_tok->tok;
		assert(is_binary_operator(op));
		
		Expr_Val rhs;
		
		syntaxdev(tok = expression(tok, &rhs, flags, prec), "binary_expression:: expression()");
		
		assert((val->type & ~(VT_VEC_BIT|VT_SCALAR_TYPE_BIT|VT_UNIT_BIT)) == 0);
		
		switch (val->type & (VT_VEC_BIT|VT_SCALAR_TYPE_BIT)) {
			case VT_FLT: case VT_FV2: case VT_FV3: case VT_FV4:
			case VT_SINT:
				break;
			default: assert(false);
		}
		
		switch (op) {
			case MULTIPLY: {
				syntax((val->type & (VT_VEC_BIT|VT_SCALAR_TYPE_BIT)) == (rhs.type & (VT_VEC_BIT|VT_SCALAR_TYPE_BIT)),
								op_tok, op_tok, "Type mismatch in multiplication!");
				{
					auto l=(val->type & VT_ANGLE), r=(rhs.type & VT_ANGLE);
					syntax(!(l && r), op_tok, op_tok, "Cannot multiply two angles!");
					val->type = (val->type & ~VT_ANGLE) | (l | r);
				}
			} break;
			case DIVIDE: {
				syntax( (val->type & (VT_VEC_BIT|VT_SCALAR_TYPE_BIT)) == (rhs.type & (VT_VEC_BIT|VT_SCALAR_TYPE_BIT)),
						op_tok, op_tok, "Type mismatch in division!");
				{
					auto l=(val->type & VT_ANGLE), r=(rhs.type & VT_ANGLE);
					if (r) {
						if (l) { // ang/ang
							val->type &= ~VT_ANGLE; // Two angles divided by each other result in a unitless ratio
						} else { // n/ang
							assert(false, "Inverse angles not supported for now!");
						}
					} else {
						if (l) { // ang/n
							// Angle divided by number is still angle
						}
					}
				}
			} break;
			case PLUS: { //	 & ~VT_COLOR to allow colors to be mixed with unitless values
				syntax((val->type & ~VT_COLOR) == (rhs.type & ~VT_COLOR), op_tok, op_tok, "Type or unit mismatch in addition!");
			} break;
			case MINUS: {
				syntax((val->type & ~VT_COLOR) == (rhs.type & ~VT_COLOR), op_tok, op_tok, "Type or unit mismatch in substraction!");
			} break;
			default: assert(false);
		}
		
		using namespace sse_op;
		auto l = load(&val->fm.arr[0]);
		auto r = load(&rhs.fm.arr[0]);
		
		switch (op) { // no binary operators not defined for matrices for now
			case MULTIPLY: {
				if ((val->type & VT_SCALAR_TYPE_BIT) == VT_FLT) {
					l = mul(l, r);
				} else {
					val->fm.arr[0] *= rhs.fm.arr[0];
				}
			} break;
			case DIVIDE: {
				if ((val->type & VT_SCALAR_TYPE_BIT) == VT_FLT) {
					l = div(l, r);
				} else {
					auto r = rhs.fm.arr[0];
					
					syntax(r.x != 0 && r.y != 0 && r.z != 0 && r.w != 0, op_tok, op_tok, "Integer div by zero!");
					
					val->fm.arr[0] /= rhs.fm.arr[0];
				}
			} break;
			case PLUS: {
				if ((val->type & VT_SCALAR_TYPE_BIT) == VT_FLT) {
					l = add(l, r);
				} else {
					l = cast<DV4>( add(cast<S64V4>(l), cast<S64V4>(r)) );
				}
			} break;
			case MINUS: {
				if ((val->type & VT_SCALAR_TYPE_BIT) == VT_FLT) {
					l = sub(l, r);
				} else {
					l = cast<DV4>( sub(cast<S64V4>(l), cast<S64V4>(r)) );
				}
			} break;
			default: {}
		}
		
		store(&val->fm.arr[0], l);
		
		return tok;
	}
	
	constexpr u32 MAX_CALL_ARGS = 16;
	
	_DECL Token* call_expression (Token* tok, Expr_Val* val, parse_flags_e flags) {
		//PROFILE_SCOPED(THR_ENGINE, "call_expression");
		
		struct Arg_Tok {
			Token*	f;
			Token*	l;
		};
		
		Expr_Val	args[MAX_CALL_ARGS];
		Arg_Tok		toks[MAX_CALL_ARGS];
		
		parse_flags_e recurse_flags = flags;
		
		assert(tok->tok == KEYWORD);
		Token* func = tok++;
		
		keyword_e kw = func->kw;
		switch (kw) {
			case KW_ANGLE_UNIT_CASES:
			case KW_COLOR_UNIT_CASES:
			case KW_FVEC_CASES:
			case KW_FMAT_CASES:
			case TYPE_QUAT:
				recurse_flags |= PF_INFER_TYPE;
				for (u32 i=0; i<MAX_CALL_ARGS; ++i) {
					args[i].type = VT_FLT;
				}
				break;
			default:
				{} // Handled in switch down below
		}
		
		syntax_token(&tok, PAREN_OPEN, "call_expression:: needs open paren");
		
		u32 arg_counter = 0;
		for (;;) {
			if (tok->tok == PAREN_CLOSE) {
				++tok;
				break;
			}
			
			toks[arg_counter].f = tok;
			syntaxdev(tok = expression(tok, &args[arg_counter], recurse_flags, 0), "call_expression:: argument expression()");
			toks[arg_counter].l = tok -1;
			
			syntax(arg_counter++ < MAX_CALL_ARGS, func, tok-1,
					"Too many arguments to call expression (MAX_CALL_ARGS is %)!", MAX_CALL_ARGS);
			
			if (tok->tok == COMMA) {
				++tok;
			} else {
				syntax(tok->tok == PAREN_CLOSE, tok, tok, "call_expression:: need parenthesis close after arguments");
			}
		}
		
		switch (kw) {
			case KW_ANGLE_UNIT_CASES: {
				
				syntax(arg_counter == 1, func, tok-1, "% unit ctor takes exactly 1 arg!", KEYWORDS[kw]);
				*val = args[0];
				
				if (val->type == VT_KEYWORD || !is_scalar_or_vec(val->type)) {
					char const* targ = KEYWORDS[kw].str;
					char const* arg0 = get_vec_name(val->type);
					syntax(false, toks[0].f, toks[0].l, "Args to % unit ctor must be scalar or vector (arg% is a %)!",
							targ, 0, arg0);
				}
				
				syntax(is_flt(val->type), func, tok-1, "Angles and colors have to be floats!");
				syntax(!(val->type & VT_UNIT_BIT), func, tok-1, "Cannot convert to unit because arg already has a unit!");
				
				convert_unit(val, kw);
				
				return tok;
			}
			
			case KW_COLOR_UNIT_CASES:
			case KW_VEC_CASES: {
				
				syntax(arg_counter > 0, func, tok-1, "Vector ctor: Need at least 1 arg!");
				
				auto targ_dim = get_kw_vec_dim(kw);
				auto targ_vec = set_vec_dimensions(targ_dim);
				
				val->type = targ_vec|(args[0].type & VT_SCALAR_TYPE_BIT);
				
				if (arg_counter == 1) { // Set all vector members with single value
					
					if (args[0].type == VT_KEYWORD || !is_scalar_or_vec(args[0].type)) {
						char const* targ = KEYWORDS[kw].str;
						char const* arg0 = get_vec_name(args[0].type);
						syntax(false, func, tok-1, "Vector ctor: Args to % must be scalar or vec (arg% is a %)!",
								targ, 0, arg0);
					}
					
					if (args[0].type & VT_COLOR) {
						syntax(false, func, tok-1, "Vector ctor: Can't pass color vector to vector ctor!");
					}
					
					auto arg0_dim = get_vec_dimensions(args[0].type);
					
					if (arg0_dim > 1 && !is_color_unit(kw)) {
						char const* targ_name = KEYWORDS[kw].str;
						char const* arg0_name = get_vec_name(args[0].type);
						if (arg0_dim == targ_dim) {
							syntax(false, func, tok-1, "Vector ctor: Vector is already a % (passing % to % ctor)!",
									targ_name, arg0_name, targ_name);
						} else {
							syntax(false, func, tok-1,
									"Vector ctor: Cannot contruct % with one % (% components)!\n"
									"	 NOTE: Vector ctor with 1 argument is supposed to set all components with one scalar.",
									targ_name, arg0_name, arg0_dim < targ_dim ? "not enough" : "too many");
						}
					}
					
					val->im.arr[0] = expr_val_im::v_t(args[0].im.arr[0].x);
					
				} else {
					
					u32 targ_i = 0;
					for (u32 arg_i=0; arg_i<arg_counter; ++arg_i) {
						
						if (args[arg_i].type == VT_KEYWORD || !is_scalar_or_vec(args[arg_i].type)) {
							char const* targ = KEYWORDS[kw].str;
							char const* argn = get_vec_name(args[arg_i].type);
							syntax(false, toks[arg_i].f, toks[arg_i].l, "Vector ctor: Args to % must be scalar or vec (arg# is a %)!",
									targ, arg_i, argn);
						}
						
						u32 arg_dim = get_vec_dimensions(args[arg_i].type);
						
						if (args[arg_i].type & VT_COLOR) {
							syntax(false, toks[arg_i].f, toks[arg_i].l, "Vector ctor: Can't pass color vector to vector ctor!");
						}
						
						if ((targ_i +arg_dim) > targ_dim) {
							char const* targ = KEYWORDS[kw].str;
							char const* arg0 = get_vec_name(args[0].type);
							syntax(false, toks[arg_i].f, toks[arg_i].l, "Vector ctor: Too many components for % (Already have % and trying to pass a % as arg%)!",
									targ, targ_i, arg0, arg_i);
						}
						
						if ((val->type & VT_SCALAR_TYPE_BIT) != (args[arg_i].type & VT_SCALAR_TYPE_BIT)) {
							char const* arg0 = get_scalar_type_name(val->type);
							char const* argn = get_scalar_type_name(args[arg_i].type);
							syntax(false, toks[arg_i].f, toks[arg_i].l, "Vector ctor: Scalar type mismatch, the previous args were <%> but arg% is <%>!",
									arg0, arg_i, argn);
						}
						if ((val->type & VT_ANGLE) != (args[arg_i].type & VT_ANGLE)) {
							char const* arg0 = val->type & VT_ANGLE ?			"angle" : "unitless";
							char const* argn = args[arg_i].type & VT_ANGLE ?	"angle" : "unitless";
							syntax(false, toks[arg_i].f, toks[arg_i].l, "Vector ctor: Unit mismatch, the previous args were <%> but arg% is <%>!",
									arg0, arg_i, argn);
						}
						
						for (u32 i=0; i<arg_dim; ++i) {
							val->im.arr[0][targ_i++] = args[arg_i].im.arr[0][i];
						}
						
					}
					
					if (targ_i != targ_dim) {
						char const* targ_name = KEYWORDS[kw].str;
						syntax(false, func, tok-1, "Vector ctor: Not enough components for % (Only got % components in % args)!",
								targ_name, targ_i, arg_counter);
					}
				}
				
				if (is_color_unit(kw)) {
					convert_unit(val, kw);
				}
				
				return tok;
			}
			
			case KW_FMAT_CASES: {
				
				syntax(arg_counter > 0, func, tok-1, "Vector ctor: Need at least 1 arg!");
				
				auto targ_dim = get_kw_mat_dim(kw);
				auto targ_elem = targ_dim*targ_dim;
				auto targ_mat = set_mat_dim(targ_dim);
				
				val->type = targ_mat|(args[0].type & VT_SCALAR_TYPE_BIT);
				
				syntax(arg_counter == 1 || arg_counter == targ_dim || arg_counter == targ_elem, func, tok-1,
						"Matrix ctor: Need either 1 arg ('ident') or all members or all rows (row major order) (gives % args)!", arg_counter);
				
				if (arg_counter == 1) { // Set all vector members with single value
					
					if (args[0].type != VT_KEYWORD || args[0].kw == LIT_IDENTITY) {
						char const* targ = KEYWORDS[kw].str;
						char const* arg0 = get_vec_name(args[0].type);
						syntax(false, func, tok-1, "Matrix ctor: 1 arg version must be the keyword 'ident' (arg% is a %)!",
								targ, 0, arg0);
					}
					
					val->fm = expr_val_fm::ident();
					
				} else {
					
					u32 x = 0;
					u32 y = 0;
					for (u32 arg_i=0; arg_i<arg_counter; ++arg_i) {
						
						u32 arg_dim = get_vec_dimensions(args[arg_i].type);
						
						if (	(args[arg_i].type == VT_KEYWORD || !is_vec(args[arg_i].type)) &&
								arg_dim == (arg_counter == targ_dim ? targ_dim : targ_elem) ) {
							char const* targ = KEYWORDS[kw].str;
							char const* argn = get_vec_name(args[arg_i].type);
							syntax(false, toks[arg_i].f, toks[arg_i].l, "Matrix ctor: Args to % with % args must be v% (arg% is a %)!",
									targ, arg_counter, targ_dim, arg_i, argn);
						}
						
						if (args[arg_i].type & VT_COLOR) {
							syntax(false, toks[arg_i].f, toks[arg_i].l, "Matrix ctor: Can't pass color vector to matrix ctor!");
						}
						
						if ((val->type & VT_SCALAR_TYPE_BIT) != (args[arg_i].type & VT_SCALAR_TYPE_BIT)) {
							char const* arg0 = get_scalar_type_name(val->type);
							char const* argn = get_scalar_type_name(args[arg_i].type);
							syntax(false, toks[arg_i].f, toks[arg_i].l, "Matrix ctor: Scalar type mismatch, the previous args were <%> but arg% is <%>!",
									arg0, arg_i, argn);
						}
						if ((val->type & VT_ANGLE) != (args[arg_i].type & VT_ANGLE)) {
							char const* arg0 = val->type & VT_ANGLE ?			"angle" : "unitless";
							char const* argn = args[arg_i].type & VT_ANGLE ?	"angle" : "unitless";
							syntax(false, toks[arg_i].f, toks[arg_i].l, "Vector ctor: Unit mismatch, the previous args were <%> but arg% is <%>!",
									arg0, arg_i, argn);
						}
						
						if (arg_counter == targ_dim) {
							val->im.row(y, args[arg_i].im.arr[0]);
							
							++y;
						} else {
							val->im.row(x,y, args[arg_i].im.arr[0].x);
							
							++x;
							if (x == targ_dim) {
								x = 0;
								++y;
							}
						}
						
					}
					
					assert(x == 0 && y == targ_dim);
					
				}
				
				return tok;
			}
			
			case TYPE_QUAT: {
				
				syntax(arg_counter == 1 || arg_counter == 2, func, tok-1,
					"Quat ctor: Need exactly 2 args!\n"
					"  NOTE: 'quat(xyz=v3<flt>, w=flt)' or 'quat(ident)' for identity quat!");
				
				val->type = VT_FQUAT;
				
				if (arg_counter == 1) {
					
					syntax(args[0].type == VT_KEYWORD && args[0].kw == LIT_IDENTITY, func, tok-1, "call_expression:: quat(): 1 argument version needs argument to be 'ident' keyword");
					
					val->fm.arr[0] = expr_val_fm::v_t(0, 0, 0, 1);
					
					return tok;
				}
				
				if (args[0].type != VT_FV3) {
					char const* targ = KEYWORDS[kw].str;
					char const* arg0 = get_vec_name(args[0].type);
					char const* arg0s = get_scalar_type_name(args[0].type);
					syntax(false, toks[0].f, toks[0].l, "Quat ctor: Arg0 to % must be a v3<flt> (arg0 is a %<%>)!\n"
							"  NOTE: 'quat(xyz=v3<flt>, w=flt)' or 'quat(ident)' for identity quat!",
							targ, arg0, arg0s);
				}
				if (args[1].type != VT_FLT) {
					char const* targ = KEYWORDS[kw].str;
					char const* arg1 = get_vec_name(args[1].type);
					char const* arg1s = get_scalar_type_name(args[1].type);
					syntax(false, toks[1].f, toks[1].l, "Quat ctor: Arg1 to % must be a flt (arg1 is a %<%>)!\n"
							"  NOTE: 'quat(xyz=v3<flt>, w=flt)' or 'quat(ident)' for identity quat!",
							targ, arg1, arg1s);
				}
				
				val->fm.arr[0] = expr_val_fm::v_t(args[0].fm.arr[0].xyz(), args[1].fm.arr[0].x);
				
				return tok;
			}
			
			case FUNC_NORMALIZE: {
				
				syntax(arg_counter == 1, func, tok-1, "Function '%' takes exactly 1 arg!", KEYWORDS[kw]);
				*val = args[0];
				
				if (!is_vec_or_quat(val->type) || (val->type & VT_ANGLE)) { // Accept color vector
					char const* targ = KEYWORDS[kw].str;
					char const* arg0 = get_vec_name(val->type);
					syntax(false, toks[0].f, toks[0].l, "Args to function '%' must be vec or quat (arg% is a %)!",
							targ, 0, arg0);
				}
				
				syntax(is_flt(val->type), toks[0].f, toks[0].l, "Function '%' only takes floats!", KEYWORDS[kw]);
				//syntax(!(val->type & VT_ANGLE)); // Don't accept angles?
				
				#if 0 // 691 lines asm
				switch (val->type & VT_VEC_BIT) {
					case VT_VEC2:	val->fv.xy(		normalize(val->fv.xy()) );	break;
					case VT_VEC3:	val->fv.xyz(	normalize(val->fv.xyz()) ); break;
					case VT_VEC4:
					case VT_QUAT:	val->fv =		normalize(val->fv);			break;
				}
				#else // 640 lines asm
				using namespace sse_op;
				DV4 v;
				v.xy = _mm_loadu_pd(&val->fm.arr[0].x);
				v.zw = _mm_loadu_pd(&val->fm.arr[0].z);
				__m128d zero = _mm_setzero_pd();
				
				switch (val->type & VT_VEC_BIT) {
					case VT_VEC2:
						v.zw = zero;
						break;
					case VT_VEC3:
						//v.zw = _mm_and_pd(v.zw, toM128d(-1, 0)); // zero w
						v.zw = _mm_move_sd(zero, v.zw); // zero w
						break;
					case VT_QUAT:
					case VT_VEC4:
						break;
					default: assert(false);
				}
				
				v = normalize(v);
				
				_mm_storeu_pd(&val->fm.arr[0].x, v.xy);
				_mm_storeu_pd(&val->fm.arr[0].z, v.zw);
				#endif
				
				return tok;
			}
			
			default: return 0;
		}
	}
	_DECL Token* parenthesis_expression (Token* tok, Expr_Val* val, parse_flags_e flags) {
		
		assert(tok->tok == PAREN_OPEN);
		++tok;
		
		syntaxdev(tok = expression(tok, val, flags, 0),
				"parenthesis_expression:: expression()");
		
		syntax_token(&tok, PAREN_CLOSE, "parenthesis_expression:: need close paren");
		
		return tok;
	}
	
	_DECL Token* quoted_string (Token* tok, Expr_Val* val, parse_flags_e flags) {
		
		assert(tok->tok == STRING);
		
		if (flags & PF_ONLY_SYNTAX_CHECK) {
			val->type = VT_LSTR;
			val->lstr = lstr{ nullptr, 0 };
			
		} else {
			
			char const* cur = tok->begin;
			assert(tok->len >= 2);
			assert(*cur++ == '"');
			
			char const* str_copy = string_buf_cur;
			
			if (flags & PF_STRING_APPEND) {
				assert(val->type == VT_LSTR);
				
				assert(ptr_sub(string_buf, string_buf_cur) >= 1 && string_buf_cur[-1] == '\0');
				--string_buf_cur;
			}
			
			val->type = VT_LSTR;
			
			char const* end = cur +(tok->len -2);
			while (cur != end) {
				assert(*cur != '\0' && *cur != '"');
				
				if (*cur == '`') {
					if (cur[1] == '`' || cur[1] == '"') {
						++cur; // escaped '"' or '`'
					} else {
						assert(false);
					}
				}
				
				assert(string_buf_cur < (string_buf +arrlenof<u32>(string_buf)));
				*string_buf_cur++ = *cur++;
			}
			
			assert(*cur++ == '"');
			
			assert(string_buf_cur < (string_buf +arrlenof<u32>(string_buf)));
			*string_buf_cur++ = '\0';
			
			u32 len = safe_cast_assert(u32, ptr_sub(str_copy, string_buf_cur -1));
			if (flags & PF_STRING_APPEND) {
				val->lstr.len += len;
			} else {
				val->lstr = lstr{ str_copy, len };
			}
			
		}
		
		return ++tok;
	}
	
	_DECL Token* expression (Token* tok, Expr_Val* val, parse_flags_e flags, ui prec) {
		//PROFILE_SCOPED(THR_ENGINE, "expression");
		
		Token* expr = tok;
		
		parse_flags_e	orig_flags = flags;
		var_types_e		orig_type = val->type;
		if (!is_unary_operator(tok->tok)) {
			Token* cur = tok;
			for (;;) {
				++cur;
				switch(cur->tok) {
					case BINARY_OPERATOR_CASES:
					case NNARY_COND:
						flags &= ~PF_INFER_TYPE;
						// Fallthrough
					case SEMICOLON:
					case COMMA:
					case PAREN_CLOSE:
					case CURLY_CLOSE:
					case BRACKET_CLOSE:
					case EOF_:
					case EOF_MARKER:
						goto end;
					default: {}
				}
			} end:;
		}
		
		switch (tok->tok) { // P
			
			case NUMBER: {
				syntaxdev(tok = number_literal(tok, val, flags), "expression::P:: number_literal()");
			} break;
			
			case UNARY_OPERATOR_CASES: {
				syntaxdev(tok = unary_expression(tok, val, flags), "expression::P:: unary_expression()");
			} break;
			
			case KEYWORD: {
				
				switch (tok->kw) {
					
					case KW_KEYWORD_LITERAL_CASES: {
						syntaxdev(tok = keyword(tok, val, flags), "expression::P:: keyword()");
					} break;
					
					case KW_FUNC_CALL_CASES: {
						syntaxdev(tok = call_expression(tok, val, flags), "expression::P:: call_expression()");
					} break;
					
					default: {
						syntax(false, tok,tok, "expression::P:: unknow keyword %", (u32)tok->kw);
					}
				}
			} break;
			
			case PAREN_OPEN: {
				syntaxdev(tok = parenthesis_expression(tok, val, flags), "expression::P:: parenthesis_expression()");
			} break;
			
			case STRING: {
				for (;;) {
					syntaxdev(tok = quoted_string(tok, val, flags), "expression::P:: quoted_string()");
					
					if (tok->tok != STRING) {
						break;
					}
					
					flags |= PF_STRING_APPEND;
				}
				return tok;
			} break;
			
			default:
				syntax(false, tok,tok, "expression:: P default case");
		}
		
		for (;;) {
			
			switch (tok->tok) {
				
				case BINARY_OPERATOR_CASES: {
					
					ui recurse_prec = binary_precedence(tok->tok);
					if (recurse_prec < prec) {
						return tok;
					}
					
					recurse_prec += 1; // left associativity
					
					syntaxdev(tok = binary_expression(tok, val, flags, recurse_prec), "expression:: binary_expression()");
					
				} break;
				
				case NNARY_COND: {
					
					if (flags & PF_IN_NNARY) {
						warning_(false, tok,tok, "expression:: n-nary:: suggest adding parentheses, n-nary is: precedence=lowest, associativity=right");
					}
					
					ui ternary_prec = 0;
					if (ternary_prec < prec) {
						return tok;
					}
					
					syntax(val->type == VT_SINT || val->type == VT_BOOL, expr,tok-1, "expression:: n-nary condition must be bool or integer");
					
					++tok; // consume NNARY_COND
					
					uptr cond = (uptr)val->im.arr[0].x; // cast negative values to very large positive values which will still trigger the cond > i check
					
					ui rec_prec = ternary_prec;
					if (0) { // left associativity
						rec_prec += 1;
					}
					
					flags |= PF_IN_NNARY; // Set IN_NNARY for all the whole follwing and recursive chain for providing precedence warnings
					parse_flags_e	rec_flags = orig_flags|PF_IN_NNARY;
					
					for (uptr i=0;; ++i) {	// n-nary cases are allowed to be different types because this increases
											//	flexibility and the only drawback is potential code-rot like with #if statements
											//	but this should not be an issue at expression scales
						
						Expr_Val tmp;
						tmp.type = orig_type; // restore inferred type
						
						syntaxdev(tok = expression(tok, &tmp, rec_flags, rec_prec), "expression:: n-nary expression %", i);
						
						bool last =			tok->tok != NNARY_SEPER;
						bool active_case =	i == cond || (last && cond > i);
						if (active_case) {
							*val = tmp;
						}
						
						if (last) {
							break;
						}
						++tok; // consume NNARY_SEPER
					}
					
				} break;
				
				default:
					return tok;
			}
			
		}
	}

	#define DIAGNOSTIC_PRINT 0
	
	_DECL void print_member (lstr name, parse_flags_e flags, ui depth, uptr indx) {
		if (flags & PF_MEMBER_OF_ARRAY) {
			print("%.[%]\n", repeat("  ", depth), indx);
		} else {
			print("%.%\n", repeat("	 ", depth), name);
		}
	}
	
	_DECL bool member_parsed (Var const* var, Expr_Val& v, parse_flags_e flags, void* pdata,
			ui depth, uptr indx, Token const* f, Token const* l) {
		
		#if 1
		#if DIAGNOSTIC_PRINT
		
		if (flags & PF_ONLY_SYNTAX_CHECK) {
			print(">>> ONLY_SYNTAX_CHECK\n");
		}
		
		if (flags & PF_MEMBER_OF_ARRAY) {
			print(depth, ".[%] -> ", indx);
		} else {
			print(depth, ".% -> ", var->name);
		}
		
		if (v.type == VT_KEYWORD) {
			if (v.kw == LIT_NULL) {
										print("str{null}\n");
			} else {
				assert(false, "Unimplemented type in member_parsed(), %!", bin(v.type));
			}
		} else {
			switch (v.type & ~VT_ANGLE) {
				case VT_FLT:			print("flt{%}\n",			v.fv.x);							break;
				case VT_FV2:			print("fv2{%, %}\n",			v.fv.x, v.fv.y);					break;
				case VT_FV3:			print("fv3{%, %, %}\n",		v.fv.x, v.fv.y, v.fv.z);			break;
				case VT_FV4:			print("fv4{%, %, %, %}\n",	v.fv.x, v.fv.y, v.fv.z, v.fv.w);	break;
				case VT_FQUAT:			print("quat{%, %, %,  %}\n",	v.fv.x, v.fv.y, v.fv.z, v.fv.w);	break;
				case VT_SINT:			print("int{%}\n",			v.iv.x);							break;
				case VT_SV2:			print("iv2{%, %}\n",			v.iv.x, v.iv.y);					break;
				case VT_SV3:			print("iv3{%, %, %}\n",		v.iv.x, v.iv.y, v.iv.z);			break;
				case VT_SV4:			print("iv4{%, %, %, %}\n",	v.iv.x, v.iv.y, v.iv.z, v.iv.w);	break;
				case VT_BOOL:			print("bool{%}\n",			v.iv.x ? "true":"false");			break;
				case VT_LSTR:			print("str \"%\"\n",			v.lstr);							break;
				case VT_COLOR|VT_VEC3:	print("col{%, %, %}\n",		v.fv.x, v.fv.y, v.fv.z);			break;
				default: assert(false, "Unimplemented type in member_parsed(), %!", bin(v.type));
			}
		}
		#endif
		#endif
		
		if ((var->type & VT_SCALAR_TYPE_BIT) != VT_EXTRAT) {
			
			syntax(v.type != VT_KEYWORD, f,l, v.kw == LIT_NULL ?	"flt and int are not nullable":
																	"No keywords allowed for flt or int");
			syntax((var->type & VT_VEC_BIT) == (v.type & VT_VEC_BIT), f,l, "Vector mismatch");
			syntax((var->type & VT_UNIT_BIT) == (v.type & VT_UNIT_BIT), f,l, "Unit mismatch");
			
			switch (var->type & VT_SCALAR_TYPE_BIT) {
				case VT_FLT: {
					syntax((v.type & VT_SCALAR_TYPE_BIT) == VT_FLT, f,l, "Type mismatch, should be flt");
					// Double to single float cast always OK
				} break;
				case VT_SINT: {
					syntax((v.type & VT_SCALAR_TYPE_BIT) == VT_SINT, f,l, "Type mismatch, should be int");
					
					auto size = get_scalar_size_bits(var->type);
					auto min = (s64)0xffffffffffffffffull << (size -1);
					auto max = (s64)(((u64)0b1 << (size -1)) -1);
					
					auto* arr = (s64*)&v.im;
					for (u32 i=0; i<get_elements(var->type); ++i) {
						//conerr.print("min: % max: % val: %\n", min, max, v.iv[i]);
						syntax(arr[i] >= min && arr[i] <= max, f,l, "Number in expression out of range for signed output (s%)", size);
					}
					
				} break;
				case VT_UINT: {
					syntax((v.type & VT_SCALAR_TYPE_BIT) == VT_SINT, f,l, "Type mismatch, should be int");
					
					auto* arr = (f64*)&v.fm;
					for (u32 i=0; i<get_elements(var->type); ++i) {
						syntax(arr[i] >= 0, f,l, "Negative number in expression for unsigned output");
					}
					
					auto indx = get_scalar_size_indx(var->type);
					if (indx == 3) { // u64
						break; // u64 to u64, OK
					}
					
					auto size = get_scalar_size_bits(var->type);
					auto max = (u64)1 << size;
					
					for (u32 i=0; i<get_vec_dimensions(var->type); ++i) {
						syntax((u64)arr[i] < max, f,l, "Too big of a number in expression for unsigned output (u%)", size);
					}
					
				} break;
				default: {
					assert(false, "Not implemented");
				} break;
			}
			
		} else {
			
			switch (var->type) {
				case VT_BOOL: {
					syntax(v.type == VT_BOOL || v.type == VT_SINT, f,l, "expression for bool var accepts only bool literals or integer scalars");
					// bool literal or integer can't be out of range
				} break;
				case VT_LSTR:
				case VT_CSTR: {
					bool null = v.type == VT_KEYWORD && v.kw == LIT_NULL;
					syntax(v.type == VT_LSTR || null, f,l, "expression for cstr or lstr only accepts string literal or null %",
							bin(v.type));
					if (null) {
						v.type = VT_LSTR;
						v.lstr = {nullptr, 0};
					}
					// str literal or null can't be out of range
				} break;
				default: {
					assert(false, "Not implemented");
				} break;
			}
			
		}
		
		if (flags & PF_ONLY_SYNTAX_CHECK) {
			#if 0
			#if DIAGNOSTIC_PRINT
			print(">>> ONLY_SYNTAX_CHECK\n");
			#endif
			#endif
			return true;
		}
		
		if (var->type & VT_FPTR_VAR) {
			
			((setter_fp)var->pdata)(v);
			
		} else {
			byte	temp_data[MAX_VAR_SIZE];
			
			byte*	cur = temp_data;
			
			if ((var->type & VT_SCALAR_TYPE_BIT) != VT_EXTRAT) {
				
				auto elem = get_elements(var->type);
				
				switch (var->type & VT_SCALAR_TYPE_BIT) {
					case VT_FLT: {
						assert(is_size_4_or_8(var->type));
						
						auto* arr = (f64*)&v.fm;
						for (u32 i=0; i<elem; ++i) {
							switch (var->type & VT_SCALAR_SIZE_BIT) {
								case VT_SIZE4: {
									*(f32*)cur = (f32)arr[i];
									cur += 4;
								} break;
								case VT_SIZE8: {
									*(f64*)cur = arr[i];
									cur += 8;
								} break;
								default: assert(false);
							}
						}
					} break;
					case VT_SINT:
					case VT_UINT: {
						
						auto* arr = (s64*)&v.im;
						for (u32 i=0; i<elem; ++i) {
							switch (var->type & VT_SCALAR_SIZE_BIT) {
								case VT_SIZE1: {
									*(u8*)cur = (u8)arr[i];
									cur += 1;
								} break;
								case VT_SIZE2: {
									*(u16*)cur = (u16)arr[i];
									cur += 2;
								} break;
								case VT_SIZE4: {
									*(u32*)cur = (u32)arr[i];
									cur += 4;
								} break;
								case VT_SIZE8: {
									*(u64*)cur = (u64)arr[i];
									cur += 8;
								} break;
								default: assert(false);
							}
						}
					} break;
					default: assert(false);
				}
				
			} else {
				
				switch (var->type) {
					case VT_BOOL: {
						*(bool*)cur = !!v.im.arr[0].x;
						cur += sizeof(bool);
					} break;
					case VT_LSTR: {
						*(lstr*)cur = v.lstr;
						cur += sizeof(lstr);
					} break;
					case VT_CSTR: {
						*(char const**)cur = v.lstr.str;
						cur += sizeof(char const*);
					} break;
					default: assert(false);
				}
				
			}
			
			u32		data_size = (u32)(cur -temp_data);
			
			bool changed = !cmemcmp(pdata, (void*)temp_data, data_size);
			if (changed) {
				
				#if 0
				if ((flags & PF_WRITE_BIT) != PF_INITIAL_LOAD) {
					while (depth--) {
						print("	 ");
					}
					if (flags & PF_MEMBER_OF_ARRAY) {
						print("?.[%] changed ->	 ", indx);
					} else {
						print("?.% changed ->  ", var->name);
					}
					
					switch (v.type & ~VT_ANGLE) {
						case VT_FLT:	print("flt{%}\n",			v.fv.x);							break;
						case VT_FV2:			print("fv2{%, %}\n",			v.fv.x, v.fv.y);					break;
						case VT_FV3:			print("fv3{%, %, %}\n",		v.fv.x, v.fv.y, v.fv.z);			break;
						case VT_FV4:			print("fv4{%, %, %, %}\n",	v.fv.x, v.fv.y, v.fv.z, v.fv.w);	break;
						case VT_FQUAT:			print("quat{%, %, %,  %}\n",	v.fv.x, v.fv.y, v.fv.z, v.fv.w);	break;
						case VT_SINT:			print("int{%}\n",			v.iv.x);							break;
						case VT_SIV2:			print("iv2{%, %}\n",			v.iv.x, v.iv.y);					break;
						case VT_SIV3:			print("iv3{%, %, %}\n",		v.iv.x, v.iv.y, v.iv.z);			break;
						case VT_SIV4:			print("iv4{%, %, %, %}\n",	v.iv.x, v.iv.y, v.iv.z, v.iv.w);	break;
						case VT_BOOL:			print("bool{%}\n",			v.iv.x ? "true":"false");			break;
						case VT_LSTR:			print("str \"%\"\n",			v.lstr);							break;
						case VT_COLOR|VT_VEC3:	print("col{%, %, %}\n",		v.fv.x, v.fv.y, v.fv.z);			break;
						default: assert(false, "Unimplemented type in member_parsed()!");
					}
				}
				#endif
				
				cmemcpy(pdata, (void*)temp_data, data_size);
			}
			
		}
		
		//print("Sucess\n");
		return true;
	}
	
	_DECL Var const* match_var_identifier (Token const* tok, Var_Struct cr strct) {
		
		Var const* ret = nullptr;
		Var const* var = strct.members;
		
		lstr iden = tok->get_lstr();
		
		while (var) {
			if (str::comp(var->name, iden)) {
				ret = var;
				break;
			}
			var = var->next;
		}
		
		syntax(ret, tok,tok, "var ident '%' does not match any identifier registered in struct '%')!",
				iden, strct.name);
		
		return ret;
	}
	
	template<typename... Ts> DECL FORCEINLINE void push_str (cstr format, Ts... args) {
		print_copy(format, args...);
	}
	
	_DECL void print_basic_type (Var const* var, void const* pdata) {
		
		byte const* ptr = (byte*)pdata;
		Val_Union v;
		
		if ((var->type & VT_SCALAR_TYPE_BIT) != VT_EXTRAT) {
			
			auto elem = get_elements(var->type);
			
			switch (var->type & VT_SCALAR_TYPE_BIT) {
				case VT_FLT: {
					
					auto* arr = (f64*)&v.fm;
					for (u32 i=0; i<elem; ++i) {
						switch (var->type & VT_SCALAR_SIZE_BIT) {
							case VT_SIZE4: {
								arr[i] = (f64) *(f32*)ptr;
								ptr += 4;
							} break;
							case VT_SIZE8: {
								arr[i] = *(f64*)ptr;
								ptr += 8;
							} break;
							default: assert(false);
						}
					}
				} break;
				case VT_SINT:
				case VT_UINT: {
					
					auto* arr = (s64*)&v.im;
					for (u32 i=0; i<elem; ++i) {
						s64 out = 0; // stop 'may be used uninitialized' warning
						switch (var->type & VT_SCALAR_SIZE_BIT) {
							case VT_SIZE1: {
								out = (s64) (u64) *(u8*)ptr;
								if ((var->type & VT_SCALAR_TYPE_BIT) == VT_SINT) {
									out = (s64)(s8)(u8)out;
								}
								ptr += 1;
							} break;
							case VT_SIZE2: {
								out = (s64) (u64) *(u16*)ptr;
								if ((var->type & VT_SCALAR_TYPE_BIT) == VT_SINT) {
									out = (s64)(s16)(u16)out;
								}
								ptr += 2;
							} break;
							case VT_SIZE4: {
								out = (s64) (u64) *(u32*)ptr;
								if ((var->type & VT_SCALAR_TYPE_BIT) == VT_SINT) {
									out = (s64)(s32)(u32)out;
								}
								ptr += 4;
							} break;
							case VT_SIZE8: {
								out = (s64) *(u64*)ptr;
								if ((var->type & VT_SCALAR_TYPE_BIT) == VT_SINT) {
									// No sign extension
								} else {
									assert((u64)out <= (u64)limits::max<s64>());
								}
								ptr += 8;
							} break;
							default: assert(false);
						}
						arr[i] = out;
					}
				} break;
				default: {
					assert(false, "Not implemented");
				} break;
			}
			
		} else {
			
			switch (var->type) {
				case VT_BOOL: {
					v.im.arr[0].x = (s64) *(bool*)ptr;
					//ptr += sizeof(bool);
				} break;
				case VT_LSTR: {
					v.cstr = ((lstr*)ptr)->str;
					//ptr += sizeof(lstr);
				} break;
				case VT_CSTR: {
					v.cstr = *(char const**)ptr;
					//ptr += sizeof(char const*);
				} break;
				default: {
					assert(false, "Not implemented");
				} break;
			}
			
		}
		
		if ((var->type & VT_SCALAR_TYPE_BIT) != VT_EXTRAT) {
			switch (var->type & VT_SCALAR_TYPE_BIT) {
				case VT_FLT: {
					
					char const* lit="";
					char const* func="";
					char const* pare="";
					
					if (var->type & VT_ANGLE) {
						v.fm.arr[0] *= tv4<f64>(RAD_TO_DEGd);
						v.fm.arr[1] *= tv4<f64>(RAD_TO_DEGd);
						v.fm.arr[2] *= tv4<f64>(RAD_TO_DEGd);
						v.fm.arr[3] *= tv4<f64>(RAD_TO_DEGd);
						lit="_deg";
						func="deg(";
						pare=")";
					}
					if (var->type & VT_COLOR) {
						func="col(";
						pare=")";
					}
					
					auto str = out_file_data.len;
					
					switch (var->type & VT_VEC_BIT) {
						case VT_SCALAR:
							push_str("%%",						prec(v.fm.arr[0].x,9), lit);
							break;
						case VT_VEC2:
							push_str("%v2(%, %)%", func,		prec(v.fm.arr[0].x,9), prec(v.fm.arr[0].y,9), pare);
							break;
						case VT_VEC3:
							push_str("%v3(%, %, %)%", func,		prec(v.fm.arr[0].x,9), prec(v.fm.arr[0].y,9), prec(v.fm.arr[0].z,9), pare);
							break;
						case VT_VEC4:
							push_str("%v4(%, %, %, %)%", func,	prec(v.fm.arr[0].x,9), prec(v.fm.arr[0].y,9), prec(v.fm.arr[0].z,9), prec(v.fm.arr[0].w,9), pare);
							break;
						case VT_QUAT:
							push_str("quat(v3(%, %, %), %)",	prec(v.fm.arr[0].x,9), prec(v.fm.arr[0].y,9), prec(v.fm.arr[0].z,9), prec(v.fm.arr[0].w,9));
							break;
						case VT_MAT2:
						case VT_MAT3:
						case VT_MAT4:
							assert(false, "Not implemented");
							break;
						default: assert(false);
					}
					
					auto end = out_file_data.len;
					
					while (str != end) {	// Windows generates codes when printing special flaot values like infiniy or NaN (for ex. 1.#IND)
											//	since we use '#' for comments we have to replace those with something else ('@' in our case)
						if (out_file_data[str] == '#') {
							out_file_data[str] = '@';
						}
						++str;
					}
					
				} break;
				case VT_SINT:
				case VT_UINT: {
					u32 dim = get_vec_dimensions(var->type);
					if (dim != 1) {
						push_str("v%(", dim);
					}
					for (u32 i=0; i<dim; ++i) {
						if ((var->type & VT_SCALAR_TYPE_BIT) == VT_UINT) {
							push_str("%", (uptr)v.im.arr[0][i]);
						} else {
							push_str("%", v.im.arr[0][i]);
						}
					}
					if (dim != 1) {
						push_str(")");
					}
				} break;
				case VT_EXTRAT: {
					switch (var->type & (VT_SCALAR_SIZE_BIT|VT_SCALAR_TYPE_BIT)) {
						case VT_BOOL: {
							push_str(v.im.arr[0].x ? "true":"false");
						} break;
						default: {
							assert(false, "Not implemented");
						} break;
					}
				} break;
				default: assert(false);
			}
			
		} else {
			
			switch (var->type) {
				case VT_BOOL: {
					push_str(v.im.arr[0].x ? "true":"false");
				} break;
				case VT_LSTR:
				case VT_CSTR: {
					push_str("\"%\"", v.cstr);
				} break;
				default: assert(false);
			}
			
		}
	}
	
	_DECL void insert_val (Var const* var, void const* pdata, ui depth) {
		
		switch (var->type) {
			
			case VT_STRUCT: {
				
				auto* member_var = var->strct.members;
				
				push_str("{ ");
				
				if (member_var) {
					for (uptr member_i=0;; ++member_i) {
						
						void const* member_pdata = ptr_add(pdata, (uptr)member_var->pdata);
						
						insert_val(member_var, member_pdata, depth +1);
						
						member_var = member_var->next;
						if (!member_var) {
							break;
						}
						
						push_str(", ");
					}
				}
				
				push_str(" }");
				
			} break;
			
			case VT_ARRAY: {
				
				auto* member_var = var->arr.members;
				assert(member_var);
				
				uptr arr_len = var->arr.arr_len;
				uptr arr_stride = var->arr.arr_stride;
				
				push_str("[%] {\n%", arr_len, repeat("\t", depth +1));
				
				for (u32 i=0;;) {
					
					insert_val(member_var, ptr_add(pdata, i*arr_stride), depth +1);
					
					if (++i == arr_len) break;
					
					push_str(",\n%", repeat("\t", depth +1));
				}
				
				push_str("\n%}", repeat("\t", depth));
				
			} break;
			
			case VT_DYNARR: {
				
				auto* member_var = var->arr.members;
				assert(member_var);
				
				auto*	arr = (dynarr<byte>*)pdata;
				
				u32 arr_len = arr->len;
				uptr arr_stride = var->arr.arr_stride;
				
				push_str("[%] {\n%", arr_len, repeat("\t", depth +1));
				
				for (u32 i=0;;) {
					
					insert_val(member_var, arr->arr +(i*arr_stride), depth +1);
					
					if (++i == arr_len) break;
					
					push_str(",\n%", repeat("\t", depth +1));
				}
				
				push_str("\n%}", repeat("\t", depth));
				
			} break;
			
			default: {
				if (var->type & VT_FPTR_VAR) {
					print("VT_FPTR_VAR can't be written since it refers to a setter function!\n");
					return;
				}
				
				print_basic_type(var, pdata);
			} break;
		}
	}
	
	Token* member_value (Token* tok, Var const* var, parse_flags_e flags, void* pdata,
					ui depth, uptr index, Token* assgn_tok);
	
	_DECL Token* dollar_cmd_list (Token* tok, Var const* var, parse_flags_e flags, void* pdata,
			ui depth, uptr index, Token* assgn_tok) {
		
		dollar_cmd_set_e cmd_seen = (dollar_cmd_set_e)0;
		
		do {
			parse_flags_e recurse_flags = flags;
			
			dollar_command_e dollar_cmd = DOLLAR_COMMAND_COUNT;
			
			Token* write_insert_after;
			bool whitespace_exists;
			
			if (tok->tok == DOLLAR_CMD) {
				
				syntax(!(flags & PF_IN_DOLLAR_CMD_VAL), tok,tok, "dollar_cmd_list:: Dollar command values can't contain dollar commands");
				recurse_flags |= PF_IN_DOLLAR_CMD_VAL;
				
				dollar_cmd = tok->dollar_cmd;
				switch (dollar_cmd) {
					
					case DC_CURRENT: {
						syntax(!(cmd_seen & CURRENT), tok,tok, "dollar_cmd_list:: '$' encountered twice");
						cmd_seen |= CURRENT;
						
						recurse_flags |= PF_ONLY_SYNTAX_CHECK;
					} break;
					
					case DC_INITIAL:
					case DC_SAVE: {
						syntax(!(cmd_seen & INITIAL), tok,tok, "dollar_cmd_list:: '$i or $s' encountered twice (can only have one $i or one $s)");
						cmd_seen |= INITIAL;
						
						if ((flags & PF_WRITE_BIT) == PF_INITIAL_LOAD && !(cmd_seen & SET)) { // since dollar commandless value has to come before the first $ this check works
							// Load value after $i/$s
						} else {
							recurse_flags |= PF_ONLY_SYNTAX_CHECK;
						}
					} break;
					
					default:
						syntax(false, tok,tok, "dollar_cmd_list:: Invalid dollar command ('%')", tok->get_lstr());
				}
				
				#if DIAGNOSTIC_PRINT
				print(">>> %\n", tok->get_lstr());
				#endif
				
				char* tok_end = tok->begin +tok->len;
				whitespace_exists = is_whitespace_c(*tok_end) || is_newline_c(*tok_end);
				write_insert_after = tok;
				
				++tok; // consume DOLLAR_CMD
			}
			
			if (tok->tok != DOLLAR_CMD && tok->tok != SEMICOLON && tok->tok != COMMA) { // Non empty dollar command
				
				if (dollar_cmd == DOLLAR_COMMAND_COUNT) { // No dollar command
					
					assert(!(cmd_seen & SET));
					cmd_seen |= SET;
				}
				
				syntaxdev(tok = member_value(tok, var, recurse_flags, pdata, depth, index, assgn_tok),
						"dollar_cmd_list:: member_value()");
				
			}
			
			if (	(dollar_cmd == DC_CURRENT && (flags & PF_WRITE_BIT) == PF_WRITE_CURRENT) ||
					(dollar_cmd == DC_SAVE && (flags & PF_WRITE_BIT) == PF_WRITE_SAVE) ) {
				
				char* write_insert_cur =
						write_insert_after->begin +write_insert_after->len;
				write_insert_cur += whitespace_exists ? 1 : 0; // Keep first whitespace char after dollar command
				
				write_copy_until(write_insert_cur);
				
				//print("Skipped '%'\n", lstr{write_insert_pos, ptr_sub(write_insert_pos, cur)});
				
				{ // Keep trailing whitespace after existing value
					char* trail = write_insert_cur;
					Token* cur = write_insert_after;
					
					for (;;) {
						if (cur == tok) { break; } // Trailing whitespace found
						trail = cur->begin +cur->len;
						++cur;
					}
					
					write_skip_until(trail);
				}
				
				if (!whitespace_exists) {
					write_copy(" ");
				}
				
				insert_val(var, pdata, depth);
			}
			
		} while (tok->tok == DOLLAR_CMD);
		
		return tok;
	}
	
	_DECL Token* array (Token* tok, Var_Array cr aray, parse_flags_e flags, void* pdata,
			ui depth) {
		//PROFILE_SCOPED(THR_ENGINE, "array");
		
		assert(depth > 0);
		
		Token*	arr_tok; // '[] = { ... }'
		
		bool length_specified;
		uptr max_arrlen;
		{ // array syntax (var = [N] {})
			arr_tok = tok;
			++tok; // consume BRACKET_OPEN
			
			if (aray.type != VT_DYNARR) {
				max_arrlen = aray.arr_len;
			}
			
			length_specified = tok->tok != BRACKET_CLOSE;
			if (length_specified) {
				// Array brackets with expression
				Expr_Val val;
				Token* expr = tok;
				syntaxdev(tok = expression(tok, &val, flags, 0), "array::expression() (array length expr)");
				
				syntax(val.type == VT_SINT && val.im.arr[0].x >= 0, expr,tok-1,
						"array:: Array length must be a integer expression and >= 0.");
				
				if (aray.type == VT_DYNARR) {
					syntax((uptr)val.im.arr[0].x <= ((uptr)limits::max<u32>() +1), expr,tok-1,
							"array:: dynarr length must be a u32 integer expression.");
					
					max_arrlen = (uptr)val.im.arr[0].x;
				} else {
					syntax(max_arrlen == (uptr)val.im.arr[0].x, expr,tok-1, "array:: Array length must match the legnth of the fixed-size array defined in source code");
				}
			} else {
				// Empty array brackets
			}
			
			if (aray.type == VT_DYNARR) {
				if (!(flags & PF_ONLY_SYNTAX_CHECK)) {
					auto* arr = (dynarr<byte>*)pdata;
					uptr stride = aray.arr_stride;
					
					u32 cap = 16;
					if (length_specified) {
						cap = (u32)max_arrlen;
					}
					
					if (!(flags & PF_ONLY_SYNTAX_CHECK)) {
						if (!arr->arr) {
							_dyn_array_generic__init(arr, 0, cap, stride);
						} else {
							if (length_specified) {
								_dyn_array_generic__resize_cap(arr, cap, stride);
							}
							
							_dyn_array_generic__grow_to(arr, 0, stride);
						}
					}
					
				}
				
				if (!length_specified) {
					max_arrlen = (uptr)limits::max<u32>() +1;
				}
			}
			
			syntax_token(&tok, BRACKET_CLOSE, "array:: array brackets close");
			syntax_token(&tok, CURLY_OPEN, "array:: Array must follow 'var = [] {}' syntax");
		}
		
		Var const*	var = aray.members;
		assert(aray.members && !aray.members->next); // aray.members points ONE node with the basic member type or the tree of the member struct
		
		uptr		member_i = 0;
		bool		named_members = tok->tok == MEMBER_DOT;
		
		for (;;) {
			
			Token* assgn_tok = tok;
			{
				bool	member_named;
				member_named = tok->tok == MEMBER_DOT;
				if (member_named) {
					++tok;
				}
				syntax(member_named == named_members, arr_tok,tok, "array:: Don't mix named and ordered members in one member list.");
			}
			
			uptr index;
			if (named_members) {
				// Named member assignment
				
				syntax(length_specified, arr_tok,tok, "array:: named member assignment requires the array length to be specified");
				
				syntax(false, tok,tok, "array:: named member assignment not implemented");
				index = 0; // fix warning
				
			} else {
				// Ordered member assignment
				index = member_i;
			}
			
			syntax(index < max_arrlen, arr_tok,tok, "array:: Too many values for array (max_arrlen=%)!", max_arrlen);
			
			void* member_pdata;
			
			if (aray.type == VT_DYNARR) {
				auto* arr = (dynarr<byte>*)pdata;
				if (flags & PF_ONLY_SYNTAX_CHECK) {
					member_pdata = (void*)_dyn_array_generic__index(arr, (u32)index, aray.arr_stride);
				} else {
					assert(arr->len == index, "% %", arr->len, index);
					member_pdata = (void*)_dyn_array_generic__append(arr, aray.arr_stride);
				}
			} else {
				member_pdata = ptr_add(pdata, index * aray.arr_stride);
			}
			
			syntaxdev(tok = dollar_cmd_list(tok, var, flags|PF_MEMBER_OF_ARRAY, member_pdata,
					depth, index, assgn_tok),
					"array:: dollar_cmd_list()");
			
			++member_i;
			
			switch (tok->tok) {
				case COMMA:
				case SEMICOLON:
					++tok; // consume COMMA or SEMICOLON
					break;
				
				case CURLY_CLOSE:
				case EOF_:
				case EOF_MARKER:
					break;
					
				default:
					syntax(false, arr_tok,tok, "array:: need comma, semicolon or curly close");
					
			}
			
			switch (tok->tok) {
				case CURLY_CLOSE:
					++tok; // consume CURLY_CLOSE
					goto end_l;
				case EOF_:
				case EOF_MARKER:
					syntax(false, arr_tok,tok, "array:: need curly close");
				
				default: {}
			}
			
		} end_l:;
		
		if (length_specified) {
			syntax(member_i == max_arrlen, arr_tok,tok,
						"array:: Array length must match array length specified in '= [] {}' syntax (len=%)", max_arrlen);
		} else {
			if (aray.type == VT_DYNARR) {
				assert(member_i <= ((uptr)limits::max<u32>() +1));
				//syntax((member_i +1) == )
				
				auto* arr = (dynarr<byte>*)pdata;
				uptr stride = aray.arr_stride;
				
				_dyn_array_generic__fit_cap(arr, stride);
			} else {
				syntax(member_i == max_arrlen, arr_tok,tok,
						"array:: Array length must match lengh of static array specified in source code (len=%)", max_arrlen);
			}
		}
		
		return tok;
	}
	
	_DECL Token* structure (Token* tok, Var_Struct cr strct, parse_flags_e flags, void* pdata,
			ui depth) {
		//PROFILE_SCOPED(THR_ENGINE, "structure");
		
		Var const*	ordered_vars = strct.members;
		lstr		iden;
		uptr		member_i = 0;
		bool		named_members = false;
		
		Token* strct_tok = tok;
		if (depth > 0) {
			++tok; // consume CURLY_OPEN
		}
		
		if (depth == 0) {
			named_members = tok->tok == IDENTIFIER;
		} else {
			named_members = tok->tok == MEMBER_DOT;
		}
		
		for (;;) {
			
			Token* assgn_tok = tok;
			{
				bool	member_named;
				if (depth == 0) {
					member_named = tok->tok == IDENTIFIER;
				} else {
					member_named = tok->tok == MEMBER_DOT;
					if (member_named) {
						++tok;
					}
				}
				syntax(member_named == named_members, assgn_tok,tok, "structure:: Don't mix named and ordered members in one member list.");
			}
			
			ui			inline_depth = depth;
			void*		inline_pdata = pdata;
			Var const*	var;
			
			if (named_members) {
				// Named member assignment
				
				{
					auto inline_strct = strct;
					
					for (;; ++inline_depth) {
						syntax_token(&tok, IDENTIFIER, "structure:: named var iden");
						
						auto tmp = inline_strct;
						var = match_var_identifier(tok-1, inline_strct);
						syntaxdev(var, "structure::match_var_identifier()");
						
						if (tok->tok != MEMBER_DOT) { break; }
						++tok;
						
						syntax(tmp.type == VT_STRUCT,
								assgn_tok,tok, "structure:: inline dot syntax can only be used with structs");
						
						inline_strct = var->strct;
						
						inline_pdata = ptr_add(inline_pdata, var->pdata);
						
						#if DIAGNOSTIC_PRINT
						print_member(var->name, (parse_flags_e)0, inline_depth, 0);
						#endif
					}
				}
				
				syntax_token(&tok, EQUALS, "structure:: member assign");
				
			} else {
				// Ordered member assignment
				syntax(depth > 0, assgn_tok,tok, "structure:: No ordered member assignment in gloabl scope.");
				
				var = ordered_vars;
				
				ordered_vars = ordered_vars->next;
				
				syntax(var, strct_tok,tok,
						"structure:: Too many members in ordered member list, '%' only takes % members.",
						strct.name, member_i);
				
			}
			
			inline_pdata = ptr_add(inline_pdata, var->pdata);
			
			syntaxdev(tok = dollar_cmd_list(tok, var, flags & ~PF_MEMBER_OF_ARRAY, inline_pdata,
					inline_depth, 0, assgn_tok),
					"structure:: dollar_cmd_list()");
			
			++member_i;
			
			switch (tok->tok) {
				case COMMA:
					syntax(depth > 0, tok,tok, "structure:: ',' as member terminator not allowed in global scope.");
					// fallthrough
				case SEMICOLON:
					++tok; // consume COMMA or SEMICOLON
					break;
				
				case CURLY_CLOSE:
				case EOF_:
				case EOF_MARKER:
					break;
					
				default:
					syntax(false, strct_tok,tok, "structure:: need comma, semicolon or curly close");
			}
			
			switch (tok->tok) {
				case CURLY_CLOSE:
					syntax(depth > 0, tok,tok, "structure:: stray '}' in global scope.");
					++tok; // consume CURLY_CLOSE
					goto end_l;
				case EOF_:
				case EOF_MARKER:
					syntax(depth == 0, strct_tok,tok, "structure::structure need curly close");
					goto end_l;
				
				default: {}
			}
			
		} end_l:;
		
		return tok;
	}
	
	Token* member_value (Token* tok, Var const* var, parse_flags_e flags, void* pdata,
					ui depth, uptr index, Token* assgn_tok) {
		
		bool member_array = tok->tok == BRACKET_OPEN;
		syntax( member_array == is_array(var->type),
				assgn_tok,tok,
				member_array ?	"member_value:: Non-array-variable cannot have 'var = []' syntax" :
								"member_value:: Array variable must have 'var = [] {}' syntax" );
		
		bool strct = tok->tok == CURLY_OPEN;
		syntax( strct == !!(var->type == VT_STRUCT),
				assgn_tok,tok, "member_value:: 'var = { ... }' syntax can only be used for structs");
		
		if (var->type & VT_FPTR_VAR) {
			syntax(!member_array && !strct && depth == 0, assgn_tok,tok,
					"member_value:: func-ptr variable can only be at global scope and can only take basic types");
		}
		
		if (member_array) {
			
			#if DIAGNOSTIC_PRINT
			print_member(var->name, flags, depth, index);
			#endif
			
			syntaxdev(tok = array(tok, var->arr, flags, pdata, depth +1),
					"member_value::array()");
			
		} else if (strct) {
			
			#if DIAGNOSTIC_PRINT
			print_member(var->name, flags, depth, index);
			#endif
			
			syntaxdev(tok = structure(tok, var->strct, flags, pdata, depth +1),
					"member_value::structure()");
			
		} else {
			
			assert(basic_type(var->type));
			
			Expr_Val	val;
			
			if (is_flt(var->type)) {
				flags |= PF_INFER_TYPE;
				val.type = VT_FLT;
			}
			
			Token* expr = tok;
			syntaxdev(tok = expression(tok, &val, flags, 0), "member_value:: expression()");
			
			member_parsed(var, val, flags, pdata, depth, index, expr,tok-1);
			
		}
		
		return tok;
	}
	
	_DECL Token* file (Token* tokens, parse_flags_e flags) {
		
		assert(root_var->type == VT_STRUCT);
		
		Token* tok = structure(tokens, root_var->strct, flags, nullptr, 0);
		syntaxdev(tok && tok[0].tok == EOF_MARKER && tok[1].tok == EOF_, "File parsing error!");
		
		++tok; // consume EOF_MARKER
		
		return tok;
	}
	
	#if 0
	namespace zero_strings_n {
		_DECL void member_found (Var const* var, void* pdata, ui depth, uptr indx, uptr member_of_array) {
			#if 0
			++depth;
			while (depth--) {
				print("	 ");
			}
			if (member_of_array) {
				print(".[%]\n", indx);
			} else {
				print(".%\n", var->name);
			}
			#endif
			
			switch (var->type) {
				case VT_CSTR: {
					*(char const**)pdata = nullptr;
				} break;
				case VT_LSTR: {
					*(lstr*)pdata = { nullptr, 0 };
				} break;
				default: {	}
			}
		}
		_DECL void member_list (Var const* parent, void* pdata, ui depth, uptr array) {
			
			Var const*	var = parent->strct.members;
			
			for (uptr member_i=0; var; ++member_i) {
				
				void*		recurse_pdata = pdata;
				
				if (!array) {
					
					if (var->type == VT_DYNARR) {
						auto* arr = (dynarr<byte>*)pdata;
						recurse_pdata = arr->arr;
					} else {
						recurse_pdata = ptr_add(recurse_pdata, var->pdata);
					}
				} else {
					
					if (member_i == array) {
						return;
					}
					
					recurse_pdata = ptr_add(recurse_pdata, member_i * parent->arr.arr_stride);
				}
				
				{
					uptr member_array = 0;
					if (var->type & VT_ARRAY) {
						member_array = parent->arr.arr_len;
					} else if (var->type & VT_DYNARR) {
						auto* arr = (dynarr<byte>*)pdata;
						member_array = arr->len;
					}
					
					bool recurse = var->type & (VT_STRUCT|VT_ARRAY|VT_DYNARR);
					
					if (recurse) {
						
						//print_member_or_array_elem(var->name, depth, member_i, array);
						
						member_list(var, recurse_pdata, depth +1, member_array);
						
						
					} else {
						
						member_found(var, recurse_pdata, depth, member_i, array);
						
					}
				}
				
				var = var->next;
			}
		}
		
	}
	
	_DECL void zero_strings () {
		//print_member(root_var->name, -1);
		zero_strings_n::member_list(root_var, nullptr, 0, 0);
	}
	#endif
	
	//////
	
	namespace entities {
		
		/////////
		typedef u32 eid;
		
		struct Entity {
			lstr			name;
			
			eid				next;
			eid				parent;
			eid				children;
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
		
		struct eMesh_Base : public Entity {
			Mesh*			mesh;
			materials_e		material;
		};
		struct eMesh : public eMesh_Base {
			Textures_Generic	tex;
		};
		struct eMesh_Nanosuit : public eMesh_Base {
			Textures_Nanosuit	tex;
		};
		struct Material_Showcase_Grid : public Entity {
			Mesh*			mesh;
			v2				grid_offs;
		};
		struct Group : public Entity {
			
		};
		struct Light : public Entity {
			light_type_e	type;
			light_flags_e	flags;
			v3				luminance;
			
			Mesh*			mesh;
		};
		struct Scene : public Entity {
			bool			draw;
			dynarr<Light*>	lights;
		};
		
		union Entity_union {
			Entity					base;
			eMesh_Base				eMesh_Base;
			eMesh					eMesh;
			eMesh_Nanosuit			eMesh_Nanosuit;
			Material_Showcase_Grid	Material_Showcase_Grid;
			Group					Group;
			Light					Light;
			Scene					Scene;
			Root					Root;
			
			DECLM FORCEINLINE Entity_union () {}
			DECLM static FORCEINLINE Entity_union null () {
				Entity_union ret;
				ret.base = Entity{	"<null>", 0,0,0,
								v3(QNAN), quat(v3(QNAN), QNAN), v3(QNAN),
								AABB::qnan(), AABB::qnan(),
								(entity_flag)-1, (entity_tag)-1 };
				cmemset(&ret.base +1, DBG_MEM_UNINITIALIZED_BYTE, sizeof(Entity_union) -sizeof(Entity));
				return ret;
			}
		};
		/////////
		
		enum ctor_e : u32 {
			CTOR_SCENE=0,
			CTOR_LIGHT_SUNLIGHT,
			CTOR_LIGHT_LIGHTBULB,
			CTOR_MESH,
			CTOR_MESH_NANOSUIT,
			CTOR_GROUP,
			CTOR_COUNT
		};
		
		DECLD Str_Enum ctors[] = {
			{"scene",			CTOR_SCENE				},
			{"sunlight",		CTOR_LIGHT_SUNLIGHT		},
			{"lightbulb",		CTOR_LIGHT_LIGHTBULB	},
			{"mesh",			CTOR_MESH				},
			{"mesh_nanosuit",	CTOR_MESH_NANOSUIT		},
			{"group",			CTOR_GROUP				},
		};
		
		DECL ctor_e match_ctor (lstr cr iden) {
			return (ctor_e)match_iden(iden, ctors, CTOR_COUNT);
		}
		
		DECLD dynarr<char> str_storage;
		
		_DECL Token* quoted_string_ (Token* tok, lstr* val) {
			
			syntax(tok->tok == STRING, tok,tok, "entities::quoted_string need string");
			
			#if 0
			if (flags & PF_ONLY_SYNTAX_CHECK)
				*val = lstr{ nullptr, 0 };
				
				while ((++tok)->tok == STRING) {} 
				
			#endif
			
			bool append = false;
			for (;;) {
				
				char const* cur = tok->begin;
				assert(tok->len >= 2);
				assert(*cur++ == '"');
				
				auto str_copy = str_storage.len;
				
				if (append) {
					assert(str_storage.len >= 1 && str_storage[str_storage.len -1] == '\0');
					str_storage.pop();
				}
				
				char const* end = cur +(tok->len -2);
				while (cur != end) {
					assert(*cur != '\0' && *cur != '"');
					
					if (*cur == '`') {
						if (cur[1] == '`' || cur[1] == '"') {
							++cur; // escaped '"' or '`'
						} else {
							assert(false);
						}
					}
					
					str_storage.push(*cur++);
				}
				
				assert(*cur++ == '"');
				
				str_storage.push('\0');
				
				u32 len = (str_storage.len -str_copy) -1;
				if (append) {
					val->len += len;
				} else {
					*val = lstr{ (char*)(uptr)str_copy, len };
				}
				
				if ((++tok)->tok != STRING) break;
				
				append = true;
			}
			
			return tok;
		}
		
		_DECL Token* position (Token* tok, v3* val) {
			auto* tok_expr = tok;
			
			Expr_Val pos;
			syntaxdev(tok = expression(tok, &pos, (parse_flags_e)0, 0), "position:: expression()");
			syntax(pos.type == VT_FV3, tok_expr,tok, "position:: need v3 expression");
			
			*val = cast_v3<v3>(pos.fm.arr[0].xyz());
			return tok;
		}
		_DECL Token* orientation (Token* tok, quat* val) {
			auto* tok_expr = tok;
			
			Expr_Val ori;
			syntaxdev(tok = expression(tok, &ori, (parse_flags_e)0, 0), "orientation:: expression()");
			syntax(ori.type == VT_FQUAT, tok_expr,tok, "orientation:: need quat expression");
			
			auto tmp = cast_v4<v4>(ori.fm.arr[0]);
			*val = quat(v3(tmp.xyz()), tmp.w);
			return tok;
		}
		
		_DECL Token* luminance (Token* tok, v3* val) {
			auto* tok_expr = tok;
			
			Expr_Val pos;
			syntaxdev(tok = expression(tok, &pos, (parse_flags_e)0, 0), "luminance:: expression()");
			syntax(pos.type == (VT_FV3|VT_COLOR), tok_expr,tok, "luminance:: need col expression");
			
			*val = cast_v3<v3>(pos.fm.arr[0].xyz());
			return tok;
		}
		
		_DECL Token* enum_expression (Token* tok, u64* val, u64* mask, ::array<Enum_Member> cr en) {
			
			*val = 0;
			*mask = 0;
			
			for (;;) {
				
				auto* ident = tok++;
				switch (ident->tok) {
					case IDENTIFIER: {
						
						lstr str = ident->get_lstr();
						
						Enum_Member const* m = nullptr;
						for (u32 i=0; i<en.len; ++i) {
							if (str::comp(en[i].identifier, str)) m = &en[i];
						}
						syntax(m, ident,ident, "enum_expression:: identifier '%' not found for enum", str);
						
						u64 tmp;
						
						if (tok->tok == PAREN_OPEN) { ++tok;
							auto* expr_tok = tok;
							
							Expr_Val tmp_;
							syntaxdev(tok = expression(tok, &tmp_, (parse_flags_e)0, 0), "enum_expression:: MEMBER(VALUE) syntax:: VALUE expression()");
							syntax(tmp_.type == VT_SINT && tmp_.im.arr[0].x >= 0, expr_tok,tok, "enum_expression:: MEMBER(VALUE) syntax:: need uint for VALUE");
							
							tmp = tmp_.im.arr[0].x;
							
							u64 valid_range = ((u64)0b1 << m->bit_len) -1;
							syntax((tmp & ~valid_range) == 0, expr_tok,tok, "enum_expression:: MEMBER(VALUE) syntax:: VALUE out of range");
							
							syntax_token(&tok, PAREN_CLOSE, "enum_expression:: paren close");
						} else {
							syntax(m->bit_len == 1, ident,ident, "enum_expression:: multi-bit enum members require MEMBER(VALUE) syntax");
							
							tmp = (u64)0b1;
						}
						
						tmp <<= m->bit_offs;
						
						syntax((*mask & tmp) == 0, ident,ident, "enum_expression:: some bit in enum set twice by orring %", m->identifier);
						*mask |= tmp;
						*val |= tmp;
						
					} break;
					case NUMBER: {
						
					} break;
					default: syntax(false, tok,tok, "enum_expression need nember or enum expression");
				}
				
				if (tok->tok == OR) {
					
					
					++tok;
				} else {
					return tok;
				}
			}
			
		}
		
		_DECL Token* light_flags (Token* tok, light_flags_e* val) {
			u64 flags, mask;
			syntaxdev(tok = enum_expression(tok, &flags,&mask, LIGHT_FLAGS_E), "light_flags:: expression()");
			assert(safe_cast(u32, mask));
			
			*val = (light_flags_e)flags;
			return tok;
		}
		
		_DECL Token* e_scene (Token* tok, Scene* e) {
			e->tag = ET_SCENE;
			print("scene % % %\n", e->name.to_abs(str_storage.arr), e->pos, e->ori);
			return tok;
		}
		_DECL Token* e_light_sunlight (Token* tok, Light* e) {
			syntax_token(&tok, COMMA, "e_light_lightbulb:: comma");
			
			syntaxdev(tok = light_flags(tok, &e->flags), "e_light_lightbulb:: light_flags()");
			syntax_token(&tok, COMMA, "e_light_sunlight:: comma");
			
			syntaxdev(tok = luminance(tok, &e->luminance), "e_light_sunlight:: luminance()");
			
			e->tag = ET_LIGHT;
			print("sunlight % % % % %\n", e->name.to_abs(str_storage.arr), e->pos, e->ori, (u32)e->flags, e->luminance);
			return tok;
		}
		_DECL Token* e_light_lightbulb (Token* tok, Light* e) {
			syntax_token(&tok, COMMA, "e_light_lightbulb:: comma");
			
			syntaxdev(tok = light_flags(tok, &e->flags), "e_light_lightbulb:: light_flags()");
			syntax_token(&tok, COMMA, "e_light_lightbulb:: comma");
			
			v3 lum;
			syntaxdev(tok = luminance(tok, &e->luminance), "e_light_lightbulb:: luminance()");
			
			e->tag = ET_LIGHT;
			print("lightbulb % % % % %\n", e->name.to_abs(str_storage.arr), e->pos, e->ori, (u32)e->flags, e->luminance);
			return tok;
		}
		_DECL Token* e_mesh (Token* tok, eMesh* e) {
			syntax_token(&tok, COMMA, "e_mesh:: comma");
			
			lstr mesh;
			syntaxdev(tok = quoted_string_(tok, &mesh), "e_mesh:: quoted_string_()");
			
			e->tag = ET_MESH;
			print("mesh % % % %\n", e->name.to_abs(str_storage.arr), e->pos, e->ori, mesh.to_abs(str_storage.arr));
			return tok;
		}
		_DECL Token* e_mesh_nanosuit (Token* tok, eMesh_Nanosuit* e) {
			syntax_token(&tok, COMMA, "e_mesh_nanosuit:: comma");
			
			lstr mesh;
			syntaxdev(tok = quoted_string_(tok, &mesh), "e_mesh_nanosuit:: quoted_string_()");
			
			e->tag = ET_MESH_NANOSUIT;
			print("mesh_nanosuit % % % %\n", e->name.to_abs(str_storage.arr), e->pos, e->ori, mesh.to_abs(str_storage.arr));
			return tok;
		}
		_DECL Token* e_group (Token* tok, Group* e) {
			e->tag = ET_GROUP;
			print("group % % %\n", e->name.to_abs(str_storage.arr), e->pos, e->ori);
			return tok;
		}
		
		DECLD dynarr<Entity_union> entities;
		
		_DECL Token* nodes (Token* tok, eid paren, u32 depth) {
			
			eid* prev_next = paren ? &entities[paren].base.children : nullptr;
			
			for (;;) {
				auto* node_iden = tok;
				syntax_token(&tok, IDENTIFIER, "entities::node:: IDENTIFIER");
				
				syntax_token(&tok, PAREN_OPEN, "entities::node:: PAREN_OPEN");
				
				print("%", repeat("  ", depth));
				
				auto ctor = match_ctor(node_iden->get_lstr());
				
				auto id = entities.len;
				auto* e = &entities.push();
				
				syntaxdev(tok = quoted_string_(tok, &e->base.name), "e_scene:: quoted_string_()");
				syntax_token(&tok, COMMA, "e_scene:: comma");
				
				if (prev_next) *prev_next = id;
				
				e->base.next =		0;
				e->base.parent =	paren;
				e->base.children =	0;
				
				syntaxdev(tok = position(tok, &e->base.pos), "pos_ori:: position()");
				syntax_token(&tok, COMMA, "pos_ori:: comma");
				
				syntaxdev(tok = orientation(tok, &e->base.ori), "pos_ori:: orientation()");
				
				e->base.scale = v3(1);
				e->base.aabb_mesh_paren = AABB::qnan();
				e->base.aabb_mesh_world = AABB::qnan();
				e->base.eflags = (entity_flag)0;
				
				switch (ctor) {
					case CTOR_SCENE:			tok = e_scene(tok, &e->Scene);					break;
					case CTOR_LIGHT_SUNLIGHT:	tok = e_light_sunlight(tok, &e->Light);			break;
					case CTOR_LIGHT_LIGHTBULB:	tok = e_light_lightbulb(tok, &e->Light);		break;
					case CTOR_MESH:				tok = e_mesh(tok, &e->eMesh);					break;
					case CTOR_MESH_NANOSUIT:	tok = e_mesh_nanosuit(tok, &e->eMesh_Nanosuit);	break;
					case CTOR_GROUP:			tok = e_group(tok, &e->Group);					break;
					default:
						syntax(false, tok,tok, "entities::node:: unknown ctor '%'", node_iden->get_lstr());
				}
				syntaxdev(tok, "entities::node:: ctor func '%'", node_iden->get_lstr())
				
				
				syntax_token(&tok, PAREN_CLOSE, "entities::node:: PAREN_CLOSE");
				
				if (tok->tok == ANGULAR_OPEN) { ++tok;
					
					Expr_Val val;
					syntaxdev(tok = number_literal(tok, &val, (parse_flags_e)0), "entities::node:: entity_id number_literal()");
					
					syntax_token(&tok, CURLY_CLOSE, "entities::node:: PAREN_CLOSE");
					
					assert(false, "not implemented");
					entities.pop();
					e = &entities.push(*e);
					
				} else {
					
				}
				
				if (tok->tok == CURLY_OPEN) { ++tok;
					
					if (tok->tok != CURLY_CLOSE) {
						syntaxdev(tok = nodes(tok, id, depth +1), "entities::node:: recurse nodes()");
					}
					
					syntax_token(&tok, CURLY_CLOSE, "entities::node:: PAREN_CLOSE");
				}
				
				if (tok->tok == CURLY_CLOSE || tok->tok == EOF_) {
					break;
				}
				
				prev_next = &e->base.next;
			}
			
			return tok;
		}
		
		_DECL Token* file (Token* tok) {
			
			auto* e = &entities.push();
			*e = Entity_union::null();
			syntaxdev(tok = nodes(tok, 0, 0), "entities::file:: node()");
			
			return tok;
		}
	}
	
	_DECL void test_parse_entitites () {
		
		print(">>>>>>>>>> test_parse_entitites\n");
		
		HANDLE fh;
		assert(!win32::open_existing_file("all.entities", &fh,
				GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE));
		
		//
		entities::str_storage.free();
		entities::entities.free();
		
		auto data = ::array<char>::alloc(win32::get_file_size(fh) +1);
		entities::str_storage = entities::str_storage.alloc(kibi(4));
		entities::entities = entities::entities.alloc(256);
		
		win32::set_filepointer(fh, 0);
		
		assert(!win32::read_file(fh, data.arr, data.len -1));
		data[data.len -1] = '\0';
		
		auto tokens = tokenize(data);
		defer { tokens.free(); };
		
		if (tokens.len >= 1 && tokens[tokens.len -1].tok == EOF_) {
			
			auto* tok = entities::file(tokens.arr);
			if (tok && tok->tok == EOF_) {
				
			} else {
				// failed parse
			}
			
		} else {
			warning("tokenize() error or ^EOF_MARKER^ missing (which might mean the file was incomplete)");
		}
		
		print(">>>>>>>>>> \n");
		
	}
	
	void init () {
		inplace_sort(keywords, KEYWORDS_COUNT);
		inplace_sort(dollar_commands, DOLLAR_COMMAND_COUNT);
		inplace_sort(entities::ctors, entities::CTOR_COUNT);
		init_vars();
	}
	
}

#undef S

DECLD HANDLE				var_file_h;
DECLD FILETIME				var_file_filetime;

DECLD array<char>			var_file_data;
DECLD array<var::Token>		var_file_tokens;

DECLD HANDLE				cmd_pipe_handle;
DECLD OVERLAPPED			cmd_pipe_ov =					{}; // initialize to zero

DECL bool pipe_test () {
	
	if (cmd_pipe_handle == INVALID_HANDLE_VALUE) {
		return false;
	}
	
	{
		auto ret = HasOverlappedIoCompleted(&cmd_pipe_ov);
		if (ret == FALSE) {
			// still pending
			return false;
		} else {
			//print("ConnectNamedPipe() completed\n");
		}
	}
	
	bool cmd_read = false;
	
	do {
		constexpr DWORD SIZE = 64;
		char buf[SIZE +1];
		
		DWORD bytes_read;
		{
			auto ret = ReadFile(cmd_pipe_handle, buf, SIZE, &bytes_read, &cmd_pipe_ov);
			
			if (ret != 0) {
				//print("ReadFile() completed instantly\n");
				assert(bytes_read <= SIZE);
			} else {
				auto err = GetLastError();
				if (err == ERROR_MORE_DATA) {
					warning("Message bigger than buffer recieved over %!, ignoring.\n", CMD_PIPE_NAME);
					break;
				} else {
					
					assert(err == ERROR_IO_PENDING, "%", err);
					assert(bytes_read == 0, "%", bytes_read);
					
					{
						auto ret = GetOverlappedResult(cmd_pipe_handle, &cmd_pipe_ov, &bytes_read, TRUE);
						assert(ret != 0);
					}
					print("ReadFile() completed after waiting\n");
					assert(bytes_read <= SIZE);
					
				}
			}
		}
		buf[bytes_read] = '\0';
		
		lstr iden = { "", 0 };
		{
			char* cur = buf;
			char* res;
			
			res = parse_n::whitespace(cur);
			if (res) { cur = res; }
			
			if (parse_n::is_identifier_start_c(*cur)) {
				
				cur = var::identifier(cur, &iden);
				
				res = parse_n::whitespace(cur);
				if (res) { cur = res; }
			}
			
			res = parse_n::newline(cur);
			if (res) { cur = res; }
		}
		
		if (	str::comp(iden, "update") ) {
			//print("update cmd recieved.\n");
			cmd_read = true;
		} else {
			print("unknown cmd(-format) recieved (cmd: '%')\n",
					escaped( lstr{buf, bytes_read} ) );
		}
		//bool cmd = str::comp(buf, bytes_read, "var_write_save");
		//bool cmd = str::comp(buf, bytes_read, "var_read");
		
	} while (0);
	
	{
		auto ret = DisconnectNamedPipe(cmd_pipe_handle);
		assert(ret != 0, "%", GetLastError());
	}
	
	{
		auto ret = ConnectNamedPipe(cmd_pipe_handle, &cmd_pipe_ov);
		
		if (ret != 0) {
			print("ConnectNamedPipe() completed instantly\n");
		} else {
			auto err = GetLastError();
			assert(err == ERROR_IO_PENDING, "%", err);
			//print("ConnectNamedPipe() pending\n");
		}
	}
	
	return cmd_read;
}

DECL bool parse_var_file (var::parse_flags_e flags) {
	PROFILE_SCOPED(THR_ENGINE, "parse_var_file");
	
	//print("parsing var file:\n");
	
	constexpr ui MAX_RETRIES = 2;
	for (ui i=0;; ++i) {
		
		var_file_data.free();
		var_file_tokens.free();
		
		#if DBG_MEMORY
		cmemset(string_buf, DBG_MEM_UNINITIALIZED_BYTE, arrlenof(string_buf));
		#endif
		string_buf_cur = string_buf;
		
		var_file_data = var_file_data.alloc(win32::get_file_size(var_file_h) +1);
		
		
		win32::set_filepointer(var_file_h, 0);
		
		assert(!win32::read_file(var_file_h, var_file_data.arr, var_file_data.len -1));
		var_file_data[var_file_data.len -1] = '\0';
		
		var_file_tokens = var::tokenize(var_file_data);
		
		var::Token* tok;
		if (var_file_tokens.len >= 2 && var_file_tokens[var_file_tokens.len -2].tok == var::EOF_MARKER && var_file_tokens[var_file_tokens.len -1].tok == var::EOF_) {
			
			tok = var::file(&var_file_tokens[0], flags);
			
			if (!tok && i == 0) {
				platform::write_whole_file("wierd_file_parse_error.txt", var_file_data.arr, var_file_data.len -1);
			}
			
			if (tok) {
				break; // success
			}
			
		} else {
			platform::write_whole_file("wierd_file_parse_error.txt", var_file_data.arr, var_file_data.len -1);
			warning("tokenize() error or ^EOF_MARKER^ missing (which might mean the file was incomplete)");
		}
		
		platform::write_whole_file("wierd_file_parse_error.txt", var_file_data.arr, var_file_data.len -1);
		
		if (i < MAX_RETRIES) {
			warning("File parse error, trying again.");
		} else {
			warning("File parse error, MAX_RETRIES reached, failed to parse!");
			return false;
		}
		
		Sleep(i);
	}
	
	//heap.dbg_enumerate_heap();
	
	print("File parse sucessful.\n");
	return true;
}
DECL void parse_prev_file_data (var::parse_flags_e flags) {
	PROFILE_SCOPED(THR_ENGINE, "parse_prev_file_data");
	
	write_copy_cur = &var_file_data[0];
	out_file_data = out_file_data.alloc(var_file_data.len +(var_file_data.len / 4));
	defer { out_file_data.free(); };
	
	print_copy = print_n::make_printer_dynarr(&out_file_data);
	
	var::Token* tok = var::file(&var_file_tokens[0], flags);
	if (!tok) { return; }
	
	write_copy_until(tok->begin);
	
	win32::set_filepointer_and_eof(var_file_h, 0);
	
	assert(!win32::write_file(var_file_h, out_file_data.arr, out_file_data.len));
	
	assert(GetFileTime(var_file_h, NULL, NULL, &var_file_filetime) != 0); // Try to prevent the CompareFileTime in frame() from triggering on our own changes, although we still seem to do sometimes
	
}

