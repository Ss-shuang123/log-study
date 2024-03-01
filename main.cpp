#include "log.h"

int main()
{
    LOG::detail::log(LOG::log_level::debug, "Hello, {}!", "debug");
    LOG::detail::log(LOG::log_level::error, "Hello, {}!", "error");

    LOG::set_log_lev(LOG::log_level::error);
    LOG::log_debug("Hello, {}!", "debug");
    return 0;
}