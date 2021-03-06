
// we are in Cproj/<project>/src/ so this is Cproj/flamegraph/src/...
#include "../../flamegraph/src/flamegraph_data_file.hpp"
#include "../../flamegraph/src/streaming.hpp"

#define NOINLINE_

namespace profile {
	
	struct Sample {
		u64			qpc;
		GLuint		gpu;
		
		u32			index; // generic integer data
		cstr		name;
		
	};
	
	//DECLD constexpr u32		FRAME_BUFFER_COUNT =		0; // How many frames to wait to process the data (0 is between the measured frame and the next, etc.) (because gpu samples need to become avalible)
	DECLD constexpr u32		MAX_SAMPLES =				kibi<u32>(64); // 4096
	
	DECLD constexpr u32		THREAD_COUNT =				2; // engine and gpu
	DECLD constexpr cstr 	LATEST_FILE_FILENAME =		"profiling/latest.profile";
	
	DECLD sync::Mutex		mutex;
	
	DECLD Sample			samples[MAX_SAMPLES] = {}; // zero
	DECLD Sample*			cur_sample; // next to be written
	DECLD u32				samples_remain;
	
	DECLD u32				prev_chunk_count = 0;
	DECLD flamegraph_data_file::Chunk	prev_chunk;
	
	DECLD u32				dropped_samples;
	
	DECLD u64				qpc_zerotime;
	DECLD u64				rdtsc_zerotime;
	
	DECLD HANDLE			latest_file;
	
	DECLD streaming::Client	stream;
	
	void init_file ();
	
	void init () {
		mutex.init();
		
		cur_sample = &samples[0];
		samples_remain = MAX_SAMPLES;
		dropped_samples = 0;
		
		winsock::init();
		stream.connect_to_server();
		
		init_file();
	}
	void init_gl () {
		SYNC_SCOPED_MUTEX_LOCK(&mutex);
		
		for (u32 i=0; i<MAX_SAMPLES; ++i) {
			glGenQueries(1, &samples[i].gpu);
		}
	}
	
	NOINLINE_ void record_sample (char const* name, u32 index=0) {
		SYNC_SCOPED_MUTEX_LOCK(&mutex);
		
		if (samples_remain == 0) {
			++dropped_samples;
		} else {
			Sample* s = cur_sample;
			cur_sample = &samples[ ((u32)(cur_sample -&samples[0]) +1) % MAX_SAMPLES ];
			--samples_remain;
			
			QueryPerformanceCounter((LARGE_INTEGER*)&s->qpc);
			if (s->gpu != 0) glQueryCounter(s->gpu, GL_TIMESTAMP);
			
			s->index = index;
			s->name = name;
		}
	}
	NOINLINE_ void record_sample (u64 qpc_now, char const* name, u32 index=0) {
		SYNC_SCOPED_MUTEX_LOCK(&mutex);
		
		if (samples_remain == 0) {
			++dropped_samples;
		} else {
			Sample* s = cur_sample;
			cur_sample = &samples[ ((u32)(cur_sample -&samples[0]) +1) % MAX_SAMPLES ];
			--samples_remain;
			
			s->qpc = qpc_now;
			if (s->gpu != 0) glQueryCounter(s->gpu, GL_TIMESTAMP);
			
			s->index = index;
			s->name = name;
		}
	}
	
	//////////
	
	u32 push_str (dynarr<char>* arr, lstr name) {
		u32 offs = arr->len;
		arr->pushn(name.str, name.len +1);
		return offs;
	}
	
