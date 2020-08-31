/*
 * tcp_communication.cpp
 * 
 * Thread that supports TCP communication
 *
 *  Created on: Jul 28, 2020
 *      Author: blew
 */


#include "../server/tcp_communication.h"

#include <string>
#include <iostream>
#include <tuple>
#include <pthread.h>

#include "../server/StringParser.h"
#include "../server/TCPcommand.h"

using boost::asio::ip::tcp;
//using namespace boost::asio;
//using boost::asio::ip::tcp;
using std::string;
using std::cout;
using std::endl;

namespace rt32 {

	time_t get_daytime() {
			using namespace std; // For time_t, time and ctime;
			time_t timestamp=time(0);
			return timestamp;
	}
	
	std::string make_daytime_string(time_t timestamp) {
/*
		using namespace std; // For time_t, time and ctime;
		time_t local_time=time(0);
		struct tm* now_utc = gmtime(&local_time);
*/
		struct tm* utc_now = gmtime(&timestamp);
		std::string s=asctime(utc_now);
		StringParser::rtrim_nl(s);
		StringParser::rtrim_cr(s);
//		std::cout << s << "|\n";
		return s;
	}

	namespace tmsrv {
		
		
		TCPtmServer::TCPtmServer(boost::program_options::variables_map opt,
				spdlog::logger *logger, YAML::Node& srvmemo) :
		_srvmemo(const_cast<YAML::Node&>(srvmemo)),
		emitter(opt,logger,srvmemo)
		{
			_opt=opt;
			_logger=logger;
			_terminate=false;
//			_emitterTh=0;
		}
		
		
		TCPtmServer::~TCPtmServer() {
//			if (_emitterTh!=0) {
//				delete _emitterTh;
//			}
		}
		
		/*
string read_(tcp::socket & socket) {
	boost::asio::streambuf buf;
	boost::asio::read_until( socket, buf, "\n" );
	string data = boost::asio::buffer_cast<const char*>(buf.data());
	return data;
}

void send_(tcp::socket & socket, const string& message) {
	const string msg = message + "\n";
	boost::asio::write( socket, boost::asio::buffer(message) );
}
		 */
		
		//const int max_length = 1024;
		
		/* ******************************************************************************************** */
		std::pair <std::string,boost::system::error_code> TCPtmServer::read_msg(
				tcp::socket & socket, const int max_length) {
			
			char data[max_length];
			boost::system::error_code error;
			size_t length = socket.read_some(boost::asio::buffer(data,max_length), error);
			
			if (error == boost::asio::error::eof)
				return std::pair <std::string,boost::system::error_code> ("",error);
			//		break; // Connection closed cleanly by peer.
			else if (error)
				throw boost::system::system_error(error); // Some other error.
			
			
			string msg(data);
			msg.erase(length);
			rt32::StringParser::rtrim_nl(msg);
			rt32::StringParser::rtrim_cr(msg);
			
			//	boost::asio::streambuf buf;
			//	boost::asio::read_until( socket, buf, "\n" );
			//	string data = boost::asio::buffer_cast<const char*>(buf.data());
			return std::pair <std::string, boost::system::error_code> (msg,error);
		}
		/* ******************************************************************************************** */
		void TCPtmServer::send_msg(tcp::socket & socket, const string& message) {
//			const string msg = message + "\n";
			boost::asio::write( socket, boost::asio::buffer(message) );
		}
		
		
		
