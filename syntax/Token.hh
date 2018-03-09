#pragma once

#include <string>
#include <vector>

struct Token {    
    static std::string	DIV;
    static std::string	DIV_AFF;
    static std::string	DOT;
    static std::string	DDOT; 
    static std::string	TDOT;
    static std::string	AND;
    static std::string	AND_AFF;
    static std::string	DAND;
    static std::string	PIPE;
    static std::string	PIPE_EQUAL;
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
    static std::string 	NOT_INF;
    static std::string 	NOT_INF_EQUAL;
    static std::string 	NOT_SUP;
    static std::string 	NOT_SUP_EQUAL;
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
    static std::string 	STAR_EQUAL;
    static std::string 	PERCENT;
    static std::string 	PERCENT_EQUAL; 
    static std::string 	XOR;
    static std::string 	XOR_EQUAL;
    static std::string 	DXOR;
    static std::string 	DXOR_EQUAL;
    static std::string 	TILDE;
    static std::string 	TILDE_EQUAL;
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
    static std::string 	ARROW;
    static std::string 	BSTRING;
    static std::string 	ESTRING;
    static std::string 	DCOLON;
    static std::string  MACEX;
    
    static std::vector <std::string> members () {
	return {DIV,		DIV_AFF,	DOT,		DDOT,		TDOT,		AND,		AND_AFF,	DAND,		PIPE,	PIPE_EQUAL,
		DPIPE,		MINUS,		MINUS_AFF,	DMINUS,		PLUS,		PLUS_AFF,	DPLUS,		INF,
		INF_EQUAL,	LEFTD,		LEFTD_AFF,	SUP,		SUP_EQUAL,	RIGHTD,		RIGHTD_AFF,
		NOT,		NOT_EQUAL,	NOT_INF,	NOT_INF_EQUAL,	NOT_SUP,	NOT_SUP_EQUAL,
		LPAR,		RPAR,		LCRO,		RCRO,		LACC,		RACC,		INTEG,		COMA,		SEMI_COLON,
		COLON,		DOLLAR,		EQUAL,		DEQUAL,		STAR,		STAR_EQUAL,	PERCENT,	PERCENT_EQUAL,
		XOR,		XOR_EQUAL,	DXOR_EQUAL,	DXOR,		TILDE,		TILDE_EQUAL,	AT,		DARROW,
		SHARP,		SPACE,		RETURN,		RRETURN, ANTI, 	LCOMM1,		RCOMM1,		LCOMM2,
		GUILL,		APOS,		TAB,		LCOMM3,		RCOMM3,		ARROW,		BSTRING,	ESTRING,	DCOLON, MACEX
		};
    }
    
};
