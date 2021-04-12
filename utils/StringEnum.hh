#pragma once

#include <map>
#include <string>
#include <vector>
#include <time.h>
#include <stdio.h>

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
	const T & get (E##Value code);			\
    }						

#define DECLARE_ENUM_WITH_TYPE(E, T, ...)				\
    namespace E								\
    {									\
	const T & get (E##Value code) {					\
	    static std::vector <T> MapName;				\
	    static bool init = false;					\
	    if (!init) {						\
		init = true;						\
		auto splits = std::move (splitString (#__VA_ARGS__));	\
		MapName.reserve (splits.size ());			\
		for (auto & it : splits) {				\
		    std::vector <std::string> value = splitString (it, '='); \
		    MapName.push_back (strip ((T) value [1]));		\
		}							\
	    }								\
	    return MapName [code];					\
	}								\
    };
