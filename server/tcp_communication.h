/*
 * tcp_communication.h
 *
 *  Created on: Jul 28, 2020
 *      Author: blew
 */

#ifndef SERVER_TCP_COMMUNICATION_H_
#define SERVER_TCP_COMMUNICATION_H_

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <yaml-cpp/yaml.h>
#include <mutex>
#include <condition_variable>

#include <cstdlib>
#include <thread>
#include <utility>

#include "../server/StringParser.h"
#include "../server/TCPcommand.h"
#include "../server/UDPemitter.h"

namespace rt32 {
	
	time_t get_daytime();
	std::string make_daytime_string(time_t timestamp);
	
	namespace tmsrv {
		
		class TCPtmServer {
			public:
				TCPtmServer(
						boost::program_options::variables_map opt, 
						spdlog::logger* logger, 
						YAML::Node& srvmemo);
				~TCPtmServer();
				
				//			std::thread 
				void server(boost::asio::io_service& io_service, std::string host, unsigned short port);
				void session(boost::asio::ip::tcp::socket sock, int max_length);
				TCPcommand::command_status_t run(TCPcommand::command_t cmd, 
						std::unordered_map<std::string,std::string> kwargs,
						std::string &reply);
				int tcp_communication_th();
				
				std::pair <std::string,boost::system::error_code> read_msg(
						boost::asio::ip::tcp::socket & socket, const int max_length);
				void send_msg(boost::asio::ip::tcp::socket & socket, const std::string& message);
				int save_server_state();
				
			protected:
				
				TCPcommand::command_status_t cmd_set(
						std::unordered_map<std::string,std::string> kwargs,
						std::string &reply);
				
				
				boost::program_options::variables_map _opt;
				spdlog::logger* _logger;
				YAML::Node& _srvmemo; /*!< internal server status yaml container.
				 Stuff that is stored under "state" branch is saved to 
				 status file on server exit and read from on server start. 
				 Anything saved under other branches can be used as 
				 server wide general purpose storage. 
				 */
				std::list<std::thread::id> _active_sessions;
//				std::thread* _emitterTh;
				UDPemitter emitter;

				std::mutex _srvmemo_mtx;
				std::condition_variable _cond_terminate;
				std::condition_variable _cond_terminate_session;
				bool _terminate;
		};
		
/*
		class TCPsession {
				TCPsession() {}
				~TCPsession() {}
				void session(boost::asio::ip::tcp::socket sock, int max_length);
				
		};
*/
		
		
		
	}	
}  // namespace rt32




#endif /* SERVER_TCP_COMMUNICATION_H_ */
