#include "log_interface.hpp"
#include <boost/core/null_deleter.hpp>
#include "module_config.hpp"
#include <boost/log/support/date_time.hpp>
#include <stdexcept>
#include <iostream>
namespace zbot {
	std::ostream& operator<< (std::ostream& t_strm, severity_level t_level)
	{
		if (t_level <= severity_level::e_critical)
			t_strm << COLOR_BUFFER.find(t_level)->second << "(" << LEVEL_BUFFER.find(t_level)->second << ") ";
		else
			t_strm << static_cast<int>(t_level);
		return t_strm;
	}

	LogInterface::LogInterface(const std::filesystem::path& t_globalLoggerPath, bool t_enableConsoleLogging) : m_globalLoggerPath(t_globalLoggerPath), b_enableConsoleLogging(t_enableConsoleLogging)
	{
		m_core = logging::core::get();
	}

	void LogInterface::setupConsoleLogging(bool t_autoFlush) {
		//Pass all info
		m_textOstreamBackend = boost::make_shared< sinks::text_ostream_backend >();
		m_textOstreamBackend->add_stream(
			boost::shared_ptr< std::ostream >(&std::clog, boost::null_deleter()));
		m_textOstreamBackend->auto_flush(t_autoFlush);

		boost::shared_ptr< m_sinkToConsole > console_sink(new m_sinkToConsole(m_textOstreamBackend));
		console_sink->set_formatter
		(
			expr::stream
			<< expr::attr< severity_level >("Severity")
			<< expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
			<< " [" << expr::attr< std::string >("Channel") << "] "
			<< expr::smessage << WHITE
		);
		m_core->add_sink(console_sink);
	}


	void LogInterface::setDiffConfig(std::vector<ModuleConfig>* t_diffConfig) {
		m_diffConfig = t_diffConfig;
		initModuleSeverityFilters();
	}

	void LogInterface::initModuleSeverityFilters(){
		if (m_diffConfig == nullptr || m_diffConfig->empty())
			throw std::invalid_argument("Log Interface error : diffConfig empty at module severity filter init");

		for (auto item : *m_diffConfig) {
			m_SeverityFilter moduleSeverityFilter;
			moduleSeverityFilter.first = item.moduleName + "." + item.version + "." + item.subversion;
			uint_fast16_t severity{0};
			severity |= (uint_fast16_t)severity_level::e_error;
			severity |= (uint_fast16_t)severity_level::e_info;
			severity |= (uint_fast16_t)severity_level::e_warning;
			moduleSeverityFilter.second = severity;
			m_moduleSeverityFilter.push_back(moduleSeverityFilter);
		}
	}

	void LogInterface::enableModuleSeverity(const std::string& t_moduleName, severity_level t_level) {
		if (m_moduleSeverityFilter.empty())
			return;

		for (int i = 0; i < m_moduleSeverityFilter.size(); i++) {
			if (m_moduleSeverityFilter[i].first == t_moduleName) {
				m_moduleSeverityFilter[i].second |= ((uint_fast16_t)t_level);
				init();
				return;
			}
		}
	}

	void LogInterface::disableModuleSeverity(const std::string& t_moduleName, severity_level t_level) {

		for (int i = 0; i < m_moduleSeverityFilter.size(); i++) {
			if (m_moduleSeverityFilter[i].first == t_moduleName) {
				m_moduleSeverityFilter[i].second &= ~((uint_fast16_t)t_level);
				init();
				return;
			}
		}

	}

	void LogInterface::setModuleSeverity(const std::string& t_moduleName, uint_fast16_t t_level) {
		// jasaliek dazus printus

		for (int i = 0; i < m_moduleSeverityFilter.size(); i++) {
			if (m_moduleSeverityFilter[i].first == t_moduleName) {
				uint_fast16_t newSeverity = ((uint_fast16_t)t_level);
				m_moduleSeverityFilter[i].second = newSeverity;
				init();
				return;
			}
		}

	}


	void LogInterface::setupFileLogging(const std::filesystem::path& t_path, const boost::log::v2_mt_posix::filter& t_filter, bool t_autoFlush) {

		boost::shared_ptr<sinks::text_file_backend> tmpBackend;
		tmpBackend = boost::make_shared< sinks::text_file_backend >(
			keywords::file_name = t_path,
			keywords::rotation_size = 10 * 1024 * 1024,
			keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0) ,
			keywords::open_mode = std::ios::out|std::ios::app
			);
		tmpBackend->auto_flush(true);

		typedef sinks::synchronous_sink<sinks::text_file_backend> sinkToFile;
		boost::shared_ptr< sinkToFile > file_sink(new sinkToFile(tmpBackend));
		file_sink->set_filter(
			t_filter
		);

		file_sink->set_formatter
		(
			expr::stream
			<< expr::attr< severity_level >("Severity")
			<< expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
			<< " [" << expr::attr< std::string >("Channel") << "] "
			<< expr::smessage << WHITE
		);

		m_core->add_sink(file_sink);
	}


	void LogInterface::init() {
		if (m_diffConfig == nullptr || m_diffConfig->empty())
			throw std::invalid_argument("Log Interface error : diffConfig empty at logging initialization");
		m_core->remove_all_sinks();

		for (auto item : *m_diffConfig) {
			if (item.b_runModule) {
				boost::shared_ptr<sinks::text_file_backend> tmpBackend;
				std::string moduleName = item.moduleName + "." + item.version + "." + item.subversion;
				std::filesystem::path path = m_globalLoggerPath.string() + "/" + item.moduleName + item.version + item.subversion + ".log";
				uint_fast16_t severityLevel{};
				for(auto value : m_moduleSeverityFilter) // Could be optimized  
					if (value.first == moduleName) {
						severityLevel = value.second;
						break;
					}

				severity_level info{ severityLevel & (uint_fast16_t)severity_level::e_info};
				severity_level warning{ severityLevel & (uint_fast16_t)severity_level::e_warning };
				severity_level error{ severityLevel & (uint_fast16_t)severity_level::e_error };
				severity_level critical{ severityLevel & (uint_fast16_t)severity_level::e_critical };
				severity_level debug{ severityLevel & (uint_fast16_t)severity_level::e_debug };
				severity_level notification{ severityLevel & (uint_fast16_t)severity_level::e_notification };

				setupFileLogging(path, 
					((severity == info || severity == warning || severity == error || severity == critical || severity == debug || severity == notification)
						&& (expr::attr<std::string>("Channel") == moduleName)) || (severity == severity_level::e_critical));
			}

			// temp & (1 << N)

		}
		setupFileLogging( m_globalLoggerPath.string() + "/critical.log", (severity == severity_level::e_critical || severity == severity_level::e_error));
		if (b_enableConsoleLogging)
			setupConsoleLogging();
		boost::log::add_common_attributes();
	}

}