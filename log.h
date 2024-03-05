#ifndef __LOG_H_
#define __LOG_H_
#include <iostream>
#include <format>
#include <source_location>
#include <cstdint>
#include <fstream>
#include <chrono>
namespace LOG
{
// f代表函数 XMACRO技术
#define LOG_FOREACH_LOG_LEVEL(f) \
    f(trace)                     \
        f(debug)                 \
            f(info)              \
                f(critical)      \
                    f(warning)   \
                        f(error) \
                            f(fatal)
    enum class log_level : std::uint8_t
    {
#define _FUNCTION(name) name, // 带进去自动展开 _FUNCTION=>f, f(trace) => trace,
        LOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
    };

#if defined(__linux__) || defined(__APPLE__) // Linux 终端颜色显示
    inline constexpr char k_level_ansi_colors[(std::uint8_t)log_level::fatal + 1][8] = {
        "\E[37m",
        "\E[35m",
        "\E[32m",
        "\E[34m",
        "\E[33m",
        "\E[31m",
        "\E[31;1m",
    };
    inline constexpr char k_reset_ansi_color[4] = "\E[m";
#define _MINILOG_IF_HAS_ANSI_COLORS(x) x
#else
#define _MINILOG_IF_HAS_ANSI_COLORS(x)
    inline constexpr char k_level_ansi_colors[(std::uint8_t)log_level::fatal + 1][1] = {
        "",
        "",
        "",
        "",
        "",
        "",
        "",
    };
    inline constexpr char k_reset_ansi_color[1] = "";
#endif

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

        inline log_level log_level_from_name(std::string lev)
        {
#define _FUNCTION(name) \
    if (lev == #name)   \
        return log_level::name;
            LOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
            return log_level::info;
        }

        template <class T>
        struct with_source_location // 默认参数初始化会自动定位到行号，但是Arg展开包，影响到默认展开技术
                                    // 包装一层
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

        // static log_level g_max_level = log_level::error;
        inline log_level g_max_level = []() -> log_level // static initialization 内联初始化 声明就初始化
        {
            if (auto lev = std::getenv("LOG_LEVEL"))
            {
                return detail::log_level_from_name(lev);
            }
            return log_level::info;
        }();

        inline std::ofstream g_log_file = []() -> std::ofstream // static initialization 内联初始化 声明就初始化
        {
            if (auto path = std::getenv("LOG_FILE"))
            {
                return std::ofstream(path, std::ios::app);
            }
            return std::ofstream();
        }();

        // inline std::ofstream g_log_file{"mini.log"};
        inline void output_log(log_level lev, std::string msg, std::source_location const &loc)
        {
            std::chrono::zoned_time now{std::chrono::current_zone(), std::chrono::high_resolution_clock::now()}; // 时间戳

            msg = std::format("{} {} {} {} {}", now, loc.file_name(), loc.line(), detail::log_level_name(lev), msg);
            if (lev >= g_max_level)
            {
                std::cout << k_level_ansi_colors[(std::uint8_t)lev] + msg + k_reset_ansi_color + '\n';
            }

            if (g_log_file)
            {
                g_log_file << msg + '\n';
            }
        }

        template <class... Args>
        void log(log_level level, with_source_location<std::format_string<Args...>> fmt, Args &&...args)
        {
            // if (g_max_level >= level)
            // {
            //     auto const &loc = fmt.location();
            //     std::cout << loc.file_name() << ":" << loc.line() << " [" << log_level_name(level) << "] " << std::vformat(fmt.format().get(), std::make_format_args(args...)) << std::endl;
            // }
            auto const &loc = fmt.location();
            auto msg = std::vformat(fmt.format().get(), std::make_format_args(args...));
            detail::output_log(level, std::move(msg), loc);
        }

    }

    inline void set_log_file(std::string path)
    {
        detail::g_log_file = std::ofstream(path, std::ios::app);
    }

    inline void set_log_level(log_level lev)
    {
        detail::g_max_level = lev;
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
