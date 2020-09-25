#include <ymir/lexing/Token.hh>

std::string Token::DIV = "/";
std::string Token::DIV_AFF = "/=";
std::string Token::DOT = ".";
std::string Token::DDOT = "..";
std::string Token::TDOT = "...";
std::string Token::AND = "&";
std::string Token::DAND = "&&";
std::string Token::PIPE = "|";
std::string Token::DPIPE = "||";
std::string Token::MINUS = "-";
std::string Token::MINUS_AFF = "-=";
std::string Token::DMINUS = "--";
std::string Token::PLUS = "+";
std::string Token::PLUS_AFF = "+=";
std::string Token::DPLUS = "++";
std::string Token::INF = "<";
std::string Token::INF_EQUAL = "<=";
std::string Token::LEFTD = "<<";
std::string Token::LEFTD_AFF = "<<=";
std::string Token::SUP = ">";
std::string Token::SUP_EQUAL = ">=";
std::string Token::RIGHTD = ">>";
std::string Token::RIGHTD_AFF = ">>=";
std::string Token::NOT = "!";
std::string Token::NOT_EQUAL = "!=";
std::string Token::LPAR = "(";
std::string Token::RPAR = ")";
std::string Token::LCRO = "[";
std::string Token::RCRO = "]";
std::string Token::LACC = "{";
std::string Token::RACC = "}";
std::string Token::INTEG = "?";
std::string Token::COMA = ",";
std::string Token::SEMI_COLON = ";";
std::string Token::COLON = ":";
std::string Token::DOLLAR = "$";
std::string Token::EQUAL = "=";
std::string Token::DEQUAL = "==";
std::string Token::STAR = "*";
std::string Token::STAR_AFF = "*=";
std::string Token::PERCENT = "%";
std::string Token::PERCENT_AFF = "%=";
std::string Token::XOR = "^";
std::string Token::DXOR = "^^";
std::string Token::DXOR_AFF = "^^=";
std::string Token::TILDE = "~";
std::string Token::TILDE_AFF = "~=";
std::string Token::AT = "@";
std::string Token::DARROW = "=>";
std::string Token::SHARP = "#";
std::string Token::SPACE = " ";
std::string Token::RETURN = "\n";
std::string Token::RRETURN = "\r";
std::string Token::ANTI = "\\";
std::string Token::LCOMM1 = "#*";
std::string Token::RCOMM1 = "*#";
std::string Token::LCOMM2 = "//";

std::string Token::LCOMM3 = "/*";
std::string Token::RCOMM3 = "*/";

std::string Token::LCOMM4 = "/**";

std::string Token::LCOMM5 = "/++";
std::string Token::RCOMM5 = "+/";

std::string Token::GUILL = "\"";
std::string Token::APOS = "'";
std::string Token::TAB = "\t";
std::string Token::ARROW = "->";
std::string Token::DCOLON = "::";
std::string Token::MACRO_ACC = "#{";
std::string Token::MACRO_CRO = "#[";
std::string Token::MACRO_PAR = "#(";
std::string Token::MACRO_FOR = "#for";

std::string Token::BROK_SPACE = "\xc2\xa0";
