
#include "global_includes.hpp"

void global_init () {
	
	assert_page_size();
	
	print_n::init();
	
	time::QPC::init();
}

struct Cmd_Line {
	u32		argc;
	cstr*	argv;
};
DECLD Cmd_Line cmd_line;

DECL s32 app_main ();

s32 main (s32 argc, cstr* argv) {
	
	cmd_line.argc = MAX(argc, 0);
	cmd_line.argv = argv;
	
	global_init();
	
	return app_main();
}
