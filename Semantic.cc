#include "Semantic.hh"

namespace Semantic {

    Ast empty () {
	return Ast (Lexical::Token::makeEof ());
    }
    

};
