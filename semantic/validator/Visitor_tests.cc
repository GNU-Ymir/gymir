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

	bool Visitor::canImplicitAlias (const Generator & value) {
	    if (value.is<Copier> () || value.is <Aliaser> () || value.is <Referencer> ()) return true; // It is aliased or copied, that's ok
	    
	    {
		// Totally ok for implicit alias 
		match (value) {
		    s_of_u (ArrayValue) return true;
		    s_of_u (StructCst) return true;
		    s_of_u (ClassCst) return true;
		    s_of_u (ArrayAlloc) return true;
		    s_of_u (NullValue) return true;
		}
	    }
	    
	    return false;
	}

	bool Visitor::isVoidArrayType (const Generator & gen) {
	    if (!gen.is <Array> () && !gen.is <Slice> ()) return false;
	    if (!gen.to <Type> ().getInners () [0].is <Void> ()) return false;
	    return true;
	}

	bool Visitor::isAncestor (const Generator & left_, const Generator & right_) {
	    auto right = right_;
	    auto left = left_;
	    
	    if (right.is <ClassPtr> ()) right = right.to <Type>().getInners ()[0];
	    if (left.is <ClassPtr> ()) left = left.to <Type>().getInners ()[0];
	    	    
	    if (right.is <ClassRef> () && !right.to <ClassRef> ().getRef ().to <semantic::Class> ().getAncestor ().isEmpty ()) {
		auto ancestor = right.to <ClassRef> ().getAncestor ();
		while (!ancestor.isEmpty ()) {
		    if (left.to <Type> ().isCompatible (ancestor)) return true;
		    else {
			if (!ancestor.to <ClassRef> ().getRef ().to <semantic::Class> ().getAncestor ().isEmpty ())
			ancestor = ancestor.to <ClassRef> ().getAncestor ();
			else ancestor = Generator::empty ();
		    }
		}
	    }
	    return false;
	}


	bool Visitor::isIntConstant (const Generator & value) {
	    if (!value.to <Value> ().getType ().is <Integer> ()) return false;
	    try {
		auto val = retreiveValue (value);
		return val.is <Fixed> ();
	    } catch (Error::ErrorList err) {
		return false;
	    }
	}

	bool Visitor::isFloatConstant (const Generator & value) {
	    if (!value.to <Value> ().getType ().is <Float> ()) return false;
	    try {
		auto val = retreiveValue (value);
		return val.is <FloatValue> ();		
	    } catch (Error::ErrorList list) {
		return false;
	    }
	}
	bool Visitor::isUseless (const Generator & value) {
	    match (value) {
		s_of_u (Affect)  return false;
		s_of_u (Block)  return false;
		s_of_u (Break)  return false;
		s_of_u (Call)  return false;
		s_of_u (Conditional) return false;
		s_of_u (Loop)  return false;
		s_of_u (generator::VarDecl) return false;
		s_of_u (Return) return false;
		s_of_u (Set) return false;
		s_of_u (Throw) return false;
		s_of_u (ExitScope) return false;
	    }
	    return true;
	}	

	
	
    }

}
