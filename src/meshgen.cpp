
#define WORKING_STK_CAP mebi<uptr>(128)
#include "startup.hpp"

#include <GL/gl.h>
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

fHeader		header = { FILE_ID }; // zero rest

DECL byte* parse_ply_header (Mem_Block cr data,
		uptr* vert_count, uptr* face_count, format_e* format, ui* indx_size) {
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
	*format = (format_e)0;
	{
		req(		prop_vert(cur, "float", "x") );
		req(		prop_vert(cur, "float", "y") );
		req(		prop_vert(cur, "float", "z") );
		// always at least a xyz
		
		opt(		prop_vert(cur, "float", "nx") );
		if (res) {
			req(	prop_vert(cur, "float", "ny") );
			req(	prop_vert(cur, "float", "nz") );
			*format |= F_NORM_XYZ;
		}
		
		opt(		prop_vert(cur, "float", "s") );
		if (res) {
			req(	prop_vert(cur, "float", "t") );
			*format |= F_UV_UV;
		}
		
		opt(		prop_vert(cur, "uchar", "red") );
		if (res) {
			req(	prop_vert(cur, "uchar", "green") );
			req(	prop_vert(cur, "uchar", "blue") );
			*format |= F_COL_RGB;
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

DECL bool mesh_type (lstr cr filename, fMesh* fmesh, lstr* out_name) {
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
	
	format_e src_format, dst_format;
	
	if (!ext)								dst_format = F_NORM_XYZ|F_UV_UV|F_TANG_XYZW; // default
	else if ( str::comp(ext, "nouv") )		dst_format = F_NORM_XYZ;
	else if ( str::comp(ext, "nouv_vcol") )	dst_format = F_NORM_XYZ|F_COL_RGB;
	else if ( str::comp(ext, "vcol") )		dst_format = F_NORM_XYZ|F_UV_UV|F_COL_RGB;
	else {
		warning("unknown mesh type '%'", ext);
		return false;
	}
	
	src_format = dst_format & ~F_TANG_XYZW;
	
	assert(src_format == fmesh->format, "mesh type mismatch, have % should be %", bin(fmesh->format), bin(src_format));
	
	assert(fmesh->vertex_count > 0);
	if (!safe_cast(u16, fmesh->vertex_count -1)) {
		warning("Too many vertices to use u16 vertex indexing, u32 indices currently not supported!");
		return false;
	}
	fmesh->format = dst_format | F_INDX_U16;
	
	*out_name = print_working_stk("%\\0", lstr(filename.str, filename.len -real_ext.len -1));
	return true;
}

DECL void gen_src_code_mesh (cstr out_filename, lstr cr name, fMesh cr fmesh, f32 const* vertex_data, Triangle_Indx16 const* triangles) {
	
	assert(	fmesh.format == (F_INDX_U16|F_NORM_XYZ|F_UV_UV|F_COL_RGB));
	
	auto const* vertecies = (Vertex_PNUC*)vertex_data;
	
	DEFER_POP(&working_stk);
	auto* src_code = working_stk.getTop<char>();
	
	print_working_stk(
			"// This file was auto-generated by meshgen.exe\n"
			"\n"
			"//#define F(val) reint_int_as_flt(val##u) // no constexpr hex float currently\n"
			"\n"
			"#define VERT _##MESH_INSTANCE##_vert_data\n"
			"DECL u32 VERT[% * (3+3+2+3)] = { // pos norm col\n",
			fmesh.vertex_count);
	for (u32 i=0; i<fmesh.vertex_count; ++i) {
		auto& v = vertecies[i];
		print_working_stk(
			//"	F(%),F(%),F(%), F(%),F(%),F(%), F(%),F(%), F(%),F(%),F(%),\n",
			"	%,%,%, %,%,%, %,%, %,%,%,\n",
			hex(reint_flt_as_int(v.pos.x)),		hex(reint_flt_as_int(v.pos.y)),		hex(reint_flt_as_int(v.pos.z)),
			hex(reint_flt_as_int(v.norm.x)),	hex(reint_flt_as_int(v.norm.y)),	hex(reint_flt_as_int(v.norm.z)),
			hex(reint_flt_as_int(v.uv.x)),		hex(reint_flt_as_int(v.uv.y)),
			hex(reint_flt_as_int(v.col.x)),		hex(reint_flt_as_int(v.col.y)),		hex(reint_flt_as_int(v.col.z)) );
	}
	print_working_stk("};\n");
	
	print_working_stk(
			"\n"
			"#define INDX _##MESH_INSTANCE##_indx_data\n"
			"DECL meshes_file_n::Triangle_Indx16 INDX[% * 3] = {\n",
			fmesh.triangle_count);
	for (u32 i=0; i<fmesh.triangle_count; ++i) {
		print_working_stk("	{%,%,%},\n", triangles[i].arr[0], triangles[i].arr[1], triangles[i].arr[2]);
	}
	print_working_stk("};\n");
	
	print_working_stk(
		"\n"
		"DECL Mesh MESH_INSTANCE = {\n"
		"	\"%\",\n"
		"	meshes_file_n::F_INDX_U16|meshes_file_n::F_NORM_XYZ|meshes_file_n::F_UV_UV|meshes_file_n::F_COL_RGB,\n"
		"	{\n"
		"		%,	(f32*)VERT,\n"
		"		%,	INDX,\n"
		"	},\n"
		"	{},\n"
		"	{	reint_int_as_flt(%u),reint_int_as_flt(%u),\n"
		"		reint_int_as_flt(%u),reint_int_as_flt(%u),\n"
		"		reint_int_as_flt(%u),reint_int_as_flt(%u),\n"
		"	},\n"
		"};\n"
		"\n"
		"#undef VERT\n"
		"#undef INDX\n"
		"#undef F\n",
		name,
		fmesh.vertex_count,
		fmesh.triangle_count,
		hex(reint_flt_as_int(fmesh.aabb.xl)), hex(reint_flt_as_int(fmesh.aabb.xh)),
		hex(reint_flt_as_int(fmesh.aabb.yl)), hex(reint_flt_as_int(fmesh.aabb.yh)),
		hex(reint_flt_as_int(fmesh.aabb.zl)), hex(reint_flt_as_int(fmesh.aabb.zh)) );
	
	u64 src_code_sz = ptr_sub( src_code, working_stk.getTop<char>() );
	
	platform::write_whole_file(out_filename, src_code, src_code_sz);
	
}

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
	
	dynarr<char>		f_str_tbl(512);
	dynarr<fMesh>		f_meshes(64);
	dynarr<byte, u64>	f_data(mebi(16));
	
	for (u32 i=0; i<src_files.arr.len; ++i) {
		lstr filename = src_files.get_filename(i);
		print("%:\n", filename);
		
		DEFER_POP(&working_stk);
		lstr pathname = print_working_stk("%%\\0", input_foldername, filename);
		
		Mem_Block	fdata;
		if (platform::read_file_onto_heap(pathname.str, &fdata, RD_NULL_TERMINATE)) {
			warning("'%' could not be read, skipping mesh!", pathname);
			continue;
		}
		defer { free(fdata.ptr); };
		
		fMesh fmesh;
		
		uptr		vert_count, face_count;
		ui			src_indx_size;
		byte*		mesh_data = parse_ply_header(fdata, &vert_count, &face_count, &fmesh.format, &src_indx_size);
		if (!mesh_data) {
			warning("ply header could not be parsed, skipping mesh!");
			continue;
		}
		
		fmesh.data_offs = f_data.len;
		
		if (vert_count == 0 || face_count == 0) {
			warning("no vertex or face data, skipping!");
			continue;
		}
		if (!safe_cast(u32, vert_count)) {
			warning("count of vertecies would overflow u32, skipping mesh!");
			continue;
		}
		fmesh.vertex_count = (u32)vert_count;
		
		lstr name;
		if (!mesh_type(filename, &fmesh, &name)) continue;
		
		ui out_vertex_size = get_vertex_size(fmesh.format);
		
		u64 out_vert_offs = f_data.len;
		u64 out_indx_offs;
		
		auto pop_data_from_f_data = [&] () {
			f_data.shrink_to(out_vert_offs);
		};
		
		byte* src_vertex_data = mesh_data;
		byte* src_face_data;
		
		fmesh.aabb = AABB::inf();
		
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
			
			u64 vertex_size = (u64)out_vertex_size * (u64)fmesh.vertex_count;
			byte* out = f_data.pushn(vertex_size);
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
			
			for (u32 i=0; i<fmesh.vertex_count; ++i) {
				
				v3 pos =		read_v3();
				
				write_v3(		pos ); // pos xyz
				
				fmesh.aabb.minmax(pos);
				
				if (fmesh.format & F_NORM_XYZ)
					write_v3(	read_v3() ); // norm xyz
				
				if (fmesh.format & F_UV_UV)
					write_v2(	read_v2() ); // uv uv
				
				if (fmesh.format & F_TANG_XYZW)
					write_v4(	v4(0) ); // tang xyzw // write zero for now, will get filed in in tangent calculation step
				
				if (fmesh.format & F_COL_RGB) {
					write_v3(	cast_v3<v3>( read_uc3() ) / v3(255.0f) ); // vcol rgb
				}
				
			}
			
			assert(out_cur == (out +vertex_size));
			
			src_face_data = cur;
			
			header.total_vertex_size += vertex_size;
		}
		
		assert((fmesh.format & F_INDX_MASK) == F_INDX_U16);
		
		fmesh.triangle_count = 0;
		{
			out_indx_offs = f_data.len;
			
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
			
			for (uptr i=0; i<face_count; ++i) {
				
				auto ngon = read_u8();
				switch (ngon) {
					case 3: {
						assert(safe_add(fmesh.triangle_count, 1));
						fmesh.triangle_count += 1;
						auto* out = (Triangle_Indx16*)f_data.pushn(1*sizeof(Triangle_Indx16));
						out->arr[0] = read_indx();
						out->arr[1] = read_indx();
						out->arr[2] = read_indx();
					} break;
					case 4: {
						assert(safe_add(fmesh.triangle_count, 2));
						fmesh.triangle_count += 2;
						auto* out = (Triangle_Indx16*)f_data.pushn(2*sizeof(Triangle_Indx16));
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
			
			u64 index_size = (u64)fmesh.triangle_count * sizeof(Triangle_Indx16);
			header.total_index_size += index_size;
			
			assert(cur <= (fdata.ptr +fdata.size));
			if (cur < (fdata.ptr +fdata.size)) {
				warning("here");
			}
			//assert(cur == (fdata.ptr +fdata.size));
			
		}
		
		auto* vertex_data = (f32*)&f_data[out_vert_offs];
		auto* triangles = (Triangle_Indx16*)&f_data[out_indx_offs];
		
		if (fmesh.format & F_TANG_XYZW) {
			assert(fmesh.format == (F_INDX_U16|F_NORM_XYZ|F_UV_UV|F_TANG_XYZW));
			
			struct Connected_Triangle {
				uptr				tri_i;
				Connected_Triangle*	next;
			};
			struct TB {
				v3 t;
				v3 b;
			};
			constexpr Connected_Triangle* SPLIT_ME = (Connected_Triangle*)-1;
			
			DEFER_POP(&working_stk);
			auto* tri_tangents = working_stk.pushArr<TB>(fmesh.triangle_count);
			auto* vert_connections = working_stk.pushArr<Connected_Triangle*>(fmesh.vertex_count);
			cmemzero(vert_connections, sizeof(Connected_Triangle*) * fmesh.vertex_count);
			
			auto* vertex = (Vert_PNUT*)vertex_data;
			
			for (u32 tri_i=0; tri_i<fmesh.triangle_count; ++tri_i) {
				
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
					
					pos[i] =	vertex[indx].pos;
					norm[i] =	vertex[indx].norm;
					uv[i] =		vertex[indx].uv;
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
			
			for (u32 vert_i=0; vert_i<fmesh.vertex_count; ++vert_i) {
				
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
				
				f32 bitang_sign = calc_bitang_sign(avg_tang, avg_bitang, vertex[vert_i].norm);
				
				vertex[vert_i].tang = v4(avg_tang, bitang_sign);
			}
		}
		
		if (str::comp(name, "missing_mesh.vcol")) {
			
			assert(	fmesh.format == (F_INDX_U16|F_NORM_XYZ|F_UV_UV|F_COL_RGB));
			gen_src_code_mesh(SRC_DIR "missing_mesh.hpp", name, fmesh, vertex_data, triangles);
			
			pop_data_from_f_data();
			
		} else {
			
			fmesh.mesh_name = f_str_tbl.len;
			f_str_tbl.pushn(name.str, name.len +1);
			
			f_meshes.push(fmesh);
		}
	}
	
	u64 meshes_size =	f_meshes.len * sizeof(fMesh);
	u64 headers_size =	sizeof(header) +f_str_tbl.len +meshes_size;
	
	for (u32 i=0; i<f_meshes.len; ++i) {
		f_meshes[i].data_offs += headers_size;
	}
	
	header.file_size = headers_size +f_data.len;
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
		win32::write_file(h, &f_data[0], f_data.len);
		
		assert(win32::get_filepointer(h) == header.file_size);
		
		win32::close_file(h);
	}
	
	return 0;
}
