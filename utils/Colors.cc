#include <ymir/utils/Colors.hh>

// /**
//  * \enum Colors
//  * Definition of the values of the colors
//  * To retreive the values use the following expression : 
//  * \verbatim
//  Colors::get (PURPLE)
//  \endverbatim
//  */
// DECLARE_ENUM_WITH_TYPE (
//     Colors, std::string,
//     PURPLE = 
//     BLUE = "[1;36m",
//     YELLOW ="[1;33m",
//     RED = "[1;31m",
//     GREEN = "[1;32m",
//     BOLD = "[1;50m",
//     UNDERLINE = "[4m",
//     RESET = "[0m"
// )

const std::string Colors::PURPLE = "[1;35m";
const std::string Colors::BLUE = "[1;36m";
const std::string Colors::YELLOW = "[1;33m";
const std::string Colors::RED = "[1;31m";
const std::string Colors::GREEN = "[1;32m";
const std::string Colors::BOLD = "[1;50m";
const std::string Colors::UNDERLINE = "[4m";
const std::string Colors::RESET = "[0m";

