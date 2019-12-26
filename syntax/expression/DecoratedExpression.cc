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
	return Expression {new (Z0) DecoratedExpression (location, decos, content)};
    }
        
    Expression DecoratedExpression::clone () const {
	return Expression {new DecoratedExpression (*this)};
    }

    bool DecoratedExpression::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	DecoratedExpression thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
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
}
