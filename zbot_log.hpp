#ifndef ZBOT_LOG_HPP
#define ZBOT_LOG_HPP

#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/keywords/channel.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <cstdarg>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;
namespace zbot {
	enum class severity_level : uint_fast16_t
	{
		e_debug = 1U << 0U,
		e_notification = 1U << 1U,
		e_info = 1U << 2U,
		e_warning = 1U << 3U,
		e_error = 1U << 4U,
		e_critical = 1U << 5U,
	};

	typedef src::severity_channel_logger_mt<
		severity_level,     // the type of the severity level
		std::string         // the type of the channel name
	> my_logger_mt;


#define ZBOT_LOG_INLINE_GLOBAL_LOGGER_INIT(logger_tag, module_name_string)\
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(logger_tag, my_logger_mt)\
    {\
        return my_logger_mt(keywords::channel = module_name_string);\
    }

#define ZLOG(logger_tag, level, strFmtString, ...) \
	do{											   \
		char data[1024];                           \
        snprintf(data, 1024-1, strFmtString, __VA_ARGS__);\
		BOOST_LOG_SEV(logger_tag::get(), level) << data;\
	} while(0)


}
#endif // !ZBOT_LOG_HPP
