#pragma once

#include "config.h"
#include "system.h"
#include "input.h"

#include <string>
#include <tr1/memory>

#define YMIR_TOKEN_LIST					\
  YMIR_TOKEN (FIRST_TOKEN, "<first-token-marker>")	\
  YMIR_TOKEN (END_OF, "end of file")		\
  YMIR_TOKEN (MAIN, "main")				\
  YMIR_TOKEN (STD, "std")				\
  YMIR_TOKEN (IMPORT, "import")				\
  YMIR_TOKEN (DEF, "def")				\
  YMIR_TOKEN (NEW, "new")				\
  YMIR_TOKEN (DELETE, "delete")				\
  YMIR_TOKEN (IF, "if")					\
  YMIR_TOKEN (RETURN, "return")				\
  YMIR_TOKEN (FOR, "for")				\
  YMIR_TOKEN (FOREACH, "foreach")			\
  YMIR_TOKEN (WHILE, "while")				\
  YMIR_TOKEN (BREAK, "break")				\
  YMIR_TOKEN (THROW, "throw")				\
  YMIR_TOKEN (TRY, "try")				\
  YMIR_TOKEN (SWITCH, "switch")				\
    YMIR_TOKEN (DEFAULT, "default")			\
  YMIR_TOKEN (IN, "in")					\
  YMIR_TOKEN (ELSE, "else")				\
  YMIR_TOKEN (CATCH, "catch")				\
  YMIR_TOKEN (TRUE_T, "true")				\
  YMIR_TOKEN (FALSE_T, "false")				\
  YMIR_TOKEN (NULL_T, "null")				\
  YMIR_TOKEN (DEUX_POINT, ":")				\
  YMIR_TOKEN (POINT_VIG, ";")				\
  YMIR_TOKEN (VIRGULE, ",")				\
  YMIR_TOKEN (CROCHET_G, "[")				\
  YMIR_TOKEN (CROCHET_D, "]")				\
  YMIR_TOKEN (INTER, "?")				\
  YMIR_TOKEN (APOS, "'")				\
  YMIR_TOKEN (GUILL, "\"")				\
  YMIR_TOKEN (ESPACE, " ")				\
  YMIR_TOKEN (RETOUR, "\n")				\
  YMIR_TOKEN (TAB, "\t")				\
  YMIR_TOKEN (AFFECT, "=")				\
  YMIR_TOKEN (PLUS_AFF, "+=")				\
  YMIR_TOKEN (MUL_AFF, "*=")				\
  YMIR_TOKEN (SUB_AFF, "-=")				\
  YMIR_TOKEN (DIV_AFF, "/=")				\
  YMIR_TOKEN (S_AND, "&")				\
  YMIR_TOKEN (STAR, "*")				\
  YMIR_TOKEN (AND, "&&")				\
  YMIR_TOKEN (OR, "||")					\
  YMIR_TOKEN (INF, "<")					\
  YMIR_TOKEN (SUP, ">")					\
  YMIR_TOKEN (EQUALS, "==")				\
  YMIR_TOKEN (SUP_EQUALS, ">=")				\
  YMIR_TOKEN (INF_EQUALS, "<=")				\
  YMIR_TOKEN (NOT_EQUALS, "!=")				\
  YMIR_TOKEN (PLUS, "+")				\
  YMIR_TOKEN (SUB,  "-")				\
  YMIR_TOKEN (OR_BIT, "|")				\
  YMIR_TOKEN (LEFT_BIT, "<<")				\
  YMIR_TOKEN (RIGHT_BIT, ">>")				\
  YMIR_TOKEN (XOR, "^")					\
  YMIR_TOKEN (MUL, "*")					\
  YMIR_TOKEN (DIV, "/")					\
  YMIR_TOKEN (AND_BIT, "&")				\
  YMIR_TOKEN (MODULO, "%")				\
  YMIR_TOKEN (DOT, ".")					\
  YMIR_TOKEN (IS, "is")					\
  YMIR_TOKEN (NOT_IS, "!is")				\
  YMIR_TOKEN (NOT, "!")					\
  YMIR_TOKEN (MINUS, "-")				\
  YMIR_TOKEN (PPLUS, "++")				\
  YMIR_TOKEN (SSUB, "--")				\
  YMIR_TOKEN (PRE_RETOUR, "\r")				\
  YMIR_TOKEN (PAR_G, "(")				\
  YMIR_TOKEN (PAR_D, ")")				\
  YMIR_TOKEN (ACC_G, "[")				\
  YMIR_TOKEN (ACC_D, "]")				\
  YMIR_TOKEN (POINT_POINT, "..")			\
  YMIR_TOKEN (COME_DEB, "/*")				\
  YMIR_TOKEN (COME_END, "*/")				\
  YMIR_TOKEN (UNDER_SCORE, "_")				\
  YMIR_TOKEN (ZERO, "0")				\
  YMIR_TOKEN (NEUF, "9")				\
  YMIR_TOKEN (ANTI, "\\")				\
  YMIR_TOKEN (LX, "x")					\
  YMIR_TOKEN (VOID, "void")				\
  YMIR_TOKEN (SYS, "sys")				\
  YMIR_TOKEN (DOLLAR, "$")				\
  YMIR_TOKEN (CAST, "cast")				\
  YMIR_TOKEN (FUNCTION, "function")			\
  YMIR_TOKEN (OTHER, "")				\
  YMIR_TOKEN (LAST, "<last-token-marker>")

namespace Lexical {

  enum TokenId {
#define YMIR_TOKEN(name, _) name,
#define YMIR_TOKEN_KEYWORD(x, y) YMIR_TOKEN (x, y)
    YMIR_TOKEN_LIST
#undef YMIR_TOKEN
#undef YMIR_TOKEN_KEYWORD       
  };

  struct Token;
  typedef std::tr1::shared_ptr<Token> TokenPtr;
  typedef std::tr1::shared_ptr<const Token> const_TokenPtr;
  
  struct Token {

    static TokenPtr make (TokenId token_id, location_t locus) {
      return TokenPtr (new Token (token_id, locus));
    }

    static TokenPtr makeEof () {
      return eof;
    }
    
    static TokenId EOF_TOKEN () {
      return END_OF;
    }

    static TokenPtr make (const std::string & value, location_t locus) {
      return TokenPtr (new Token (OTHER, locus, value));
    }

    TokenId getId () const {
      return token_id;
    }

    location_t getLocus () const {
      return locus;
    }

    const std::string & getStr () const {
      gcc_assert (str != NULL);
      return *str;
    }
    
    ~Token ();
    
  private:

    TokenId token_id;
    location_t locus;
    std::string * str;

    static TokenPtr eof;
    
  private:

    
    Token (TokenId token_id, location_t locus);
    Token (TokenId token_id, location_t locus, const std::string & str);
    Token ();

    Token (const Token &);
    Token & operator = (const Token &);
        
  };

  const char * tokenIdToStr (TokenId tid);
  const char * getTokenDescription (TokenId tid);
  TokenId getFromStr (const std::string &);
  
};
