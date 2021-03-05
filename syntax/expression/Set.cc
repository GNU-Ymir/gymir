#include <ymir/syntax/expression/Set.hh>

namespace syntax {

    Set::Set () :
	IExpression (lexing::Word::eof ())
    {}    
    
    Set::Set (const lexing::Word & loc,  const std::vector <Expression> & params) :
	IExpression (loc),
	_params (params)
    {}

    Expression Set::init (const lexing::Word & location, const std::vector <Expression> & params) {
	return Expression {new (NO_GC) Set (location, params)};
    }

    void Set::treePrint (Ymir::OutBuffer & stream, int i) const {
	for (auto & it : this-> _params)
	    it.treePrint (stream, i);
    }

    const std::vector <Expression> & Set::getContent () const {
	return this-> _params;
    }
    
    std::string Set::prettyString () const {
	Ymir::OutBuffer buf;
	int i = 0;
	for (auto & it : this-> _params) {
	    if (i != 0) buf.write ("\n");
	    buf.write (it.prettyString ());
	    i += 1;
	}
	return buf.str ();
    }
	
    const std::set <std::string> & Set::computeSubVarNames () {
	std::set <std::string> s;
	for (auto & it : this-> _params) {
	    auto & iSet = it.getSubVarNames ();
	    s.insert (iSet.begin (), iSet.end ());
	}
	this-> setSubVarNames (s);
	return this-> getSubVarNames ();
    }

}
