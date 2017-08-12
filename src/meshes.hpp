#define MESHES \
	X0(	nouv_MSH_AXIS_CROSS_PX ) \
	X(	nouv_MSH_AXIS_CROSS_NX ) \
	X(	nouv_MSH_AXIS_CROSS_PY ) \
	X(	nouv_MSH_AXIS_CROSS_NY ) \
	X(	nouv_MSH_AXIS_CROSS_PZ ) \
	X(	nouv_MSH_AXIS_CROSS_NZ ) \
	X(	nouv_MSH_AXIS_CROSS_PLANE_XY ) \
	X(	nouv_MSH_3D_LINE_COMPONENT ) \
	X(	nouv_MSH_SPHERE ) \
	 \
	X(	nouv_MSH_LIGHT_BULB ) \
	X(	nouv_MSH_SUN_LAMP_MSH ) \
	 \
	X(	nouv_MSH_STRUCTURE_RING ) \
	X(	nouv_MSH_TERRAIN ) \
	X(	nouv_MSH_UTAHTEAPOT ) \
	 \
	X(	nouv_MSH_PLANETARIUM ) \
	X(	nouv_MSH_PLANETARIUM_PROJECTOR ) \
	 \
	X(	nouv_MSH_ICO_SPHERE ) \
	X(	nouv_MSH_STFD_BUNNY ) \
	X(	nouv_MSH_STFD_BUDDHA ) \
	X(	nouv_MSH_STFD_DRAGON ) \
	 \
	X(	nouv_MSH_WINDOW_PILLAR ) \
	X(	nouv_MSH_SHADOW_TEST_0 ) \
	 \
	X(	uv_MSH_UNIT_PLANE ) \
	 \
	X(	uv_SCENE_GROUND0 ) \
	X(	uv_MSH_UGLY ) \
	X(	uv_MSH_TERRAIN_TREE ) \
	X(	uv_MSH_TERRAIN_TREE_CUTS ) \
	X(	uv_MSH_TERRAIN_TREE_BLOSSOMS ) \
	X(	uv_MSH_TERRAIN_CUBE ) \
	X(	uv_MSH_TERRAIN_SPHERE ) \
	X(	uv_MSH_TERRAIN_OBELISK ) \
	X(	uv_MSH_STRUCTURE_WALLS ) \
	X(	uv_MSH_STRUCTURE_GROUND ) \
	X(	uv_MSH_STRUCTURE_BLOCK1 ) \
	X(	uv_MSH_STRUCTURE_BLOCK2 ) \
	X(	uv_MSH_STRUCTURE_BLOCK3 ) \
	X(	uv_MSH_STRUCTURE_BLOCK4 ) \
	X(	uv_MSH_STRUCTURE_BEAM ) \
	X(	uv_MSH_STRUCTURE_BEAM_CUTS ) \
	X(	uv_MSH_PLANETARIUM_STAND ) \
	 \
	X(	uv_tang_MSH_UNIT_PLANE ) \
	 \
	X(	uv_tang_NANOSUIT_TORSO ) \
	X(	uv_tang_NANOSUIT_LEGS ) \
	X(	uv_tang_NANOSUIT_NECK ) \
	X(	uv_tang_NANOSUIT_HELMET ) \
	 \
	X(	uv_tang_NORM_TEST_00 ) \
	\
	X(	uv_tang_CERBERUS )

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

DECLD constexpr mesh_id_e		NOUV_MSH_FIRST =				nouv_MSH_AXIS_CROSS_PX;
DECLD constexpr mesh_id_e		NOUV_MSH_END =					(mesh_id_e)(nouv_MSH_SHADOW_TEST_0 +1);
DECLD constexpr u32				NOUV_MSH_COUNT =				NOUV_MSH_END -NOUV_MSH_FIRST;

DECLD constexpr mesh_id_e		UV_MSH_FIRST =					uv_MSH_UNIT_PLANE;
DECLD constexpr mesh_id_e		UV_MSH_END =					(mesh_id_e)(uv_MSH_PLANETARIUM_STAND +1);
DECLD constexpr u32				UV_MSH_COUNT =					UV_MSH_END -UV_MSH_FIRST;

DECLD constexpr mesh_id_e		UV_TANG_MSH_FIRST =				uv_tang_MSH_UNIT_PLANE;
DECLD constexpr mesh_id_e		UV_TANG_MSH_END =				(mesh_id_e)(uv_tang_CERBERUS +1);
DECLD constexpr u32				UV_TANG_MSH_COUNT =				UV_TANG_MSH_END -UV_TANG_MSH_FIRST;

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
		DECLM void reload_meshes (format_t format, mesh_id_e first_mesh, u32 mesh_count, Mesh_Ref* meshes) {
			
			uptr sizePerVertex = 0;
			uptr sizePerIndex;
			{
				{
					assert((format & INTERLEAVED_MASK) ==	INTERLEAVED);
					
					assert((format & POS_MASK) ==			POS_XYZ);
					sizePerVertex += 3;
					
					assert((format & NORM_MASK) ==			NORM_XYZ);
					sizePerVertex += 3;
					
					
					switch (format & TANG_MASK) {
						case TANG_NONE:	sizePerVertex += 0;	break;
						case TANG_XYZW:	sizePerVertex += 4;	break;
						default: assert(false);
					}
					
					switch (format & UV_MASK) {
						case UV_NONE:	sizePerVertex += 0;	break;
						case UV_UV:		sizePerVertex += 2;	break;
						default: assert(false);
					}
					
					assert((format & COL_MASK) ==			COL_NONE);
					
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
							actual.f, actual.tang, actual.uv, actual.col,		(format_t)mesh->dataFormat,
							desired.f, desired.tang, desired.uv, desired.col,	(format_t)format);
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

DECL std140_Transforms _set_transforms (hm mp model_to_cam, hm mp cam_to_model) {
	std140_Transforms temp;
	
	temp.model_to_cam.set(model_to_cam);
	temp.normal_model_to_cam.set(transpose(cam_to_model.m3())); // cancel out scaling and translation -> only keep rotaton
	
	hm test = cam_to_model * model_to_cam;
	
	return temp;
}
DECL void set_transforms (hm mp model_to_cam, hm mp cam_to_model) {
	std140_Transforms temp = _set_transforms(model_to_cam, cam_to_model);
	
	GLOBAL_UBO_WRITE(transforms, &temp);
}
DECL void set_transforms_and_mat (hm mp model_to_cam, hm mp cam_to_model, std140_Material const* mat) {
	
	std140_Transforms_Material temp;
	
	temp.transforms = _set_transforms(model_to_cam, cam_to_model);
	
	temp.mat = *mat;
	
	GLOBAL_UBO_WRITE(transforms, &temp);
}
DECL void set_transforms_and_solid_color (hm mp model_to_cam, hm mp cam_to_model, v3 vp col) {
	
	std140_Material mat;
	mat.albedo.set(col);
	mat.metallic.set(0);
	mat.roughness.set(0);
	mat.roughness.set(0);
	
	set_transforms_and_mat(model_to_cam, cam_to_model, &mat);
}
DECL void set_mat (std140_Material const* mat) {
	
	GLOBAL_UBO_WRITE(mat, mat);
}
