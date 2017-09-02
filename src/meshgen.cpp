
#define WORKING_STK_CAP mebi<uptr>(128)
#include "startup.hpp"

#include "gl/gl.h"
#include "gl/glext.h"

#include "platform_utility.hpp"

//
DECLD lstr input_foldername =	"assets_src/meshes/";
#define SRC_DIR					"src/"

DECL void print_usage () {
	print(	"Usage: meshgen [<input_foldername>]\n"
			"default for <input_foldername> is '%'\n", input_foldername);
}

DECLD Filenames src_files;

#include "math.hpp"
using namespace math;

//
#include "meshes_file.hpp"
using namespace meshes_file_n;

struct Format {
	lstr							file_ext;
	u32								vertex_stride;
	Mesh_Format_Components			components;
	
	dynarr<byte, u64>				vertex_data;
	dynarr<Triangle_Indx16, u64>	triangle_data;
	
	u64								vert_file_offs;
	u64								indx_file_offs;
};

#define X(file_ext,stride, u,c,t) {file_ext,stride, {u,c,t}},

DECLD Format formats[MESH_FORMAT_COUNT] = {
	MESH_FORMATS
};

#undef X

DECL byte* parse_ply_header (Mem_Block cr data, mesh_format_e format,
		uptr* vert_count, uptr* face_count, ui* indx_size) {
	using namespace parse_n;
	#define req(call)	if (!(cur = (call)))	return 0;
	#define reqn(call)	if ((call) != 0)		return 0;
	#define opt(call)	if ((res = (call)))		cur = res;
	
	auto token = [] (char* cur, cstr str) -> char* {
		char* res;
		opt( whitespace(cur) );
		
		for (;;) {
			if (*str == '\0')	return cur;
			if (*str != *cur)	return nullptr;
			++str; ++cur;
		}
	};
	auto comments = [&] (char* cur) -> char* {
		char* ret = nullptr;
		char* res;
		
		for (;;) {
			opt( token(cur, "comment") );
			if (!res) break;
			
			for (;;) {
				opt( newline(cur) );
				if (res) break;
				// Ignore comment characters
				++cur;
			}
			
			ret = cur;
		}
		return ret;
	};
	auto parse_uint = [] (char* cur, uptr* out) -> char* {
		using namespace parse_n;
		
		char* res;
		opt( whitespace(cur) );
		
		if (!is_dec_digit_c(*cur)) return nullptr;
		
		uptr i = 0;
		
		do {
			
			uptr temp = (i * 10) +(*cur -'0');
			if (temp < i) {
				//assert(false); // overflow
				return nullptr;
			}
			i = temp;
			
			++cur;
		} while (is_dec_digit_c(*cur));
		
		*out = i;
		return cur;
	};
	
	char* cur = (char*)data.ptr;
	char* res;
	
	{
		req( token(cur, "ply") );
		req( newline(cur) );
	}
	opt( comments(cur) );
	
	{
		req( token(cur, "format") );
		
		reqn(token(cur, "ascii") );
		reqn(token(cur, "binary_big_endian") );
		req( token(cur, "binary_little_endian") );
		
		req( token(cur, "1.0") );
		req( newline(cur) );
	}
	opt( comments(cur) );
	{
		req( token(cur, "element") );
		req( token(cur, "vertex") );
		
		req( parse_uint(cur, vert_count) );
		
		req( newline(cur) );
	}
	
	auto prop_vert = [&] (char* cur, cstr type, cstr component) -> char* {
		char* res;
		opt( comments(cur) );
	
		req( token(cur, "property") );
		req( token(cur, type) );
		req( token(cur, component) );
		req( newline(cur) );
		
		return cur;
	};
	{
		auto& c = formats[format].components;
		
		req(		prop_vert(cur, "float", "x") );
		req(		prop_vert(cur, "float", "y") );
		req(		prop_vert(cur, "float", "z") );
		
		req(	prop_vert(cur, "float", "nx") );
		req(	prop_vert(cur, "float", "ny") );
		req(	prop_vert(cur, "float", "nz") );
		
		if (c.uv) {
			req(	prop_vert(cur, "float", "s") );
			req(	prop_vert(cur, "float", "t") );
		}
		
		if (c.col) {
			req(	prop_vert(cur, "uchar", "red") );
			req(	prop_vert(cur, "uchar", "green") );
			req(	prop_vert(cur, "uchar", "blue") );
		}
	}
	opt( comments(cur) );
	{
		req( token(cur, "element") );
		req( token(cur, "face") );
		
		req( parse_uint(cur, face_count) );
		
		req( newline(cur) );
	}
	opt( comments(cur) );
	{
		req( token(cur, "property") );
		req( token(cur, "list") );
		req( token(cur, "uchar") );
		
		if (		res = token(cur, "uint8") )		*indx_size =	1;
		else if (	res = token(cur, "uint16") )	*indx_size =	2;
		else if (	res = token(cur, "uint32") )	*indx_size =	4;
		else return 0;
		cur = res;
		
		req( token(cur, "vertex_indices") );
		
		req( newline(cur) );
	}
	opt( comments(cur) );
	{
		req( token(cur, "end_header") );
		req( newline(cur) );
	}
	
	return (byte*)cur;
	
	#undef opt
	#undef req
}

