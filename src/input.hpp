
enum mouselook_e : u32 {
	FPS_MOUSELOOK=0b01,
	GUI_MOUSELOOK=0b10,
};

namespace input {
	
	// button states are c++ bools, ie. false=0 and true=1
	// false:	button up
	// true:	button down
	
	typedef s32						freecam_dir_t;
	typedef tv3<freecam_dir_t>		freecam_dir_v;
	
	namespace buttons {
		enum button_e : u8 {
			_FIRST_BUTTON=0,
			
			// Mouse buttons
			LMB=0, MMB, RMB, MB4, MB5,
			// Keyboard buttons
			L_CTRL, R_CTRL,
			L_SHIFT, R_SHIFT,
			L_ALT, R_ALT,
			L_LOGO, R_LOGO,
			ENTER, NP_ENTER,
			APP, SPACE, CAPSLOCK, TABULATOR,
			BACKSPACE,
			LEFT, DOWN, RIGHT, UP,
			A, B, C, D, E, F, G, H, I, J, K, L, M,
			N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
			N_1, N_2, N_3, N_4, N_5, N_6, N_7, N_8, N_9, N_0,
			F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
			NP_0, NP_1, NP_2, NP_3, NP_4, NP_5, NP_6, NP_7, NP_8, NP_9,
			
			_NUM_BUTTONS
		};
		DEFINE_ENUM_ITER_OPS(button_e, u8)
		
		constexpr cstr buttonCStrs[_NUM_BUTTONS] = {
			// Mouse buttons
			"LMB", "MMB", "RMB", "MB4", "MB5",
			// Keyboard buttons
			"L_CTRL", "R_CTRL", "L_SHIFT", "R_SHIFT",
			"L_ALT", "R_ALT", "L_LOGO", "R_LOGO", "APP",
			"SPACE", "CAPSLOCK", "TABULATOR", "ENTER", "NP_ENTER", "BACKSPACE",
			"LEFT", "DOWN", "RIGHT", "UP",
			"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
			"N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
			"N_1", "N_2", "N_3", "N_4", "N_5", "N_6", "N_7", "N_8", "N_9", "N_0",
			"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
			"NP_0", "NP_1", "NP_2", "NP_3", "NP_4", "NP_5", "NP_6", "NP_7", "NP_8", "NP_9"
		};
		
		DECLD constexpr button_e	_FIRST_MOUSE_BUTTON =	LMB;
		DECLD constexpr button_e	_MAX_MOUSE_BUTTON =		(button_e)(MB5 +1);
		DECLD constexpr u8			_NUM_MOUSE_BUTTON =		_MAX_MOUSE_BUTTON -_FIRST_MOUSE_BUTTON;
	}
	
	namespace button_bindings {
		enum group_e : u8 {
			UNBOUND=0,
			MISC,
			MOUSELOOK,
			FREECAM,
			CAMSAVE,
			FOV_CONTROL,
			TIME_CONTROL,
			GRAPHICS,
			EDITOR,
		};
		enum function_e : u8 {
			NONE=0,
			MISC_RELOAD_SHADERS=0,
			MISC_SAVE_STATE,
			MOUSELOOK_ENABLE_IN_GUI=0,
			FREECAM_LEFT=0,
			FREECAM_RIGHT,
			FREECAM_BACKWARD,
			FREECAM_FORWARD,
			FREECAM_DOWN,
			FREECAM_UP,
			FREECAM_FAST,
			CAMSAVE_RESTORE_SAVE,
			CAMSAVE_SLOT_0,
			CAMSAVE_SLOT_1,
			CAMSAVE_SLOT_2,
			CAMSAVE_SLOT_3,
			CAMSAVE_SLOT_4,
			CAMSAVE_SLOT_5,
			CAMSAVE_SLOT_6,
			CAMSAVE_SLOT_7,
			CAMSAVE_SLOT_8,
			CAMSAVE_SLOT_9,
			FOV_CONTROL_LOWER=0,
			FOV_CONTROL_MW,
			FOV_CONTROL_RAISE,
			TIME_CONTROL_TOGGLE_TOD_PAUSE=0,
			TIME_CONTROL_TOGGLE_LIGHTS_PAUSE,
			TIME_CONTROL_DECELERATE,
			TIME_CONTROL_ACCELERATE,
			TIME_CONTROL_MANUAL_BACKWARD,
			TIME_CONTROL_MANUAL_FORWARD,
			GRAPHICS_SWITCH_ENV_DECREASE=0,
			GRAPHICS_SWITCH_ENV_INCREASE,
			GRAPHICS_TOGGLE_ENV,
			GRAPHICS_POST_EXPOSURE_MW,
			GRAPHICS_POST_MANUAL_EXPOSURE_TOGGLE,
			EDITOR_CLICK_SELECT=0,
			EDITOR_ENTITY_CONTROL,
		};
		struct Binding {
			group_e			group;
			function_e		function;
		};
		
