#include "syntax/Word.hh"
#include <sstream>
#include <algorithm>

std::string Word::toString () const {
    if (this-> isEof ()) return ":\u001B[32meof\u001B[0m()";
    auto reset = "\u001B[0m";
    auto purple = "\u001B[36m";
    auto green = "\u001B[32m";
    std::stringstream out;
    out << ":" << green << this-> str << reset << "(" 
	<< this-> locFile.c_str () << " -> "
	<< purple<< LOCATION_LINE (this-> locus)
	<< reset << ", "
	<< purple << LOCATION_COLUMN (this-> locus) << reset << ")";
    return out.str ();
}

std::string Word::getFile () const {
    return this-> locFile;
}

bool Word::isToken () const {
    auto mem = Token::members ();
    return std::find (mem.begin (), mem.end (), this-> str) != mem.end ();
}