DECL bool mesh_type (lstr cr filename, lstr* out_name, mesh_format_e* out_format) {
	lstr fn = filename;
	
	lstr real_ext={0,0}, ext={0,0};
	for (u32 i=0; i<fn.len; ++i) {
		if (fn[i] == '.') {
			ext = {real_ext.str, (u32)ptr_sub(real_ext.str, &fn[i])};
			real_ext.str = &fn[i +1];
		}
	}
	real_ext.len = (u32)ptr_sub(real_ext.str, &fn[fn.len]);
	
	assert(real_ext && str::comp(real_ext, "ply"));
	
	if (!ext) {
		*out_format = MF_COMMON;
	} else {
		bool found = false;
		for (auto i=MESH_FORMAT_FIRST; i<MESH_FORMAT_COUNT; ++i) {
			if (str::comp(formats[i].file_ext, ext)) {
				*out_format = i;
				*out_name = print_working_stk("%\\0", lstr(filename.str, filename.len -real_ext.len -1));
				found = true;
				break;
			}
		}
		if (!found) {
			warning("unknown mesh type '%'", ext);
			return false;
		}
	}
	
	*out_name = print_working_stk("%\\0", lstr(filename.str, filename.len -real_ext.len -1));
	return true;
}

#if 0
assert(fmesh->vertex_count > 0);
if (!safe_cast(u16, fmesh->vertex_count -1)) {
	warning("Too many vertices to use u16 vertex indexing, u32 indices currently not supported!");
	return false;
}


