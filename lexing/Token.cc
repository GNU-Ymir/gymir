#include <ymir/lexing/Token.hh>

const std::string Token::DIV = "/";
const std::string Token::DIV_AFF = "/=";
const std::string Token::DOT = ".";
const std::string Token::DOT_AND = ":.";
const std::string Token::DDOT = "..";
const std::string Token::TDOT = "...";
const std::string Token::AND = "&";
const std::string Token::DAND = "&&";
const std::string Token::PIPE = "|";
const std::string Token::DPIPE = "||";
const std::string Token::MINUS = "-";
const std::string Token::MINUS_AFF = "-=";
const std::string Token::DMINUS = "--";
const std::string Token::PLUS = "+";
const std::string Token::PLUS_AFF = "+=";
const std::string Token::DPLUS = "++";
const std::string Token::INF = "<";
const std::string Token::INF_EQUAL = "<=";
const std::string Token::LEFTD = "<<";
const std::string Token::LEFTD_AFF = "<<=";
const std::string Token::SUP = ">";
const std::string Token::SUP_EQUAL = ">=";
const std::string Token::RIGHTD = ">>";
const std::string Token::RIGHTD_AFF = ">>=";
const std::string Token::NOT = "!";
const std::string Token::NOT_EQUAL = "!=";
const std::string Token::LPAR = "(";
const std::string Token::RPAR = ")";
const std::string Token::LCRO = "[";
const std::string Token::RCRO = "]";
const std::string Token::LACC = "{";
const std::string Token::RACC = "}";
const std::string Token::INTEG = "?";
const std::string Token::COMA = ",";
const std::string Token::SEMI_COLON = ";";
const std::string Token::COLON = ":";
const std::string Token::DOLLAR = "$";
const std::string Token::EQUAL = "=";
const std::string Token::DEQUAL = "==";
const std::string Token::STAR = "*";
const std::string Token::STAR_AFF = "*=";
const std::string Token::PERCENT = "%";
const std::string Token::PERCENT_AFF = "%=";
const std::string Token::XOR = "^";
const std::string Token::DXOR = "^^";
const std::string Token::DXOR_AFF = "^^=";
const std::string Token::TILDE = "~";
const std::string Token::TILDE_AFF = "~=";
const std::string Token::AT = "@";
const std::string Token::DARROW = "=>";
const std::string Token::SHARP = "#";
const std::string Token::SPACE = " ";
const std::string Token::RETURN = "\n";
const std::string Token::RRETURN = "\r";
const std::string Token::ANTI = "\\";
const std::string Token::LCOMM1 = "#*";
const std::string Token::RCOMM1 = "*#";
const std::string Token::LCOMM2 = "//";

const std::string Token::LCOMM3 = "/*";
const std::string Token::RCOMM3 = "*/";

const std::string Token::LCOMM4 = "/**";

const std::string Token::LCOMM5 = "/*!";

const std::string Token::GUILL = "\"";
const std::string Token::APOS = "'";
const std::string Token::TAB = "\t";
const std::string Token::ARROW = "->";
const std::string Token::DCOLON = "::";
const std::string Token::MACRO_ACC = "#{";
const std::string Token::MACRO_CRO = "#[";
const std::string Token::MACRO_PAR = "#(";
const std::string Token::MACRO_FOR = "#for";

const std::string Token::BROK_SPACE = "\xc2\xa0";

const std::vector <std::string> Token::__members__ = {Token::DIV,	Token::DIV_AFF,		Token::DOT,		Token::DDOT,		Token::TDOT,		Token::DOT_AND,		Token::AND,	Token::DAND,	Token::PIPE,
	Token::DPIPE,		Token::MINUS,		Token::MINUS_AFF,	Token::DMINUS,		Token::PLUS,		Token::PLUS_AFF,	Token::DPLUS,	Token::INF,
	Token::INF_EQUAL,	Token::LEFTD,		Token::LEFTD_AFF,	Token::SUP,		Token::SUP_EQUAL,	Token::RIGHTD,		Token::RIGHTD_AFF,
	Token::NOT,		Token::NOT_EQUAL,	Token::LPAR,		Token::RPAR,		Token::LCRO,		Token::RCRO,		Token::LACC,
	Token::RACC,		Token::INTEG,		Token::COMA,		Token::SEMI_COLON,
	Token::COLON,		Token::DOLLAR,		Token::EQUAL,		Token::DEQUAL,		Token::STAR,		Token::STAR_AFF,	Token::PERCENT,	Token::PERCENT_AFF,
	Token::XOR,		Token::DXOR_AFF,	Token::DXOR,		Token::TILDE,		Token::TILDE_AFF,	Token::AT,		Token::DARROW,
	Token::SHARP,		Token::SPACE,		Token::RETURN,		Token::RRETURN,		Token::ANTI,		Token::LCOMM1,		Token::RCOMM1,	Token::LCOMM2,
	Token::GUILL,		Token::APOS,		Token::TAB,		Token::LCOMM3,		Token::RCOMM3,		Token::LCOMM4,		Token::LCOMM5,
	Token::ARROW,		Token::MACRO_ACC,	Token::MACRO_CRO,	Token::MACRO_PAR,	Token::DCOLON

};

const std::vector <std::string> & Token::members () {
	return Token::__members__;
}