		#define BUTTON_BINDINGS \
				X( C,			MISC,			RELOAD_SHADERS ) \
				X( B,			MISC,			SAVE_STATE ) \
				X( RMB,			MOUSELOOK,		ENABLE_IN_GUI ) \
				X( A,			FREECAM,		LEFT ) \
				X( D,			FREECAM,		RIGHT ) \
				X( S,			FREECAM,		BACKWARD ) \
				X( W,			FREECAM,		FORWARD ) \
				X( L_CTRL,		FREECAM,		DOWN ) \
				X( SPACE,		FREECAM,		UP ) \
				X( L_SHIFT,		FREECAM,		FAST ) \
				X( TABULATOR,	CAMSAVE,		RESTORE_SAVE ) \
				X( N_0,			CAMSAVE,		SLOT_0 ) \
				X( N_1,			CAMSAVE,		SLOT_1 ) \
				X( N_2,			CAMSAVE,		SLOT_2 ) \
				X( N_3,			CAMSAVE,		SLOT_3 ) \
				X( N_4,			CAMSAVE,		SLOT_4 ) \
				X( N_5,			CAMSAVE,		SLOT_5 ) \
				X( N_6,			CAMSAVE,		SLOT_6 ) \
				X( N_7,			CAMSAVE,		SLOT_7 ) \
				X( N_8,			CAMSAVE,		SLOT_8 ) \
				X( N_9,			CAMSAVE,		SLOT_9 ) \
				X( NP_7,		FOV_CONTROL,	LOWER ) \
				X( NP_8,		FOV_CONTROL,	MW ) \
				X( NP_9,		FOV_CONTROL,	RAISE ) \
				X( U,			TIME_CONTROL,	TOGGLE_LIGHTS_PAUSE ) \
				X( I,			TIME_CONTROL,	TOGGLE_TOD_PAUSE ) \
				X( O,			TIME_CONTROL,	DECELERATE ) \
				X( P,			TIME_CONTROL,	ACCELERATE ) \
				X( K,			TIME_CONTROL,	MANUAL_BACKWARD ) \
				X( L,			TIME_CONTROL,	MANUAL_FORWARD ) \
				X( T,			GRAPHICS,		SWITCH_ENV_DECREASE ) \
				X( Y,			GRAPHICS,		SWITCH_ENV_INCREASE ) \
				X( R,			GRAPHICS,		TOGGLE_ENV ) \
				X( E,			GRAPHICS,		POST_EXPOSURE_MW ) \
				X( F,			GRAPHICS,		POST_MANUAL_EXPOSURE_TOGGLE ) \
				X( LMB,			EDITOR,			CLICK_SELECT ) \
				X( H,			EDITOR,			ENTITY_CONTROL )
		
		DECL Binding getButtonBinding (buttons::button_e button) {
			using namespace buttons;
			switch (button) {
				
				#define X(key, binding_group, binding_func) \
						case key:	return { binding_group, binding_group##_##binding_func };
				
				BUTTON_BINDINGS
				
				#undef X
				
				default:		return { UNBOUND, NONE };
			}
		}
		
	}
	
	namespace print_button_bindings {
		
		#define X(key, binding_group, binding_func) buttons::key,
		DECLD constexpr u8 bound_key_arr[] = {
			BUTTON_BINDINGS
		};
		DECLD constexpr uptr count = arrlenof(bound_key_arr);
		#undef X
		
		#define X(key, binding_group, binding_func) TO_STR(binding_group) "." TO_STR(binding_func),
		DECLD constexpr char const* str_arr[] = {
			BUTTON_BINDINGS
		};
		static_assert(count == arrlenof(str_arr), "");
		#undef X
		
