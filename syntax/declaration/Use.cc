#include <ymir/syntax/declaration/Use.hh>

namespace syntax {

    Use::Use () :
	IDeclaration (lexing::Word::eof (), ""),
	_module (Expression::empty ())
    {}


    Use::Use (const lexing::Word & loc, const std::string & comment, const Expression & module) :
	IDeclaration (loc, comment),
	_module (module)
    {}

    Declaration Use::init (const lexing::Word & loc, const std::string & comment, const Expression & module) {
	return Declaration {new (NO_GC) Use (loc, comment, module)};
    }

    bool Use::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Use thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }

    void Use::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Use> ", i, '\t');
	this-> _module.treePrint (stream, i + 1);
    }

    const Expression & Use::getModule () const {
	return this-> _module;
    }
    
}
