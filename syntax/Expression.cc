#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Declaration.hh>
#include <ymir/syntax/declaration/ExpressionWrapper.hh>

namespace syntax {
    
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
    
    Declaration Expression::toDeclaration (const Expression & value) {
	return ExpressionWrapper::init (value.getLocation (), "", value);
    }
    
    void Expression::treePrint (Ymir::OutBuffer & stream, int i)  const {	
	if (this-> _value == nullptr) {
	    stream.writef ("%*", i, '\t');
	    stream.writeln ("<null>");
	} else this-> _value-> treePrint (stream, i);
    }

    std::string Expression::prettyString () const {
	if (this-> _value == nullptr) return "";
	else return this-> _value-> prettyString ();
    }

    bool IExpression::isOf (const IExpression *) const {
	return false;
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

    // std::string IExpression::prettyString () const {
    // 	return "";
    // }
    
    IExpression::~IExpression () {}


    
}
