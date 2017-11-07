#include "Symbol.hh"

namespace Semantic {
    
    SymbolPtr Symbol::_empty = SymbolPtr (new Symbol (Lexical::Token::makeEof (), NULL));
    
}
