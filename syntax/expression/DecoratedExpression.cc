#include <ymir/syntax/expression/DecoratedExpression.hh>

namespace syntax {

    DecoratedExpression::DecoratedExpression () :
	IExpression (lexing::Word::eof ()),
	_decos (),
	_content (Expression::empty ())
    {}
    
    DecoratedExpression::DecoratedExpression (const lexing::Word & loc, const std::vector <DecoratorWord> & decos, const Expression & content) :
	IExpression (loc),
	_decos (decos),
	_content (content)
    {}
    
    Expression DecoratedExpression::init (const lexing::Word & location, const std::vector <DecoratorWord> & decos, const Expression & content) {
	return Expression {new (NO_GC) DecoratedExpression (location, decos, content)};
    }        

    void DecoratedExpression::treePrint (Ymir::OutBuffer & stream, int i) const {
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
	
	stream.writef ("%*<DecoratedExpression> : ", i, '\t');
	stream.writeln (this-> getLocation (), "{", decosName, "}");
	this-> _content.treePrint (stream, i + 1);
    }

    const Expression & DecoratedExpression::getContent () const {
	return this-> _content;
    }

    bool DecoratedExpression::hasDecorator (Decorator deco) const {
	for (auto it : Ymir::r (0, this-> _decos.size ())) {
	    if (this-> _decos [it].getValue () == deco) return true;
	}
	return false;
    }

    const DecoratorWord & DecoratedExpression::getDecorator (Decorator deco) const {
	for (auto it : Ymir::r (0, this-> _decos.size ())) {
	    if (this-> _decos [it].getValue () == deco) return this-> _decos [it];
	}
	
	Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	return this-> _decos [0];
    }
    
    const std::vector <DecoratorWord> & DecoratedExpression::getDecorators () const {
	return this-> _decos;
    }

    std::string DecoratedExpression::prettyDecorators () const {
	Ymir::OutBuffer buf;
	int i = 0;
	for (auto it : this-> _decos) {
	    if (i != 0) buf.write (" ");
	    switch (it.getValue ()) {
	    case Decorator::REF : buf.write  ("ref"); break;
	    case Decorator::CONST : buf.write ("const"); break;
	    case Decorator::MUT : buf.write ("mut"); break;
	    case Decorator::DMUT : buf.write ("dmut"); break;
	    case Decorator::STATIC : buf.write ("static"); break;
	    case Decorator::CTE : buf.write ("cte"); break;
	    }
	    i += 1;
	}
	return buf.str ();
    }

    std::string DecoratedExpression::prettyString () const {
	Ymir::OutBuffer buf;
	int i = 0;
	for (auto it : this-> _decos) {
	    if (i != 0) buf.write (" ");
	    switch (it.getValue ()) {
	    case Decorator::REF : buf.write  ("ref"); break;
	    case Decorator::CONST : buf.write ("const"); break;
	    case Decorator::MUT : buf.write ("mut"); break;
	    case Decorator::DMUT : buf.write ("dmut"); break;
	    case Decorator::STATIC : buf.write ("static"); break;
	    case Decorator::CTE : buf.write ("cte"); break;
	    }
	    i += 1;
	}
	if (i != 0) buf.write (" ");
	buf.write (this-> _content.prettyString ());
	return buf.str ();	
    }
    
}
