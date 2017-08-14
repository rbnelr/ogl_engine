
#include "global_includes.hpp"

#include <GL/gl.h>

#include "meshes_file.hpp"

cstr parse_uint (cstr cur, uptr* out) {
	using namespace parse_n;
	
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
}

uptr parsePlyHeader (cstr data, uptr dataMaxSize,
		uptr & outVertCount, uptr & outFaceCount, meshes_file_n::format_e& outFormat) {
	using namespace meshes_file_n;
	
	auto cur = data;
	
	uptr		vertCount;
	uptr		faceCount;
	format_e	format = (format_e)0;
	
	auto skipWhitespace = [&] (char const** cur_) -> void {
		auto cur = *cur_;
		while (*cur == ' ' || *cur == '\t') {
			++cur;
		}
		*cur_ = cur;
	};
	auto token = [=] (char const** cur_, cstr cstr) -> bool {
		auto cur = *cur_;
		for (;;) {
			
			if (*cur != *cstr) {
				if (*cstr == '\0' && !((*cur >= 'A' && *cur <= 'Z') || (*cur >= 'a' && *cur <= 'z') || *cur == '_')) {
					break;
				} else {
					return false;
				}
			}
			
			++cur;
			++cstr;
		}
		skipWhitespace(&cur);
		
		*cur_ = cur;
		return true;
	};
	
	auto newline = [&] (char const** cur_) -> bool {
		auto cur = *cur_;
		char c = *cur;
		if (c == '\n' || c == '\r') {
			++cur;
			if (*cur != c && (*cur == '\n' || *cur == '\r')) {
				++cur;
			}
		} else {
			return false;
		}
		
		*cur_ = cur;
		return true;
	};
	auto comments = [&] (char const** cur_) -> void {
		for (;;) {
			if (token(cur_, "comment")) {
				while (!newline(cur_)) {
					// Ignore comment characters
					++cur;
				}
			} else {
				return;
			}
		}
	};
	
	{
		assert(token(&cur, "ply"));
		assert(newline(&cur));
	}
	comments(&cur);
	{
		assert(token(&cur, "format"));
		
		assert(!token(&cur, "ascii"));
		assert(!token(&cur, "binary_big_endian"));
		assert(token(&cur, "binary_little_endian"));
		
		assert(token(&cur, "1.0"));
		assert(newline(&cur));
	}
	comments(&cur);
	{
		assert(token(&cur, "element"));
		assert(token(&cur, "vertex"));
		assert(cur = parse_uint(cur, &vertCount));
		assert(newline(&cur));
	}
	
	auto prop_vert = [&] (char const** cur_, cstr type, cstr component) -> bool {
		auto cur = *cur_;
		
		comments(&cur);
		
		if (!token(&cur, "property")) { return false; };
		if (!token(&cur, type)) { return false; };
		if (!token(&cur, component)) { return false; };
		if (!newline(&cur)) { return false; };
		
		*cur_ = cur;
		return true;
	};
	{
		format |= INTERLEAVED;
		
		assert(prop_vert(&cur, "float", "x"));
		assert(prop_vert(&cur, "float", "y"));
		assert(prop_vert(&cur, "float", "z"));
		format |= POS_XYZ;
		
		assert(prop_vert(&cur, "float", "nx"));
		assert(prop_vert(&cur, "float", "ny"));
		assert(prop_vert(&cur, "float", "nz"));
		format |= NORM_XYZ;
		
		if (		prop_vert(&cur, "float", "s") ) {
			assert(	prop_vert(&cur, "float", "t") );
			format |= UV_UV;
		}
		
		if (		prop_vert(&cur, "uchar", "red") ) {
			assert(prop_vert(&cur, "uchar", "green"));
			assert(prop_vert(&cur, "uchar", "blue"));
			format |= COL_RGB;
		}
	}
	#undef PROP_FLT_VERT
	
	comments(&cur);
	{
		assert(token(&cur, "element"));
		assert(token(&cur, "face"));
		assert(cur = parse_uint(cur, &faceCount));
		assert(newline(&cur));
	}
	comments(&cur);
	{
		assert(token(&cur, "property"));
		assert(token(&cur, "list"));
		assert(token(&cur, "uchar"));
		
		if (		token(&cur, "uint8") ) {
			//format |= INDEX_UBYTE;
			assert(false);
		} else if (	token(&cur, "uint16") ) {
			format |= INDEX_USHORT;
		} else if (	token(&cur, "uint32") ) {
			//format |= INDEX_UINT;
			assert(false);
		} else {
			assert(false);
		}
		assert(token(&cur, "vertex_indices"));
		assert(newline(&cur));
	}
	comments(&cur);
	{
		assert(token(&cur, "end_header"));
		assert(newline(&cur));
	}
	#undef INTEGER
	#undef token
	
	outVertCount = vertCount;
	outFaceCount = faceCount;
	outFormat = format;
	return ptr_sub(data, cur);
}

