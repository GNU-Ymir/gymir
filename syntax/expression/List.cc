#include <ymir/syntax/expression/List.hh>

namespace syntax {

    List::List () :
	IExpression (lexing::Word::eof ())
    {}
    
    List::List (const lexing::Word & loc) :
	IExpression (loc)
    {}

    Expression List::init (const lexing::Word & location, const lexing::Word & end, const std::vector <Expression> & params) {
	auto ret = new (Z0) List (location);
	ret-> _end = end;
	ret-> _params = params;
	return Expression {ret};
    }

    Expression List::clone () const {
	return Expression {new List (*this)};
    }

    bool List::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	List thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void List::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<List> ", i, '\t');
	stream.writeln (this-> getLocation (), " ", this-> _end);
	for (auto & it : this-> _params)
	    it.treePrint (stream, i + 1);
    }


    bool List::isTuple () const {
	return this-> _end == Token::RPAR;
    }

    bool List::isArray () const {
	return this-> _end == Token::RCRO;
    }

    const std::vector <Expression> & List::getParameters () const {
	return this-> _params;
    }    

    const lexing::Word & List::getEnd () const {
	return this-> _end;
    }

    std::string  List::prettyString () const {
	Ymir::OutBuffer buf;
	buf.write (this-> getLocation ().str);
	int i = 0;
	for (auto &it : this-> _params) {
	    if (i != 0) buf.write (", ");
	    buf.write (it.prettyString ());
	    i ++;
	}
	buf.write (this-> _end.str);
	return buf.str ();	    
    }
    
}