		/* ******************************************************************************************** */
		void TCPtmServer::session(boost::asio::ip::tcp::socket sock, int max_length) {
			_logger->debug("{}: new connection from {}",
					rt32::thread_id2str(std::this_thread::get_id()),
					sock.remote_endpoint().address().to_string());
			try	{
				for (;;) {
					// read command from socket
					auto [msg, err]=read_msg(sock,max_length);
					if (err==boost::asio::error::eof) break;  // Connection closed cleanly by peer.
					//				if (err==boost::asio::error::eof) return;  // Connection closed cleanly by peer.
					_logger->info("{}: received: '{}' from {}",
							rt32::thread_id2str(std::this_thread::get_id()),
							msg, sock.remote_endpoint().address().to_string());
					
					
					// process TCP request - parse incoming command
					std::string reply;
					tmsrv::TCPcommand command;
					auto [cmd,kwargs]=command.parse(msg);
					// check kwargs here
					
					// execute command
					auto res=run(cmd,kwargs,reply);
					
					
					
					// send answer to client
					send_msg(sock,reply+"\n");
					
					// log server reply
					if (cmd==TCPcommand::cmd_data) {
						_logger->trace("{} reply: {}",
								rt32::thread_id2str(std::this_thread::get_id()),
								reply);						
						
					}
					else {
						_logger->debug("{} reply: {}",
								rt32::thread_id2str(std::this_thread::get_id()),
								reply);						
					}
					
					// commands that require special treatment
					if (cmd==TCPcommand::cmd_close)	{ sock.close(); break; }
					if (cmd==TCPcommand::cmd_terminate)	{ sock.close(); exit(100); } //TODO: make this a graceful exit
					//			boost::asio::write(sock, boost::asio::buffer(msg.c_str(), msg.size()));
				}
				_logger->debug("{}: session closed",
						rt32::thread_id2str(std::this_thread::get_id())
				);
			}
			catch (std::exception& e) {
				send_msg(sock,"Command failed\n");
				std::cerr << "Exception in thread: " << e.what() << "\n";
			}
		}
		
		
		/* ******************************************************************************************** */
		TCPcommand::command_status_t TCPtmServer::run(TCPcommand::command_t cmd, 
				std::unordered_map<std::string,std::string> kwargs,
				std::string &reply) {
			TCPcommand::command_status_t status=TCPcommand::cmd_success;
			TCPcommand Cmd;
			std::mutex srvmemo_mutex;

			switch (cmd) {
				case TCPcommand::cmd_help:
					reply=Cmd.get_help();
					break;
				case TCPcommand::cmd_close:
					reply="OK";
					break;
				case TCPcommand::cmd_terminate:
					reply="OK";
					break;
				case TCPcommand::cmd_status:
					reply=Cmd.get_status(_srvmemo);
					break;
				case TCPcommand::cmd_data:
					reply=Cmd.get_data(_srvmemo);
					break;
				case TCPcommand::cmd_startAstroTime:
					{
						std::lock_guard<std::mutex> lockGuard(srvmemo_mutex);
//						for (long i=0;i<1000000000;i++) { double x=sqrt(i); }
						if (_srvmemo["state"]["astroTime"].as<int>()==1) {
							reply="astroTime is started";
						}
						else {
							reply="OK";
							_srvmemo["state"]["astroTime"]=1;
							
//							UDPemitter emitter(_opt,std::ref(_srvmemo));
							std::thread emitterTh(&UDPemitter::start,&emitter);
							_srvmemo["astroTimeTh"]=static_cast<unsigned long int>(emitterTh.native_handle());
							emitterTh.detach();
//							_emitterTh = new std::thread(&UDPemitter::start,&emitter);
//							_srvmemo["astroTimeTh"]=static_cast<unsigned long int>(_emitterTh->native_handle());
//							_emitterTh->detach();
						}
						
					}
					save_server_state();
					break;
				case TCPcommand::cmd_stopAstroTime:
					{
						std::lock_guard<std::mutex> lockGuard(srvmemo_mutex);
						if (_srvmemo["state"]["astroTime"].as<int>()==0) {
							reply="astroTime is stopped";
						}
						else {
							reply="OK";
							_srvmemo["state"]["astroTime"]=0;
							pthread_cancel(
									static_cast<std::thread::native_handle_type>(_srvmemo["astroTimeTh"].as<unsigned long int>())
							);
							
						}
					}
					save_server_state();
					break;
				case TCPcommand::cmd_set:
					status=cmd_set(kwargs,reply);
					if (status==TCPcommand::cmd_success) {
						if (save_server_state()!=0) {
							_logger->error("Could not save server state");
						}
					}

					break;
				case TCPcommand::unrecognised_cmd:
					reply="Unrecognized command";
					status=TCPcommand::cmd_failed;
					break;
				default:
					reply="Command not implemented";
					break;
			}
			
			return status;
		}
		
		
		/* ******************************************************************************************** */
		void TCPtmServer::server(boost::asio::io_service& io_service, 
				std::string host, unsigned short port) {
			std::stringstream port_str;
			port_str<<port;

//			boost::asio::ip::address_v4 ip4addr(host.c_str());
			tcp::acceptor a(io_service, tcp::endpoint(
					boost::asio::ip::address_v4::from_string(host), port));
			for (;;) {
				tcp::socket sock(io_service);
				a.accept(sock);

//				boost::asio::ip::tcp::resolver resolver(io_service);
//				boost::asio::ip::tcp::endpoint endpoint = 
//						*resolver.resolve({
//					boost::asio::ip::tcp::v4(), host.c_str(),port_str.str().c_str()});

				std::thread th(&TCPtmServer::session,this, std::move(sock), 
				_opt["cmd-max-length"].as<int>());
				th.detach();
//				_active_sessions.push_back(th.get_id());
			}
			
			// terminate all active sessions
//			for (auto sess : _active_sessions) {
//				std::terminate();
//			}
		}
		
		
		/* ******************************************************************************************** */
		int TCPtmServer::tcp_communication_th() {
			string host=_opt["host"].as<string>();
			int port=_opt["port"].as<int>();
			
			_logger->info("Starting TCP command server on {}:{}",host,port);
			boost::asio::io_service io_service;
			server(io_service, host,port);
			
			/*
	try
	{
		boost::asio::io_service io_service;
		
		tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), port));
		
		for (;;)
		{
			tcp::socket socket(io_service);
			acceptor.accept(socket);
			
			std::string message = make_daytime_string();
			
			boost::system::error_code ignored_error;
			boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
			 */
			
			_logger->info("TCP command server has stopped");
			
			return 0;
			
			
			
			
			
			
			/*
	boost::asio::io_context io_context;
	tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));
	for (;;) {
		std::thread(session, a.accept()).detach();
	}
			 */
			
			/*
	boost::asio::io_service io_service;
	//listen for new connection
	tcp::acceptor acceptor_(io_service, tcp::endpoint(tcp::v4(), port ));
	//socket creation 
	tcp::socket socket_(io_service);
	//waiting for connection
	acceptor_.accept(socket_);
	//read operation
	string message = read_(socket_);
	cout << message << endl;
	//write operation
	send_(socket_, "Hello From Server!");
	cout << "Servent sent Hello message to Client!" << endl;
			 */
			
			
			/*
	boost::asio::io_service io_service;
	//socket creation
	tcp::socket socket(io_service);
	//connection
	socket.connect( tcp::endpoint( boost::asio::ip::address::from_string(host), port ));
	// request/message from client
	const std::string msg = "Hello from Client!\n";
	boost::system::error_code error;
	boost::asio::write( socket, boost::asio::buffer(msg), error );
	if( !error ) {
		std::cout << "Client sent hello message!" << std::endl;
	}
	else {
		std::cout << "send failed: " << error.message() << std::endl;
	}
	// getting response from server
	boost::asio::streambuf receive_buffer;
	boost::asio::read(socket, receive_buffer, boost::asio::transfer_all(), error);
	if( error && error != boost::asio::error::eof ) {
		cout << "receive failed: " << error.message() << std::endl;
	}
	else {
		const char* data = boost::asio::buffer_cast<const char*>(receive_buffer.data());
		std::cout << data << std::endl;
	}
	return 0;
			 */
			
			//	return 0;
		}
		
