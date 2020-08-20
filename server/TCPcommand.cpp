/*
 * TCPcommand.cpp
 *
 *  Created on: Jul 29, 2020
 *      Author: blew
 */

#include "../server/TCPcommand.h"

namespace rt32 {
	namespace tmsrv {
		
		TCPcommand::TCPcommand() {
			// TODO Auto-generated constructor stub
			
		}
		
		TCPcommand::~TCPcommand() {
			// TODO Auto-generated destructor stub
		}
		
		/* ******************************************************************************************** */
		std::pair<TCPcommand::command_t, std::unordered_map<std::string, std::string> > TCPcommand::parse(std::string cmd) {
			
			TCPcommand::command_t status=unrecognised_cmd;
			std::unordered_map<std::string, std::string> kwargs;
			StringParser parser;
			auto words=parser.split(cmd,' ');
			
			if (words.size()==0)
				return std::pair<TCPcommand::command_t, std::unordered_map<std::string, std::string> >(
						status,kwargs);
			
			if (words[0]=="help") { 
				return std::pair<TCPcommand::command_t, std::unordered_map<std::string, std::string> >(
						cmd_help,kwargs); 
			}
			if (words[0]=="close" or cmd=="quit") { 
				return std::pair<TCPcommand::command_t, std::unordered_map<std::string, std::string> >(
						cmd_close,kwargs); 
			}
			if (words[0]=="startAstroTime") { 
				return std::pair<TCPcommand::command_t, std::unordered_map<std::string, std::string> >(
						cmd_startAstroTime,kwargs); 
			}
			if (words[0]=="stopAstroTime") { 
				return std::pair<TCPcommand::command_t, std::unordered_map<std::string, std::string> >(
						cmd_stopAstroTime,kwargs); 
			}
			if (words[0]=="status") { 
				return std::pair<TCPcommand::command_t, std::unordered_map<std::string, std::string> >(
						cmd_status,kwargs);  
			}
			if (words[0]=="data") { 
				return std::pair<TCPcommand::command_t, std::unordered_map<std::string, std::string> >(
						cmd_data,kwargs);  
			}
			if (words[0]=="terminate") { 
				return std::pair<TCPcommand::command_t, std::unordered_map<std::string, std::string> >(
						cmd_terminate,kwargs);  
			}
			if (words[0]=="set") { 
				words.erase(words.begin(),words.begin()+1);
				kwargs=parser.parse(vec2str<std::string>(words,' '),' ','=');
				return std::pair<TCPcommand::command_t, std::unordered_map<std::string, std::string> >(
						cmd_set,kwargs);  
			}

			return std::pair<TCPcommand::command_t, std::unordered_map<std::string, std::string> >(
					status,kwargs);
		}
		
		/* ******************************************************************************************** */
/*
		TCPcommand::command_status_t TCPcommand::run(command_t cmd, 
				YAML::Node& srvmemo, std::string &reply) {
			TCPcommand::command_status_t status=cmd_success;
			
			switch (cmd) {
				case cmd_help:
					reply=get_help();
					break;
				default:
					break;
			}
			
			return status;
		}
*/

