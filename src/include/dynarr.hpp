
#define DBG_DO_MEMSET DBG_MEMORY
#define DYNARR_RANGE_CHECK DEBUG

template <typename T, typename LEN_T=u32>
struct array {
	T*		arr;
	LEN_T	len;
	
	DECLM void alloc (LEN_T init_len);
	
	DECLM FORCEINLINE array () {}
	DECLM array (LEN_T init_len) {
		alloc(init_len);
	}
	
	DECLM void free ();
	
	DECLM T cr operator[] (LEN_T indx) const {
		#if ARRAYS_BOUNDS_ASSERT
		assert(indx < len, "operator[]: indx: % len: %", indx, len);
		#endif
		return arr[indx];
	}
	DECLM T& operator[] (LEN_T indx) {
		#if ARRAYS_BOUNDS_ASSERT
		assert(indx < len, "operator[]: indx: % len: %", indx, len);
		#endif
		return arr[indx];
	}
	
	#if 0
	DECLM T cr get_last () const {
		#if ARRAYS_BOUNDS_ASSERT
		assert(len > 0, "array::get_last len: %", len);
		#endif
		return arr[len -1];
	}
	DECLM T& get_last () {
		#if ARRAYS_BOUNDS_ASSERT
		assert(len > 0, "array::get_last len: %", len);
		#endif
		return arr[len -1];
	}
	#endif
	
	DECLM T* copy_values (LEN_T dst_indx, T const* source_ptr, LEN_T count) {
		return (T*)cmemcpy(&arr[dst_indx], source_ptr, count * sizeof(T));
	}
	
	#if 0
	template <typename F>
	void templ_forall (F func) const {
		for (LEN_T i=0; i<len; ++i) {
			func(arr[i]);
		}
	}
	#endif
	
};

template <typename T, typename LEN_T=u32>
struct dynarr : public array<T, LEN_T> { // 'this->' everywhere to fix 'lookup into dependent bases' problem
	LEN_T	cap;
	
	DECLM void alloc (LEN_T init_len, LEN_T init_cap);
	DECLM void alloc (LEN_T init_cap);
	
	constexpr dynarr (T* p, LEN_T l, LEN_T c): array<T, LEN_T>{p, l}, cap{c} {}
	
	//#if DBG_INTERNAL
	//dynarr (): array{nullptr, 0} {} // 
	//#else
	DECLM FORCEINLINE dynarr () {}
	//#endif
	
	DECLM dynarr (LEN_T init_len, LEN_T init_cap) {
		alloc(init_len, init_cap);
	}
	DECLM dynarr (LEN_T init_cap) {
		alloc(init_cap);
	}
	
	DECLM void free ();
	
	// 
	// CAUTION!
	// 
	// Any references or pointers to the actual values in arr[] returned by these methods
	//	could become invalid after any methods that realloc the arr
	//	because the allocation block could be moved by the allocator
	//
	
	template<u32 ALIGN>
	DECLM LEN_T align_len () {
		STATIC_ASSERT(ALIGN >= sizeof(T));
		assert(is_aligned(this->arr, ALIGN));
		
		LEN_T new_len = align_up(len*sizeof(T), ALIGN) -arr;
		return new_len -this->len;
	}
	
	DECLM void _resize_cap (LEN_T new_cap);
	
	DECLM void fit_cap (LEN_T req_cap) {
		_resize_cap( alignup_power_of_two(req_cap) );
	}
	DECLM void fit_cap () { // fit cap to length
		fit_cap(this->len);
	}
	#if 0
	DECLM void fit_cap_exact () {	// realloc to the excact size required,
									//	if you don't intend to push for some time or ever again, to reduce memory wasteage
		_resize_cap( this->len );
	}
	#endif
	
	DECLM void clear (LEN_T reset_len, LEN_T reset_cap) { // Can be called instead of alloc()
		
		if (this->arr) {
			
			assert(reset_len <= reset_cap);
			fit_cap(reset_cap);
			
			this->len = reset_len;
			
		} else {
			alloc(reset_len, reset_cap);
		}
	}
	
