#pragma once

#pragma pack (push, 1)

namespace meshes_file_n {
	
	enum format_e : u16 {
						INTERLEAVED=		0b1 << 0,
						
						POS_XYZ=			0b1 << 1,
						
						NORM_XYZ=			0b1 << 2,
						
						TANG_XYZW=			0b1 << 3,
						
						UV_UV=				0b1 << 4,
						
						COL_RGB=			0b1 << 5,
						
						//INDEX_NONE=			0b00 << 6,
						//INDEX_UBYTE=		0b01 << 6,
						INDEX_USHORT=		0b10 << 6,
						//INDEX_UINT=			0b11 << 6,
	};
	DEFINE_ENUM_FLAG_OPS(format_e, u16);
	
	DECLD constexpr u16	INDEX_MASK=			0b11 << 6;
	
	struct Format_Print {
		char const*	f;
		char const*	tang;
		char const*	uv;
		char const*	col;
	};
	
	DECL Format_Print format_strs (format_e f) {
		Format_Print ret;
		ret.f = "USUAL_FORMAT";
		
		switch (f & TANG_XYZW) {
			case 0:			ret.tang = "";			break;
			case TANG_XYZW:	ret.tang = "|TANG_XYZW";	break;
			default: assert(false);
		}
		
		switch (f & UV_UV) {
			case 0:			ret.uv = "";		break;
			case UV_UV:		ret.uv = "|UV_UV";	break;
			default: assert(false);
		}
		
		switch (f & COL_RGB) {
			case 0:			ret.col = "";			break;
			case COL_RGB:	ret.col = "|COL_RGB";	break;
			default: assert(false);
		}
		return ret;
	}
	
	DECLD constexpr uptr FILE_ALIGN =					64;
	DECLD constexpr uptr HEADER_MESH_ENTRY_ALIGN =	16;
	DECLD constexpr uptr DATA_ALIGN =					64;
	
	// values in the mesh data are also aligned to their type alignment f32 to 4 etc.
	
	static_assert(is_aligned(FILE_ALIGN, HEADER_MESH_ENTRY_ALIGN), "");
	static_assert(is_aligned(FILE_ALIGN, DATA_ALIGN), "");
	
	struct Header_Mesh_Entry {
		u64			dataOffset; // From start of file, aligned to DATA_ALIGN
		GLsizei		indexCount; // 0 for NOT_INDEXED
		union {
			GLsizei		glsizei;
			GLubyte		glubyte;
			GLushort	glushort;
			GLuint		gluint;
		} vertexCount;
		
		format_e	dataFormat;
		
		//char		meshNameCstr[/* Null terminated */];
	};
	
	DECLD constexpr char magicString[8] = {'R','A','Z','_','M','S','H','S'};
	
	struct Header {
		char		magicString[8];
		u64			file_size;
		
		u64			meshEntryCount;
		
		u64			totalVertexSize;
		u64			totalIndexSize;
	};
}

#pragma pack (pop)
