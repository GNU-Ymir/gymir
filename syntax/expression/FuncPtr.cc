#include <ymir/syntax/expression/FuncPtr.hh>

namespace syntax {

    FuncPtr::FuncPtr () :	
	IExpression (lexing::Word::eof ()),
	_ret (Expression::empty ())
    {}
    
    FuncPtr::FuncPtr (const lexing::Word & loc) :
	IExpression (loc),
	_ret (Expression::empty ())
    {}
    
    Expression FuncPtr::init (const lexing::Word & location, const Expression & retType, const std::vector <Expression> & params) {
	auto ret = new (Z0) FuncPtr (location);
	ret-> _ret = retType;
	ret-> _params = params;
	return Expression {ret};
    }

    Expression FuncPtr::clone () const {
	return Expression {new FuncPtr (*this)};
    }

    bool FuncPtr::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	FuncPtr thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    const Expression & FuncPtr::getRetType () const {
	return this-> _ret;
    }

    const std::vector <Expression> & FuncPtr::getParameters () const {
	return this-> _params;
    }
    
    void FuncPtr::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<FuncPtr> ", i, '\t');
	this-> _ret.treePrint (stream, i + 1);
	for (auto & it : this-> _params) {
	    it.treePrint (stream, i + 2);
	}
    }
    
}
