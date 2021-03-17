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

	Generator Visitor::inferTypeBranching (const lexing::Word & lloc, const lexing::Word & rloc, const generator::Generator & left, const generator::Generator & right) {
	    /**
	     * Proxy verification 
	     */
	    if (!left.to <Type> ().getProxy ().isEmpty () || !right.to <Type> ().getProxy ().isEmpty ()) {
		if (left.to <Type> ().isCompatible (right)) {
		    if (left.to <Type> ().getProxy ().isEmpty () || right.to <Type> ().getProxy ().isEmpty ()) {
			if (left.to <Type> ().mutabilityLevel () < right.to <Type> ().mutabilityLevel ()) {
			    return Type::init (Type::init (left.to <Type> (), left.to <Type> ().isMutable ()).to<Type> (), Generator::empty ());
			} else {
			    return Type::init (Type::init (right.to <Type> (), right.to <Type> ().isMutable ()).to<Type> (), Generator::empty ());
			}
		    }
		    else {		    
			if (!left.to<Type> ().getProxy ().to <Type> ().isCompatible (right.to<Type> ().getProxy ())) {
			    if (left.to <Type> ().mutabilityLevel () < right.to <Type> ().mutabilityLevel ()) {
				return Type::init (Type::init (left.to <Type> (), left.to <Type> ().isMutable ()).to <Type> (), Generator::empty ()); // remove the proxy
			    } else {
				return Type::init (Type::init (right.to <Type> (), right.to <Type> ().isMutable ()).to <Type> (), Generator::empty ()); // remove the proxy
			    }
			} else {
			    if (left.to <Type> ().mutabilityLevel () < right.to <Type> ().mutabilityLevel ()) {
				return Type::init (Type::init (left.to<Type> (), left.to <Type> ().isMutable ()).to <Type> (), left.to<Type> ().getProxy ());
			    } else {
				return Type::init (Type::init (right.to <Type> (), right.to <Type> ().isMutable ()).to <Type> (), left.to <Type> ().getProxy ()); // remove the proxy
			    }
			}
		    }
		} else { // the type are incompatible, the proxy has no impact, we just remove it 
		    auto ltype = Type::init (left.to <Type> (), Generator::empty ());
		    auto rtype = Type::init (right.to <Type> (), Generator::empty ());
		    return inferTypeBranching (lloc, rloc, ltype, rtype); // and retry
		}
	    }

	    if (left.to <Type> ().isCompatible (right)) {
		if (left.to <Type> ().mutabilityLevel () < right.to <Type> ().mutabilityLevel ()) return left;
		else return right;
	    } else {
		match (left) {
		    of (Slice, ls) {
			if (right.is<Slice> ()) {
			    auto & rs = right.to <Slice> ();
			    if (ls.getInners ()[0].is <Void> ()) return right;
			    if (rs.getInners ()[0].is <Void> ()) return left;
			    else {
				try {
				    auto slc = Slice::init (lloc, this-> inferTypeBranching (lloc, rloc, ls.getInners ()[0], rs.getInners ()[0]));
				    if (ls.mutabilityLevel () < rs.mutabilityLevel ()) {
					slc = Type::init (slc.to<Type> (), ls.isMutable ());
				    } else {
					slc = Type::init (slc.to<Type> (), rs.isMutable ());
				    }
				    return slc;
				} catch (Error::ErrorList list) {
				    throwIncompatibleTypes (lloc, rloc, left, right, list.errors);
				}
			    }
			} 
		    } elof (ClassPtr, lp) {
			if (right.is <ClassPtr> ()) {			    
			    auto & rp = right.to <ClassPtr> ();
			    Generator ancestor = Generator::empty ();
			    if (isAncestor (left, right)) { // right over left
				ancestor = left;
			    } else if (isAncestor (right, left)) {
				ancestor = right;
			    } else {
				ancestor = this-> getCommonAncestor (left, right);
			    }
			    			
			    if (ancestor.isEmpty ()) {
				this-> verifyCompatibleType (lloc, rloc, left, right);
			    } else {
				if (lp.mutabilityLevel () < rp.mutabilityLevel ()) {
				    auto inner = Type::init (ancestor.to <Type> ().getInners ()[0].to <Type> (), lp.getInners ()[0].to <Type> ().isMutable ());
				    return Type::init (ClassPtr::init (ancestor.getLocation (), inner).to <Type> (), lp.isMutable ());
				} else  {
				    auto inner = Type::init (ancestor.to <Type> ().getInners ()[0].to <Type> (), rp.getInners ()[0].to <Type> ().isMutable ());
				    return Type::init (ClassPtr::init (ancestor.getLocation (), inner).to<Type> (), rp.isMutable ());
				}			    
			    }
			}
		    } elof (Tuple, lt) {
			if (right.is <Tuple> ()) {
			    auto & rt = right.to <Tuple> ();
			    if (lt.getInners ().size () != rt.getInners ().size ()) {
				this-> verifyCompatibleType (lloc, rloc, left, right);
			    }
			    
			    try {
				std::vector <Generator> innerParams;
				for (auto it : Ymir::r (0, rt.getInners ().size ())) {
				    innerParams.push_back (this-> inferTypeBranching (
							       lt.getInners ()[it].getLocation (),
							       rt.getInners ()[it].getLocation (),
							       lt.getInners ()[it], rt.getInners ()[it]));
				}
				bool isMutable = lt.isMutable () && rt.isMutable ();
				return Type::init (Tuple::init (lloc, innerParams).to <Type> (), isMutable);
			    } catch (Error::ErrorList list) {
				throwIncompatibleTypes (lloc, rloc, left, right, list.errors);
			    }
			}
		    } fo;
		}
	    }

	    // To throw the error only
	    throwIncompatibleTypes (lloc, rloc, left, right, {});
	    return Generator::empty ();
	}	

	
    }
}
