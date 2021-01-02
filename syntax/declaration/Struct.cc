#include <ymir/syntax/declaration/Struct.hh>

namespace syntax {

    Struct::Struct () :
	IDeclaration (lexing::Word::eof (), "")
    {}

    Struct::Struct (const lexing::Word & loc, const std::string & comment, const std::vector <lexing::Word> & attrs, const std::vector <Expression> & vars, const std::vector <std::string> & fieldComms) :
	IDeclaration (loc, comment),
	_decls (vars),
	_cas (attrs),
	_field_comments (fieldComms)
    {}        
    
    Declaration Struct::init (const lexing::Word & name, const std::string & comment, const std::vector <lexing::Word> & attrs, const std::vector <Expression> & vars, const std::vector <std::string> & fieldComms) {
	return Declaration {new (NO_GC) Struct (name, comment, attrs, vars, fieldComms)};
    }

    void Struct::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Struct> ", i, '\t');
	stream.writeln (this-> getLocation (), " @{", this-> _cas, "}");

	for (auto & it : this-> _decls) {
	    it.treePrint (stream, i + 1);
	}
    }
    
    const std::vector <lexing::Word> & Struct::getCustomAttributes () const {
	return this-> _cas;
    }
    
    const std::vector <Expression> & Struct::getDeclarations () const {
	return this-> _decls;
    }

    const std::vector <std::string> & Struct::getDeclComments () const {
	return this-> _field_comments;
    }
    
}
