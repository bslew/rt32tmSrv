/*!
 * \file StringParser.h
 *
 *  Created on: Jun 17, 2020
 *      Author: blew
 */

#ifndef SERVER_STRINGPARSER_H_
#define SERVER_STRINGPARSER_H_

#include <algorithm>
#include <string>
#include <unordered_map>	
#include <vector>
#include <list>
#include <sstream>
#include <iostream>
//#include <tuple>
#include <list>
#include <thread>

namespace rt32 {
	
	
	std::string thread_id2str(std::thread::id id);
	

	template <typename T> std::string list2str(std::list<T> L, char sep=',');
	template <typename T> std::string vec2str(std::vector<T> L, char sep=',');

/*!
 \class 
 \brief Encapsulates 
 \details 
 
 \date Jun 17, 2020, 4:56:23 PM 
 \author Bartosz Lew
 */

class StringParser {
	public:
		StringParser();
		StringParser(std::unordered_map<std::string, std::string> dict);
		virtual ~StringParser();
		
		std::unordered_map<std::string, std::string> parse(std::string s, char fieldDelim=',', char kvDelim=':');
		
		bool hasKey(std::string key);
		
		
		int getValsCount(std::string key);

//		static inline void remove_extension(std::string &s) {
//			s.erase(std::find_if(s.rbegin(),s.rend(), [](int ch) {
//				return ch=='.';
//			}).base(), s.end());
//		}

		static inline std::string remove_extension(std::string s) {
			s.erase(--std::find_if(s.rbegin(),s.rend(), [](int ch) {
				return ch=='.';
			}).base(), s.end());
			return s;
		}

		static inline std::string get_extension(std::string s) {
			s.erase(s.begin(), std::find_if(s.rbegin(),s.rend(), [](int ch) {
				return ch=='.';
			}).base());
			return s;
		}

		static inline std::pair<std::string,std::string> break_extension(std::string s) {
			return std::pair<std::string,std::string> (
					remove_extension(s),get_extension(s));
		}
		
		static inline std::string add_suffix(std::string s, std::string suff) {
			return remove_extension(s)+suff+"."+get_extension(s);
		}

		// trim from end (in place)
		static inline void rtrim_nl(std::string &s) {
		    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		        return ch!='\n';
		    }).base(), s.end());
		}
		static inline void rtrim_cr(std::string &s) {
		    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		        return ch!='\r';
		    }).base(), s.end());
		}
		
		// trim from start (in place)
		static inline void ltrim(std::string &s) {
		    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		        return !std::isspace(ch);
		    }));
		}

		// trim from end (in place)
		static inline void rtrim(std::string &s) {
		    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		        return !std::isspace(ch);
		    }).base(), s.end());
		}

		// trim from both ends (in place)
		static inline void trim(std::string &s) {
		    ltrim(s);
		    rtrim(s);
		}		
		
		static inline int count_spaces(std::string &s) {
			return std::count_if(s.begin(),s.end(),[](char ch) { return std::isspace(ch); });
		}

		std::vector<std::string> split(std::string cmd,char delim);
		
		std::string operator[](std::string key) { return dict[key]; }

	protected:
		
		std::unordered_map<std::string, std::string> dict;
		
		
		
		template <typename T> std::vector<T> getValuesAs(std::string s);
};



}

template<typename T>
inline std::vector<T> rt32::StringParser::getValuesAs(std::string s) {
	std::stringstream ss(s);
	T v;
	std::vector<T> V;
	while (ss >> v) {
		V.push_back(v);
	}
	return V;
}

template<typename T>
inline std::string rt32::list2str(std::list<T> L, char sep) {
	std::stringstream ss;
	for (auto el : L) { ss << el << sep; }
	std::string s=ss.str();
	s.erase(--s.end());
	return s;

}
template<typename T>
inline std::string rt32::vec2str(std::vector<T> L, char sep) {
	std::stringstream ss;
	for (auto el : L) { ss << el << sep; }
	std::string s=ss.str();
	s.erase(--s.end());
	return s;

}

#endif /* SERVER_STRINGPARSER_H_ */

