#include <ymir/syntax/expression/VarDecl.hh>
#include <algorithm>

namespace syntax {

    VarDecl::VarDecl () :
	IExpression (lexing::Word::eof ()),
	_type (Expression::empty ()),
	_value (Expression::empty ())
    {}
    
    VarDecl::VarDecl (const lexing::Word & loc) :
	IExpression (loc),
	_type (Expression::empty ()),
	_value (Expression::empty ())
    {}

    Expression VarDecl::init (const lexing::Word & name, const std::vector <DecoratorWord> & decos, const Expression & type, const Expression & value) {
	auto ret = new (Z0) VarDecl (name);
	ret-> _name = name;
	ret-> _value = value;
	ret-> _type = type;
	ret-> _decos = decos;
	return Expression {ret};
    }

    Expression VarDecl::clone () const {
	return Expression {new VarDecl (*this)};
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
	    switch (it.getValue ()) {
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

    const lexing::Word & VarDecl::getName () const {
	return this-> _name;
    }

    const Expression & VarDecl::getType () const {
	return this-> _type;
    }

    const Expression & VarDecl::getValue () const {
	return this-> _value;
    }

    bool VarDecl::hasDecorator (Decorator deco) const {
	for (auto it : Ymir::r (0, this-> _decos.size ())) {
	    if (this-> _decos [it].getValue () == deco) return true;
	}
	return false;
    }

    const DecoratorWord & VarDecl::getDecorator (Decorator deco) const {
	for (auto it : Ymir::r (0, this-> _decos.size ())) {
	    if (this-> _decos [it].getValue () == deco) return this-> _decos [it];
	}
	
	Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	return this-> _decos [0];
    }
    
    const std::vector <DecoratorWord> & VarDecl::getDecorators () const {
	return this-> _decos;
    }
    
}
