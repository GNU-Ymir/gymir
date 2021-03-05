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
	return Expression {new (NO_GC) Match (location, content, matchs, actions, isFinal)};
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

    std::string Match::prettyString () const {
	Ymir::OutBuffer buf;
	//if (this-> _isFinal) buf.write ("final ");
	buf.writef ("match (%)", this-> _content.prettyString ());
	for (auto it : Ymir::r (0, this-> _matchs.size ())) {
	    auto in = this-> _matchs [it].prettyString ();
	    for (auto & j : in) {
		buf.write (j);
		if (j == '\n') buf.write ('\t');
	    }
	    
	    buf.write (" => " );
	    
	    in = this-> _actions [it].prettyString ();
	    for (auto & j : in) {
		buf.write (j);
		if (j == '\n') buf.write ('\t');
	    }
	}
	buf.write ("}");
	return buf.str ();
    }

    const std::set <std::string> & Match::computeSubVarNames () {
	auto cSet = this-> _content.getSubVarNames ();
	for (auto &it : this-> _matchs) {
	    auto &iSet = it.getSubVarNames ();
	    cSet.insert (iSet.begin (), iSet.end ());	    
	}

	for (auto &it : this-> _actions) {
	    auto &iSet = it.getSubVarNames ();
	    cSet.insert (iSet.begin (), iSet.end ());	    
	}
	
	this-> setSubVarNames (cSet);
	return this-> getSubVarNames ();
    }
    
}
