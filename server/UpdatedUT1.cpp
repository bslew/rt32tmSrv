/*
 * UpdatedUT1.cpp
 *
 *  Created on: Aug 19, 2020
 *      Author: blew
 */

#include "../server/UpdatedUT1.h"

namespace rt32 {
	namespace tmsrv {
		
		Update_dUT1::Update_dUT1(
				boost::program_options::variables_map opt,
				spdlog::logger* logger, 
				YAML::Node& srvmemo
				) {
			// TODO Auto-generated constructor stub
			
		}
		
		Update_dUT1::~Update_dUT1() {
			// TODO Auto-generated destructor stub
		}
		
		void Update_dUT1::start() {
			try
			{
				_logger->info("Starting dUT1 updater thread");
/*
				for (int i=0; i<host.size();i++) {
					_logger->info("	emitter destination host:port={}:{}",
							host[i],port[i%port.size()]);
				}
*/

				auto [raw,res]=fetch_raw_iers_data()
				
				
			}
			catch (std::exception& e)
			{
				std::cerr << "Exception: " << e.what() << "\n";
			}
		}
		
		std::string Update_dUT1::fetch_raw_iers_data(std::string addr) {
		}

		std::string Update_dUT1::update() {
		}
	
	} /* namespace tmsrv */
} /* namespace rt32 */
