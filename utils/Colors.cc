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
    PURPLE = "\u001B[1;35m",
    BLUE = "\u001B[1;36m",
    YELLOW ="\u001B[1;33m",
    RED = "\u001B[1;31m",
    GREEN = "\u001B[1;32m",
    BOLD = "\u001B[1;50m",
    RESET = "\u001B[0m"
)