meshes_file_n::format_e processFile (byte * fileData, uptr fileSize, meshes_file_n::Header * file_header,
		byte * data_stk_begin, cstr relativePath, Stack & data_stk, Stack & header_stk,
		uptr & outVertexCount, uptr & outIndexCount) {
	using namespace meshes_file_n;
	
	format_e	format;
	uptr		vertCount;
	uptr		faceCount;
	byte *		binaryData;
	{
		auto data = reinterpret_cast<char*>(fileData);
		auto headerSize = parsePlyHeader(data, fileSize, vertCount, faceCount, format);
		binaryData = fileData +headerSize;
	}
	
	lstr		ext = lstr::null();
	lstr		ext1 = lstr::null();
	{
		cstr	cur = relativePath;
		while (*cur) {
			if (*cur == '.') {
				ext1 = {ext.str, (u32)ptr_sub(ext.str, cur)}; // ext.str can be null, we don't care about the invalid lstr.len we will generate since lstr.str is still null
				ext.str = cur +1;
			}
			++cur;
		}
		ext.len = ptr_sub(ext.str, cur); // ext.str can be null, we don't care about invalid lstr.len
		
		assert(ext.str && str::comp(ext, "ply"),
				"file extension is not 'ply' ('%')", relativePath);
		
		if (!ext1.str) {
			
			auto form = INTERLEAVED|INDEX_USHORT|POS_XYZ|NORM_XYZ|UV_UV;
			assert(format == form, "mesh type mismatch have % should be %", bin(format), bin(form));
			
			format |= TANG_XYZW;
			
		} else if (ext1.str && str::comp(ext1, "nouv")) {
			
			auto form = INTERLEAVED|INDEX_USHORT|POS_XYZ|NORM_XYZ;
			assert(format == form, "mesh type mismatch have % should be %", bin(format), bin(form));
			
		} else if (ext1.str && str::comp(ext1, "nouv_vcol")) {
			
			auto form = INTERLEAVED|INDEX_USHORT|POS_XYZ|NORM_XYZ|COL_RGB;
			assert(format == form, "mesh type mismatch have % should be %", bin(format), bin(form));
			
		} else {
			assert(false, "unknown mesh type '%'", ext1);
		}
	}
	
	data_stk.zeropad_to_align<DATA_ALIGN>();
	
	auto meshData = data_stk.getTop<byte>();
	uptr		indexCount;
	u32			vertex_len;
	GLfloat*	outVertexData;
	GLushort*	outIndexData;
	
	{
		auto cur = binaryData;
		
		assert(format & INTERLEAVED);
		assert(format & POS_XYZ);
		assert(format & NORM_XYZ);
		
		{
			auto readFloat32 = [&] () {
				auto ret = *reinterpret_cast<f32*>(cur);
				cur += sizeof(f32);
				return ret;
			};
			auto readUchar = [&] () {
				auto ret = *reinterpret_cast<u8*>(cur);
				cur += sizeof(u8);
				return ret;
			};
			
			assert(vertCount <= limits::max<GLushort>());
			
			auto count = vertCount;
			
			if (			!(format & UV_UV) ) {
				
				if (		!(format & COL_RGB) ) {
					
					assert(is_aligned(data_stk.getTop(), sizeof(GLfloat)));
					outVertexData = data_stk.pushArrNoAlign<GLfloat>(vertCount * (3 +3)); // pos_xyz +norm_xyz
					auto out = outVertexData;
					
					while (count--) {
						*out++ = readFloat32(); // x
						*out++ = readFloat32(); // y
						*out++ = readFloat32(); // z
						*out++ = readFloat32(); // nx
						*out++ = readFloat32(); // ny
						*out++ = readFloat32(); // nz
					}
				} else {
					
					assert(is_aligned(data_stk.getTop(), sizeof(GLfloat)));
					outVertexData = data_stk.pushArrNoAlign<GLfloat>(vertCount * (3 +3 +3)); // pos_xyz +norm_xyz +col_rgb
					auto out = outVertexData;
					
					while (count--) {
						*out++ = readFloat32(); // x
						*out++ = readFloat32(); // y
						*out++ = readFloat32(); // z
						*out++ = readFloat32(); // nx
						*out++ = readFloat32(); // ny
						*out++ = readFloat32(); // nz
						*out++ = static_cast<f32>(readUchar()) / 255.0f; // r
						*out++ = static_cast<f32>(readUchar()) / 255.0f; // g
						*out++ = static_cast<f32>(readUchar()) / 255.0f; // b
					}
				}
			} else {
				
				if (		!(format & COL_RGB) ) {
					
					if (format & TANG_XYZW) {
						
						vertex_len = 3 +3 +4 +2;
						
						assert(is_aligned(data_stk.getTop(), sizeof(GLfloat)));
						outVertexData = data_stk.pushArrNoAlign<GLfloat>(vertCount * vertex_len); // pos_xyz +norm_xyz +tang_xyzw +uv_uv
						auto out = outVertexData;
						
						while (count--) {
							*out++ = readFloat32(); // x
							*out++ = readFloat32(); // y
							*out++ = readFloat32(); // z
							*out++ = readFloat32(); // nx
							*out++ = readFloat32(); // ny
							*out++ = readFloat32(); // nz
							*out++ = 0; // tx
							*out++ = 0; // ty
							*out++ = 0; // tz
							*out++ = 0; // tw
							*out++ = readFloat32(); // u
							*out++ = readFloat32(); // v
						}
						
					} else {
						
						assert(is_aligned(data_stk.getTop(), sizeof(GLfloat)));
						outVertexData = data_stk.pushArrNoAlign<GLfloat>(vertCount * (3 +3 +2)); // pos_xyz +norm_xyz +uv_uv
						auto out = outVertexData;
						
						while (count--) {
							*out++ = readFloat32(); // x
							*out++ = readFloat32(); // y
							*out++ = readFloat32(); // z
							*out++ = readFloat32(); // nx
							*out++ = readFloat32(); // ny
							*out++ = readFloat32(); // nz
							*out++ = readFloat32(); // u
							*out++ = readFloat32(); // v
						}
						
					}
					
				} else {
					
					assert(is_aligned(data_stk.getTop(), sizeof(GLfloat)));
					outVertexData = data_stk.pushArrNoAlign<GLfloat>(vertCount * (3 +3 +2 +3)); // pos_xyz +norm_xyz +uv_uv +col_rgb
					auto out = outVertexData;
					
					while (count--) {
						*out++ = readFloat32(); // x
						*out++ = readFloat32(); // y
						*out++ = readFloat32(); // z
						*out++ = readFloat32(); // nx
						*out++ = readFloat32(); // ny
						*out++ = readFloat32(); // nz
						*out++ = readFloat32(); // u
						*out++ = readFloat32(); // v
						*out++ = static_cast<f32>(readUchar()) / 255.0f; // r
						*out++ = static_cast<f32>(readUchar()) / 255.0f; // g
						*out++ = static_cast<f32>(readUchar()) / 255.0f; // b
					}
					
				}
			}
		}
		
		indexCount = 0;
		
		if (		(format & INDEX_USHORT) == 0 ) {
			assert(false);
			outIndexData = nullptr;
		} else if (	(format & INDEX_USHORT) ) {
			
			data_stk.zeropad_to_align<sizeof(GLushort)>();
			
			outIndexData = data_stk.getTop<GLushort>();
			
			auto readUchar = [&] () {
				auto ret = *reinterpret_cast<u8*>(cur);
				cur += sizeof(u8);
				return ret;
			};
			auto readUint16 = [&] () {
				auto ret = *reinterpret_cast<u16*>(cur);
				cur += sizeof(u16);
				return ret;
			};
			
			auto count = faceCount;
			while (count--) {
				
				assert(is_aligned(data_stk.getTop(), sizeof(GLushort)));
				
				auto indices = readUchar();
				switch (indices) {
					case 3: { // Triangles
						auto out = data_stk.pushArrNoAlign<GLushort>(3);
						indexCount += 3;
						out[0] = readUint16();
						out[1] = readUint16();
						out[2] = readUint16();
					} break;
					case 4: { // Quads
						// Convert quad into triangles
						auto out = data_stk.pushArrNoAlign<GLushort>(6);
						indexCount += 6;
						auto a = readUint16();
						auto b = readUint16();
						auto c = readUint16();
						auto d = readUint16();
						out[0] = a;
						out[1] = b;
						out[2] = c;
						out[3] = c;
						out[4] = d;
						out[5] = a;
					} break;
					default: assert(false, "%", indices);
				}
			}
		} else {
			assert(false);
		}
	}
	
	if (format & TANG_XYZW) {
		
		auto require = POS_XYZ|NORM_XYZ|TANG_XYZW|UV_UV; // Can only calculate tangents if pos normal and uv are given
		assert((format & require) == require);
		assert(vertex_len >= 12); // x y z  nx ny nz  tx ty tz tw  u v
		
		
		auto test = [&data_stk] (uptr& vert_count, uptr& indx_count, f32*& vert_data, u16*& indx_data,
				u32 vert_len) -> void {
			
			struct Connected_Triangle {
				uptr				tri_i;
				Connected_Triangle*	next;
			};
			struct TB {
				v3 t;
				v3 b;
			};
			
			constexpr Connected_Triangle* SPLIT_ME = (Connected_Triangle*)-1;
			
			assert((indx_count % 3) == 0);
			
			uptr tri_count = indx_count / 3;
			
			DEFER_POP(&data_stk);
			auto* tri_tangents = data_stk.pushArr<TB>(indx_count);
			auto* vert_connections = data_stk.pushArr<Connected_Triangle*>(vert_count);
			cmemzero(vert_connections, sizeof(Connected_Triangle*)*vert_count);
			
			for (uptr tri_i=0; tri_i<tri_count; ++tri_i) {
				
				v3 pos[3];
				v3 norm[3];
				v2 uv[3];
				for (uptr i=0; i<3; ++i) {
					auto indx = indx_data[(tri_i * 3) +i];
					{ // Insert Connected_Triangle with our tri_i at the front of linked list at vert_connections[indx] 
						auto* conn = vert_connections[indx];
						
						auto* new_conn = data_stk.push<Connected_Triangle>();
						new_conn->tri_i = tri_i;
						new_conn->next = conn;
						
						vert_connections[indx] = new_conn;
					}
					
					auto* vert = &vert_data[indx * vert_len];
					pos[i] =	v3(vert[0], vert[1], vert[2]);
					norm[i] =	v3(vert[3], vert[4], vert[5]);
					uv[i] =		v2(vert[10], vert[11]);
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
			
			auto calc_bitang_sign = [&] (v3 tang, v3 bitang, v3 norm) -> f32 {
				return fp::or_sign(1.0f, fp::get_sign( dot(cross(norm, tang), bitang) ));
			};
			
			for (uptr vert_i=0; vert_i<vert_count; ++vert_i) {
				
				auto* vert = &vert_data[vert_i * vert_len];
				v3 norm = v3(vert[3], vert[4], vert[5]);
				
				u32	count = 0;
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
				
				f32 bitang_sign = calc_bitang_sign(avg_tang, avg_bitang, norm);
				
				vert[6] = avg_tang.x;
				vert[7] = avg_tang.y;
				vert[8] = avg_tang.z;
				vert[9] = bitang_sign; // tangent w
				
			}
			
		};
		test(vertCount, indexCount, outVertexData, outIndexData, vertex_len);
		
	}
	
	++file_header->meshEntryCount;
	
	header_stk.zeropad_to_align<meshes_file_n::HEADER_MESH_ENTRY_ALIGN>();
	
	auto meshEntry = header_stk.pushNoAlign<Header_Mesh_Entry>({});
	meshEntry->dataOffset = ptr_sub(data_stk_begin, meshData);
	meshEntry->dataFormat = format;
	
	uptr sizePerIndex;
	
	switch (format & INDEX_MASK) {
		case INDEX_USHORT:
			meshEntry->vertexCount.glushort = safe_cast_assert(GLushort, vertCount);
			sizePerIndex = 2;
			STATIC_ASSERT(sizeof(GLushort) == 2);
			break;
		default: assert(false);
	}
	file_header->totalVertexSize += vertCount * (vertex_len * sizeof(f32));
	file_header->totalIndexSize += indexCount * sizePerIndex;
	meshEntry->indexCount = safe_cast_assert(GLsizei, indexCount);
	
	{
		assert(ext.str[-1] == '.');
		
		uptr len = ptr_sub(relativePath, ext.str -1);
		char* out = header_stk.pushArr<char>(len +1);
		
		cmemcpy(out, relativePath, len);
		out[len] = '\0';
	}
	
	outVertexCount = vertCount;
	outIndexCount = indexCount;
	
	return format;
}

uptr strcpy (cstr src, uptr len, char * dst) {
	auto l = len;
	while (l--)
		*dst++ = *src++;
	return len;
}
uptr strcpyNoterm (cstr src, char * dst) {
	auto c = src;
	for (;;) {
		if (*c == '\0') break;
		*dst++ = *c++;
	}
	return ptr_sub(src, c);
}

template <typename LAMB_PUTC>
uptr strcpyNoterm (cstr src, LAMB_PUTC putc) {
	auto c = src;
	for (;;) {
		if (*c == '\0') break;
		putc(*c++);
	}
	return ptr_sub(src, c);
}
template <typename LAMB_PUTC>
uptr strcpy (cstr src, uptr len, LAMB_PUTC putc) {
	auto l = len;
	while (l--) {
		putc(*src++);
	}
	return len;
}
template <typename LAMB_PUTC>
uptr strcpy (cstr src, LAMB_PUTC putc) {
	auto c = src;
	for (;;) {
		auto temp = *c++;
		putc(temp);
		if (temp == '\0') break;
	}
	return ptr_sub(src, c);
}

template <typename LAMB_DO_FOR_FILE>
void forAllFiles (cstr basePath, WIN32_FIND_DATAA & info, Stack & workingStack, LAMB_DO_FOR_FILE doForFile) {
	ui pathListLen;
	byte * pathList;
	{
		bool nextFileFound = true;
		
		HANDLE searchHandle;
		{
			DEFER_POP(&workingStack);
			auto filepath = workingStack.getTop<char>();
			
			strcpyNoterm( basePath, [&] (char c) { workingStack.push(c); } );
			
			workingStack.push('*');
			workingStack.push('\0');
			
			searchHandle = FindFirstFileA(filepath, &info);
			if (searchHandle == INVALID_HANDLE_VALUE) {
				auto err = GetLastError();
				assert(err == ERROR_FILE_NOT_FOUND);
				// Should never happen since every directory should contain at least "." and ".."
				nextFileFound = false;
			}
		}
		
		pathListLen = 0;
		pathList = workingStack.getTop<byte>();
		
		while (nextFileFound) {
			if (	(info.cFileName[0] == '.' && info.cFileName[1] == '\0') ||
					(info.cFileName[0] == '.' && info.cFileName[1] == '.' && info.cFileName[2] == '\0')) {
				// Ignore "." and ".." directories
			} else {
				auto isDirectory = (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
				
				auto top = workingStack.getTop();
				
				workingStack.push<bool>(isDirectory);
				
				{
					char *	in = info.cFileName;
					char *	max = info.cFileName +MAX_PATH;
					
					char * lastDot = nullptr;
					while (in != max) {
						workingStack.push(*in);
						if (*in == '.') lastDot = workingStack.getTop<char>() -1;
						if (*in++ == '\0') break;
					}
					if (!isDirectory) {
						bool isPly = true;
						if (!lastDot) {
							isPly = false;
						} else {
							auto plyCstr = ".ply";
							auto ext = lastDot;
							for (;;) {
								if (*ext != *plyCstr) {
									isPly = false;
									break;
								}
								if (*ext == '\0') {
									break;
								}
								++ext;
								++plyCstr;
							}
						}
						
						if (!isPly) {
							workingStack.pop(top);
						} else {
							workingStack.pop(lastDot);
							workingStack.push('\0');
							++pathListLen;
						}
					} else {
						++pathListLen;
					}
				}
			}
			auto ret = FindNextFile(searchHandle, &info);
			if (ret == 0) {
				auto err = GetLastError();
				assert(err == ERROR_NO_MORE_FILES);
				nextFileFound = false;
			}
		}
		
		{
			auto ret = FindClose(searchHandle);
			assert(ret != 0);
		}
	}
	
	auto filepath = workingStack.getTop<char>();
	strcpyNoterm( basePath, [&] (char c) { workingStack.push(c); } );
	
	while (pathListLen--) {
		auto isDirectory = *reinterpret_cast<bool*>(pathList);
		pathList += sizeof(bool);
		
		DEFER_POP(&workingStack);
		
		auto file = workingStack.getTop<char>();
		uptr len = strcpyNoterm(reinterpret_cast<char*>(pathList), [&] (char c) { workingStack.push(c); });
		pathList += len +1;
		
		if (isDirectory) {
			workingStack.push('\\');
			workingStack.push('\0');
			forAllFiles(filepath, info, workingStack, doForFile);
		} else {
			for (ui i=0; i<ptr_sub(filepath, file +len); ++i) {
				if (filepath[i] == '\\')
						filepath[i] = '/';
			}
			auto plyCstr = ".ply";
			auto plyLen = strcpy(plyCstr, str::len(plyCstr) +1, [&] (char c) { workingStack.push(c); });
			doForFile(filepath, ptr_sub(filepath, file +len +plyLen -1), workingStack);
		}
	}
}

DECL s32 entry () {
	
	auto workingStack =		makeStack(0, mebi(16)); // Hard upper limit for now, 16 MiB should be enough
	auto header_stk =		makeStack<meshes_file_n::FILE_ALIGN>(0, mebi(16)); // Hard upper limit for now, 16 MiB should be enough
	auto data_stk =			makeStack<meshes_file_n::FILE_ALIGN>(0, mebi(16)); // Hard upper limit for now, 16 MiB should be enough
	
	auto header_stk_begin = header_stk.getTopNoAlign<byte>();
	auto data_stk_begin = data_stk.getTopNoAlign<byte>();
	
	auto file_header = header_stk.push<meshes_file_n::Header>({});
	
	{
		strcpy(meshes_file_n::magicString, arrlenof(file_header->magicString), file_header->magicString);
		file_header->meshEntryCount = 0;
		file_header->totalVertexSize = 0;
		file_header->totalIndexSize = 0;
	}
	auto indexMeshEntries = header_stk.getTopAlign<meshes_file_n::HEADER_MESH_ENTRY_ALIGN, meshes_file_n::Header_Mesh_Entry>();
	
	HANDLE outputFileHandle;
	{ // 
		auto file = "all.meshes";
		auto ret = CreateFileA(file, GENERIC_WRITE, FILE_SHARE_READ, NULL,
				CREATE_ALWAYS, 0, NULL);
		assert(ret != INVALID_HANDLE_VALUE, "CreateFileA failed for '%'", file);
		outputFileHandle = ret;
	}
	
	{
		DEFER_POP(&workingStack);
		auto buf = workingStack.pushArr<char>(MAX_PATH);
		uptr len;
		{
			auto ret = GetCurrentDirectory(MAX_PATH, buf);
			assert(ret != 0);
			len = ret;
		}
		if (buf[len -1] != '\\') {
			buf[len] = '\\';
			++len;
		}
		
		auto cstr = "assets_src\\meshes\\";
		auto l = str::len(cstr) +1;
		assert((len +l) <= MAX_PATH);
		strcpy(cstr, l, buf +len);
		{
			auto ret = SetCurrentDirectory(buf);
			assert(ret != 0);
		}
	}
	
	{
		auto do_ = [&] (cstr filepathCstr, uptr filepathStrLen, Stack & workingStack) -> void {
			print(filepathCstr);
			
			HANDLE			fileHandle;
			{ // 
				auto ret = CreateFileA(filepathCstr, GENERIC_READ, FILE_SHARE_READ, NULL,
						OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
				if (ret == INVALID_HANDLE_VALUE) {
					auto err = GetLastError();
					assert(err == ERROR_FILE_NOT_FOUND, "CreateFileA failed for '%' with error code %\n", filepathCstr, err);
					// ERROR_FILE_NOT_FOUND; file might have been deleted between call to FindFirstFileA/FindNextFile and CreateFileA
					return; // Ignore this file
				}
				fileHandle = ret;
			}
			defer {
				auto ret = CloseHandle(fileHandle);
				assert(ret != 0);
			};
			
			uptr fileSize;
			{ // 
				LARGE_INTEGER size;
				auto ret = GetFileSizeEx(fileHandle, &size);
				assert(ret != 0, "GetFileSizeEx failed for '%'", filepathCstr);
				fileSize = size.QuadPart;
			}
			
			auto fileData = workingStack.pushArr<byte>(fileSize +1);
			{ // Read file contents onto stack
				DWORD bytesRead;
				auto ret = ReadFile(fileHandle, fileData, safe_cast_assert(DWORD, fileSize), &bytesRead, NULL);
				assert(ret != 0, "ReadFile failed for '%'", filepathCstr);
				assert(bytesRead == fileSize, "ReadFile 'bytesRead != fileSize' for '%'", filepathCstr);
			}
			fileData[fileSize] = '\0';
			
			uptr vertCount, indxCount;
			auto f = processFile(fileData, fileSize, file_header, data_stk_begin, filepathCstr, data_stk, header_stk,
					vertCount, indxCount);
			
			print(" % vertices", vertCount);
			
			if (indxCount) {
				print(" % indices", indxCount);
			}
			
			if (f & meshes_file_n::UV_UV) {
				print(" UV_UV");
			}
			if (f & meshes_file_n::COL_RGB) {
				print(" UV_RGB");
			}
			
			print("\n");
		};
		
		DEFER_POP(&workingStack);
		auto info = workingStack.push<WIN32_FIND_DATAA>();
		
		char *	basePath = workingStack.getTop<char>();
		workingStack.push('\0');
		forAllFiles(basePath, *info, workingStack, do_);
	}
	
	STATIC_ASSERT(meshes_file_n::DATA_ALIGN == meshes_file_n::FILE_ALIGN);
	
	header_stk.zeropad_to_align<meshes_file_n::DATA_ALIGN>();
	uptr header_size = ptr_sub(header_stk_begin, header_stk.getTop());
	
	data_stk.zeropad_to_align<meshes_file_n::DATA_ALIGN>();
	uptr data_size = ptr_sub(data_stk_begin, data_stk.getTop());
	
	file_header->file_size = header_size +data_size;
	
	{ // Add header_size to dataOffset of every entry since offset in data stack was saved
		using namespace meshes_file_n;
		
		auto cur = reinterpret_cast<byte*>(indexMeshEntries);
		
		for (ui i=0; i<file_header->meshEntryCount; ++i) {
			
			cur = align_up(cur, meshes_file_n::HEADER_MESH_ENTRY_ALIGN);
			
			auto entry = reinterpret_cast<Header_Mesh_Entry*>(cur);
			entry->dataOffset += header_size;
			
			cur += sizeof(Header_Mesh_Entry);
			
			auto pc = reinterpret_cast<char*>(cur);
			while (*pc++ != '\0') {}
			
			cur = reinterpret_cast<byte*>(pc);
		}
	}
	
	
	{ // 
		DWORD bytesWritten;
		auto size = header_size;
		auto ret = WriteFile(outputFileHandle, header_stk_begin, safe_cast_assert(DWORD, size),
				&bytesWritten, NULL);
		assert(ret != FALSE);
		assert(size == bytesWritten);
	}
	{ // 
		DWORD bytesWritten;
		auto size = data_size;
		auto ret = WriteFile(outputFileHandle, data_stk_begin, safe_cast_assert(DWORD, size),
				&bytesWritten, NULL);
		assert(ret != FALSE);
		assert(size == bytesWritten);
	}
	
	return 0;
}


void global_init () {
	
	assert_page_size();
	
	print_n::init(); // TODE: DEPENDS on print_working_stk, so do threadlocal_init() before doing global_init() in main thread
	
	time::QPC::init();
	
}

struct Cmd_Line {
	u32		argc;
	cstr*	argv;
};
DECLD Cmd_Line cmd_line;

s32 main (s32 argc, cstr* argv) {
	
	cmd_line.argc = MAX(argc, 0);
	cmd_line.argv = argv;
	
	global_init();
	
	return entry();
}