	DECLM T* grow_to (LEN_T new_len) {
		LEN_T old_len = this->len;
		assert(new_len >= old_len);
		if (new_len > cap) {
			fit_cap(new_len);
		}
		assert(new_len <= cap, "% %", new_len, cap);
		#if DBG_MEMORY
		cmemset(&this->arr[old_len], DBG_MEM_UNINITIALIZED_BYTE, (new_len -old_len)*sizeof(T));
		#endif
		this->len = new_len;
		
		return &arr[old_len];
	}
	DECLM void shrink_to (LEN_T new_len) {
		assert(new_len <= cap, "% %", new_len, cap);
		
		LEN_T old_len = this->len;
		assert(new_len <= old_len);
		// Do nothing (no resizing happens on shrinking, if desired capacity fitting should be done manually by calling fit_cap())
		this->len = new_len;
	}
	DECLM void len_to (LEN_T new_len) {
		if (this_len > this->len) {
			grow_to(new_len);
		} else {
			shrink_to(new_len);
		}
	}
	
	DECLM bool will_overflow (LEN_T diff) { // overflow of length variable
		return !safe_add(this->len, diff);
	}
	DECLM bool will_overflow_align (LEN_T diff, LEN_T align) {
		return !safe_add(this->len, diff, align);
	}
	template<u32 ALIGN>
	DECLM bool will_overflow_align (LEN_T diff) {
		return !safe_add(this->len, diff, align_len<ALIGN>());
	}
	
	DECLM T* grow_by (LEN_T diff) {
		#if DYNARR_RANGE_CHECK
		assert(!will_overflow(diff), "dynarr LEN_T overflow");
		#endif
		return grow_to(this->len +diff);
	}
	#if 0
	DECLM T* grow_by_unchecked (LEN_T diff) {
		return grow_to(this->len +diff);
	}
	
	template<u32 ALIGN>
	DECLM T* grow_by_align (LEN_T diff) {
		auto align = align_len<ALIGN>();
		assert(!will_overflow_align(diff, align));
		
		LEN_T old_len = len +align;
		grow_to_align(this->len +align +diff);
		return &this->arr[old_len];
	}
	template<u32 ALIGN>
	DECLM T* grow_by_align_unchecked (LEN_T diff) {
		auto align = align_len<ALIGN>();
		LEN_T old_len = this->len +align;
		grow_to_align(this->len +(diff +align));
		return &this->arr[old_len];
	}
	
	DECLM void resize_to_exactly (LEN_T new_len) {
		len = new_len;
		fit_cap_exact();
	}
	#endif
	
	DECLM T& push () {
		return *grow_by(1);
	}
	DECLM T& push (T cr val) {
		return push() = val;
	}
	
	DECLM T* pushn (LEN_T count) {
		LEN_T old_len = this->len;
		grow_by(count);
		return &this->arr[old_len];
	}
	DECLM T* pushn (T const* val, LEN_T count) {
		LEN_T old_len = this->len;
		grow_by(count);
		return copy_values(old_len, val, count);
	}
	
	#if 0
	template<u32 ALIGN>
	DECLM T& push_align () {
		return *grow_by_align<ALIGN>(1);
	}
	template<u32 ALIGN>
	DECLM T& push_align (T cr val) {
		return push_align<ALIGN>() = val;
	}
	
	template<u32 ALIGN>
	DECLM T* pushn_align (LEN_T count) {
		LEN_T old_len = this->len;
		return grow_by_align<ALIGN>(count);
	}
	template<u32 ALIGN>
	DECLM T* pushn_align (T const* val, LEN_T count) {
		LEN_T old_len = this->len;
		T* out = grow_by_align<ALIGN>(count);
		cmemcpy(out, val, count*sizeof(T));
		return out;
	}
	#endif
	
	DECLM void pop () {
		#if DYNARR_RANGE_CHECK
		assert(len >= 1, "dynarr::pop underflow (len is %, want to pop % elements)", len, 1);
		#endif
		grow_to(len -1);
	}
	DECLM void popn (LEN_T count) {
		#if DYNARR_RANGE_CHECK
		assert(len >= count, "dynarr::pop underflow (len is %, want to pop % elements)", len, count);
		#endif
		grow_to(len -count);
	}
	
};