		/* ******************************************************************************************** */
		std::string TCPcommand::get_help() {
			std::stringstream ss;
			
			ss << "\n"
			   << "HELP\n"
			   << "----\n"
			   << "\n"
			   << "help - shows this info\n"
			   << "close/quit - close this session\n"
			   << "status - print server status\n"
			   << "data - return server's data in kw=val format\n"
				   "	(useful for machine-to-machine communication)\n"
			   << "set param=value - set server parameter param to value\n"
				   "	possible params/values combinations:\n"
				   "		astroTime=0 - same as stopAstroTime\n"
				   "		astroTime=1 - same as startAstroTime\n"
				   "		tmsp-delay=xxx - set delay between subsequent UDP time stamp datagrams [ms]\n"
				   "		TAI_UTC=xx - set difference TAI-UTC in integer number of seconds\n"
				   "		dUT1=x.xxx - set UT1-UTC difference (should be <0.6s)\n"
				   "		polarX=val - set polar X rotation angle [mas]\n"
				   "		polarY=val - set polar Y rotation angle [mas]\n"
			   << "terminate - stop TCP server. Communication will be lost and\n "
				   "	the server will be automatically restarted.\n"
			   << "startAstroTime - start astro time distribution via UDP datagrams.\n"
				   "	The UDP datagrams will be sent to recipients defined in the "
				   "configuration file.\n"
				   "	For backward compatibility currently the time datagrams have format:\n"
				   "	'1 tstamp+dUT1 dUT1 dAT'\n"
				   "	where\n"
				   "	tstamp=number of seconds and microseconds returned by gettimeofday "
				   "(i.e. since 1970-01-01 00:00:00 +0000 (UTC).\n"
				   "	dUT1=UT1-UTC (in seconds)\n"
				   "	dAT=TAI-UTC (in full seconds)\n"
			   << "stopAstroTime - stop astro time distribution via UDP datagrams.\n"
				   "\n"
			   ;
			
			return ss.str();
		}
		/* ******************************************************************************************** */
		std::string TCPcommand::get_status(YAML::Node &srvmemo) {
			std::stringstream ss;
			std::string dUT1moddate="unknown";
			std::lock_guard<std::mutex> lockGuard(_srvmemo_mtx);

			if (srvmemo["state"]["dUT1-modification-date"])
				dUT1moddate=srvmemo["state"]["dUT1-modification-date"].as<std::string>();
			
			ss << std::endl;
			ss << "Server status:" << std::endl;
			ss << "--------------" << std::endl;
			ss << "astro time server: " << srvmemo["state"]["astroTime"].as<int>() << std::endl;
			ss << "tmsp-delay [ms]: " << srvmemo["state"]["tmsp-delay"].as<int>() << std::endl;
			ss << "dUT1 [s]: " << srvmemo["state"]["dUT1"].as<double>()  << std::endl;
//					<< " (updated: " 
//					<< dUT1moddate
//					<< ")" << std::endl;
			ss << "TAI_UTC [s]: " << srvmemo["state"]["TAI_UTC"].as<int>() << std::endl;
			ss << "polarX [mas]: " << srvmemo["state"]["polarX"].as<double>() << std::endl;
			ss << "polarY [mas]: " << srvmemo["state"]["polarY"].as<double>() << std::endl;
			ss << std::endl;
			ss << "Earth rotation data last updated: " << dUT1moddate << std::endl;
			ss << std::endl;
			return ss.str();
		}
		/* ******************************************************************************************** */
		std::string TCPcommand::get_data(YAML::Node &srvmemo) {
			std::stringstream ss;
			std::string dUT1moddate="unknown";
			std::lock_guard<std::mutex> lockGuard(_srvmemo_mtx);

			if (srvmemo["state"]["dUT1-modification-date"])
				dUT1moddate=srvmemo["state"]["dUT1-modification-date"].as<std::string>();
			
			ss << "astroTime=" << srvmemo["state"]["astroTime"].as<int>() << ",";
			ss << "tmsp-delay=" << srvmemo["state"]["tmsp-delay"].as<int>() << ",";
			
			std::string ut1;
			ss << "UT1=";
			if (srvmemo["UT1"])
				ss << srvmemo["UT1"].as<std::string>();
			ss << ",";
			
			ss << "dUT1=" << srvmemo["state"]["dUT1"].as<double>()  << ",";
			ss << "TAI_UTC=" << srvmemo["state"]["TAI_UTC"].as<int>() << ",";
			ss << "polarX=" << srvmemo["state"]["polarX"].as<double>() << ",";
			ss << "polarY=" << srvmemo["state"]["polarY"].as<double>() << ",";
			ss << "updated=" << srvmemo["state"]["dUT1-modification-date_unix"];
			return ss.str();
		}
	
	} /* namespace tmsrv */
} /* namespace rt32 */
