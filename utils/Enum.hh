#pragma once

#include <map>
#include <string>
#include <vector>

#define STRING_REMOVE_CHAR(str, ch) str.erase(std::remove(str.begin(), str.end(), ch), str.end())

/**
 * \brief Split the string in multiple substring separated by sep
 * \param str the string to split
 * \param sep the char separating the final string (not inserted in the results)
 * implemented in utils/Enum.cc
 */
std::vector<std::string> splitString(const std::string& str, char sep = ',');

/**
 * \brief remove the space in the prefix and suffix of the string
 * implemented in utils/Enum.cc
 */
std::string strip (const std::string & elem);       

#define DECLARE_ENUM_HEADER(E, T, ...)		\
    enum E##Value {				\
	__VA_ARGS__				\
	    };					\
    namespace E {				\
	T get (E##Value code);			\
    }						

#define DECLARE_ENUM_WITH_TYPE(E, T, ...)				\
    namespace E								\
    {									\
	T get (E##Value code) {						\
	    static std::map <int, T> MapName;				\
	    if (MapName.empty ()) {					\
		auto splits = splitString (#__VA_ARGS__);		\
		int i = 0;						\
		for (auto it : splits) {				\
		    std::vector <std::string> value = splitString (it, '='); \
		    MapName [i] = (T) value [1];			\
		    i ++;						\
		}							\
	    }								\
	    return MapName [code];					\
	}								\
    };
