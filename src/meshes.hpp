#define MESHES \
	X0(	MSH_nouv_col_LIGHT_BULB ) \
	X(	MSH_nouv_col_SUN_LAMP ) \
	 \
	X(	MSH_nouv_AXIS_CROSS_PX ) \
	X(	MSH_nouv_AXIS_CROSS_NX ) \
	X(	MSH_nouv_AXIS_CROSS_PY ) \
	X(	MSH_nouv_AXIS_CROSS_NY ) \
	X(	MSH_nouv_AXIS_CROSS_PZ ) \
	X(	MSH_nouv_AXIS_CROSS_NZ ) \
	X(	MSH_nouv_AXIS_CROSS_PLANE_XY ) \
	X(	MSH_nouv_SPHERE ) \
	 \
	X(	MSH_nouv_STRUCTURE_RING ) \
	X(	MSH_nouv_TERRAIN ) \
	X(	MSH_nouv_UTAHTEAPOT ) \
	 \
	X(	MSH_nouv_PLANETARIUM ) \
	X(	MSH_nouv_PLANETARIUM_PROJECTOR ) \
	 \
	X(	MSH_nouv_ICO_SPHERE ) \
	X(	MSH_nouv_STFD_BUNNY ) \
	X(	MSH_nouv_STFD_BUDDHA ) \
	X(	MSH_nouv_STFD_DRAGON ) \
	 \
	X(	MSH_nouv_WINDOW_PILLAR ) \
	X(	MSH_nouv_SHADOW_TEST_0 ) \
	 \
	X(	MSH_UNIT_PLANE ) \
	 \
	X(	MSH_SCENE_GROUND0 ) \
	X(	MSH_UGLY ) \
	X(	MSH_TERRAIN_TREE ) \
	X(	MSH_TERRAIN_TREE_CUTS ) \
	X(	MSH_TERRAIN_TREE_BLOSSOMS ) \
	X(	MSH_TERRAIN_CUBE ) \
	X(	MSH_TERRAIN_SPHERE ) \
	X(	MSH_TERRAIN_OBELISK ) \
	X(	MSH_STRUCTURE_WALLS ) \
	X(	MSH_STRUCTURE_GROUND ) \
	X(	MSH_STRUCTURE_BLOCK1 ) \
	X(	MSH_STRUCTURE_BLOCK2 ) \
	X(	MSH_STRUCTURE_BLOCK3 ) \
	X(	MSH_STRUCTURE_BLOCK4 ) \
	X(	MSH_STRUCTURE_BEAM ) \
	X(	MSH_STRUCTURE_BEAM_CUTS ) \
	 \
	X(	MSH_NANOSUIT_TORSO ) \
	X(	MSH_NANOSUIT_LEGS ) \
	X(	MSH_NANOSUIT_NECK ) \
	X(	MSH_NANOSUIT_HELMET ) \
	 \
	X(	MSH_NORM_TEST_00 ) \
	\
	X(	MSH_CERBERUS ) \
	 \
	X(	MSH_PG_DAVID ) \
	X(	MSH_PG_STOOL ) \
	X(	MSH_PG_DAVID3 )

#define X0(name)	name=0,
#define X(name)		name,
enum mesh_id_e: u32 {
	MESHES
	MESHES_COUNT,
};
#undef X0
#undef X
DEFINE_ENUM_ITER_OPS(mesh_id_e, u32)

#define X0(name)	TO_STR(name),
#define X(name)		TO_STR(name),
DECLD constexpr char const* MESH_ID_NAMES[MESHES_COUNT] {
	MESHES
};
#undef X0
#undef X

#undef MESHES

DECLD constexpr mesh_id_e		MSH_NOUV_COL_FIRST =			MSH_nouv_col_LIGHT_BULB;
DECLD constexpr mesh_id_e		MSH_NOUV_COL_END =				(mesh_id_e)(MSH_nouv_col_SUN_LAMP +1);
DECLD constexpr u32				MSH_NOUV_COL_COUNT =			MSH_NOUV_COL_END -MSH_NOUV_COL_FIRST;

DECLD constexpr mesh_id_e		MSH_NOUV_FIRST =				MSH_nouv_AXIS_CROSS_PX;
DECLD constexpr mesh_id_e		MSH_NOUV_END =					(mesh_id_e)(MSH_nouv_SHADOW_TEST_0 +1);
DECLD constexpr u32				MSH_NOUV_COUNT =				MSH_NOUV_END -MSH_NOUV_FIRST;

DECLD constexpr mesh_id_e		MSH_COMMON_FIRST =				MSH_UNIT_PLANE;
DECLD constexpr mesh_id_e		MSH_COMMON_END =				(mesh_id_e)(MSH_PG_DAVID3 +1);
DECLD constexpr u32				MSH_COMMON_COUNT =				MSH_COMMON_END -MSH_COMMON_FIRST;

DECLD char const*				mesh_names[MESHES_COUNT];

#include "meshes_file.hpp"

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
