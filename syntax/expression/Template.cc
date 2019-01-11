#include <ymir/syntax/expression/Template.hh>

namespace syntax {

    TemplateCall::TemplateCall () : _content (Expression::empty ())
    {}
    
    Expression TemplateCall::init () {
	return Expression {new (Z0) TemplateCall ()};
    }

    Expression TemplateCall::init (const TemplateCall & tmpl) {
	auto ret = new (Z0) TemplateCall ();
	ret-> _parameters = tmpl._parameters;
	ret-> _content = tmpl._content;
	return Expression {ret};
    }

    Expression TemplateCall::init (const std::vector <Expression> & params, const Expression & content) {
	auto ret = new (Z0) TemplateCall ();
	ret-> _parameters = params;
	ret-> _content = content;
	return Expression {ret};
    }

    Expression TemplateCall::clone () const {
	return TemplateCall::init (*this);
    }
    
    bool TemplateCall::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	TemplateCall thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }	    

    void TemplateCall::addParameter (const Expression & param) {
	this-> _parameters.push_back (param);
    }

    void TemplateCall::setContent (const Expression& content) {
	this-> _content = content;
    }
    
    
}
