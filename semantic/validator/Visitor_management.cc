#include <ymir/semantic/validator/_.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/semantic/declarator/Visitor.hh>
#include <ymir/semantic/generator/Mangler.hh>
#include <ymir/utils/map.hh>
#include <ymir/global/Core.hh>
#include <ymir/utils/Path.hh>
#include <ymir/global/State.hh>
#include <string>
#include <algorithm>

using namespace global;

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;       

	void Visitor::enterForeign () {
	    this-> _usedSyms.push_back ({});
	    this-> _symbols.push_back ({});
	    this-> _loopSaved.push_back (this-> _loopBreakTypes);
	    this-> _loopBreakTypes = {};
	}
	
	void Visitor::exitForeign () {
	    this-> _usedSyms.pop_back ();
	    this-> _symbols.pop_back ();
	    this-> _loopBreakTypes = this-> _loopSaved.back ();
	    this-> _loopSaved.pop_back ();
	}
	
	void Visitor::enterClosure (bool isRef, uint refId, uint index) {
	    insert_or_assign (this-> _symbols.back ()[0], "#{CLOSURE}", BoolValue::init (lexing::Word::eof (), Bool::init (lexing::Word::eof ()), isRef));
	    this-> _usedSyms.back ()[0].insert ("#{CLOSURE}");

	    insert_or_assign (this-> _symbols.back ()[0], "#{CLOSURE-TYPE}", Closure::init (lexing::Word::eof (), {}, {}, index));
	    this-> _usedSyms.back ()[0].insert ("#{CLOSURE-TYPE}");

	    insert_or_assign (this-> _symbols.back ()[0], "#{CLOSURE-VARREF}", VarRef::init (lexing::Word::eof (), "#{CLOSURE-VARREF}", Void::init (lexing::Word::eof ()), refId, false, Generator::empty ()));
	    this-> _usedSyms.back () [0].insert ("#{CLOSURE-VARREF}");
	    this-> _enclosed.push_back ({});	    
	}

	Generator Visitor::exitClosure () {
	    return this-> getLocal ("#{CLOSURE-TYPE}", false);
	}

	void Visitor::enterContext (const std::vector <lexing::Word> & Cas) {
	    this-> _contextCas.push_back (Cas);
	}

	void Visitor::exitContext () {
	    if (this-> _contextCas.empty ()) {
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    }
	    
	    this-> _contextCas.pop_back ();
	}

		
	

	void Visitor::enterClassDef (const semantic::Symbol & sym) {
	    this-> _classContext.push_back (sym);
	}

	void Visitor::exitClassDef (const semantic::Symbol & sym) {
	    if (this-> _classContext.back ().getPtr () != sym.getPtr ()) {
		Ymir::Error::halt ("", "");
	    }
	    
	    this-> _classContext.pop_back ();
	}

	bool Visitor::isInContext (const std::string & context) {
	    if (!this-> _contextCas.empty ()) {
		for (auto & it : this-> _contextCas.back ())
		if (it.getStr () == context) return true;
	    }
	    return false;
	}

	bool Visitor::isInTrusted () const {
	    return this-> _referent.back ().isTrusted ();
	}

	void Visitor::enterBlock () {
	    this-> _usedSyms.back ().push_back ({});
	    this-> _symbols.back ().push_back ({});
	}
	
	void Visitor::quitBlock (bool warnUnused) {	    
	    if (this-> _symbols.back ().empty ()) {
		Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    }

	    std::list <Ymir::Error::ErrorMsg> errors;
	    if (warnUnused) {
		for (auto & sym : this-> _symbols.back ().back ()) {
		    if (sym.first != Keys::SELF) { // SELF is like "_", we don't need it to be used
			if (this-> _usedSyms.back ().back ().find (sym.first) == this-> _usedSyms.back ().back ().end ()) {
			    errors.push_back (Error::makeWarn (sym.second.getLocation (), ExternalError::get (NEVER_USED), sym.second.getName ()));
			}
		    }
		}
	    }

	    this-> _usedSyms.back ().pop_back ();
	    this-> _symbols.back ().pop_back ();

	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }
	}	

	void Visitor::quitBlock (bool warnUnused, std::list <Error::ErrorMsg> & errors) {
	    try {
		this-> quitBlock (warnUnused);
	    } catch (Error::ErrorList lst) {
		errors.insert (errors.end (), lst.errors.begin (), lst.errors.end ());
	    }
	}
	
	std::map <std::string, generator::Generator> Visitor::discardAllLocals () {
	    auto ret = this-> _symbols.back ().back ();
	    this-> _symbols.back ().back () = {};
	    return ret;
	}


	void Visitor::insertLocal (const std::string & name, const Generator & gen) {
	    if (this-> _symbols.back ().empty ())
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    if (this-> _symbols.back ().back ().find (name) == this-> _symbols.back ().back ().end ()) {
		insert_or_assign (this-> _symbols.back ().back (), name, gen);
	    } else {
		insert_or_assign (this-> _symbols.back ().back (), name, gen);
	    }
	}       

	
	Generator Visitor::getLocal (const std::string & name, bool canBeInClosure) {
	    for (auto _it : Ymir::r (0, this-> _symbols.back ().size ())) {
		auto it = (this-> _symbols.back ().size () - _it) - 1;
		auto ptr = this-> _symbols.back () [it].find (name); 		    
		if (ptr != this-> _symbols.back () [it].end ()) {
		    this-> _usedSyms.back () [it].insert (name);
		    return ptr-> second;
		}		
	    }

	    if (canBeInClosure)
	    return this-> getInClosure (name);
	    
	    return Generator::__empty__;
	}
	
	void Visitor::printLocal () const {
	    for (auto _it : Ymir::r (0, this-> _symbols.back ().size ())) {
		println (Ymir::format ("%* {", (int) _it, '\t'));
		for (auto it : this-> _symbols.back () [_it]) {		    
		    println (Ymir::format ("%* %-> %", (int) (_it + 1), '\t', it.first, it.second.prettyString ()));
		}
		println (Ymir::format ("%* }", (int)_it, '\t'));
	    }
	}	


	void Visitor::lockAliasing (const lexing::Word & loc, const generator::Generator & gen_) {
	    if (gen_.to <Value> ().getType ().to <Type> ().isMutable ()) { // not necessary to store immutable element
		auto gen = gen_;
		while (gen.is <Aliaser> () || gen.is<Referencer> ()) {
		    if (gen.is <Aliaser> ()) gen = gen.to <Aliaser> ().getWho ();
		    else if (gen.is <Referencer> ()) gen = gen.to <Referencer> ().getWho ();
		}
		
		this-> _lockedAlias.push_back (gen);
		this-> _lockedAliasLoc.push_back (loc);
	    }
	}

	void Visitor::unlockAliasing (const generator::Generator & gen_) {
	    if (this-> _lockedAlias.size () != 0 && gen_.to <Value> ().getType ().to <Type> ().isMutable ()) {
		std::vector <Generator> keeps;
		std::vector <lexing::Word> keepLoc;
		auto gen = gen_;
		while (gen.is <Aliaser> () || gen.is<Referencer> ()) {
		    if (gen.is <Aliaser> ()) gen = gen.to <Aliaser> ().getWho ();
		    else if (gen.is <Referencer> ()) gen = gen.to <Referencer> ().getWho ();
		}

		int i = 0;
		for (auto & it : this-> _lockedAlias) {
		    if (!it.equals (gen)) {
			keeps.push_back (it);
			keepLoc.push_back (this-> _lockedAliasLoc [i]);
		    }
		    i += 1;
		}
		
		this-> _lockedAlias = keeps;
		this-> _lockedAliasLoc = keepLoc;
	    }	    
	}
	
	void Visitor::enterLoop () {
	    this-> _loopBreakTypes.push_back (Generator::empty ());	    
	}

	Generator Visitor::quitLoop () {
	    auto last = this-> _loopBreakTypes.back ();
	    this-> _loopBreakTypes.pop_back ();
	    return last;
	}



	void Visitor::enterDollar (const Generator & gen) {
	    this-> _dollars.push_back (gen);
	}

	void Visitor::quitDollar () {
	    this-> _dollars.pop_back ();
	}


	const Generator & Visitor::getCurrentLoopType () const {
	    return this-> _loopBreakTypes.back ();
	}

	void Visitor::setCurrentLoopType (const Generator & type) {
	    this-> _loopBreakTypes.back () = type;
	}

	bool Visitor::isInLoop () const {
	    return !this-> _loopBreakTypes.empty ();
	}
	

	Generator Visitor::getCurrentFuncType () {
	    return getLocal ("#{RET}", false);
	}

	void Visitor::setCurrentFuncType (const Generator & type) {
	    insert_or_assign (this-> _symbols.back () [0], "#{RET}", type);
	    this-> _usedSyms.back () [0].insert ("#{RET}");
	}


	std::vector <Symbol> Visitor::getGlobal (const std::string & name) {					       
	    return this-> _referent.back ().get (name);
	}

	std::vector <Symbol> Visitor::getGlobalPrivate (const std::string & name) {
	    return this-> _referent.back ().getPrivate (name);
	}	
	
	void Visitor::pushReferent (const semantic::Symbol & sym, const std::string &) {
	    // print (this-> _referent.size (), ' ');
	    // println ("IN : ", msg, " => ", sym.getRealName ());
	    this-> _referent.push_back (sym);
	}

	void Visitor::popReferent (const std::string &) {
	    // print (this-> _referent.size ());
	    // println ("Out : ", msg, " => ", this-> _referent.back ().getRealName ());
	    this-> _referent.pop_back ();
	}
	


	Generator Visitor::getInClosure (const std::string & name) {
	    if (!isInClosure ()) return Generator::__empty__;
	    auto closureType = this-> getLocal ("#{CLOSURE-TYPE}", false);
	    //bool isRefClosure = this-> getLocal ("#{CLOSURE}").to <BoolValue> ().getValue ();
	    auto field = closureType.to <Closure> ().getField (name);
	    if (field.isEmpty ()) { // need to get it from upper closure
		auto & syms = this-> _symbols [closureType.to <Closure> ().getIndex ()];
		auto & used = this-> _usedSyms [closureType.to <Closure> ().getIndex ()]; 
		for (auto _it : Ymir::r (0, syms.size ())) {
		    auto ptr = syms [_it].find (name);
		    if (ptr != syms [_it].end ()) {
			used [_it].insert (name);
			Generator type (Generator::empty ());
			if (ptr-> second.is <generator::VarDecl> ()) {
			    type = ptr-> second.to <generator::VarDecl> ().getVarType ();
			} else type = ptr-> second.to <Value> ().getType ();
			
			auto types = closureType.to <Type> ().getInners ();
			
			auto names = closureType.to <Closure> ().getNames ();
			types.push_back (type);
			names.push_back (name);

			closureType = Closure::init (lexing::Word::eof (), types, names, closureType.to <Closure> ().getIndex ());
			closureType = Type::init (closureType.to <Type> (), closureType.to <Type> ().isMutable (), true);
			
			insert_or_assign (this-> _symbols.back ()[0], "#{CLOSURE-TYPE}", closureType);
			auto closureRef = this-> getLocal ("#{CLOSURE-VARREF}", false);
			insert_or_assign (this-> _symbols.back ()[0], "#{CLOSURE-VARREF}", VarRef::init (lexing::Word::eof (), "#{CLOSURE-VARREF}", closureType, closureRef.to <VarRef> ().getRefId (), false, Generator::empty ()));
			
			return StructAccess::init (lexing::Word::eof (), type, this-> getLocal ("#{CLOSURE-VARREF}", false), name);
		    }
		}
	    } else {
		auto closureRef = this-> getLocal ("#{CLOSURE-VARREF}", false);
		return StructAccess::init (lexing::Word::eof (), field, closureRef, name);
	    }

	    return Generator::__empty__;
	}	
	
	bool Visitor::isInClosure () {
	    auto ret = this-> getLocal ("#{CLOSURE}", false);
	    return !ret.isEmpty ();
	}
	
	bool Visitor::isInRefClosure () {
	    auto ret = this-> getLocal ("#{CLOSURE}", false);
	    return !ret.isEmpty () && ret.to<BoolValue> ().getValue ();
	}
	
    }
}
