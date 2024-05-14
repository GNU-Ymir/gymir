#pragma once

#include <string>
#include <vector>

/**
 * \struct Token 
 * The list of token
 * implemented in lexing/Token.cc
 */
struct Token {

    static const std::vector <std::string> __members__;
    
    static const std::string	DIV;
    static const std::string	DIV_AFF;
    static const std::string	DOT;
    static const std::string	DDOT; 
    static const std::string	TDOT;
    static const std::string    DOT_AND;
    static const std::string	AND;
    static const std::string	DAND;
    static const std::string	PIPE;
    static const std::string	DPIPE;
    static const std::string	MINUS;
    static const std::string	MINUS_AFF;
    static const std::string	DMINUS;
    static const std::string	PLUS;
    static const std::string	PLUS_AFF;
    static const std::string	DPLUS;
    static const std::string	INF;
    static const std::string 	INF_EQUAL;
    static const std::string 	LEFTD;
    static const std::string 	LEFTD_AFF;
    static const std::string 	SUP;
    static const std::string 	SUP_EQUAL;
    static const std::string 	RIGHTD_AFF;
    static const std::string 	RIGHTD;
    static const std::string 	NOT;
    static const std::string 	NOT_EQUAL;
    static const std::string 	LPAR;
    static const std::string 	RPAR;
    static const std::string 	LCRO;
    static const std::string 	RCRO;
    static const std::string 	LACC;
    static const std::string 	RACC;
    static const std::string 	INTEG;
    static const std::string 	COMA;
    static const std::string 	SEMI_COLON;
    static const std::string 	COLON;
    static const std::string 	DOLLAR;
    static const std::string 	EQUAL;
    static const std::string 	DEQUAL;
    static const std::string 	STAR;
    static const std::string 	STAR_AFF;
    static const std::string 	PERCENT;
    static const std::string 	PERCENT_AFF; 
    static const std::string 	XOR;
    static const std::string 	DXOR;
    static const std::string 	DXOR_AFF;
    static const std::string 	TILDE;
    static const std::string 	TILDE_AFF;
    static const std::string 	AT;
    static const std::string 	DARROW;
    static const std::string 	SHARP;
    static const std::string 	SPACE;
    static const std::string 	RETURN;
    static const std::string 	RRETURN;
    static const std::string 	ANTI;
    static const std::string	LCOMM1;
    static const std::string	RCOMM1;
    static const std::string	LCOMM2;
    static const std::string 	GUILL;
    static const std::string 	APOS;
    static const std::string 	TAB;
    static const std::string	LCOMM3;
    static const std::string	RCOMM3;
    static const std::string	LCOMM4;
    static const std::string	LCOMM5;
    static const std::string 	ARROW;
    static const std::string 	DCOLON;
    static const std::string  MACRO_ACC;
    static const std::string  MACRO_CRO;
    static const std::string  MACRO_PAR;
    static const std::string  MACRO_FOR;
    static const std::string  BROK_SPACE;
    
    static const std::vector <std::string> & members () ;
    
};
