#include <ymir/semantic/generator/value/ExitScope.hh>
#include <ymir/semantic/generator/value/Conditional.hh>

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
			      const std::vector <Generator> & exit,
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
	    _exit (exit),
	    _failure (failure)
	{
	    std::vector <Generator> thrs;
	    if (this-> _catchingAction.isEmpty ()) {
		// If the catchingAction exists, it catch all the exceptions
		// otherwise, the scope is not safe
		auto &wth = this-> _who.getThrowers ();
		thrs.insert (thrs.end (), wth.begin (), wth.end ());	    
	    }
	    
	    for (auto & it : this-> _failure) {
		auto &fth = it.getThrowers ();
		thrs.insert (thrs.end (), fth.begin (), fth.end ());
	    }

	    for (auto & it : this-> _exit) {
		auto &sth = it.getThrowers ();
		thrs.insert (thrs.end (), sth.begin (), sth.end ());
	    }

	    auto &cth = this-> _catchingAction.getThrowers ();
	    thrs.insert (thrs.end (), cth.begin (), cth.end ());
	    
	    this-> setThrowers (thrs);

	    this-> setReturner (this-> _who.to <Value> ().isReturner ());
	    this-> setReturnerLocation (this-> _who.to <Value> ().getReturnerLocation ());
	    this-> setBreaker (this-> _who.to <Value> ().isBreaker ());
	    this-> setBreakerLocation (this-> _who.to <Value> ().getBreakerLocation ());
	    
	    if (!this-> _catchingAction.isEmpty ()) {
		this-> setReturner (
		    this-> isReturner () && this-> _catchingAction.to <Value> ().isReturner ()
		);

		if (this-> _catchingAction.to <Value> ().isReturner ())
		    this-> setReturnerLocation (this-> _catchingAction.to <Value> ().getReturnerLocation ());

		this-> setBreaker (
		    this-> isBreaker () && this-> _catchingAction.to <Value> ().isBreaker ()
		);

		if (this-> _catchingAction.to <Value> ().isBreaker ())
		    this-> setReturnerLocation (this-> _catchingAction.to <Value> ().getBreakerLocation ());
	    }
	}
	
	Generator ExitScope::init (const lexing::Word & loc,
				   const Generator & type,
				   const Generator & jmpBuf,
				   const Generator & who,
				   const std::vector <Generator> & exit,
				   const std::vector <Generator> & failure,
				   const Generator & catchingVar,
				   const Generator & catchingInfo,
				   const Generator & catchingAction)
	{	    
	    return Generator {new (NO_GC) ExitScope (loc, type, jmpBuf, who, exit, failure, catchingVar, catchingInfo, catchingAction)};
	}
    
	Generator ExitScope::clone () const {
	    return Generator {new (NO_GC) ExitScope (*this)};
	}

	bool ExitScope::equals (const Generator & gen) const {
	    if (!gen.is <ExitScope> ()) return false;
	    auto bin = gen.to<ExitScope> ();	    
	    if (!this-> _who.equals (bin._who)) return false;
	    if (bin._exit.size () != this-> _exit.size ()) return false;
	    if (bin._failure.size () != this-> _failure.size ()) return false;
	    for (auto it : Ymir::r (0, this-> _exit.size ()))
		if (!bin._exit [it].equals (this-> _exit [it])) return false;
	    for (auto it : Ymir::r (0, this-> _failure.size ()))
		if (!bin._failure [it].equals (this-> _failure [it])) return false;
	    return true;
	}

	const Generator & ExitScope::getWho () const {
	    return this-> _who;
	}

	const std::vector <Generator> & ExitScope::getExit () const {
	    return this-> _exit;
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
	    Ymir::OutBuffer exit;
	    if (this-> _exit.size () != 0) {
		exit.write ("exit ");
		for (auto &it : this-> _exit)
		    exit.writeln (it.prettyString ());
	    }

	    Ymir::OutBuffer failure;
	    if (this-> _failure.size() != 0) {
		failure.write ("failure ");
		for (auto &it : this-> _failure)
		    failure.writeln (it.prettyString ());
	    }
	    
	    if (!this->_catchingAction.isEmpty ()) 
		return Ymir::format ("try %catch %%%", this-> _who.prettyString (), this-> _catchingAction.prettyString (), failure.str (), exit.str ());
	    else 
		return Ymir::format ("try %%%", this-> _who.prettyString (), failure.str (), exit.str ());
	}
    }
    
}
