
#include "Token.hh"

namespace Lexer {

  Token::Token (TokenId token_id, location_t locus)
    : token_id(token_id),
      locus (locus),
      str (NULL)
  {}

  Token::Token (TokenId token_id, location_t locus, const std::string & str)
    : token_id (token_id),
      locus (locus),
      str (new std::string (str))
  {}
  
  Token::~Token () {
    delete str;
  }


  const char * token_id_to_str (TokenId tid) {
    switch (tid) {
#define YMIR_TOKEN(name, _)			\
    case name:					\
      return #name;
#define YMIR_TOKEN_KEYWORD(x, y) YMIR_TOKEN(x, y)
      YMIR_TOKEN_LIST
#undef YMIR_TOKEN_KEYWORD
#undef YMIR_TOKEN
    default : gcc_unreachable ();
    }
  }


  const char * get_token_description (TokenId tid) {
    switch (tid)
      {
#define YMIR_TOKEN(name, descr)						\
	case name:							\
	  return descr;
#define YMIR_TOKEN_KEYWORD(x, y) YMIR_TOKEN (x, y)
	YMIR_TOKEN_LIST
#undef YMIR_TOKEN_KEYWORD
#undef YMIR_TOKEN
      default:
	gcc_unreachable ();
      } 
  }
};
