#pragma once

#include <ymir/utils/StringEnum.hh>

// /**
//  * \enum Colors
//  * String enumeration that encode coloration for shell pretty printing
//  * implemented in utils/Colors.cc
//  */
// DECLARE_ENUM_HEADER (
//     Colors, std::string,
//     PURPLE,
//     BLUE,
//     YELLOW,
//     RED,
//     GREEN,
//     BOLD,
//     UNDERLINE,
//     RESET
// )

struct Colors {
  static const std::string PURPLE;
  static const std::string BLUE;
  static const std::string YELLOW;
  static const std::string RED;
  static const std::string GREEN;
  static const std::string BOLD;
  static const std::string UNDERLINE;
  static const std::string RESET;
  static const std::string STRAIGHT_LINE;
  static const std::string CROSS_LINE;
  static const std::string CORNER_LINE;
  static const std::string BOTTOM_LINE;
  static const std::string COL_LINE;
};
