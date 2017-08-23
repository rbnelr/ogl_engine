
namespace meshes_file_n {
	#pragma pack (push, 1)

	enum format_e : u32 {
								F_NORM_XYZ =				0b00001,
								F_UV_UV =					0b00010,
								F_TANG_XYZW =				0b00100,
								F_COL_RGB =					0b01000,
								F_INDX_U32 =				0b10000,
	};
	DEFINE_ENUM_FLAG_OPS(format_e, u32);
	
	DECLD constexpr format_e	F_INDX_MASK = (format_e)	0b10000;
	DECLD constexpr format_e	F_INDX_U16 = (format_e)		0b00000;
	
	DECL ui get_vertex_size (format_e f) {
		ui ret =						(ui)sizeof(v3); // pos xyz
		if (f & F_NORM_XYZ)		ret +=	(ui)sizeof(v3);
		if (f & F_UV_UV)		ret +=	(ui)sizeof(v2);
		if (f & F_TANG_XYZW)	ret +=	(ui)sizeof(v4);
		if (f & F_COL_RGB)		ret +=	(ui)sizeof(v3);
		return ret;
	}
	
	DECLD constexpr file_id_8 FILE_ID = {{'R','A','Z','M','E','S','H','S'}};
	
	struct fHeader {
		file_id_8	id;
		u64			file_size;
		
		u32			meshes_count;
		u32			str_tbl_size;
		
		u64			total_vertex_size;
		u64			total_index_size;
	};
	
	struct fMesh {
		u64			data_offs; // From start of file
		u32			index_count;
		u32			vertex_count;
		
		format_e	format;
		u32			mesh_name; // offs into str_tbl
	};
	
	#if 0
	struct fFile {
		fHeader		header;
		char		str_tbl[header.str_tbl_size];
		fMesh		meshes[header.meshes_count];
		
		byte		mesh_data[];
	};
	#endif
	
	#pragma pack (pop)
}
