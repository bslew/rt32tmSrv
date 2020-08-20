/*
 * UDPemitter.h
 *
 *  Created on: Jul 30, 2020
 *      Author: blew
 */

#ifndef SERVER_UDPEMITTER_H_
#define SERVER_UDPEMITTER_H_


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
		class UDPemitter {
			public:
				UDPemitter(
						boost::program_options::variables_map opt,
						spdlog::logger* logger, 
						YAML::Node& srvmemo
						);
				virtual ~UDPemitter();
				
				void start();
				
				std::string get_timestamp_datagram();
				
				spdlog::logger* _logger;
				std::vector<std::string> host;
				std::vector<std::string> port;
//				double sleep_time; // us
				YAML::Node& _srvmemo;
				boost::program_options::variables_map _opt;
				std::mutex _mtx;
		};
	
	} /* namespace tmsrv */
} /* namespace rt32 */

#endif /* SERVER_UDPEMITTER_H_ */
