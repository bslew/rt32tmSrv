/*!
 * \file rt32tmsrv.cpp
 *
 *  Project: rt32tmSrv
 *  Created on: Jul 28, 2020 11:48:39 AM
 *  Author: blew
 *  
 *  
 *  TODO: 	add docker volumes to keep logs, config and state files
 *  		on docker host and not inside container. This will ease
 *  		server updates.
 *  
 */

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <thread>
#include <utility>

#include <yaml-cpp/yaml.h>

#include "../server/tcp_communication.h"
#include "../server/UDPemitter.h"

#define PARAMETER_FILE "config.txt"

using namespace std;
using namespace rt32;


spdlog::logger getLogger(int verbosity);
string getCmdString(int argc, char** argv);
string getProgramVersionString();
boost::program_options::variables_map  parseOptions(int argc, char** argv);


/* ******************************************************************************************** */
/* ******************************************************************************************** */
/* ******************************************************************************************** */
int main(int argc, char **argv) {
	
	// initialize parser
	boost::program_options::variables_map opt=parseOptions(argc,argv);

	// initialize logger
	spdlog::logger logger=getLogger(opt["verbosity"].as<int>());
	logger.info("rt32tmsrv - NEW RUN");
	logger.info(getCmdString(argc,argv));
	
	// load stored server data and status
	logger.info("Loading server status data from: {}",opt["status-file"].as<string>());
	
	// this is server-wide shared variable
	YAML::Node srvstat=YAML::LoadFile(opt["status-file"].as<string>()); 
	YAML::Node srvmemo;
	srvmemo["state"]=srvstat; // under "status" node we keep stuff that
	// is saved to server status file. Stuff that is generated during server operations 
	// that should not be saved to status file, should be kept at other branches

	
	// overriding config file if server state has an updated value
	if (!srvmemo["state"]["tmsp-delay"]) srvmemo["state"]["tmsp-delay"]=opt["tmsp-delay"].as<int>();
	if (!srvmemo["state"]["polarX"]) srvmemo["state"]["polarX"]=double(0);
	if (!srvmemo["state"]["polarY"]) srvmemo["state"]["polarY"]=double(0);
	if (!srvmemo["state"]["astroTime"]) { srvmemo["state"]["astroTime"]=int(0); }

	//
	// start UDP emitter thread if needed
	//
//	cout << "dUT1: "<< srvmemo["dUT1"].as<double>() << "\n";
	
	tmsrv::UDPemitter emitter(opt,&logger,std::ref(srvmemo));
	std::thread* emitterTh=0;
	if (srvmemo["state"]["astroTime"].as<int>()==1) {
		emitterTh = new std::thread(&tmsrv::UDPemitter::start,&emitter);
//		udpcomm.detach();
		srvmemo["astroTimeTh"]=static_cast<unsigned long int>(emitterTh->native_handle());
	}

	// start TCP server thread
	tmsrv::TCPtmServer server(opt,&logger,std::ref(srvmemo));
	std::thread tcpcomm(&tmsrv::TCPtmServer::tcp_communication_th,&server);
	srvmemo["serverTh"]=tcpcomm.native_handle();

	
	
	tcpcomm.join();
	
	// delete emitter thread if it is running (it would be shut down on program exit anyway)
	if (srvmemo["state"]["astroTime"].as<int>()==1) {
		pthread_cancel(
				static_cast<std::thread::native_handle_type>(
						srvmemo["astroTimeTh"].as<unsigned long int>())
		);
	}
	
//	std::cout << opt["input_file"].as< vector<string> >() << std::endl;
//	if (opt.count("input-file")) {
//	vector<string> input_files=opt["input-file"].as< vector<string> >();
		
	
	return 0;
}

