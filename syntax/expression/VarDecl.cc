#include <ymir/syntax/expression/VarDecl.hh>

namespace syntax {

    VarDecl::VarDecl () :
	_type (Expression::empty ()),
	_value (Expression::empty ())
    {}

    Expression VarDecl::init (const VarDecl & decl) {
	auto ret = new (Z0) VarDecl ();
	ret-> _name = decl._name;
	ret-> _value = decl._value;
	ret-> _type = decl._type;
	ret-> _decos = decl._decos;
	return Expression {ret};
    }

    Expression VarDecl::init (const lexing::Word & name, const std::vector <Decorator> & decos, const Expression & type, const Expression & value) {
	auto ret = new (Z0) VarDecl ();
	ret-> _name = name;
	ret-> _value = value;
	ret-> _type = type;
	ret-> _decos = decos;
	return Expression {ret};
    }

    Expression VarDecl::clone () const {
	return VarDecl::init (*this);
    }

    bool VarDecl::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	VarDecl thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

}
