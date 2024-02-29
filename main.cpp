#include <iostream>
#include <format>
#include <source_location>
#include <cstdint>

// f代表函数 XMACRO技术
#define FOREACH_LOG_LEVEL(f)     \
    f(trace)                     \
        f(debug)                 \
            f(info)              \
                f(critical)      \
                    f(warn)      \
                        f(error) \
                            f(fatal)

enum class log_level : std::uint8_t
{
#define _FUNCTION(name) name, // 带进去自动展开 _FUNCTION=>f, f(trace) => trace,
    FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
};

std::string log_level_name(log_level lev)
{
    switch (lev)
    {
#define _FUNCTION(name)   \
    case log_level::name: \
        return #name; // 代表返回字符串
        FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
    }
}
template <class T>
struct with_source_location
{
private:
    T inner;
    std::source_location loc;

public:
    template <class U>
        requires std::constructible_from<T, U>                                                            // C++20 特性 要求T能从u构造
    consteval with_source_location(U &&inner, std::source_location loc = std::source_location::current()) // 利用consteval来保证在编译期间执行
        : inner(std::forward<U>(inner)), loc(loc)                                                         // 利用移动语义来避免不必要的拷贝，forward实现完美转发
    {
    }
    constexpr const T &format() const { return inner; }
    constexpr const std::source_location &location() const { return loc; }
};

static log_level max_level = log_level::error;
template <class... Args>
void log(log_level level, with_source_location<std::format_string<Args...>> fmt, Args &&...args)
{
    if (max_level >= level)
    {
        auto const &loc = fmt.location();
        std::cout << loc.file_name() << ":" << loc.line() << " [" << log_level_name(level) << "] " << std::vformat(fmt.format().get(), std::make_format_args(args...)) << std::endl;
    }
}

/*
    第二个括号不能加 \ ，否则变成整个一句话了
*/
#define _FUNCTION(name)                                                                    \
    template <class... Args>                                                               \
    void log_##name(with_source_location<std::format_string<Args...>> fmt, Args &&...args) \
    {                                                                                      \
        return log(log_level::name, std::move(fmt), std::forward<Args>(args)...);          \
    }
FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION

int main()
{
    log(log_level::debug, "Hello, {}!", "debug");
    log(log_level::error, "Hello, {}!", "error");
    log_debug("Hello, {}!", "debug");
    return 0;
}