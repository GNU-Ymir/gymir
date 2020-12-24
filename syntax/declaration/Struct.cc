#include <ymir/syntax/declaration/Struct.hh>

namespace syntax {

    Struct::Struct () :
	IDeclaration (lexing::Word::eof ())
    {}

    Struct::Struct (const lexing::Word & loc, const std::vector <lexing::Word> & attrs, const std::vector <Expression> & vars) :
	IDeclaration (loc),
	_decls (vars),
	_cas (attrs)
    {}        
    
    Declaration Struct::init (const lexing::Word & name, const std::vector <lexing::Word> & attrs, const std::vector <Expression> & vars) {
	return Declaration {new (NO_GC) Struct (name, attrs, vars)};
    }

    bool Struct::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Struct thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
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

}
