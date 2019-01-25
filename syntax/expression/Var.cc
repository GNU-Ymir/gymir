#include <ymir/syntax/expression/Var.hh>

namespace syntax {

    Var::Var () :
	IExpression (lexing::Word::eof ())
    {}
    
    Var::Var (const lexing::Word & loc) :
	IExpression (loc)
    {}
    
    Expression Var::init (const lexing::Word & location) {
	return Expression {new (Z0) Var (location)};
    }
    
    Expression Var::init (const lexing::Word & location, const std::vector<Decorator> & decos) {
	auto ret = new (Z0) Var (location);
	ret-> _decos = decos;
	return Expression {ret};
    }
    
    Expression Var::clone () const {
	return Expression {new Var (*this)};
    }

    bool Var::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Var thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }


    void Var::treePrint (Ymir::OutBuffer & stream, int i) const {
	std::vector<std::string> decosName;
	for (auto it : this-> _decos)
	    switch (it) {
	    case Decorator::REF : decosName.push_back ("ref"); break;
	    case Decorator::CONST : decosName.push_back ("const"); break;
	    case Decorator::MUT : decosName.push_back ("mut"); break;
	    case Decorator::STATIC : decosName.push_back ("static"); break;
	    case Decorator::CTE : decosName.push_back ("cte"); break;
	    }
	
	stream.writef ("%*<Var> : ", i, '\t');
	stream.writeln (this-> getLocation (), "{", decosName, "}");
    }

    const lexing::Word & Var::getName () const {
	return this-> getLocation ();
    }
    
}
