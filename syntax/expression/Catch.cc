#include <ymir/syntax/expression/Catch.hh>

namespace syntax {

    Catch::Catch (const lexing::Word & loc, const std::vector<Expression> & matchs, const std::vector<Expression> & actions) : 
	IExpression (loc),
	_matchs (matchs),
	_actions (actions)
    {}

    Catch::Catch () :
	IExpression (lexing::Word::eof ())
    {}
    
    Expression Catch::init (const lexing::Word & location, const std::vector<Expression> & matchs, const std::vector<Expression> & actions) {
	return Expression {new (Z0) Catch (location, matchs, actions)};
    }

    bool Catch::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Catch thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Catch::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Catch> ", i, '\t');
	stream.writeln (this-> getLocation ());
	for (auto it : Ymir::r (0, this-> _matchs.size ())) {
	    this-> _matchs [it].treePrint (stream, i + 1);
	    this-> _actions [it].treePrint (stream, i + 2);
	}	    
    }    

    const std::vector<Expression>& Catch::getMatchs () const {
	return this-> _matchs;
    }

    const std::vector<Expression>& Catch::getActions () const {
	return this-> _actions;
    }
    
}
