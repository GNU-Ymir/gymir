#pragma once

#include <ymir/utils/Enum.hh>

/**
 * \enum Colors
 * String enumeration that encode coloration for shell pretty printing
 * implemented in utils/Colors.cc
 */
DECLARE_ENUM_HEADER (
    Colors, std::string,
    PURPLE,
    BLUE,
    YELLOW,
    RED,
    GREEN,
    BOLD,
    RESET
)
