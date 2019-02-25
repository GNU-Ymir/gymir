#include <ymir/syntax/declaration/Import.hh>
#include <ymir/utils/Path.hh>

namespace syntax {

    Import::Import () {}

    Declaration Import::init () {
	return Declaration {new (Z0) Import ()};
    }

    Declaration Import::init (const Import & imp) {
	auto ret = new (Z0) Import ();
	ret-> _module = imp._module;
	ret-> _as = imp._as;
	return Declaration {ret};
    }

    Declaration Import::init (const lexing::Word & module, const lexing::Word & as) {
	auto ret = new (Z0) Import ();
	ret-> _module = module;
	ret-> _as = as;
	return Declaration {ret};
    }

    Declaration Import::clone () const {
	return Import::init (*this);
    }

    bool Import::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Import thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }

    void Import::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Import> ", i, '\t');
	stream.writeln (this-> _module, " as ", this-> _as);	
    }    
    
    void Import::setModule (const lexing::Word & name) {
	this-> _module = name;
    }

    void Import::setName (const lexing::Word & name) {
	this-> _as = name;
    }

    const lexing::Word & Import::getModule () const {
	return this-> _module;
    }
    
    std::string Import::getPath () const {
	auto path = Ymir::Path {this-> _module.str, "::"};
	return path.toString ();
    }
        
}
