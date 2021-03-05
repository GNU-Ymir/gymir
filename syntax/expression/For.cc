#include <ymir/syntax/expression/For.hh>

namespace syntax {

    For::For () :
	IExpression (lexing::Word::eof ()),
	_iter (Expression::empty ()),
	_block (Expression::empty ())
    {}
    
    For::For (const lexing::Word & loc, const std::vector <Expression> & vars, const Expression & iter, const Expression & block) :
	IExpression (loc),
	_vars (vars),
	_iter (iter),
	_block (block)
    {}


    Expression For::init (const lexing::Word & location, const std::vector <Expression> & vars, const Expression & iter, const Expression & block) {
	return Expression {new (NO_GC) For (location, vars, iter, block)};
    }
    
    void For::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<For>", i, '\t');
	stream.writefln ("%*<Vars>", i + 1, '\t');
	for (auto & it : this-> _vars)
	    it.treePrint (stream, i + 2);

	stream.writefln ("%*<Iter>", i + 1, '\t');
	this-> _iter.treePrint (stream, i + 2);
	this-> _block.treePrint (stream, i + 1);
    }

    const Expression & For::getIter () const {
	return this-> _iter;
    }

    const Expression & For::getBlock () const {
	return this-> _block;
    }

    const std::vector <Expression> & For::getVars () const {
	return this-> _vars;
    }
    
    std::string For::prettyString () const {
	Ymir::OutBuffer buf;
	buf.write ("for ");
	int i = 0;
	for (auto & it : this-> _vars) {
	    if (i != 0) buf.write (", ");
	    buf.write (it.prettyString ());
	    i += 1;
	}
	buf.write (" in ");
	buf.write (this-> _iter.prettyString ());
	buf.write (" ");
	buf.write (this-> _block.prettyString ());
	return buf.str ();
    }

    const std::set <std::string> & For::computeSubVarNames () {
	auto sSet = this-> _iter.getSubVarNames ();
	auto & bSet = this-> _block.getSubVarNames ();
	sSet.insert (bSet.begin (), bSet.end ());
	for (auto &it : this-> _vars) {
	    auto & iSet = it.getSubVarNames ();
	    sSet.insert (iSet.begin (), iSet.end ());
	}
	
	this-> setSubVarNames (sSet);
	return this-> getSubVarNames ();
    }
    
}
