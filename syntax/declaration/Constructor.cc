#include <ymir/syntax/declaration/Constructor.hh>


namespace syntax {
    

    Constructor::Constructor () :
	IDeclaration (lexing::Word::eof (), ""),
	_proto (Function::Prototype::empty ()),
	_superParams ({}),
	_construction ({}),
	_body (Expression::empty ()),
	_explicitSuperCall (lexing::Word::eof ()),
	_explicitSelfCall (lexing::Word::eof ()),
	_rename (lexing::Word::eof ())
    {}
    
    Constructor::Constructor (const lexing::Word & name, const std::string & comment, const lexing::Word & rename, const Function::Prototype & proto, const std::vector <Expression> & super, const std::vector <std::pair <lexing::Word, Expression> > & constructions, const Expression & body, const lexing::Word & explicitSuperCall, const lexing::Word & explicitSelfCall, const std::vector <lexing::Word> & cas, const std::vector <Expression> & thrower) :
	IDeclaration (name, comment),
	_proto (proto),
	_superParams (super),
	_construction (constructions),
	_body (body),
	_explicitSuperCall (explicitSuperCall),
	_explicitSelfCall (explicitSelfCall),
	_rename (rename),
	_cas (cas),
	_throwers (thrower)
    {}
    
    Declaration Constructor::init (const lexing::Word & name, const std::string& comment, const lexing::Word & rename, const Function::Prototype & proto, const std::vector <Expression> & super, const std::vector <std::pair <lexing::Word, Expression> > & constructions, const Expression & body, const lexing::Word & explicitSuperCall, const lexing::Word & explicitSelfCall, const std::vector <lexing::Word> & cas, const std::vector <Expression> & thrower) {
	return Declaration {new (NO_GC) Constructor (name, comment, rename, proto, super, constructions, body, explicitSuperCall, explicitSelfCall, cas, thrower)};
    }

    Declaration Constructor::init (const Constructor & other) {
	return Declaration {new (NO_GC) Constructor (other)}; // We need to create an allocation because we don't know how other has been constructed
    }
        
    const std::vector <Expression> & Constructor::getSuperParams () const {
	return this-> _superParams;
    }

    const std::vector <std::pair <lexing::Word, Expression> > & Constructor::getFieldConstruction () const {
	return this-> _construction;
    }
    
    const Function::Prototype & Constructor::getPrototype () const {
	return this-> _proto;
    }

    const Expression & Constructor::getBody () const {
	return this-> _body;
    }

    const lexing::Word & Constructor::getExplicitSuperCall () const {
	return this-> _explicitSuperCall;
    }

    const lexing::Word & Constructor::getExplicitSelfCall () const {
	return this-> _explicitSelfCall;
    }
    
    const std::vector <lexing::Word> & Constructor::getCustomAttributes () const {
	return this-> _cas;
    }

    const std::vector <Expression> & Constructor::getThrowers () const {
	return this-> _throwers;
    }

    const lexing::Word & Constructor::getRename () const {
	return this-> _rename;
    }

    const std::set <std::string> & Constructor::computeSubVarNames () {
	auto pSet = this-> _proto.getSubVarNames ();
	for (auto & it : this-> _superParams) {
	    auto & iSet = it.getSubVarNames ();
	    pSet.insert (iSet.begin (), iSet.end ());
	}

	for (auto & it : this-> _throwers) {
	    auto & iSet = it.getSubVarNames ();
	    pSet.insert (iSet.begin (), iSet.end ());
	}

	for (auto & it : this-> _construction) {
	    auto & iSet = it.second.getSubVarNames ();
	    pSet.insert (iSet.begin (), iSet.end ());
	}

	auto & iSet = this-> _body.getSubVarNames ();
	pSet.insert (iSet.begin (), iSet.end ());
	this-> setSubVarNames (pSet);
	return this-> getSubVarNames ();	
    }
}
