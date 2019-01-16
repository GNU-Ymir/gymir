#include <ymir/syntax/expression/Var.hh>

namespace syntax {

    Var::Var ()  {}

    Expression Var::init (const Var & alloc) {
	auto ret = new (Z0) Var ();
	ret-> _token = alloc._token;
	ret-> _decos = alloc._decos;
	return Expression {ret};
    }
    
    Expression Var::init (const lexing::Word & location) {
	auto ret = new (Z0) Var ();
	ret-> _token = location;
	return Expression {ret};
    }
    
    Expression Var::init (const lexing::Word & location, const std::vector<Decorator> & decos) {
	auto ret = new (Z0) Var ();
	ret-> _token = location;
	ret-> _decos = decos;
	return Expression {ret};
    }
    
    Expression Var::clone () const {
	return Var::init (*this);
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
	stream.writeln (this-> _token, "{", decosName, "}");
    }
}
