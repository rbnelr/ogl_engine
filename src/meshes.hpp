
#include "meshes_file.hpp"

#define MESH_INSTANCE missing_mesh
#include "missing_mesh.hpp"
#undef MESH_INSTANCE

struct Meshes {
	
	dynarr<Mesh>	loaded_meshes;
	
	DECLM bool parse_file (Mem_Block cr data) {
		using namespace meshes_file_n;
		
		//#define require(cond) if (!(cond)) return 0;
		#define require(cond) if (!(cond)) { warning(STRINGIFY(cond)); return 0; }
		
		require(data.size >= sizeof(fHeader));
		auto* header = (fHeader*)data.ptr;
		
		require(header->id.i == FILE_ID.i);
		require(header->file_size == data.size);
		
		
		
		return true;
		
		#undef require
	}
	
	DECLM void load_file () {
		Mem_Block data;
		if (platform::read_file_onto_heap(MESHES_FILENAME, &data)) {
			warning("meshes file '%' could not be loaded, no meshes loaded!", MESHES_FILENAME);
			return;
		}
		
		loaded_meshes.alloc(64);
		
		if (!parse_file(data)) {
			warning("meshes file '%' could not be parsed, no meshes loaded!", MESHES_FILENAME);
			return;
		}
		
	}
	
	DECLM Mesh* get_mesh (lstr cr name) {
		return &missing_mesh;
	}
	
	//// VBO
	enum vbo_indx_e : u32 {
		NOUV_VCOL_ARR_BUF=0,
		NOUV_VCOL_INDX_BUF,
		NOUV_ARR_BUF,
		NOUV_INDX_BUF,
		VCOL_ARR_BUF,
		VCOL_INDX_BUF,
		COMMON_ARR_BUF,
		COMMON_INDX_BUF,
		
		VBO_COUNT
	};

	//// VAO
	enum vao_indx_e : u32 {
		NOUV_VCOL_VAO=0,
		NOUV_VAO,
		VCOL_VAO,
		COMMON_VAO,
		
		VAO_COUNT
	};
	
	GLuint				VBOs[VBO_COUNT];
	GLuint				VAOs[VAO_COUNT];
	
	// Non-entity meshes
	enum global_mesh_e : u32 {
		AXIS_CROSS_POS_X=0,	AXIS_CROSS_POS_Y,	AXIS_CROSS_POS_Z,
		AXIS_CROSS_NEG_X,	AXIS_CROSS_NEG_Y,	AXIS_CROSS_NEG_Z,
		AXIS_CROSS_PLANE,
		
		LIGHTBULB_SUN_LAMP,	LIGHTBULB_LIGHTBULB,
		
		MESH_COUNT,
	};
	Mesh* global_meshes[MESH_COUNT] = {
		(Mesh*)"axis_cross_px.nouv",	(Mesh*)"axis_cross_py.nouv",	(Mesh*)"axis_cross_pz.nouv",
		(Mesh*)"axis_cross_nx.nouv",	(Mesh*)"axis_cross_ny.nouv",	(Mesh*)"axis_cross_nz.nouv",
		(Mesh*)"axis_cross_plane_xy.nouv",
		
		(Mesh*)"sun_lamp.nouv_vcol",	(Mesh*)"light_bulb.nouv_vcol",
	};
	
	DECLM void init () {
		
		load_file();
		
		for (auto i=(global_mesh_e)0; i<MESH_COUNT; i=(global_mesh_e)(i+1)) {
			global_meshes[i] = get_mesh( lstr::count_cstr( (cstr)global_meshes[i] ) );
		}
		
		{
			using namespace meshes_file_n;
			assert(missing_mesh.format == F_INDX_U16|F_NORM_XYZ|F_UV_UV|F_COL_RGB);
			
			glBindVertexArray(0); // Unbind VAO so that we don't mess with its GL_ELEMENT_ARRAY_BUFFER binding
			
			glBindBuffer(GL_ARRAY_BUFFER,			VBOs[VCOL_ARR_BUF]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,	VBOs[VCOL_INDX_BUF]);
			
			missing_mesh.vbo_data.indx_count =	safe_cast_assert(GLsizei, missing_mesh.data.triangle_count*3);
			missing_mesh.vbo_data.indx_offset =	(GLvoid*)0;
			missing_mesh.vbo_data.base_vertex =	safe_cast_assert(GLint, 0);
			
			glBufferData(GL_ARRAY_BUFFER,
					safe_cast_assert(GLsizeiptr, missing_mesh.data.vertex_count * sizeof(Vertex_PNUC)),
					missing_mesh.data.vertecies, GL_STATIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER,
					safe_cast_assert(GLsizeiptr, missing_mesh.data.triangle_count * sizeof(Triangle_Indx16)),
					missing_mesh.data.triangles, GL_STATIC_DRAW);
			
		}
	}
	
