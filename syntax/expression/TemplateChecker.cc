#include <ymir/syntax/expression/TemplateChecker.hh>

namespace syntax {

    TemplateChecker::TemplateChecker () :
	IExpression (lexing::Word::eof ())
    {}
    
    TemplateChecker::TemplateChecker (const lexing::Word & loc) :
	IExpression (loc)
    {}    
    
    Expression TemplateChecker::init (const lexing::Word & location, const std::vector <Expression> & calls, const std::vector <Expression> & params) {
	auto ret = new (Z0) TemplateChecker (location);
	ret-> _parameters = params;
	ret-> _calls = calls;
	return Expression {ret};
    }

    Expression TemplateChecker::clone () const {
	return Expression {new TemplateChecker (*this)};
    }
    
    bool TemplateChecker::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	TemplateChecker thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }	    

    void TemplateChecker::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<TemplateChecker>", i, '\t');
	stream.writefln ("%*<Calls>", i + 1, '\t');
	for (auto & it : this-> _calls)
	    it.treePrint (stream, i + 2);

	stream.writefln ("%*<Params>", i + 1, '\t');
	for (auto & it : this-> _parameters)
	    it.treePrint (stream, i + 2);       
    }

    const std::vector<Expression> & TemplateChecker::getParameters () const {
	return this-> _parameters;
    }


    const std::vector<Expression> & TemplateChecker::getCalls () const {
	return this-> _calls;
    }

    
    std::string TemplateChecker::prettyString () const {
	Ymir::OutBuffer stream;
	Ymir::OutBuffer inner;
	Ymir::OutBuffer innerCall;
	int i = 0;
	for (auto & x : this-> _parameters) {
	    if (i != 0) inner.write (",");
	    inner.writef ("%", x.prettyString ());
	}
	
	i = 0;
	for (auto & x : this-> _calls) {
	    if (i != 0) innerCall.write (",");
	    innerCall.writef ("%", x.prettyString ());
	}

	stream.writef ("template!(%)(%)", innerCall.str (), inner.str ());
	return stream.str ();
    }
}
