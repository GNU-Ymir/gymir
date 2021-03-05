#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/declaration/ExpressionWrapper.hh>

namespace syntax {

    std::set <std::string> Expression::__empty_sub_names__;
    
    IExpression::IExpression (const lexing::Word & location) :
	_location (location)
    {}
    
    Expression::Expression (IExpression * expr) : RefProxy<IExpression, Expression> (expr)
    {}

    Expression Expression::empty () {
	return Expression {nullptr};
    }

    bool Expression::isEmpty () const {
	return this-> _value == nullptr;
    }
    
    Declaration Expression::toDeclaration (const Expression & value, const std::string & comments) {
	return ExpressionWrapper::init (value.getLocation (), comments, value);
    }
    
    void Expression::treePrint (Ymir::OutBuffer & stream, int i)  const {	
	if (this-> _value == nullptr) {
	    stream.writef ("%*", i, '\t');
	    stream.writeln ("<null>");
	} else this-> _value-> treePrint (stream, i);
    }

    const std::set <std::string> & Expression::getSubVarNames () const {
	if (this-> _value == nullptr) {
	    return __empty_sub_names__;
	}
	if (this-> _value-> _set_sub_var_names) 
	    return this-> _value-> getSubVarNames ();
	else {
	    auto aux = *this;
	    std::set <std::string> names;
	    return aux._value-> computeSubVarNames ();
	}
    }

    const std::set <std::string> & Expression::computeSubVarNames () {
	if (this-> _value != nullptr)
	    return this-> _value-> computeSubVarNames ();
	else
	    return __empty_sub_names__;
    }
    
    std::string Expression::prettyString () const {
	if (this-> _value == nullptr) return "";
	else return this-> _value-> prettyString ();
    }

    const lexing::Word & IExpression::getLocation () const {
	return this-> _location;
    }    

    const lexing::Word & Expression::getLocation () const {
	if (this-> _value == nullptr)
	    Ymir::Error::halt (Ymir::ExternalError::get (Ymir::NULL_PTR));
	return this-> _value-> getLocation ();
    }    
    
    void IExpression::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<TODO>", i, '\t');
    }

    void IExpression::setSubVarNames (const std::set <std::string> & names) {
	this-> _set_sub_var_names = true;
	this-> _sub_var_names = names;
    }

    const std::set <std::string> & IExpression::getSubVarNames () const {
	return this-> _sub_var_names;
    }
    
    // std::string IExpression::prettyString () const {
    // 	return "";
    // }
    
    IExpression::~IExpression () {}


    
}
