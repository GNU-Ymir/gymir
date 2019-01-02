#include <ymir/utils/Colors.hh>

/**
 * \enum Colors
 * Definition of the values of the colors
 * To retreive the values use the following expression : 
 * \verbatim
 Colors::get (PURPLE)
 \endverbatim
 */
DECLARE_ENUM_WITH_TYPE (
    Colors, std::string,
    PURPLE = "[1;35m",
    BLUE = "[1;36m",
    YELLOW ="[1;33m",
    RED = "[1;31m",
    GREEN = "[1;32m",
    BOLD = "[1;50m",
    RESET = "[0m"
)
