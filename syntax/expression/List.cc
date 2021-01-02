#include <ymir/syntax/expression/List.hh>

namespace syntax {
    
    List::List (const lexing::Word & loc, const lexing::Word & end, const std::vector <Expression> & params) :
	IExpression (loc),
	_end (end),
	_params (params)
    {}

    Expression List::init (const lexing::Word & location, const lexing::Word & end, const std::vector <Expression> & params) {
	return Expression {new (NO_GC) List (location, end, params)};
    }

    Expression List::init (const List & list) {
	return Expression {new (NO_GC) List (list)};
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
	buf.write (this-> getLocation ().getStr ());
	int i = 0;
	for (auto &it : this-> _params) {
	    if (i != 0) buf.write (", ");
	    buf.write (it.prettyString ());
	    i ++;
	}
	buf.write (this-> _end.getStr ());
	return buf.str ();	    
    }
    
}
