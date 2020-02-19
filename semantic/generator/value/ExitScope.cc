#include <ymir/semantic/generator/value/ExitScope.hh>

namespace semantic {

    namespace generator {

	ExitScope::ExitScope () :
	    Value (),
	    _who (Generator::empty ()),
	    _jmpBuf (Generator::empty ()),
	    _catchingVar (Generator::empty ()),
	    _catchingInfoType (Generator::empty ()),
	    _catchingAction (Generator::empty ())
	{}

	ExitScope::ExitScope (const lexing::Word & loc,
			      const Generator & type,
			      const Generator & jmpBuf,
			      const Generator & who,
			      const std::vector <Generator> & success,
			      const std::vector <Generator> & failure,
			      const Generator & catchingVar,
			      const Generator & catchingInfo,
			      const Generator & catchingAction) :
	    Value (loc, type),
	    _who (who),
	    _jmpBuf (jmpBuf),
	    _catchingVar (catchingVar),
	    _catchingInfoType (catchingInfo),
	    _catchingAction (catchingAction),
	    _success (success),
	    _failure (failure)
	{}
	
	Generator ExitScope::init (const lexing::Word & loc,
				   const Generator & type,
				   const Generator & jmpBuf,
				   const Generator & who,
				   const std::vector <Generator> & success,
				   const std::vector <Generator> & failure,
				   const Generator & catchingVar,
				   const Generator & catchingInfo,
				   const Generator & catchingAction)
	{	    
	    return Generator {new ExitScope (loc, type, jmpBuf, who, success, failure, catchingVar, catchingInfo, catchingAction)};
	}
    
	Generator ExitScope::clone () const {
	    return Generator {new (Z0) ExitScope (*this)};
	}

	bool ExitScope::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    ExitScope thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool ExitScope::equals (const Generator & gen) const {
	    if (!gen.is <ExitScope> ()) return false;
	    auto bin = gen.to<ExitScope> ();	    
	    if (!this-> _who.equals (bin._who)) return false;
	    if (bin._success.size () != this-> _success.size ()) return false;
	    if (bin._failure.size () != this-> _failure.size ()) return false;
	    for (auto it : Ymir::r (0, this-> _success.size ()))
		if (!bin._success [it].equals (this-> _success [it])) return false;
	    for (auto it : Ymir::r (0, this-> _failure.size ()))
		if (!bin._failure [it].equals (this-> _failure [it])) return false;
	    return true;
	}

	const Generator & ExitScope::getWho () const {
	    return this-> _who;
	}

	const std::vector <Generator> & ExitScope::getSuccess () const {
	    return this-> _success;
	}

	const std::vector <Generator> & ExitScope::getFailure () const {
	    return this-> _failure;
	}

	const Generator & ExitScope::getJmpbufType () const {
	    return this-> _jmpBuf;
	}

	const Generator & ExitScope::getCatchingVar () const {
	    return this-> _catchingVar;
	}

	const Generator & ExitScope::getCatchingInfoType () const {
	    return this-> _catchingInfoType;
	}
	    
	const Generator & ExitScope::getCatchingAction () const {
	    return this-> _catchingAction;
	}
	
	std::string ExitScope::prettyString () const {
	    return Ymir::format ("%", this-> _who.prettyString ());
	}
    }
    
}
