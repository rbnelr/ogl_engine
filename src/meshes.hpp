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
		
		Header*		header;
		uptr		file_size;
		
		DECLM void reload () {
			
			if (header) {
				free(header);
			}
			
			//stk->alignTop<FILE_ALIGN>();
			// Ignore FILE_ALIGN, not sure what to do about alignment for windows heap
			
			{
				Mem_Block file_data;
				assert(!platform::read_whole_file_onto_heap(MESHES_FILENAME, 0, &file_data));
				header = reinterpret_cast<Header*>(file_data.ptr);
				file_size = file_data.size;
			}
			
			assert(file_size >= sizeof(Header));
			assert(cmemcmp(header->magicString, magicString, sizeof(header->magicString)));
			
			assert(is_aligned(header->file_size, FILE_ALIGN));
		}
		
		DECLM Header_Mesh_Entry* query_mesh (const char* name) {
			
			if (!name) {
				return nullptr;
			}
			
			char* cur = (char*)ptr_add(header, sizeof(Header));
			char* end = (char*)ptr_add(header, file_size);
			
			for (u32 entry=0; entry<header->meshEntryCount; ++entry) {
				
				cur = align_up(cur, HEADER_MESH_ENTRY_ALIGN);
				
				auto ret = (Header_Mesh_Entry*)cur;
				
				cur += sizeof(Header_Mesh_Entry);
				
				assert(cur < end);
				
				bool name_matched = true;
				
				auto desiredName = name;
				do {
					assert(cur != end);
					if (name_matched && *cur != *desiredName++) {
						name_matched = false;
					}
				} while (*cur++ != '\0');
				
				if (name_matched) {
					return ret;
				}
			}
			return nullptr;
		}
		DECLM void reload_meshes (format_e format, mesh_id_e first_mesh, u32 mesh_count, Mesh_Ref* meshes) {
			
			uptr sizePerVertex = 0;
			uptr sizePerIndex;
			{
				{
					assert(format & INTERLEAVED);
					
					assert(format & POS_XYZ);
					sizePerVertex += 3;
					
					assert(format & NORM_XYZ);
					sizePerVertex += 3;
					
					if (format & TANG_XYZW) {
						sizePerVertex += 4;
					}
					
					if (format & UV_UV) {
						sizePerVertex += 2;
					}
					
					if (format & COL_RGB) {
						sizePerVertex += 3;
					}
					
					sizePerVertex *= sizeof(f32);
				}
				{
					assert((format & INDEX_MASK) ==			INDEX_USHORT);
					sizePerIndex = sizeof(GLushort);
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
				
				assert(is_aligned(mesh->dataOffset, DATA_ALIGN));
				
				if (mesh->dataFormat != format) {
					auto actual = format_strs(mesh->dataFormat);
					auto desired = format_strs(format);
					warning("Wrong format of mesh % got %%%% (%) but wanted %%%% (%), not loaded!\n",
							mesh_names[i],
							actual.f, actual.tang, actual.uv, actual.col,		(u32)mesh->dataFormat,
							desired.f, desired.tang, desired.uv, desired.col,	(u32)format);
					continue;
				}
				
				meshes[i].indx_offsets =	(GLvoid*)(indexCounter * sizePerIndex);
				meshes[i].indx_count =		safe_cast_assert(GLsizei, mesh->indexCount);
				meshes[i].base_vertecies =	safe_cast_assert(GLint, vertexCounter);
				
				GLsizei vertexCount = mesh->vertexCount.glushort;
				
				vertexCounter +=	vertexCount;
				indexCounter +=		mesh->indexCount;
				
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
				
				GLsizei		vertexCount = mesh->vertexCount.glushort;
				
				u64			vertSize = vertexCount * sizePerVertex;
				u64			indexSize = mesh->indexCount * sizePerIndex;
				
				GLsizeiptr	gl_vertSize = safe_cast_assert(GLsizeiptr, vertSize);
				GLsizeiptr	gl_indexSize = safe_cast_assert(GLsizeiptr, indexSize);
				
				assert((mesh->dataOffset +vertSize) <= file_size);
				assert((mesh->dataOffset +vertSize +indexSize) <= file_size);
				
				auto vert_data = align_up(file_data +mesh->dataOffset, sizeof(GLfloat));
				auto indx_data = align_up(vert_data +vertSize, sizeof(GLushort));
				
				glBufferSubData(GL_ARRAY_BUFFER,
						safe_cast_assert(GLintptr, vertexCounter * sizePerVertex),
						gl_vertSize, vert_data);
				
				glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 
						safe_cast_assert(GLintptr, indexCounter * sizePerIndex),
						gl_indexSize, indx_data);
				
				vertexCounter +=	vertexCount;
				indexCounter +=		mesh->indexCount;
			}
		}
		
	};
}
