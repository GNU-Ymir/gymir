#include <ymir/syntax/expression/VarDecl.hh>
#include <algorithm>

namespace syntax {
    
    VarDecl::VarDecl (const lexing::Word & loc, const std::vector <DecoratorWord> & decos, const Expression & type, const Expression & value) :
	IExpression (loc),
	_name (loc),
	_type (type),
	_value (value),
	_decos (decos)
    {}

    Expression VarDecl::init (const lexing::Word & name, const std::vector <DecoratorWord> & decos, const Expression & type, const Expression & value) {
	return Expression {new (NO_GC) VarDecl (name, decos, type, value)};
    }
 
    void VarDecl::treePrint (Ymir::OutBuffer & stream, int i) const {
	std::vector<std::string> decosName;
	for (auto it : this-> _decos)
	    switch (it.getValue ()) {
	    case Decorator::REF : decosName.push_back ("ref"); break;
	    case Decorator::CONST : decosName.push_back ("const"); break;
	    case Decorator::MUT : decosName.push_back ("mut"); break;
	    case Decorator::DMUT : decosName.push_back ("dmut"); break;
	    case Decorator::STATIC : decosName.push_back ("static"); break;
	    case Decorator::CTE : decosName.push_back ("cte"); break;
	    }
	
	stream.writef ("%*<VarDecl> : ", i, '\t');
	stream.writeln (this-> _name, "{", decosName, "}");
	this->_type.treePrint (stream, i + 1);
	this->_value.treePrint (stream, i + 1);
    }

    std::string VarDecl::prettyString () const {
	Ymir::OutBuffer buf;
	for (auto it : this-> _decos)
	    switch (it.getValue ()) {
	    case Decorator::REF : buf.write ("ref "); break;
	    case Decorator::MUT : buf.write ("mut "); break;
	    default : {} break;
	    }
	
	buf.writef ("%", getLocation ().getStr ());
	if (!this-> _type.isEmpty ())
	    buf.writef (" : %", this-> _type.prettyString ());
	if (!this-> _value.isEmpty ())
	    buf.writef (" = %", this-> _value.prettyString ());
	return buf.str ();	
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
