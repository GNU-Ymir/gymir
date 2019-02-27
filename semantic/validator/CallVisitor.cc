#include <ymir/semantic/validator/CallVisitor.hh>

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;
	using namespace syntax;

	CallVisitor::CallVisitor (Visitor & context) :
	    _context (context)
	{}

	CallVisitor CallVisitor::init (Visitor & context) {
	    return CallVisitor {context};
	}

	generator::Generator CallVisitor::validate (const syntax::MultOperator & expression) {
	    auto left = this-> _context.validateValue (expression.getLeft ());
	    std::vector <Generator> rights;
	    for (auto & it : expression.getRights ()) 
		rights.push_back (this-> _context.validateValue (it));

	    int score = 0;
	    std::vector <std::string> errors;
	    if (left.is <FrameProto> ()) {
		auto gen = validateFrameProto (expression, left.to<FrameProto> (), rights, score, errors);
		if (!gen.isEmpty ()) return gen;
	    } else if (left.is <MultSym> ()) {
		auto gen = validateMultSym (expression, left.to <MultSym> (), rights, score, errors);
		if (!gen.isEmpty ()) return gen;
	    }

	    this-> error (expression, left, rights, errors);
	    return Generator::empty ();
	}

	generator::Generator CallVisitor::validateFrameProto (const syntax::MultOperator & expression, const FrameProto & proto, const std::vector <Generator> & rights_, int & score, std::vector <std::string> & errors) {
	    score = -1;
	    std::vector <Generator> params;
	    std::vector <Generator> rights = rights_;
	    for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
		auto param = findParameter (rights, proto.getParameters () [it].to<ProtoVar> ());
		if (param.isEmpty ()) return Generator::empty ();
		params.push_back (param);
	    }
	    
	    if (rights.size () != 0) return Generator::empty ();
	    std::vector <Generator> types;
	    for (auto it : Ymir::r (0, proto.getParameters ().size ())) {
		TRY (
		    this-> _context.verifyMemoryOwner (
			params [it].getLocation (),
			proto.getParameters () [it].to <Value> ().getType (),
			params [it],
			true
		    );
		    types.push_back (proto.getParameters () [it].to <Value> ().getType ());
		) CATCH (ErrorCode::EXTERNAL) {
		    GET_ERRORS_AND_CLEAR (msgs);
		    errors.insert (errors.end (), msgs.begin (), msgs.end ());
		} FINALLY;
	    }

	    if (errors.size () != 0) return Generator::empty ();
	    
	    score = 0;
	    return Call::init (expression.getLocation (), proto.getReturnType (), proto.clone (), types, params);
	}
       
	generator::Generator CallVisitor::findParameter (std::vector <Generator> & params, const ProtoVar & var) {
	    for (auto  it : Ymir::r (0, params.size ())) {
		if (params [it].is <NamedGenerator> ()) {
		    auto name = params [it].to <NamedGenerator> ().getLocation ();
		    if (name.str == var.getLocation ().str) {
			auto toRet = params [it].to <NamedGenerator> ().getContent ();
			params.erase (params.begin () + it);
			return toRet;
		    }
		}
	    }

	    // If the var has a value, it is an optional argument
	    if (!var.getValue ().isEmpty ()) return var.getValue ();
	    
	    else if (params.size () == 0) return Generator::empty ();
	    // If it does not have a value, it is a mandatory var, and its name cannot be the same as params [0] (we just verify that in the for loop)
	    else if (params [0].is <NamedGenerator> ()) return Generator::empty ();
	    
	    auto toRet = params [0];
	    params.erase (params.begin ());
	    return toRet;	    
	}


	generator::Generator CallVisitor::validateMultSym (const syntax::MultOperator & expression, const MultSym & sym, const std::vector <Generator> & rights_, int & score, std::vector <std::string> &) {
	    Generator final_gen (Generator::empty ());
	    Generator used_gen (Generator::empty ());
	    score = -1;
	    for (auto & it : sym.getGenerators ()) {
		int current = 0;
		std::vector <std::string> errors;
		if (it.is <FrameProto> ()) {
		    auto gen = validateFrameProto (expression, it.to <FrameProto> (), rights_, current, errors);
		    if (!gen.isEmpty () && current > score) {
			score = current;
			final_gen = gen;
			used_gen = it;
		    } else if (!gen.isEmpty () && current == score) {
			std::vector <std::string> names;
			for (auto & it : rights_)
			    names.push_back (it.to <Value> ().getType ().to <Type> ().getTypeName ());
			
			auto note = Ymir::Error::createNote (used_gen.getLocation ());
			note += Ymir::Error::createNote (it.getLocation ());
			Ymir::Error::occurAndNote (expression.getLocation (), note,
						   ExternalError::get (SPECIALISATION_WOTK_WITH_BOTH),
						   it.to<FrameProto> ().getName (),
						   names
			);
		    }
		}
	    }
	    return final_gen;
	}
							   
	
	void CallVisitor::error (const syntax::MultOperator & expression, const Generator & left, const std::vector <Generator> & rights, std::vector <std::string> & errors) {	    
	    std::vector <std::string> names;
	    for (auto & it : rights)
		names.push_back (it.to <Value> ().getType ().to <Type> ().getTypeName ());
	    
	    errors.push_back (Ymir::Error::makeOccur (
		expression.getLocation (),
		expression.getEnd (),
		ExternalError::get (UNDEFINED_CALL_OP),
		left.to <Value> ().getType ().to <Type> ().getTypeName (),
		names
	    ));

	    THROW (ErrorCode::EXTERNAL, errors);
	}

	

    }

}
