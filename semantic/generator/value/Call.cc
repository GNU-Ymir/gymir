#include <ymir/semantic/generator/value/Call.hh>
#include <ymir/utils/Match.hh>
#include <ymir/semantic/generator/value/_.hh>

namespace semantic {

    namespace generator {

	Call::Call () :
	    Value (),
	    _frame (Generator::empty ()),
	    _types ({}),
	    _params ({})
	{
	    this-> isLvalue (true);
	}

	Call::Call (const lexing::Word & loc, const Generator & type, const Generator & frame, const std::vector<Generator> & types, const std::vector <Generator> & params, const std::vector <Generator> & gen) :
	    Value (loc, type),
	    _frame (frame),
	    _types (types),
	    _params (params),
	    _addParams (gen)
	{
	    this-> isLvalue (type.to <Type> ().isRef ());
	    auto lth = this-> _frame.getThrowers ();
	    if (lth.size () != 0) {
		for (auto &it : lth) it = Generator::init (loc, it);		
	    }
	    
	    for (auto & it : this-> _params) {
		auto &ith = it.getThrowers ();
		lth.insert (lth.end (), ith.begin (), ith.end ());
	    }

	    for (auto & it : this-> _addParams) {
		auto &ith = it.getThrowers ();
		lth.insert (lth.end (), ith.begin (), ith.end ());
	    }
		
	    this-> setThrowers (lth);
	}
	
	Generator Call::init (const lexing::Word & loc, const Generator & type, const Generator & frame, const std::vector<Generator> & types, const std::vector <Generator> & params, const std::vector <Generator> & addParams) {
	    return Generator {new (NO_GC) Call (loc, type, frame, types, params, addParams)};
	}
    
	Generator Call::clone () const {
	    return Generator {new (NO_GC) Call (*this)};
	}

	bool Call::equals (const Generator & gen) const {
	    if (!gen.is <Call> ()) return false;
	    auto call = gen.to <Call> ();
	    if (call.getParameters ().size () != this-> _params.size ()) return false;
	    if (!call.getFrame ().equals (this-> _frame)) return false;
	    for (auto it : Ymir::r (0, this-> _params.size ())) {
		if (!this-> _params [it].equals (call.getParameters () [it])) return false;
		if (!this-> _types [it].equals (call.getTypes () [it])) return false;
	    }

	    return true;
	}

	const Generator & Call::getFrame () const {
	    return this-> _frame;
	}

	const std::vector <Generator> & Call::getTypes () const {
	    return this-> _types;
	}
	
	const std::vector <Generator> & Call::getParameters () const {
	    return this-> _params;
	}

	std::string Call::prettyString () const {
	    std::vector <std::string> params;
	    for (auto & it : this-> _params)
		params.push_back (it.prettyString ());

	    match (this-> _frame) {
		of (ConstructorProto, ctor) {
		    auto name = ctor.getName ();
		    auto sym = ctor.getReturnType ();
		    if (name != "self") {
			return Ymir::format ("%::% (%)", sym.prettyString (), name, params);
		    } else {
			return Ymir::format ("%::new (%)", sym.prettyString (), params);
		    }
		}

		elof (MethodProto, mth) {
		    if (mth.isMutable ()) {
			params.insert (params.begin (), "mut self");
			return Ymir::format ("% (%)", mth.getName (), params);
		    } else {
			params.insert (params.begin (), "self");
			return Ymir::format ("% (%)", mth.getName (), params);
		    }
		}
		
		elof (FrameProto, fr) {
		    return Ymir::format ("% (%)", fr.getName (), params);
		}

		elof (LambdaProto, lmd) {
		    return Ymir::format ("% (%)", lmd.getName (), params);
		}

		fo;
	    }
	    
	    return Ymir::format ("% (%)", this-> _frame.prettyString (), params);
	}

	const std::vector <Generator> & Call::getAddParameters () const {
	    return this-> _addParams;
	}
	
    }
    
}
