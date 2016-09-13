
#include "Token.hh"

namespace Lexical {

    TokenPtr Token::eof = TokenPtr (new Token (END_OF, UNKNOWN_LOCATION));
  
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


    const char * tokenIdToStr (TokenId tid) {
	switch (tid) {
#define YMIR_TOKEN(name, _)			\
	    case name:				\
		return #name;
#define YMIR_TOKEN_KEYWORD(x, y) YMIR_TOKEN(x, y)
	    YMIR_TOKEN_LIST
#undef YMIR_TOKEN_KEYWORD
#undef YMIR_TOKEN
	default : return NULL;
	}
    }

    TokenId getFromStr (const std::string & name_str) {
#define YMIR_TOKEN(name, descr)			\
	if (name_str == descr) return name;
#define YMIR_TOKEN_KEYWORD(x, y) YMIR_TOKEN(x, y)
	YMIR_TOKEN_LIST
#undef YMIR_TOKEN_KEYWORD
#undef YMIR_TOKEN
	    return OTHER;
    }
  
    const char * getTokenDescription (TokenId tid) {
	switch (tid)
	    {
#define YMIR_TOKEN(name, descr)			\
		case name:			\
		    return descr;
#define YMIR_TOKEN_KEYWORD(x, y) YMIR_TOKEN (x, y)
		YMIR_TOKEN_LIST
#undef YMIR_TOKEN_KEYWORD
#undef YMIR_TOKEN
	    default:
		return NULL;
	    } 
    }

    bool Token::operator==(TokenId other) {
	return this->token_id == other;
    }

    void Token::print () {
	printf (":%s (%s -> %i, %i)", getCstr(), LOCATION_FILE (getLocus()), LOCATION_LINE (getLocus()), LOCATION_COLUMN (getLocus()));
    }
    
    
};

