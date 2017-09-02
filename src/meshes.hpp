
#include "meshes_file.hpp"

enum non_entity_meshes_e : u32 {
	NON_ENTITY_MESHES_FIRST=0,
	
	AXIS_CROSS_POS_X=0,
	AXIS_CROSS_POS_Y,
	AXIS_CROSS_POS_Z,
	AXIS_CROSS_NEG_X,
	AXIS_CROSS_NEG_Y,
	AXIS_CROSS_NEG_Z,
	AXIS_CROSS_PLANE,
	
	NON_ENTITY_MESHES_COUNT
}; DEFINE_ENUM_ITER_OPS(non_entity_meshes_e, u32)

struct Meshes {
	
	Mesh*			file_meshes;
	u32				file_mesh_count;
	
	Mesh*			non_entity_meshes[NON_ENTITY_MESHES_COUNT] = {
		(Mesh*)"axis_cross_px.nouv",
		(Mesh*)"axis_cross_py.nouv",
		(Mesh*)"axis_cross_pz.nouv",
		(Mesh*)"axis_cross_nx.nouv",
		(Mesh*)"axis_cross_ny.nouv",
		(Mesh*)"axis_cross_nz.nouv",
		(Mesh*)"axis_cross_plane_xy.nouv",
	};
	
	#define X(file_ext,stride, u,c,t) {file_ext,stride},
	Mesh_Format		formats[MESH_FORMAT_COUNT] = {
		MESH_FORMATS
	};
	#undef X
	
	Mesh*			_missing_mesh;
	
	////
	
	DECLM void init () {
		load_file_and_upload_data();
		get_non_enity_meshes();
		_missing_mesh = _get_mesh("missing_mesh.vcol");
		if (!_missing_mesh) {
			warning("placeholder mesh for missing meshes '%' not found!", "missing_mesh.vcol");
		}
	}
	
	DECLM void get_non_enity_meshes () {
		for (auto i=NON_ENTITY_MESHES_FIRST; i<NON_ENTITY_MESHES_COUNT; ++i) {
			non_entity_meshes[i] = get_mesh( lstr::count_cstr((cstr)non_entity_meshes[i]) );
		} 
	}
	
	DECLM bool parse_file (Mem_Block cr data) {
		using namespace meshes_file_n;
		
		//#define require(cond) if (!(cond)) return 0;
		#define require(cond) if (!(cond)) { warning(STRINGIFY(cond)); return 0; }
		
		require(data.size >= sizeof(fHeader));
		auto* header = (fHeader*)data.ptr;
		
		require(header->id.i == FILE_ID.i);
		require(header->file_size == data.size);
		require(header->meshes_count > 0);
		require(header->str_tbl_size > 0);
		
		char* str_tbl = (char*)(header +1);
		
		file_meshes = (Mesh*)(str_tbl +header->str_tbl_size);
		file_mesh_count = header->meshes_count;
		
		for (u32 i=0; i<file_mesh_count; ++i) {
			auto& m = file_meshes[i];
			m.name.str =		ptr_add(m.name.str, str_tbl);
			m.data.vertecies =	ptr_add(m.data.vertecies, header);
			m.data.triangles =	ptr_add(m.data.triangles, header);
		}
		
		auto* fformats = (fMesh_Format*)(file_meshes +file_mesh_count);
		
		for (auto i=MESH_FORMAT_FIRST; i<MESH_FORMAT_COUNT; ++i) {
			auto& f = formats[i];
			
			f.vertex_size = fformats[i].vertex_size;
			f.index_size = fformats[i].index_size;
			
			f.vertex_data = ptr_add(fformats[i].vertex_data, header);
			f.index_data = ptr_add(fformats[i].index_data, header);
			
			#if 0
			print(">>> % % %\n", f.file_ext, f.vertex_size, f.index_size);
			
			for (u32 j=0; j<file_mesh_count; ++j) {
				if (file_meshes[j].format == i) print(">>    % % %\n", file_meshes[j].name,
						file_meshes[j].data.vertex_count, file_meshes[j].data.triangle_count);
			}
			#endif
			
			glBindVertexArray(0); // Unbind VAO so that we don't mess with its GL_ELEMENT_ARRAY_BUFFER binding
			
			glBindBuffer(GL_ARRAY_BUFFER,			f.vbo_vert);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,	f.vbo_indx);
			
			glBufferData(GL_ARRAY_BUFFER,			f.vertex_size, f.vertex_data, GL_STATIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER,	f.index_size, f.index_data, GL_STATIC_DRAW);
			
		}
		