	void init_file () {
		using namespace flamegraph_data_file;
		
		struct Header {
			File_Header	header;
			Thread		threads[THREAD_COUNT];
		};
		
		DEFER_POP(&working_stk);
		auto* header = working_stk.push<Header>();
		
		auto thr_name_str_tbl = dynarr<char>::alloc(64);
		defer { thr_name_str_tbl.free(); };
		
		header->threads[0].sec_per_unit =	time::QPC::inv_freq;
		header->threads[0].name_tbl_offs =	push_str(&thr_name_str_tbl, "engine_game_loop");
		header->threads[0].event_count =	0;
		header->threads[1].sec_per_unit =	nano<f32>(1);
		header->threads[1].name_tbl_offs =	push_str(&thr_name_str_tbl, "gpu_measurement");
		header->threads[1].event_count =	0;
		
		header->header.id =						FILE_ID;
		header->header.file_size =				sizeof(Header) +thr_name_str_tbl.len;
		header->header.total_event_count =		0;
		header->header.thr_name_str_tbl_size =	thr_name_str_tbl.len;
		header->header.thread_count =			THREAD_COUNT;
		header->header.chunks_count =			0;
		
		cmemcpy(	working_stk.pushArr<char>(thr_name_str_tbl.len),
					thr_name_str_tbl.arr, thr_name_str_tbl.len );
		
		if (win32::overwrite_file_rw(LATEST_FILE_FILENAME, &latest_file)) {; // overwrite the foe that always saves the newest profile
			warning("'%' not created, won't save profiling data!", LATEST_FILE_FILENAME);
		}
		
		u64 size = ptr_sub((byte*)header, working_stk.getTop());
		
		if (latest_file != INVALID_HANDLE_VALUE) {
			win32::write_file(latest_file, header, size);
		}
		
		if (stream.connected) {
			stream.write(header, size);
		}
		
	}
	
	DECLD u32 chunk_i = 0;
	NOINLINE_ void process_chunk (u64 qpc_begin, u64 qpc_end, cstr name, u32 index=0) {
		using namespace flamegraph_data_file;
		
		if (dropped_samples > 0) print(">> profile: dropped_samples %\n", dropped_samples);
		dropped_samples = 0;
		
		struct Header {
			File_Header	header;
			Thread		threads[THREAD_COUNT];
		};
		Header header = {};
		
		if (latest_file != INVALID_HANDLE_VALUE) {
			win32::set_filepointer(latest_file, 0);
			assert(!win32::read_file(latest_file, &header, sizeof(header)));
		}
		
		u32 events_to_process_indx;
		u32 events_to_process_count;
		{ // Get all written samples
			SYNC_SCOPED_MUTEX_LOCK(&mutex);
			u32 events_buffered = MAX_SAMPLES -samples_remain;
			
			events_to_process_count = prev_chunk_count;
			events_to_process_indx = ((u32)(cur_sample -&samples[0]) +samples_remain) % MAX_SAMPLES;
			
			prev_chunk_count = events_buffered -events_to_process_count;
			
		}
		
		DEFER_POP(&working_stk);
		Chunk* chunk = working_stk.pushNoAlign<Chunk>();
		*chunk = prev_chunk;
		
		{
			//prev_chunk.event_data_size;	// write later
			prev_chunk.event_count =		0;
			prev_chunk.index =				index;
			prev_chunk.ts_begin =			qpc_begin -time::qpc_process_begin;
			prev_chunk.ts_length =			qpc_end -qpc_begin;
		}
		
		if (chunk_i++ == 0) return;
		
		working_stk.pushn(name, str::len(name) +1);
		
		Sample* cur = &samples[events_to_process_indx];
		for (u32 remain=events_to_process_count; remain>0; --remain) {
			
			Event* cpu = working_stk.pushNoAlign<Event>();
			cpu->ts = cur->qpc -time::qpc_process_begin;
			cpu->index = cur->index;
			cpu->thread_indx = 0; // engine_game_loop
			
			working_stk.pushn(cur->name, str::len(cur->name) +1);
			++chunk->event_count;
			++header.threads[0].event_count;
			
			if ((chunk_i -1) > 1) {
				Event* gpu = working_stk.pushNoAlign<Event>();
				
				glGetQueryObjectui64v(cur->gpu, GL_QUERY_RESULT, &gpu->ts);
				gpu->ts -= time::gpu_process_begin;
				
				gpu->index = cur->index;
				gpu->thread_indx = 1; // gpu_measurement
				
				working_stk.pushn(cur->name, str::len(cur->name) +1);
				++chunk->event_count;
				++header.threads[1].event_count;
				
			}
			
			cur = &samples[ ((u32)(cur -&samples[0]) +1) % MAX_SAMPLES ];
		}
		
		{ // consume the processed samples
			SYNC_SCOPED_MUTEX_LOCK(&mutex);
			samples_remain += events_to_process_count;
		}
		
		chunk->chunk_size = ptr_sub((byte*)chunk, working_stk.getTop());
		
		u64 old_filesize = header.header.file_size;
		
		header.header.file_size = -1; // write invalid filesize now, so that incomplete file updates maybe can be detected
		header.header.total_event_count += chunk->event_count;
		header.header.chunks_count += 1; 
		
		if (latest_file != INVALID_HANDLE_VALUE) {
			win32::set_filepointer(latest_file, 0);
			assert(!win32::write_file(latest_file, &header, sizeof(header)));
			
			win32::set_filepointer(latest_file, old_filesize);
			assert(!win32::write_file(latest_file, chunk, chunk->chunk_size));
			
			header.header.file_size = old_filesize +chunk->chunk_size;
			u64 filesize = win32::get_filepointer(latest_file);
			assert(filesize == header.header.file_size);
			
			win32::set_filepointer(latest_file, 0);
			assert(!win32::write_file(latest_file, &header, sizeof(header)));
		}
		
		if (stream.connected) {
			if (stream.poll_write_avail()) { // drop frame data chunks if our write operation would block, so we don't stall our framerate
				//print(">>> chunk '%' %, size %\n", name, index, chunk->chunk_size);
				stream.write(chunk, chunk->chunk_size);
			} else {
				if (!stream.connected) {
					stream.disconnect_from_server();
				} else {
					print(">>> chunk '%' % dropped\n", name, index);
				}
			}
		}
	}
	
