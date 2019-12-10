#include <ymir/syntax/expression/Catch.hh>

namespace syntax {

    Catch::Catch (const lexing::Word & loc, const std::vector<Expression> & vars, const std::vector<Expression> & actions) : 
	IExpression (loc),
	_vars (vars),
	_actions (actions)
    {}

    Catch::Catch () :
	IExpression (lexing::Word::eof ())
    {}
    
    Expression Catch::init (const lexing::Word & location, const std::vector<Expression> & vars, const std::vector<Expression> & actions) {
	return Expression {new (Z0) Catch (location, vars, actions)};
    }

    Expression Catch::clone () const {
	return Expression {new Catch (*this)};
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
	for (auto it : Ymir::r (0, this-> _vars.size ())) {
	    this-> _vars [it].treePrint (stream, i + 1);
	    this-> _actions [it].treePrint (stream, i + 2);
	}	    
    }    

    const std::vector<Expression>& Catch::getVars () const {
	return this-> _vars;
    }

    const std::vector<Expression>& Catch::getActions () const {
	return this-> _actions;
    }
    
}
