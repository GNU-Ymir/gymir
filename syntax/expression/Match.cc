#include <ymir/syntax/expression/Match.hh>

namespace syntax {

    Match::Match () :
	IExpression (lexing::Word::eof ()),
	_content (Expression::empty ())
    {}
    
    Match::Match (const lexing::Word & loc, const Expression & content, const std::vector <Expression> & matchs, const std::vector <Expression> & actions, bool isFinal) :
	IExpression (loc),
	_content (content),
	_matchs (matchs),
	_actions (actions),
	_isFinal (isFinal)
    {}
    
    Expression Match::init (const lexing::Word & location, const Expression & content, const std::vector <Expression> & matchs, const std::vector <Expression> & actions, bool isFinal) {
	return Expression {new (Z0) Match (location, content, matchs, actions, isFinal)};
    }

    bool Match::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Match thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    
    const Expression & Match::getContent () const {
	return this-> _content;
    }


    const std::vector <Expression> & Match::getMatchers () const {
	return this-> _matchs;
    }

    const std::vector <Expression> & Match::getActions () const {
	return this-> _actions;
    }

    void Match::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Match>", i, '\t');
	this-> _content.treePrint (stream, i + 1);
	for (auto it : Ymir::r (0, this-> _matchs.size ())) {
	    this-> _matchs [it].treePrint (stream, i + 2);
	    this-> _actions [it].treePrint (stream, i + 2);
	}
    }

    bool Match::isFinal () const {
	return this-> _isFinal;
    }
    
}