		TCPcommand::command_status_t TCPtmServer::cmd_set(
				std::unordered_map<std::string, std::string> kwargs,
				std::string &reply) {
			StringParser params(kwargs);
			
			
			if (params.hasKey("tmsp-delay")) {
				std::lock_guard<std::mutex> lockGuard(_srvmemo_mtx);
				
				_srvmemo["state"]["tmsp-delay"]=std::stod(params["tmsp-delay"]);
				reply="OK";
				return TCPcommand::cmd_success;
			}
			
			
			if (params.hasKey("astroTime")) {
				if (std::stod(params["astroTime"])==0) {
					return run(TCPcommand::cmd_stopAstroTime,kwargs,reply);
				}
				if (std::stod(params["astroTime"])==1) {
					return run(TCPcommand::cmd_startAstroTime,kwargs,reply);
				}
			}
			
			
			if (params.hasKey("dUT1")) {
				double dut1=std::stod(params["dUT1"]);
				if (fabs(dut1)>0.9) { reply="NOK"; return TCPcommand::cmd_failed; }
				
				std::lock_guard<std::mutex> lockGuard(_srvmemo_mtx); /*
				 * system time operations are not thread safe, so we lock here
				 * and not only for _srvmemo
				 * 
				 */
				
				auto timestamp=get_daytime();
				_srvmemo["state"]["dUT1-modification-date"]=make_daytime_string(timestamp);
				std::stringstream ss;
				ss<<timestamp;
				_srvmemo["state"]["dUT1-modification-date_unix"]=ss.str();
				reply="OK";
/*
				if (dut1-_srvmemo["state"]["dUT1"].as<double>()>0.9) { // leap second introduced
					_srvmemo["state"]["TAI_UTC"]=_srvmemo["state"]["TAI_UTC"].as<int>()+1;
					reply="OK, leap second detected, TAI-UTC updated automatically";
				}
*/
				_srvmemo["state"]["dUT1"]=dut1;
				return TCPcommand::cmd_success;
			}
			
			
			if (params.hasKey("TAI_UTC")) {
				std::lock_guard<std::mutex> lockGuard(_srvmemo_mtx);
				_srvmemo["state"]["TAI_UTC"]=std::stoi(params["TAI_UTC"]);
				reply="OK";
				return TCPcommand::cmd_success;
			}
			
			if (params.hasKey("polarX")) {
				std::lock_guard<std::mutex> lockGuard(_srvmemo_mtx);
				_srvmemo["state"]["polarX"]=std::stod(params["polarX"]);
				reply="OK";
				return TCPcommand::cmd_success;
			}

			if (params.hasKey("polarY")) {
				std::lock_guard<std::mutex> lockGuard(_srvmemo_mtx);
				_srvmemo["state"]["polarY"]=std::stod(params["polarY"]);
				reply="OK";
				return TCPcommand::cmd_success;
			}
			
			reply="Unknown parameter";
			return TCPcommand::cmd_failed;
		}

		/* ******************************************************************************************** */
		int TCPtmServer::save_server_state() {
			_logger->info("Saving server state");
			std::lock_guard<std::mutex> lg(_srvmemo_mtx);
			std::ofstream fout(_opt["status-file"].as<string>());
			if (!fout.is_open()) {
				return 1;
			}
			fout << _srvmemo["state"];
			fout.close();
			return 0;
		}

	}
}

