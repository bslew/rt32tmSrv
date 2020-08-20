/*
 * TCPcommand.h
 *
 *  Created on: Jul 29, 2020
 *      Author: blew
 */

#include <yaml-cpp/yaml.h>
#include <tuple>
#include <unordered_map>
#include <mutex>
#include "../server/StringParser.h"

#ifndef SRC_TCPCOMMAND_H_
#define SRC_TCPCOMMAND_H_

namespace rt32 {
	namespace tmsrv {
		
		/*
		 *
		 */
		class TCPcommand {
			public:
				
				typedef enum { 
					cmd_success, 
					cmd_failed
				} command_status_t;
				
				typedef enum { 
					cmd_help, 
					cmd_close, // close session
					cmd_status, // print srv status
					cmd_data, // return server's variables in kwargs format (useful for robots)
					cmd_startAstroTime,  // start UDP time distibution
					cmd_stopAstroTime,  // stop UDP time distribution
					cmd_terminate, // exit TCP server and end application
					cmd_set, // set value
					unrecognised_cmd 
				} command_t;
				
				TCPcommand();
				virtual ~TCPcommand();

				std::pair<command_t, std::unordered_map<std::string, std::string> > parse(std::string cmd);
//				command_status_t run(command_t cmd, YAML::Node& srvmemo, std::string& reply);
				
				
				
				std::string get_help();
				std::string get_status(YAML::Node& srvmemo);
				std::string get_data(YAML::Node& srvmemo);
				std::mutex _srvmemo_mtx;
				
		};
	
	} /* namespace tmsrv */
} /* namespace rt32 */

#endif /* SRC_TCPCOMMAND_H_ */
