
typedef u32 rd_flags_t;
enum rd_flags_e : rd_flags_t {
	RD_FILENAME_ON_STK =		0b01,
	RD_NULL_TERMINATE =			0b10,
};

namespace platform {
	
	DECL err_e read_file_onto_heap (char const* filename, Mem_Block* out_data, rd_flags_t flags=0) {
		
		HANDLE handle;
		auto err = win32::open_existing_file_rd(filename, &handle);
		if (err) { return err; }
		defer {
			win32::close_file(handle);
		};
		
		u64 file_size = win32::get_file_size(handle);
		
		bool term = flags & RD_NULL_TERMINATE;
		
		// 
		auto data = (char*)malloc(file_size +(term ? 1 : 0));
		
		err = win32::read_file(handle, data, file_size);
		if (err) { return err; }
		
		if (term) {
			data[file_size] = '\0';
		}
		
		*out_data = { (byte*)data, file_size };
		return OK;
	}
	
	
	DECLD constexpr u32 FAST_FILE_READ_ALIGN = 64;
	
	DECL err_e read_file_onto (Stack* stk, char const* filename, Mem_Block* out_data, rd_flags_t flags=0) {
		
		HANDLE handle;
		auto err = win32::open_existing_file_rd(filename, &handle);
		if (err) { return err; }
		defer {
			win32::close_file(handle);
		};
		
		if (flags & RD_FILENAME_ON_STK) {
			stk->pop(filename);
		}
		
		u64 file_size = win32::get_file_size(handle);
		
		bool term = flags & RD_NULL_TERMINATE;
		
		// 
		auto data = stk->pushArrNoAlign<byte>(file_size);
		
		err = win32::read_file(handle, data, file_size);
		if (err) { return err; }
		
		if (term) {
			data[file_size] = '\0';
		}
		
		*out_data = { data, file_size };
		return OK;
	}
	
	DECL err_e write_whole_file (char const* filename, void const* data, u64 size) {
		HANDLE handle;
		auto err = win32::overwrite_file_wr(filename, &handle);
		if (err) { return err; }
		
		defer {
			// Close file
			win32::close_file(handle);
		};
		
		err = win32::write_file(handle, data, size);
		if (err) { return err; }
		
		return OK;
	}
}