	DECLM void setup_vaos () {
		glGenBuffers(VBO_COUNT, VBOs);
		glGenVertexArrays(VAO_COUNT, VAOs);
		
		{ // Default mesh buffer
			glBindVertexArray(VAOs[NOUV_VAO]);
			
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			
			glBindBuffer(GL_ARRAY_BUFFER, VBOs[NOUV_ARR_BUF]);
			
			auto vert_size = safe_cast_assert(GLsizei, (3 +3) * sizeof(f32));
			
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)((0) * sizeof(f32)));				// pos xyz
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)((3 +0) * sizeof(f32)));			// normal xyz
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOs[NOUV_INDX_BUF]);
		}
		{ // 
			glBindVertexArray(VAOs[NOUV_VCOL_VAO]);
			
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(4);
			
			glBindBuffer(GL_ARRAY_BUFFER, VBOs[NOUV_VCOL_ARR_BUF]);
			
			auto vert_size = safe_cast_assert(GLsizei, (3 +3 +3) * sizeof(f32));
			
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)((0) * sizeof(f32)));				// pos xyz
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)((3 +0) * sizeof(f32)));			// normal xyz
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)((3 +3 +0) * sizeof(f32)));		// color rgb
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOs[NOUV_VCOL_INDX_BUF]);
		}
		{ // 
			glBindVertexArray(VAOs[VCOL_VAO]);
			
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(4);
			
			glBindBuffer(GL_ARRAY_BUFFER, VBOs[VCOL_ARR_BUF]);
			
			auto vert_size = safe_cast_assert(GLsizei, (3 +3 +2 +3) * sizeof(f32));
			
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)((0) * sizeof(f32)));				// pos xyz
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)((3 +0) * sizeof(f32)));			// normal xyz
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vert_size, (void*)((3 +3 +0) * sizeof(f32)));		// uv uv
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)((3 +3 +2 +0) * sizeof(f32)));	// color rgb
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOs[VCOL_INDX_BUF]);
		}
		{ // UV TANGENT mesh buffer
			glBindVertexArray(VAOs[COMMON_VAO]);
			
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);
			
			glBindBuffer(GL_ARRAY_BUFFER, VBOs[COMMON_ARR_BUF]);
			
			auto vert_size = safe_cast_assert(GLsizei, (3 +3 +4 +2) * sizeof(f32));
			
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)((0) * sizeof(f32)));				// pos xyz
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vert_size, (void*)((3 +0) * sizeof(f32)));			// normal xyz
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vert_size, (void*)((3 +3 +0) * sizeof(f32)));		// uv
			glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, vert_size, (void*)((3 +3 +2 +0) * sizeof(f32)));	// tangent xyzw
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOs[COMMON_INDX_BUF]);
		}
		
	}
	
	DECLM void bind_vao (Mesh const* m) {
		assert(m == &missing_mesh);
		
		glBindVertexArray(VAOs[VCOL_VAO]);
	}
	
	#if 0
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
			
			vertex_count = mesh->vertex_count;
			assert(vertex_count > 0);
			
			index_count = mesh->index_count;
			assert(index_count > 0);
			
			switch (mesh->format) {
				using namespace meshes_file_n;
				
				case (F_INDX_U16| F_NORM_XYZ): {
					struct Vertex {
						v3	pos;
						v3	norm;
					};
					vertex_stride = sizeof(Vertex);
				} break;
				
				case (F_INDX_U16| F_NORM_XYZ|F_COL_RGB): {
					struct Vertex {
						v3	pos;
						v3	norm;
						v3	color;
					};
					vertex_stride = sizeof(Vertex);
				} break;
				
				case (F_INDX_U16| F_NORM_XYZ|F_UV_UV): {
					struct Vertex {
						v3	pos;
						v3	norm;
						v2	uv;
					};
					vertex_stride = sizeof(Vertex);
				} break;
				
				case (F_INDX_U16| F_NORM_XYZ|F_UV_UV|F_TANG_XYZW): {
					struct Vertex {
						v3	pos;
						v3	norm;
						v2	uv;
						v4	tang;
					};
					vertex_stride = sizeof(Vertex);
				} break; 
				
				default: assert(false); vertex_stride = 0; // shut up compiler
			}
			
			u32 vert_size = vertex_count * vertex_stride;
			u32 index_size = index_count * sizeof(GLushort);
			
			assert((mesh->data_offs +vert_size) <= meshes_file.file_size);
			assert((mesh->data_offs +vert_size +index_size) <= meshes_file.file_size);
			
			vertices = (byte*)(file_data +mesh->data_offs);
			indices = (GLushort*)((byte*)vertices +vert_size);
			
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
		glBindBuffer(GL_ARRAY_BUFFER,			VBOs[NO_UV_COL_ARR_BUF]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,	VBOs[NO_UV_COL_INDX_BUF]);
		
		meshes_file.reload_meshes(F_INDX_U16|F_NORM_XYZ|F_COL_RGB, MSH_NOUV_COL_FIRST, MSH_NOUV_COL_COUNT, meshes);
		
		//
		glBindBuffer(GL_ARRAY_BUFFER,			VBOs[NO_UV_ARR_BUF]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,	VBOs[NO_UV_INDX_BUF]);
		
		meshes_file.reload_meshes(F_INDX_U16|F_NORM_XYZ, MSH_NOUV_FIRST, MSH_NOUV_COUNT, meshes);
		
		//
		glBindBuffer(GL_ARRAY_BUFFER,			VBOs[COMMON_ARR_BUF]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,	VBOs[COMMON_INDX_BUF]);
		
		meshes_file.reload_meshes(F_INDX_U16|F_NORM_XYZ|F_UV_UV|F_TANG_XYZW, MSH_COMMON_FIRST, MSH_COMMON_COUNT, meshes);
		
		for (mesh_id_e id=(mesh_id_e)0; id<MESHES_COUNT; ++id) {
			meshes_aabb[id] = calc_AABB_model(id);
		}
	}
	#endif
	
};

