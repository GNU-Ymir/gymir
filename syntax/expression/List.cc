#include <ymir/syntax/expression/List.hh>

namespace syntax {

    List::List () {}

    Expression List::init (const List & op) {
	auto ret = new (Z0) List ();
	ret-> _location = op._location;
	ret-> _end = op._end;
	ret-> _params = op._params;
	return Expression {ret};
    }

    Expression List::init (const lexing::Word & location, const lexing::Word & end, const std::vector <Expression> & params) {
	auto ret = new (Z0) List ();
	ret-> _location = location;
	ret-> _end = end;
	ret-> _params = params;
	return Expression {ret};
    }

    Expression List::clone () const {
	return List::init (*this);
    }

    bool List::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	List thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void List::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<List> ", i, '\t');
	stream.writeln (this-> _location, " ", this-> _end);
	for (auto & it : this-> _params)
	    it.treePrint (stream, i + 1);
    }
}
