#include "Token.hh"

namespace Lexical {

    TokenPtr Token::eof = Token::make ("end of file", UNKNOWN_LOCATION);
  
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
	auto reset = "\u001B[0m";
	auto purple = "\u001B[36m";
	auto green = "\u001B[32m";
	if (token_id == OTHER)	    
	    printf (":%s%s%s (%s -> %s%i%s, %s%i%s)",
		    green,
		    getCstr(),
		    reset,
		    LOCATION_FILE (getLocus()),
		    purple,
		    LOCATION_LINE (getLocus()),
		    reset,
		    purple,
		    LOCATION_COLUMN (getLocus()), reset);
	
	else
	    printf (":%s%s%s (%s -> %s%i%s, %s%i%s)",
		    green,
		    tokenIdToStr(token_id),
		    reset,
		    LOCATION_FILE (getLocus()),
		    purple,
		    LOCATION_LINE (getLocus()),
		    reset,
		    purple,
		    LOCATION_COLUMN (getLocus()), reset);

    }

    std::string Token::locusToString () const {
	std::string s(LOCATION_FILE (getLocus()));
	s += ":" + LOCATION_LINE (getLocus());
	s += ":" + LOCATION_COLUMN (getLocus());
	return s;
    }
    
    
};