		DECL void print () {
			::print("Button bindings:\n");
			for (uptr i=0; i<count; ++i) {
				::print("  bound % to %\n", buttons::buttonCStrs[bound_key_arr[i]], str_arr[i]);
			}
		}
	}
	
	#undef BUTTON_BINDINGS
	
	struct State {
		
		bool			misc_save_state;
		bool			misc_reload_shaders;
		
		resolution_v	mouselook_accum;
		u32				mouselook;
		
		s32				mouse_wheel_accum;
		
		freecam_dir_v	freecam_dir;
		bool			freecam_fast;
		
		si				fov_control_dir;
		bool			fov_control_mw;
		
		bool			camsave_restore_save;
		u32				camsave_num_counter;
		bool			camsave_num_state;
		u32				camsave_slot;
		
		ui				lights_pause_toggle_counter;
		ui				time_of_day_pause_toggle_counter;
		si				time_of_day_log_accel_dir;
		si				time_of_day_manual_dir;
		
		si				graphics_switch_env_accum;
		ui				graphics_toggle_env_counter;
		bool			graphics_post_exposure_mw;
		
		bool			editor_click_select_state;
		u32				editor_click_select_counter;
		bool			editor_entity_control;
		bool			editor_entity_control_changed;
		
		DECLM void clear_accums () {
			
			misc_save_state =					false;
			misc_reload_shaders =				false;
			
			mouselook_accum =					resolution_v(0);
			mouse_wheel_accum =					0;
			
			camsave_num_counter =				0;
			
			lights_pause_toggle_counter =		0;
			time_of_day_pause_toggle_counter =	0;
			
			graphics_switch_env_accum =			0;
			graphics_toggle_env_counter =		0;
			
			editor_click_select_counter =		0;
			editor_entity_control_changed =		false;
		}
		
