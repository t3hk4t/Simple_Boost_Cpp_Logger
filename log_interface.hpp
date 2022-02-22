#ifndef LOG_INTERFACE_HPP
#define LOG_INTERFACE_HPP

#include <filesystem>
#include <cstddef>
#include <string>
#include <ostream>
#include <fstream>
#include <iomanip>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/move/utility_core.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include "module_config.hpp"

// In the future, this many colors could be usefull
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

namespace zbot {

	enum class severity_level : uint_fast16_t
	{
		e_none = 0,
		e_debug = 1U << 0U,
		e_notification = 1U << 1U,
		e_info = 1U << 2U,
		e_warning = 1U << 3U,
		e_error = 1U << 4U,
		e_critical = 1U << 5U,
	};

	const std::map<severity_level, char*> LEVEL_BUFFER{
		{severity_level::e_debug, "debug"},
		{severity_level::e_notification, "notification"},
		{severity_level::e_info, "info"},
		{severity_level::e_warning, "warning"},
		{severity_level::e_error, "error"},
		{severity_level::e_critical, "critical"}
	};

	const std::map<severity_level, char*> COLOR_BUFFER{
		{severity_level::e_debug, BOLDCYAN},
		{severity_level::e_notification, BOLDBLUE},
		{severity_level::e_info, BOLDWHITE},
		{severity_level::e_warning, BOLDYELLOW},
		{severity_level::e_error, BOLDRED},
		{severity_level::e_critical, BOLDRED}
	};

	BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", severity_level)
	BOOST_LOG_ATTRIBUTE_KEYWORD(severityMask, "SeverityMask", uint_fast16_t)
	BOOST_LOG_ATTRIBUTE_KEYWORD(module, "Module", ModuleConfig)
	BOOST_LOG_ATTRIBUTE_KEYWORD(timeline, "Timeline", attrs::timer::value_type)

	class LogInterface {
	public:
		LogInterface(const std::filesystem::path& t_globalLoggerPath, bool t_enableConsoleLogging);

		void setDiffConfig(std::vector<ModuleConfig>* t_diffConfig);
		void enableModuleSeverity(const std::string& t_moduleName, severity_level t_level);
		void disableModuleSeverity(const std::string& t_moduleName, severity_level t_level);
		void setModuleSeverity(const std::string& t_moduleName, uint_fast16_t t_level);
		void init();
		
	private:
		void setupConsoleLogging(bool t_autoFlush = true);
		void initModuleSeverityFilters();
		void setupFileLogging(const std::filesystem::path& t_path, const boost::log::v2_mt_posix::filter& t_filter, bool t_autoFlush = true);

		typedef sinks::synchronous_sink<sinks::text_file_backend> m_sinkToFile;
		typedef sinks::synchronous_sink< sinks::text_ostream_backend > m_sinkToConsole;
		typedef std::pair<std::string, uint_fast16_t> m_SeverityFilter;

		std::vector<m_SeverityFilter> m_moduleSeverityFilter;

		bool b_enableConsoleLogging{};
		std::filesystem::path m_globalLoggerPath{};
		boost::shared_ptr<logging::core> m_core;
		boost::shared_ptr<sinks::text_file_backend> m_textFileBackend;
		boost::shared_ptr<sinks::text_ostream_backend> m_textOstreamBackend;
		std::vector<ModuleConfig>* m_diffConfig{};
	};
}


#endif // !LOG_INTERFACE_HPP
