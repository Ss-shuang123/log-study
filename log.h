#ifndef __LOG_H_
#define __LOG_H_
#include <iostream>
#include <format>
#include <source_location>
#include <cstdint>

namespace LOG
{
#define LOG_FOREACH_LOG_LEVEL(f) \
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
        LOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
    };

    namespace detail
    {

        inline std::string log_level_name(log_level lev)
        {
            switch (lev)
            {
#define _FUNCTION(name)   \
    case log_level::name: \
        return #name; // 代表返回字符串
                LOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
            }
        }
        // f代表函数 XMACRO技术

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

        static log_level g_max_level = log_level::error;
        template <class... Args>
        void log(log_level level, with_source_location<std::format_string<Args...>> fmt, Args &&...args)
        {
            if (g_max_level >= level)
            {
                auto const &loc = fmt.location();
                std::cout << loc.file_name() << ":" << loc.line() << " [" << log_level_name(level) << "] " << std::vformat(fmt.format().get(), std::make_format_args(args...)) << std::endl;
            }
        }
    }

    inline void set_log_lev(log_level level)
    {
        detail::g_max_level = level;
    }

/*
    第二个括号不能加 \ ，否则变成整个一句话了
*/
#define _FUNCTION(name)                                                                            \
    template <class... Args>                                                                       \
    void log_##name(detail::with_source_location<std::format_string<Args...>> fmt, Args &&...args) \
    {                                                                                              \
        return log(log_level::name, std::move(fmt), std::forward<Args>(args)...);                  \
    }
    LOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
}
#endif /* __LOG_H_ */