		DECLM void process_button_binding_input (buttons::button_e button, bool down) {
			using namespace button_bindings;
			
			//print("button % (%) %\n", buttons::buttonCStrs[button], (u32)button, down? "down":"up");
			
			auto binding = getButtonBinding(button);
			if (binding.group == UNBOUND) {
				return;
			}
			
			switch (binding.group) {
				
				case MISC: {
					switch (binding.function) {
						
						case MISC_RELOAD_SHADERS: {
							if (down) {
								misc_reload_shaders = true;
							}
						} break;
						
						case MISC_SAVE_STATE: {
							if (down) {
								misc_save_state = true;
							}
						} break;
						
						default: assert(false);
					}
				} break;
				
				case MOUSELOOK: {
					switch (binding.function) {
						
						case MOUSELOOK_ENABLE_IN_GUI: {
							mouselook &= ~GUI_MOUSELOOK;
							mouselook |= down ? GUI_MOUSELOOK : 0;
						} break;
						
						default: assert(false);
					}
				} break;
				
				#if 0
				case MOUSE: {
					
					switch (binding.function) {
						
						case MOUSE_LMB: {
							lmb_down = down;
							++lmb_count;
						} break;
						case MOUSE_RMB: {
							rmb_down = down;
							++rmb_count;
						} break;
						
						default: assert(false);
					}
					
				} break;
				#endif
				
				case FREECAM: {
					
					si thing = down ? +1 : -1;
					switch (binding.function) {
						
						case FREECAM_LEFT: {
							freecam_dir.x -= thing;
						} break;
						case FREECAM_RIGHT: {
							freecam_dir.x += thing;
						} break;
						
						case FREECAM_FORWARD: {
							freecam_dir.z -= thing;
						} break;
						case FREECAM_BACKWARD: {
							freecam_dir.z += thing;
						} break;
						
						case FREECAM_DOWN: {
							freecam_dir.y -= thing;
						} break;
						case FREECAM_UP: {
							freecam_dir.y += thing;
						} break;
						
						case FREECAM_FAST: {
							freecam_fast = down;
						} break;
						
						default: assert(false);
					}
				} break;
				
				case CAMSAVE: {
					
					si thing = down ? +1 : -1;
					switch (binding.function) {
						
						case CAMSAVE_RESTORE_SAVE: {
							camsave_restore_save = down;
						} break;
						case CAMSAVE_SLOT_0:
						case CAMSAVE_SLOT_1:
						case CAMSAVE_SLOT_2:
						case CAMSAVE_SLOT_3:
						case CAMSAVE_SLOT_4:
						case CAMSAVE_SLOT_5:
						case CAMSAVE_SLOT_6:
						case CAMSAVE_SLOT_7:
						case CAMSAVE_SLOT_8:
						case CAMSAVE_SLOT_9: {
							++camsave_num_counter;
							camsave_num_state = down;
							camsave_slot = binding.function -CAMSAVE_SLOT_0;
						} break;
						
						default: assert(false);
					}
				} break;
				
				case FOV_CONTROL: {
					si thing = down ? +1 : -1;
					switch (binding.function) {
						
						case FOV_CONTROL_LOWER: {
							fov_control_dir -= thing;
						} break;
						case FOV_CONTROL_MW: {
							fov_control_mw = down;
						} break;
						case FOV_CONTROL_RAISE: {
							fov_control_dir += thing;
						} break;
						
						default: assert(false);
					}
					
				} break;
				
				case TIME_CONTROL: {
					si thing = down ? +1 : -1;
					switch (binding.function) {
						
						case TIME_CONTROL_DECELERATE: {
							time_of_day_log_accel_dir -= thing;
						} break;
						case TIME_CONTROL_ACCELERATE: {
							time_of_day_log_accel_dir += thing;
						} break;
						case TIME_CONTROL_TOGGLE_TOD_PAUSE: {
							if (down == true) {
								++time_of_day_pause_toggle_counter;
							}
						} break;
						case TIME_CONTROL_TOGGLE_LIGHTS_PAUSE: {
							if (down == true) {
								++lights_pause_toggle_counter;
							}
						} break;
						case TIME_CONTROL_MANUAL_BACKWARD: {
							time_of_day_manual_dir -= thing;
						} break;
						case TIME_CONTROL_MANUAL_FORWARD: {
							time_of_day_manual_dir += thing;
						} break;
						
						default: assert(false);
					}
					
				} break;
				
				case GRAPHICS: {
					switch (binding.function) {
						
						case GRAPHICS_SWITCH_ENV_DECREASE: {
							if (down) {
								--graphics_switch_env_accum;
							}
						} break;
						case GRAPHICS_SWITCH_ENV_INCREASE: {
							if (down) {
								++graphics_switch_env_accum;
							}
						} break;
						case GRAPHICS_TOGGLE_ENV: {
							if (down) {
								++graphics_toggle_env_counter;
							}
						} break;
						case GRAPHICS_POST_EXPOSURE_MW: {
							graphics_post_exposure_mw = down;
						} break;
						case GRAPHICS_POST_MANUAL_EXPOSURE_TOGGLE: {
							
						} break;
						
						default: assert(false);
					}
				} break;
				
				case EDITOR: {
					switch (binding.function) {
						
						case EDITOR_CLICK_SELECT: {
							editor_click_select_state = down;
							++editor_click_select_counter;
						} break;
						
						case EDITOR_ENTITY_CONTROL: {
							editor_entity_control = down;
							editor_entity_control_changed = true;
						} break;
						
						default: assert(false);
					}
				} break;
				
				case UNBOUND:	assert(false);
				default:		assert(false);
			}
		}
		
		DECLM void process_mouse_rel (resolution_v diff) {
			if (mouselook) {
				mouselook_accum += diff;
			}
		}
		
		DECLM void process_mouselook_change (bool mouselook_enabled) {
			mouselook &= ~FPS_MOUSELOOK;
			mouselook |= mouselook_enabled ? FPS_MOUSELOOK : 0;
		}
		
		DECLM void process_mouse_wheel_rel_input (s32 diff) {
			mouse_wheel_accum += diff;
		}
	};
	
	DECLD State		shared_state = {};
	
	DECL void get_shared_state (State* state) {
		
		*state = shared_state;
		
		shared_state.clear_accums();
	}
	
	// Input that comes that is queried directly in game loop
	struct Sync_Input {
		
		resolution_v		window_res;
		resolution_v		mouse_cursor_pos;
		bool				window_res_changed;
		bool				mouse_cursor_in_window;
		
	};
}
