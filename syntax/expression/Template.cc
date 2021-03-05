#include <ymir/syntax/expression/Template.hh>

namespace syntax {

    TemplateCall::TemplateCall () :
	IExpression (lexing::Word::eof ()),
	_content (Expression::empty ())
    {}
    
    TemplateCall::TemplateCall (const lexing::Word & loc, const std::vector <Expression> & params, const Expression & content) :
	IExpression (loc),
	_parameters (params),
	_content (content)
    {}    
    
    Expression TemplateCall::init (const lexing::Word & location, const std::vector <Expression> & params, const Expression & content) {
	return Expression {new (NO_GC) TemplateCall (location, params, content)};
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
	    if (i != 0) inner.write (", ");
	    inner.writef ("%", x.prettyString ());
	    i += 1;
	}
	stream.writef ("%!(%)", this-> _content.prettyString (), inner.str ());
	return stream.str ();
    }
	
    const std::set <std::string> & TemplateCall::computeSubVarNames () {
	auto cSet = this-> _content.getSubVarNames ();
	for (auto & it : this-> _parameters) {
	    auto & iSet = it.getSubVarNames ();
	    cSet.insert (iSet.begin (), iSet.end ());
	}
	this-> setSubVarNames (cSet);
	return this-> getSubVarNames ();
    }
    
}
