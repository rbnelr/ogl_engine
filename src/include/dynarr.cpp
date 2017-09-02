
template <typename T, typename LEN_T> DECLM array<T, LEN_T> array<T, LEN_T>:: alloc (LEN_T len) {
	array ret;
	ret.arr = (T*)::malloc(len*sizeof(T));
	ret.len = len;
	
	#if DBG_MEMORY
	cmemset(ret.arr, DBG_MEM_UNALLOCATED_BYTE, len*sizeof(T));
	#endif
	return ret;
}
template <typename T, typename LEN_T> DECLM void array<T, LEN_T>:: free () {
	::free(this->arr);
	#if DBG_INTERNAL
	this->arr = 0xdeadbeefDEADBEEF; // to catch use after free
	#endif
}

template <typename T, typename LEN_T> DECLM void dynarr<T, LEN_T>:: _realloc (LEN_T cap) {
	
	this->cap = cap;
	this->arr = (T*)::realloc(this->arr, this->cap*sizeof(T));
	
	#if DBG_MEMORY
	cmemset(arr, DBG_MEM_UNALLOCATED_BYTE, this->cap*sizeof(T));
	#endif
	
}
template <typename T, typename LEN_T> DECLM void dynarr<T, LEN_T>:: free () {
	::free(this->arr);
	*this = null();
}

DECL void _dyn_array_generic__init (dynarr<byte>* arr, u32 init_len, u32 init_cap, uptr stride) {
	
	arr->len = init_len;
	assert(init_cap >= init_len);
	arr->cap = init_cap;
	
	arr->cap = alignup_power_of_two(arr->cap);
	
	arr->arr = (byte*)malloc(arr->cap*stride);
	
	#if DBG_MEMORY
	cmemset(arr->arr, DBG_MEM_UNALLOCATED_BYTE, arr->cap*stride);
	#endif
	
}

DECL byte* _dyn_array_generic__index (dynarr<byte>* arr, u32 indx, uptr stride) {
	return arr->arr +(indx*stride);
}

DECL void _dyn_array_generic__resize_cap (dynarr<byte>* arr, u32 new_cap, uptr stride) {
	
	if (new_cap != arr->cap) {
		//assert(this->arr != nullptr); // Might be the correct to happen if we do realloc with size zero
		arr->arr = (byte*)realloc((void*)arr->arr, new_cap*stride);
		
		#if DBG_MEMORY
		if (new_cap > arr->cap) {
			cmemset(arr->arr +(arr->cap * stride), DBG_MEM_UNALLOCATED_BYTE, (new_cap -arr->cap)*stride);
		}
		#endif
		
		arr->cap = new_cap;
	}
}

DECL void _dyn_array_generic__fit_cap (dynarr<byte>* arr, uptr stride) {
	_dyn_array_generic__resize_cap( arr, alignup_power_of_two(arr->len), stride );
}
DECL byte* _dyn_array_generic__grow_to (dynarr<byte>* arr, u32 new_len, uptr stride) {
	u32 old_len = arr->len;
	arr->len = new_len;
	if (arr->len > arr->cap) {
		_dyn_array_generic__fit_cap(arr, stride);
	}
	#if DBG_MEMORY
	if (arr->len > old_len) {
		cmemset(&arr->arr[old_len*stride], DBG_MEM_UNINITIALIZED_BYTE, (arr->len -old_len)*stride);
	}
	#endif
	return _dyn_array_generic__index(arr, old_len, stride);
}

DECL byte* _dyn_array_generic__grow_by (dynarr<byte>* arr, u32 diff, uptr stride) {
	return _dyn_array_generic__grow_to(arr, arr->len +diff, stride);
}
DECL byte* _dyn_array_generic__append (dynarr<byte>* arr, uptr stride) {
	return _dyn_array_generic__grow_by(arr, 1, stride);
}