#endif
DECL s32 app_main () {
	
	if (cmd_line.argc == 1) {
		// use default input_foldername
	} else if (cmd_line.argc == 2) {
		input_foldername = lstr::count_cstr(cmd_line.argv[1]);
	} else {
		print_usage();
	}
	
	{
		using namespace list_of_files_in_n;
		lstr filt[1] = { "ply" };
		src_files = list_of_files_in(input_foldername,
				FILES|RECURSIVE|FILTER_FILES|NO_BASE_PATH|FILTER_INCL, 1,filt);
	}
	
	fHeader			header = { FILE_ID };
	auto			f_str_tbl = dynarr<char>::alloc(512);
	auto			f_meshes = dynarr<Mesh>::alloc(64);
	fMesh_Format	fformats[MESH_FORMAT_COUNT];
	
	for (auto i=MESH_FORMAT_FIRST; i<MESH_FORMAT_COUNT; ++i) {
		formats[i].vertex_data.alloc(mebi(32));
		formats[i].triangle_data.alloc(mebi(32) / sizeof(Triangle_Indx16));
	}
	
	for (u32 i=0; i<src_files.arr.len; ++i) {
		Mesh mesh;
		
		lstr filename = src_files.get_filename(i);
		print("% %:\n", i, filename);
		
		lstr name;
		if (!mesh_type(filename, &name, &mesh.format)) continue;
		
		auto& format = formats[mesh.format];
		
		DEFER_POP(&working_stk);
		lstr pathname = print_working_stk("%%\\0", input_foldername, filename);
		
		Mem_Block	fdata;
		if (platform::read_file_onto_heap(pathname.str, &fdata, RD_NULL_TERMINATE)) {
			warning("'%' could not be read, skipping mesh!", pathname);
			continue;
		}
		defer { free(fdata.ptr); };
		
		uptr		vert_count, face_count;
		ui			src_indx_size;
		byte*		mesh_data = parse_ply_header(fdata, mesh.format, &vert_count, &face_count, &src_indx_size);
		if (!mesh_data) {
			warning("ply header could not be parsed, skipping mesh!");
			continue;
		}
		
		if (vert_count == 0 || face_count == 0) {
			warning("no vertex or face data, skipping!");
			continue;
		}
		if (!safe_cast(u32, vert_count)) {
			warning("count of vertecies would overflow u32, skipping mesh!");
			continue;
		}
		mesh.data.vertex_count = (u32)vert_count;
		mesh.data.triangle_count = 0;
		
		mesh.aabb = AABB::inf();
		
		byte* src_vertex_data = mesh_data;
		byte* src_face_data;
		
		{
			byte* cur = src_vertex_data;
			auto read_v2 = [&] () {
				auto ret = *(v2*)cur;
				cur += sizeof(v2);
				return ret;
			};
			auto read_v3 = [&] () {
				auto ret = *(v3*)cur;
				cur += sizeof(v3);
				return ret;
			};
			auto read_uc3 = [&] () {
				auto ret = *(tv3<u8>*)cur;
				cur += sizeof(tv3<u8>);
				return ret;
			};
			
			u64 vertex_size = (u64)format.vertex_stride * (u64)mesh.data.vertex_count;
			
			assert((format.vertex_data.len % format.vertex_stride) == 0);
			
			mesh.data.vertecies = (byte*)format.vertex_data.len; // relative to dynarr buffer
			mesh.vbo_data.base_vertex = safe_cast_assert(GLint, format.vertex_data.len / format.vertex_stride);  // relative to dynarr buffer (ie. vbo)
			
			byte* out = format.vertex_data.pushn(vertex_size);
			byte* out_cur = out;
			
			auto write_v2 = [&] (v2 vp v) {
				*(v2*)out_cur = v;
				out_cur += sizeof(v);
			};
			auto write_v3 = [&] (v3 vp v) {
				*(v3*)out_cur = v;
				out_cur += sizeof(v);
			};
			auto write_v4 = [&] (v4 vp v) {
				*(v4*)out_cur = v;
				out_cur += sizeof(v);
			};
			
			for (u32 i=0; i<mesh.data.vertex_count; ++i) {
				
				v3 pos =		read_v3();
				
				mesh.aabb.minmax(pos);
				
				write_v3(		pos ); // pos xyz
				write_v3(		read_v3() ); // norm xyz
				
				if (format.components.uv)
					write_v2(	read_v2() ); // uv uv
				
				if (format.components.tang)
					write_v4(	v4(0) ); // tang xyzw // write zero for now, will get filed in in tangent calculation step
				
				if (format.components.col) {
					write_v3(	cast_v3<v3>( read_uc3() ) / v3(255.0f) ); // vcol rgb
				}
				
			}
			
			assert(out_cur == (out +vertex_size));
			
			src_face_data = cur;
			
		}
		
		{
			
			byte* cur = src_face_data;
			auto read_u8 = [&] () {
				auto ret = *(u8*)cur;
				cur += sizeof(u8);
				return ret;
			};
			
			assert(src_indx_size == 1 || src_indx_size == 2);
			auto read_indx = [&] () {
				u16 ret = 0;
				switch (src_indx_size) {
					case 1:		ret = (u16) *(u8*)cur;		break;
					case 2:		ret = (u16) *(u16*)cur;		break;
				}
				cur += src_indx_size;
				return ret;
			};
			
			auto* arr = &format.triangle_data;
			
			mesh.data.triangles = (Triangle_Indx16*)(arr->len * sizeof(Triangle_Indx16)); // relative to dynarr buffer
			mesh.vbo_data.indx_offset = (GLvoid*)mesh.data.triangles;  // relative to dynarr buffer (ie. vbo)
			
			for (uptr i=0; i<face_count; ++i) {
				
				auto ngon = read_u8();
				switch (ngon) {
					case 3: {
						assert(safe_add(mesh.data.triangle_count, 1));
						mesh.data.triangle_count += 1;
						auto* out = arr->pushn(1);
						out->arr[0] = read_indx();
						out->arr[1] = read_indx();
						out->arr[2] = read_indx();
					} break;
					case 4: {
						assert(safe_add(mesh.data.triangle_count, 2));
						mesh.data.triangle_count += 2;
						auto* out = arr->pushn(2);
						auto a = read_indx();
						auto b = read_indx();
						auto c = read_indx();
						auto d = read_indx();
						out[0].arr[0] = a;
						out[0].arr[1] = b;
						out[0].arr[2] = c;
						out[1].arr[0] = c;
						out[1].arr[1] = d;
						out[1].arr[2] = a;
					} break;
					default: assert(false);
				}
			}
			
			assert(cur <= (fdata.ptr +fdata.size));
			if (cur < (fdata.ptr +fdata.size)) {
				warning("extra data at end of file ? (read % filesize % extra %)",
						ptr_sub(fdata.ptr, cur), fdata.size,
						fdata.size -ptr_sub(fdata.ptr, cur));
			}
			//assert(cur == (fdata.ptr +fdata.size));
			
			mesh.vbo_data.indx_count = safe_cast_assert(GLsizei, mesh.data.triangle_count*3);
			
		}
		
		if (format.components.tang) {
			assert(mesh.format == MF_COMMON);
			
			struct Connected_Triangle {
				u32					tri_i;
				Connected_Triangle*	next;
			};
			struct TB {
				v3 t;
				v3 b;
			};
			//constexpr Connected_Triangle* SPLIT_ME = (Connected_Triangle*)-1;
			
			DEFER_POP(&working_stk);
			auto* tri_tangents = working_stk.pushArr<TB>(mesh.data.triangle_count);
			auto* vert_connections = working_stk.pushArr<Connected_Triangle*>(mesh.data.vertex_count);
			cmemzero(vert_connections, sizeof(Connected_Triangle*) * mesh.data.vertex_count);
			
			auto* vertices = (Vertex_COMMON*)ptr_add(format.vertex_data.arr, mesh.data.vertecies);
			auto* triangles = ptr_add(format.triangle_data.arr, mesh.data.triangles);
			
			for (u32 tri_i=0; tri_i<mesh.data.triangle_count; ++tri_i) {
				
				v3 pos[3];
				v3 norm[3];
				v2 uv[3];
				for (u32 i=0; i<3; ++i) {
					auto indx = triangles[tri_i].arr[i];
					{ // Insert Connected_Triangle with our tri_i at the front of linked list at vert_connections[indx]
						auto* conn = vert_connections[indx];
						
						auto* new_conn = working_stk.push<Connected_Triangle>();
						new_conn->tri_i = tri_i;
						new_conn->next = conn;
						
						vert_connections[indx] = new_conn;
					}
					
					pos[i] =	vertices[indx].pos;
					norm[i] =	vertices[indx].norm;
					uv[i] =		vertices[indx].uv;
				}
				
				v3 e0 = pos[1] -pos[0];
				v3 e1 = pos[2] -pos[0];
				
				f32 du0 = uv[1].x -uv[0].x;
				f32 dv0 = uv[1].y -uv[0].y;
				f32 du1 = uv[2].x -uv[0].x; 
				f32 dv1 = uv[2].y -uv[0].y; 
				
				f32 f = 1.0f / ((du0 * dv1) -(du1 * dv0));
				
				v3 tang = v3(f) * ((v3(dv1) * e0) -(v3(dv0) * e1));
				v3 bitang = v3(f) * ((v3(du0) * e1) -(v3(du1) * e0));
				
				tang = normalize(tang);
				bitang = normalize(bitang);
				
				tri_tangents[tri_i].t = tang;
				tri_tangents[tri_i].b = bitang;
			}
			
			auto calc_bitang_sign = [&] (v3 vp tang, v3 vp bitang, v3 vp norm) -> f32 {
				return fp::or_sign(1.0f, fp::get_sign( dot(cross(norm, tang), bitang) ));
			};
			
			for (u32 vert_i=0; vert_i<mesh.data.vertex_count; ++vert_i) {
				
				u32 count = 0;
				v3	total_tang = v3(0);
				v3	total_bitang = v3(0);
				
				f32 sign = 0.0f;	// What sign we found, to detect handedness mismatches in the shared vertecies,
									//  so we can know when we need to split them
				
				assert(vert_connections[vert_i] != nullptr);
				auto* cur = vert_connections[vert_i];
				do {
					v3 t = tri_tangents[cur->tri_i].t;
					v3 b = tri_tangents[cur->tri_i].b;
					
					total_tang += t;
					total_bitang += b;
					
					++count;
					cur = cur->next;
				} while (cur);
				
				// average
				f32 inv_count = 1.0f / (f32)(count);
				v3 avg_tang = total_tang * v3(inv_count);
				v3 avg_bitang = total_bitang * v3(inv_count);
				
				avg_tang = normalize(avg_tang);
				avg_bitang = normalize(avg_bitang);
				
				f32 bitang_sign = calc_bitang_sign(avg_tang, avg_bitang, vertices[vert_i].norm);
				
				vertices[vert_i].tang = v4(avg_tang, bitang_sign);
			}
		}
		
		mesh.name = lstr((char*)(u64)f_str_tbl.len, name.len);
		f_str_tbl.pushn(name.str, name.len +1);
		
		f_meshes.push(mesh);
		
	}
	
	u64 meshes_size =	f_meshes.len * sizeof(Mesh);
	u64 formats_size =	MESH_FORMAT_COUNT * sizeof(fMesh_Format);
	
	u64 headers_size =	sizeof(header) +f_str_tbl.len +meshes_size +formats_size;
	
	u64 cur = headers_size;
	for (auto i=MESH_FORMAT_FIRST; i<MESH_FORMAT_COUNT; ++i) {
		auto& ff = fformats[i];
		
		ff.vertex_size = safe_cast_assert(GLsizeiptr, formats[i].vertex_data.len);
		ff.index_size = safe_cast_assert(GLsizeiptr, formats[i].triangle_data.len * sizeof(Triangle_Indx16));
		
		ff.vertex_data = (f32*)cur; // relative to file begin
		formats[i].vert_file_offs = cur;
		cur += ff.vertex_size;
		
		ff.index_data = (Triangle_Indx16*)cur; // relative to file begin
		formats[i].indx_file_offs = cur;
		cur += ff.index_size;
		
	}
	
	for (u32 i=0; i<f_meshes.len; ++i) {
		auto& m = f_meshes[i];
		auto& f = formats[m.format];
		
		m.data.vertecies = ptr_add(m.data.vertecies, f.vert_file_offs); // relative to file begin
		m.data.triangles = ptr_add(m.data.triangles, f.indx_file_offs); // relative to file begin
	}
	
	header.file_size = cur;
	header.meshes_count = f_meshes.len;
	header.str_tbl_size = f_str_tbl.len;
	
	{
		HANDLE h;
		if (win32::overwrite_file_wr("all.meshes", &h)) {
			warning("Output file '%' could not be opened!", "all.meshes");
		}
		
		win32::write_file(h, &header, sizeof(header));
		win32::write_file(h, &f_str_tbl[0], f_str_tbl.len);
		win32::write_file(h, &f_meshes[0], meshes_size);
		win32::write_file(h, &fformats[0], formats_size);
		for (auto i=MESH_FORMAT_FIRST; i<MESH_FORMAT_COUNT; ++i) {
			auto& f = formats[i];
			
			win32::write_file(h, &formats[i].vertex_data[0], formats[i].vertex_data.len);
			win32::write_file(h, &formats[i].triangle_data[0], formats[i].triangle_data.len * sizeof(Triangle_Indx16));
		}
		
		assert(win32::get_filepointer(h) == header.file_size);
		
		win32::close_file(h);
	}
	
	return 0;
}