#if 0
struct Mesh_Ref {
	GLvoid*				indx_offsets;
	GLsizei				indx_count;
	GLint				base_vertecies;
};

namespace meshes_file_n {
	struct Meshes_File {
		
		fHeader*	header;
		char*		str_tbl;
		fMesh*		meshes;
		uptr		file_size;
		
		DECLM void reload () {
			
			free(header);
			
			//stk->alignTop<FILE_ALIGN>();
			// Ignore FILE_ALIGN, not sure what to do about alignment for windows heap
			
			{
				Mem_Block file_data;
				assert(!platform::read_file_onto_heap(MESHES_FILENAME, &file_data));
				header = (fHeader*)file_data.ptr;
				file_size = file_data.size;
			}
			
			assert(file_size >= sizeof(fHeader));
			assert(header->id.i == FILE_ID.i);
			
			str_tbl = (char*)(header +1);
			
			meshes = (fMesh*)(str_tbl +header->str_tbl_size);
		}
		
		DECLM fMesh* query_mesh (const char* name) {
			
			if (!name) return nullptr;
			
			for (u32 i=0; i<header->meshes_count; ++i) {
				if (str::comp(str_tbl +meshes[i].mesh_name, name)) return &meshes[i];
			}
			return nullptr;
		}
		DECLM void reload_meshes (format_e format, mesh_id_e first_mesh, u32 mesh_count, Mesh_Ref* meshes) {
			
			uptr sizePerVertex = 0;
			uptr sizePerIndex;
			{
				{
					sizePerVertex += 3;
					
					assert(format & F_NORM_XYZ);
					sizePerVertex += 3;
					
					if (format & F_UV_UV) {
						sizePerVertex += 2;
					}
					
					if (format & F_TANG_XYZW) {
						sizePerVertex += 4;
					}
					
					if (format & F_COL_RGB) {
						sizePerVertex += 3;
					}
					
					sizePerVertex *= sizeof(f32);
				}
				{
					assert((format & F_INDX_MASK) == F_INDX_U16);
					sizePerIndex = sizeof(u16);
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
				
				if (mesh->format != format) {
					warning("Wrong format of mesh % got (%) but wanted (%), not loaded!\n",
							mesh_names[i],
							(u32)mesh->format,
							(u32)format);
					continue;
				}
				
				meshes[i].indx_offsets =	(GLvoid*)(indexCounter * sizePerIndex);
				meshes[i].indx_count =		safe_cast_assert(GLsizei, mesh->index_count);
				meshes[i].base_vertecies =	safe_cast_assert(GLint, vertexCounter);
				
				vertexCounter +=	safe_cast_assert(GLsizei, mesh->vertex_count);
				indexCounter +=		mesh->index_count;
				
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
				
				GLsizei		vertexCount = safe_cast_assert(GLsizei, mesh->vertex_count);
				
				u64			vertSize = vertexCount * sizePerVertex;
				u64			indexSize = mesh->index_count * sizePerIndex;
				
				GLsizeiptr	gl_vertSize = safe_cast_assert(GLsizeiptr, vertSize);
				GLsizeiptr	gl_indexSize = safe_cast_assert(GLsizeiptr, indexSize);
				
				assert((mesh->data_offs +vertSize) <= file_size);
				assert((mesh->data_offs +vertSize +indexSize) <= file_size);
				
				auto vert_data = file_data +mesh->data_offs;
				auto indx_data = vert_data +vertSize;
				
				glBufferSubData(GL_ARRAY_BUFFER,
						safe_cast_assert(GLintptr, vertexCounter * sizePerVertex),
						gl_vertSize, vert_data);
				
				glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 
						safe_cast_assert(GLintptr, indexCounter * sizePerIndex),
						gl_indexSize, indx_data);
				
				vertexCounter +=	vertexCount;
				indexCounter +=		mesh->index_count;
			}
		}
		
	};
}
#endif