	void finish_file () {
		
		if (latest_file != INVALID_HANDLE_VALUE) {
			win32::close_handle(latest_file);
			
			cstr date_f_filename;
			{
				lstr exe_path =			win32::get_exe_path(&working_stk);
				lstr exe_filename =		win32::find_exe_filename(exe_path);
				lstr pc_name =			win32::get_pc_name(&working_stk);
				
				SYSTEMTIME loc;
				GetLocalTime(&loc);
				
				date_f_filename = print_working_stk("profiling/%.%.%-%.%.%_%_%.profile\\0",
					loc.wYear, loc.wMonth, loc.wDay,
					loc.wHour, loc.wMinute, loc.wSecond,
					exe_filename, pc_name ).str;
				
			}
			
			{
				auto ret = CopyFile(LATEST_FILE_FILENAME, date_f_filename, FALSE);
				assert(ret != 0, "[%]", GetLastError());
			}
		}
		
		if (stream.connected) {
			stream.disconnect_from_server();
		}
		winsock::cleanup();
		
	}
	
	void record_zerotime () {
		QueryPerformanceCounter((LARGE_INTEGER*)&qpc_zerotime);
	}
	void record_zerotime (u64 qpc) {
		qpc_zerotime = qpc;
	}
	
	#define PROFILE_BEGIN(thr, name, ...)			profile::record_sample("{" name, __VA_ARGS__)
	#define PROFILE_END(thr, name, ...)				profile::record_sample("}" name, __VA_ARGS__)
	#define PROFILE_STEP(thr, name, ...)			profile::record_sample("|" name, __VA_ARGS__)
	
	#define PROFILE_BEGIN_M(thr, qpc, name, ...)	profile::record_sample(qpc, "{" name, __VA_ARGS__)
	#define PROFILE_END_M(thr, qpc, name, ...)		profile::record_sample(qpc, "}" name, __VA_ARGS__)
	#define PROFILE_STEP_M(thr, qpc, name, ...)		profile::record_sample(qpc, "|" name, __VA_ARGS__)
	
	#define PROFILE_SCOPED(thr, name, ...) \
			profile::record_sample("{" name, __VA_ARGS__); \
			defer_by_val { profile::record_sample("}" name, __VA_ARGS__); } // defer by value (capture)
	
}
