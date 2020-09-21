#pragma once

#include <string>
#include <vector>

/**
 * \struct Token 
 * The list of token
 * implemented in lexing/Token.cc
 */
struct Token {    
    static std::string	DIV;
    static std::string	DIV_AFF;
    static std::string	DOT;
    static std::string	DDOT; 
    static std::string	TDOT;
    static std::string	AND;
    static std::string	DAND;
    static std::string	PIPE;
    static std::string	DPIPE;
    static std::string	MINUS;
    static std::string	MINUS_AFF;
    static std::string	DMINUS;
    static std::string	PLUS;
    static std::string	PLUS_AFF;
    static std::string	DPLUS;
    static std::string	INF;
    static std::string 	INF_EQUAL;
    static std::string 	LEFTD;
    static std::string 	LEFTD_AFF;
    static std::string 	SUP;
    static std::string 	SUP_EQUAL;
    static std::string 	RIGHTD_AFF;
    static std::string 	RIGHTD;
    static std::string 	NOT;
    static std::string 	NOT_EQUAL;
    static std::string 	LPAR;
    static std::string 	RPAR;
    static std::string 	LCRO;
    static std::string 	RCRO;
    static std::string 	LACC;
    static std::string 	RACC;
    static std::string 	INTEG;
    static std::string 	COMA;
    static std::string 	SEMI_COLON;
    static std::string 	COLON;
    static std::string 	DOLLAR;
    static std::string 	EQUAL;
    static std::string 	DEQUAL;
    static std::string 	STAR;
    static std::string 	STAR_AFF;
    static std::string 	PERCENT;
    static std::string 	PERCENT_AFF; 
    static std::string 	XOR;
    static std::string 	DXOR;
    static std::string 	DXOR_AFF;
    static std::string 	TILDE;
    static std::string 	TILDE_AFF;
    static std::string 	AT;
    static std::string 	DARROW;
    static std::string 	SHARP;
    static std::string 	SPACE;
    static std::string 	RETURN;
    static std::string 	RRETURN;
    static std::string 	ANTI;
    static std::string	LCOMM1;
    static std::string	RCOMM1;
    static std::string	LCOMM2;
    static std::string 	GUILL;
    static std::string 	APOS;
    static std::string 	TAB;
    static std::string	LCOMM3;
    static std::string	RCOMM3;
    static std::string	LCOMM4;
    static std::string	LCOMM5;
    static std::string	RCOMM5;
    static std::string 	ARROW;
    static std::string 	DCOLON;
    static std::string  MACRO_ACC;
    static std::string  MACRO_CRO;
    static std::string  MACRO_PAR;
    static std::string  BROK_SPACE;
    
    static std::vector <std::string> members () {
	return {DIV,		DIV_AFF,	DOT,		DDOT,		TDOT,		AND,		DAND,		PIPE,
		DPIPE,		MINUS,		MINUS_AFF,	DMINUS,		PLUS,		PLUS_AFF,	DPLUS,		INF,
		INF_EQUAL,	LEFTD,		LEFTD_AFF,	SUP,		SUP_EQUAL,	RIGHTD,		RIGHTD_AFF,
		NOT,		NOT_EQUAL,	LPAR,		RPAR,		LCRO,		RCRO,		LACC,		RACC,		INTEG,		COMA,		SEMI_COLON,
		COLON,		DOLLAR,		EQUAL,		DEQUAL,		STAR,		STAR_AFF,	PERCENT,	PERCENT_AFF,
		XOR,		DXOR_AFF,	DXOR,		TILDE,		TILDE_AFF,	AT,		DARROW,
		SHARP,		SPACE,		RETURN,		RRETURN, ANTI, 	LCOMM1,		RCOMM1,		LCOMM2,
		GUILL,		APOS,		TAB,		LCOMM3,		RCOMM3,		LCOMM4,         LCOMM5,         RCOMM5,
		ARROW,		MACRO_ACC,      MACRO_CRO,      MACRO_PAR,	DCOLON,         BROK_SPACE
		};
    }
    
};