		return true;
		
		#undef require
	}
	
	DECLM void load_file_and_upload_data () {
		Mem_Block data;
		if (platform::read_file_onto_heap(MESHES_FILENAME, &data)) {
			warning("meshes file '%' could not be loaded, no meshes loaded!", MESHES_FILENAME);
			return;
		}
		
		if (!parse_file(data)) {
			warning("meshes file '%' could not be parsed, no meshes loaded!", MESHES_FILENAME);
			return;
		}
		
	}
	
	DECLM Mesh* _get_mesh (lstr cr name) {
		for (u32 i=0; i<file_mesh_count; ++i) {
			if (str::comp(file_meshes[i].name, name)) return &file_meshes[i];
		}
		return nullptr;
	}
	DECLM Mesh* get_mesh (lstr cr name) {
		Mesh* ret = _get_mesh(name);
		if (ret) return ret;
		
		warning("mesh '%' not found.", name);
		return _missing_mesh;
	}
	
	DECLM void bind_vao (Mesh const* m) {
		glBindVertexArray(formats[m->format].vao);
	}
	
	DECLM void gen_buf (Mesh_Format* m) {
		
		glGenBuffers(1, &m->vbo_vert);
		glGenBuffers(1, &m->vbo_indx);
		glGenVertexArrays(1, &m->vao);
	}
	DECLM void setup_vaos () {
		{ auto& f = formats[MF_NOUV];
			gen_buf(&f);
			
			glBindVertexArray(f.vao);
			
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			
			glBindBuffer(GL_ARRAY_BUFFER, f.vbo_vert);
			
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, f.vertex_stride, (void*)offsetof(Vertex_NOUV, pos));
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, f.vertex_stride, (void*)offsetof(Vertex_NOUV, norm));
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, f.vbo_indx);
		}
		{ auto& f = formats[MF_NOUV_VCOL];
			gen_buf(&f);
			
			glBindVertexArray(f.vao);
			
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(4);
			
			glBindBuffer(GL_ARRAY_BUFFER, f.vbo_vert);
			
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, f.vertex_stride, (void*)offsetof(Vertex_NOUV_VCOL, pos));
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, f.vertex_stride, (void*)offsetof(Vertex_NOUV_VCOL, norm));
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, f.vertex_stride, (void*)offsetof(Vertex_NOUV_VCOL, col));
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, f.vbo_indx);
		}
		{ auto& f = formats[MF_VCOL];
			gen_buf(&f);
			
			glBindVertexArray(f.vao);
			
			glBindBuffer(GL_ARRAY_BUFFER, f.vbo_vert);
			
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(4);
			
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, f.vertex_stride, (void*)offsetof(Vertex_VCOL, pos));
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, f.vertex_stride, (void*)offsetof(Vertex_VCOL, norm));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, f.vertex_stride, (void*)offsetof(Vertex_VCOL, uv));
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, f.vertex_stride, (void*)offsetof(Vertex_VCOL, col));
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, f.vbo_indx);
		}
		{ auto& f = formats[MF_COMMON];
			gen_buf(&f);
			
			glBindVertexArray(f.vao);
			
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);
			
			glBindBuffer(GL_ARRAY_BUFFER, f.vbo_vert);
			
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, f.vertex_stride, (void*)offsetof(Vertex_COMMON, pos));
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, f.vertex_stride, (void*)offsetof(Vertex_COMMON, norm));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, f.vertex_stride, (void*)offsetof(Vertex_COMMON, uv));
			glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, f.vertex_stride, (void*)offsetof(Vertex_COMMON, tang));
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, f.vbo_indx);
		}
		
	}
	
};
