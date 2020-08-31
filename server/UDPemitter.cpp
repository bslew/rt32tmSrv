/*
 * UDPemitter.cpp
 *
 *  Created on: Jul 30, 2020
 *      Author: blew
 */

#include "../server/UDPemitter.h"

#include <sstream>
#include <iomanip> 
#include <iostream>

#include "../server/StringParser.h"


namespace rt32 {
	namespace tmsrv {
		
		UDPemitter::UDPemitter(
				boost::program_options::variables_map opt,
				spdlog::logger* logger, 
				YAML::Node& srvmemo) :
				_srvmemo(const_cast<YAML::Node&>(srvmemo))
		{
			StringParser sp;
			this->host=sp.split(opt["tmspUDPhost"].as<std::string>(),',');
			this->port=sp.split(opt["tmspUDPport"].as<std::string>(),',');
			_opt=opt;
			_logger=logger;
			
			//			_srvmemo = const_cast<YAML::Node&>(srvmemo);
			//			sleep_time=double(opt["tmsp-delay"].as<int>())*1000;
		}
		
		UDPemitter::~UDPemitter() {
			// TODO Auto-generated destructor stub
			_logger->info("Emitter thread has exited");
		}
		
		void UDPemitter::start() {
			try
			{
				_logger->info("Starting time emitter thread");
				for (int i=0; i<host.size();i++) {
					_logger->info("	emitter destination host:port={}:{}",
							host[i],port[i%port.size()]);
				}
				//				std::stringstream port_str;
				//				port_str<<port;
				
				boost::asio::io_service io_service;
				
				boost::asio::ip::udp::socket s(io_service, 
						boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0));
				
				boost::asio::ip::udp::resolver resolver(io_service);
				std::vector<boost::asio::ip::udp::endpoint> endpoint(host.size());
				for (int i=0; i<host.size();i++) {
					endpoint[i]=*resolver.resolve({
						boost::asio::ip::udp::v4(), 
								host[i].c_str(),
								port[i%port.size()].c_str()
					});
				}
				
				double sleep_time;
				for (;;) {
					auto dgram=get_timestamp_datagram();
					for (auto rcpt : endpoint) {
						s.send_to(boost::asio::buffer(dgram.c_str(), dgram.size()), rcpt);
					}
					_mtx.lock();
					sleep_time=double(_srvmemo["state"]["tmsp-delay"].as<int>())*1000;
					_mtx.unlock();
					if (_opt["verbosity"].as<int>()>20) {
						std::cout << "Emitter sleeps\n";
					}
					usleep(long(sleep_time));
				}
				
			}
			catch (std::exception& e)
			{
				std::cerr << "Exception: " << e.what() << "\n";
			}
		}
		
		std::string UDPemitter::get_timestamp_datagram() {
			struct timeval  tv;
			struct timezone tz;
			double sleepTime;
			double tstamp, dZD;
			
			double dUT=0;
			double tai_utc=37;
			
			// get current time offsets
			_mtx.lock();
			dUT=_srvmemo["state"]["dUT1"].as<double>();
			tai_utc=_srvmemo["state"]["TAI_UTC"].as<int>();
			_mtx.unlock();
			
			// get current time
			gettimeofday(&tv, &tz);
			
			// generate time stamp
			tstamp=double(tv.tv_sec)+double(tv.tv_usec)*1e-6 + dUT;
			std::stringstream ss;
			std::stringstream ssut1;
			ss << "1 " << std::fixed << std::setprecision(6) << tstamp << " "
					<< std::setprecision(6) << dUT << " "
					<< std::setprecision(0) << tai_utc;

			ssut1 << std::fixed << std::setprecision(6) << tstamp;
			_mtx.lock();
			_srvmemo["UT1"]=ssut1.str();
			_mtx.unlock();

			
			
			return ss.str();
		}
		
	} /* namespace tmsrv */
} /* namespace rt32 */
