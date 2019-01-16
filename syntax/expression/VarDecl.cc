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

    void VarDecl::treePrint (Ymir::OutBuffer & stream, int i) const {
	std::vector<std::string> decosName;
	for (auto it : this-> _decos)
	    switch (it) {
	    case Decorator::REF : decosName.push_back ("ref"); break;
	    case Decorator::CONST : decosName.push_back ("const"); break;
	    case Decorator::MUT : decosName.push_back ("mut"); break;
	    case Decorator::STATIC : decosName.push_back ("static"); break;
	    case Decorator::CTE : decosName.push_back ("cte"); break;
	    }
	
	stream.writef ("%*<VarDecl> : ", i, '\t');
	stream.writeln (this-> _name, "{", decosName, "}");
	this->_type.treePrint (stream, i + 1);
	this->_value.treePrint (stream, i + 1);
    }

    void VarDecl::setValue (const Expression & value) {
	this-> _value = value;
    }
    
}