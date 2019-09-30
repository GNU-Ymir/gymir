#include <ymir/syntax/expression/Template.hh>

namespace syntax {

    TemplateCall::TemplateCall () :
	IExpression (lexing::Word::eof ()),
	_content (Expression::empty ())
    {}
    
    TemplateCall::TemplateCall (const lexing::Word & loc) :
	IExpression (loc),
	_content (Expression::empty ())
    {}    
    
    Expression TemplateCall::init (const lexing::Word & location, const std::vector <Expression> & params, const Expression & content) {
	auto ret = new (Z0) TemplateCall (location);
	ret-> _parameters = params;
	ret-> _content = content;
	return Expression {ret};
    }

    Expression TemplateCall::clone () const {
	return Expression {new TemplateCall (*this)};
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

    void TemplateCall::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<TemplateCall>", i, '\t');
	stream.writefln ("%*<Params>", i + 1, '\t');
	for (auto & it : this-> _parameters)
	    it.treePrint (stream, i + 2);
	this-> _content.treePrint (stream, i + 1);
    }

    const std::vector<Expression> & TemplateCall::getParameters () const {
	return this-> _parameters;
    }

    const Expression & TemplateCall::getContent () const {
	return this-> _content;
    }
    
    std::string TemplateCall::prettyString () const {
	Ymir::OutBuffer stream;
	Ymir::OutBuffer inner;
	int i = 0;
	for (auto & x : this-> _parameters) {
	    if (i != 0) inner.write (",");
	    inner.writef ("%", x.prettyString ());
	}
	stream.writef ("%!(%)", this-> _content.prettyString (), inner.str ());
	return stream.str ();
    }
}