/* ******************************************************************************************** */
boost::program_options::variables_map parseOptions(int argc, char** argv) {
	namespace po = boost::program_options;
    po::variables_map vm;
	
	
    try {
    	int opt=1;
    	bool boolopt;
    	long longopt;
    	double dbl;
    	string stropt;
    	vector<string> svec;
        stringstream ss;
        ss << std::getenv("HOME") << "/.config/rt32tmSrv/config.txt";
        string config_file=ss.str();
        boost::filesystem::path cwd=boost::filesystem::current_path();
//        string parameter_file=cwd.string()+"/input.par";
        string parameter_file=PARAMETER_FILE;
    
        // Declare a group of options that will be 
        // allowed only on command line
        po::options_description generic("Generic options");
        generic.add_options()
            ("version,V", "print version string")
//			("ifile,i", po::value<string>(&stropt)->multitoken()->required(), "input file name")
//		    ("version,V", po::bool_switch()->default_value(false), "print program version and exit")
//		    ("switch,s", po::bool_switch()->default_value(false), "switch option")
            ("help", "produce help message")
			("config,c", po::value<string>(&config_file)->default_value(config_file),
				  "name of a global configuration file.")
			("param", po::value<string>(&parameter_file)->default_value(parameter_file),
				  "parameter file")
            ;
    
        // Declare a group of options that will be 
        // allowed both on command line and in
        // config file
        po::options_description config("Configuration");
        config.add_options()
            ("verbosity,v", po::value<int>(&opt)->default_value(2), 
                  "verbosity level")
//            ("include-path,I", 
//                 po::value< vector<string> >()->composing(), 
//                 "include path")
			("cmd-max-length", po::value<int>()->default_value(1024), "Maximal length"
					"of data received via TCP command")
			("host,h", po::value<string>()->default_value("127.0.0.1"), "TCP command server host")
			("port,p", po::value<int>()->default_value(33001), "TCP command server port")
			("tmspUDPport", po::value<string>()->default_value("33033"), 
					"port to send UDP time datagrams for RT32 control system.")
			("tmspUDPhost", po::value<string>()->default_value("192.168.1.4"), 
					"IP4 host address to send UDP time datagrams to.")
			("tmsp-delay", po::value<int>()->default_value(100), "delay between "
					"subsequent time stamp UDP datagrams [ms].")
		    ("status-file", po::value<string>()->default_value("rt32tmsrv-state.yaml"), 
		    		"Server data and status file.")
//		    ("show", po::value<bool>(&boolopt)->default_value(false), "shows the loaded image")
//			
//		    ("hmin", po::value<double>(&dbl)->default_value(0), "Minimal elevation threshold.")
			;

        // Hidden options, will be allowed both on command line and
        // in config file, but will not be shown to the user.
        po::options_description hidden("Hidden options");
        hidden.add_options()
            ("input-file", po::value< vector<string> >(), "input files")
            ;

        
        po::options_description cmdline_options;
        cmdline_options.add(generic).add(config).add(hidden);

        po::options_description config_file_options;
        config_file_options.add(config).add(hidden);

        po::options_description parameter_file_options;
        parameter_file_options.add(config).add(hidden);
        
        po::options_description visible("rt32tmsrv\n\n "
    			"This program is a time server for RT32 "
    			"\n\n. "
    			"\n"
    			"example usage: rt32tmsrv"
    			"\n"
    			"\n\n"
    			"Allowed options");
        visible.add(generic).add(config);
        
        po::positional_options_description p;
        p.add("input-file", -1);
        
        store(po::command_line_parser(argc, argv).
              options(cmdline_options).positional(p).run(), vm);
        notify(vm);
        
        // process config file
        ifstream ifs(config_file.c_str());
        if (!ifs) {
            cout << "cannot open config file: " << config_file << "\n";
//            return 0;
        }
        else {
            store(parse_config_file(ifs, config_file_options), vm);
            notify(vm);
        }
    

        // process parameter file 
        ifs.open(parameter_file.c_str());
        if (!ifs.is_open()) {
            cout << "cannot open parameter file: " << parameter_file << "\n";
//            return 0;
        }
        else {
            store(parse_config_file(ifs, parameter_file_options), vm);
            notify(vm);
        }

        
        if (vm.count("help")) {
            cout << visible << "\n";
            exit(0);
        }

//    	if (vm["version"].as<bool>()) {
//    		std::cout << getProgramVersionString() << std::endl;
//    		exit(0);
//    	}

        if (vm.count("version")) {
        	std::cout << getProgramVersionString() << std::endl;
            exit(0);
        }

        if (vm.count("input-file")) {
            cout << "Input files are: \n" ;
            for ( auto& s : vm["input-file"].as< vector<string> >() ) {
            	cout << s << " ";
            }
            cout << "\n";
        }
//        else {
//        	cout << "No input files provided" << std::endl;
//        	exit(0);
//        }

        cout << "Verbosity level is " << opt << "\n";                
    }
    catch(exception& e)
    {
        cout << e.what() << "\n";
        exit(1);
    }    

	return vm;
	
}
/* ******************************************************************************************** */
string getProgramVersionString() {
	string rev;
	rev="0.1.1";
	
//#ifdef GOMPI
//	rev+=" (MPI)";
//#endif

	return rev;
}
/* ******************************************************************************************** */
spdlog::logger getLogger(int verbosity) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::info);
//    console_sink->set_level(spdlog::level::debug);
//    console_sink->set_pattern("[rt32tmsrv] [%^%l%$] %v");

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("rt32tmsrv.log", false);
    file_sink->set_level(spdlog::level::debug);


    spdlog::logger logger("rt32tmsrv", {console_sink, file_sink});
    logger.set_level(spdlog::level::info); 
    logger.flush_on(spdlog::level::debug); // this should be a temporary workaround

	if (verbosity>2) {
//		cout << "setting verbosity " << opt["verbosity"].as<int>() << "\n";
		logger.sinks()[0]->set_level(spdlog::level::debug);
		logger.sinks()[1]->set_level(spdlog::level::debug);
		logger.set_level(spdlog::level::debug);
	}

    return logger;
}
/* ******************************************************************************************** */
string getCmdString(int argc, char** argv) {
	stringstream ss;
	for (long i = 0; i < argc; i++) {
		ss << string(argv[i]) << " ";
	}
	return ss.str();
}
/* ******************************************************************************************** */
