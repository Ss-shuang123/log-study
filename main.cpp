#include <iostream>
#include <format>
#include <source_location>
#include <cstdint>

enum class log_level : std::uint8_t
{
    trace,
    debug,
    info,
    warning,
    error,
    fatal
};

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

static log_level max_level = log_level::info;
template <class... Args>
void log(log_level level, with_source_location<std::format_string<Args...>> fmt, Args &&...args)
{
    if (max_level >= level)
    {
        auto const &loc = fmt.location();
        std::cout << loc.file_name() << ":" << loc.line() << " [Info] " << std::vformat(fmt.format().get(), std::make_format_args(args...)) << std::endl;
    }
}

int main()
{
    log(log_level::debug, "Hello, {}!", "world");
    log(log_level::error, "Hello, {}!", "world");
    return 0;
}