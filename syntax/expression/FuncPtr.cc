#include <ymir/syntax/expression/FuncPtr.hh>

namespace syntax {

    FuncPtr::FuncPtr () :	
	IExpression (lexing::Word::eof ()),
	_ret (Expression::empty ())
    {}
    
    FuncPtr::FuncPtr (const lexing::Word & loc, const Expression & ret, const std::vector <Expression> & params) :
	IExpression (loc),
	_params (params),
	_ret (ret)
    {}
    
    Expression FuncPtr::init (const lexing::Word & location, const Expression & retType, const std::vector <Expression> & params) {
	return Expression {new (NO_GC) FuncPtr (location, retType, params)};
    }

    const Expression & FuncPtr::getRetType () const {
	return this-> _ret;
    }

    const std::vector <Expression> & FuncPtr::getParameters () const {
	return this-> _params;
    }
    
    void FuncPtr::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<FuncPtr> ", i, '\t');
	this-> _ret.treePrint (stream, i + 1);
	for (auto & it : this-> _params) {
	    it.treePrint (stream, i + 2);
	}
    }

    std::string FuncPtr::prettyString () const {
	Ymir::OutBuffer buf;
	buf.writef ("% (", this-> getLocation ().getStr ());
	for (auto it : Ymir::r (0, this-> _params.size ())) {
	    if (it!= 0) buf.write (", ");
	    buf.write (this-> _params [it].prettyString ());
	}
	buf.writef (")-> %", this-> _ret.prettyString ());
	return buf.str ();
    }

    const std::set <std::string> & FuncPtr::computeSubVarNames () {
	auto rSet = this-> _ret.getSubVarNames ();
	for (auto & it : this-> _params) {
	    auto & iSet = it.getSubVarNames ();
	    rSet.insert (iSet.begin (), iSet.end ());
	}
	this-> setSubVarNames (rSet);
	return this-> getSubVarNames ();
    }

    
}
