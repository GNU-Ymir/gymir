#include <ymir/syntax/expression/AliasVar.hh>

namespace syntax {

    AliasVar::AliasVar () :
	IExpression (lexing::Word::eof ())
    {}
    
    AliasVar::AliasVar (const lexing::Word & loc) :
	IExpression (loc)
    {}
        
    Expression AliasVar::init (const lexing::Word & location) {
	return Expression {new (Z0) AliasVar (location)};
    }

    bool AliasVar::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	AliasVar thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void AliasVar::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<AliasVar> ", i, '\t');
	stream.writeln (this-> getLocation ());
    }

    std::string AliasVar::prettyString () const {
	return "alias " + this-> getLocation ().str;
    }
}
