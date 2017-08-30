
struct Triangle_Indx16 {
	u16 arr[3];
};

struct Vertex_NOUV {
	v3	pos;
	v3	norm;
};
struct Vertex_NOUV_VCOL {
	v3	pos;
	v3	norm;
	v3	col;
};
struct Vertex_VCOL {
	v3	pos;
	v3	norm;
	v2	uv;
	v3	col;
};
struct Vertex_COMMON {
	v3	pos;
	v3	norm;
	v2	uv;
	v4	tang;
};


enum mesh_format_e : u32 {
	MESH_FORMAT_FIRST=0,
	
	MF_NOUV=0,
	MF_NOUV_VCOL,
	MF_VCOL,
	MF_COMMON,
	
	MESH_FORMAT_COUNT
}; DEFINE_ENUM_ITER_OPS(mesh_format_e, u32)
struct Mesh_Format {
	lstr				file_ext;
	u32					vertex_stride;
	
	GLsizeiptr			vertex_size;
	GLsizeiptr			index_size;
	
	f32*				vertex_data;
	Triangle_Indx16*	index_data;
	
	GLuint				vbo_vert;
	GLuint				vbo_indx;
	GLuint				vao;
};

struct Mesh_Format_Components {
	bool	uv, col, tang;
};

#define MESH_FORMATS \
	X("nouv",		sizeof(Vertex_NOUV),		0,0,0 ) \
	X("nouv_vcol",	sizeof(Vertex_NOUV_VCOL),	0,1,0 ) \
	X("vcol",		sizeof(Vertex_VCOL),		1,1,0 ) \
	X("common",		sizeof(Vertex_COMMON),		1,0,1 )
	

struct Mesh {
	lstr					name; // relative to str_tbl
	mesh_format_e			format;
	
	struct {
		u32					vertex_count;
		u32					triangle_count;
		byte*				vertecies; // relative to file
		Triangle_Indx16*	triangles; // relative to file
	} data;
	
	struct {
		GLsizei				indx_count;
		GLvoid*				indx_offset;
		GLint				base_vertex;
	} vbo_data;
	
	AABB					aabb;
	
};

namespace meshes_file_n {
	
	DECLD constexpr file_id_8 FILE_ID = {{'R','A','Z','M','E','S','H','S'}};
	
	struct fHeader {
		file_id_8			id;
		u64					file_size;
		
		u32					meshes_count;
		u32					str_tbl_size;
	};
	
	struct fMesh_Format {
		GLsizeiptr			vertex_size;
		GLsizeiptr			index_size;
		
		f32*				vertex_data;
		Triangle_Indx16*	index_data;
	};
	#if 0
	struct fMesh_Format_Data {
		byte				vertex[ vertex_size ];
		u16					index[ index_count ];
	};
	
	struct fFile {
		fHeader				header;
		char				str_tbl[ header.str_tbl_size ];
		
		Mesh				meshes[ header.meshes_count ];
		
		fMesh_Format		formats[MESH_FORMAT_COUNT];
		fMesh_Format_Data	formats_data[MESH_FORMAT_COUNT];
	};
	#endif
	
}
