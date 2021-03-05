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
	return Expression {new (NO_GC) Catch (location, matchs, actions)};
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

    std::string Catch::prettyString () const {
	Ymir::OutBuffer buf;
	buf.write ("catch {\n");
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


    
    const std::set <std::string> & Catch::computeSubVarNames () {
	std::set <std::string> s;
	for (auto & it : this-> _matchs) {
	    auto & iSet = it.getSubVarNames ();
	    s.insert (iSet.begin (), iSet.end ());
	}
	
	for (auto & it : this-> _actions) {
	    auto & iSet = it.getSubVarNames ();
	    s.insert (iSet.begin (), iSet.end ());
	}
	this-> setSubVarNames (s);
	return this-> getSubVarNames ();
    }
    
}
