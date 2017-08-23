
#define WORKING_STK_CAP mebi<uptr>(32)
#include "startup.hpp"

#include "platform.hpp"

DECL app_main () {
	return platform::msg_thread_main();
}
