/*
 * UpdatedUT1.h
 * 
 * This is UNFINISHED self-updater thread. The server IERS values are now being updated
 * from shell scripts since https://cddis.nasa.gov/archive/products/iers/mark3.out
 * data require secure connections with authentication.
 * 
 * This can still be done via curlpp but shell approach is much faster. TBC
 *
 *  Created on: Aug 19, 2020
 *      Author: blew
 */

#ifndef SERVER_UPDATEDUT1_H_
#define SERVER_UPDATEDUT1_H_

#include <boost/program_options.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <mutex>

namespace rt32 {
	namespace tmsrv {
		
		/*
		 *
		 */
		class Update_dUT1 {
			public:
				Update_dUT1(
						boost::program_options::variables_map opt,
						spdlog::logger* logger, 
						YAML::Node& srvmemo
				);
				virtual ~Update_dUT1();

				void start();
				
				std::pair< std::string, int> fetch_raw_iers_data(std::string addr);
				std::string update();
				
				spdlog::logger* _logger;
				std::vector<std::string> source_addr;
				YAML::Node& _srvmemo;
				boost::program_options::variables_map _opt;
				std::mutex _mtx;
		
		
		};
	
	} /* namespace tmsrv */
} /* namespace rt32 */

#endif /* SERVER_UPDATEDUT1_H_ */
