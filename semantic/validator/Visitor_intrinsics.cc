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

	
	Generator Visitor::validateIntrinsics (const syntax::Intrinsics & intr) {
	    if (intr.isCopy ()) return validateCopy (intr);
	    if (intr.isAlias ()) return validateAlias (intr);
	    if (intr.isExpand ()) return validateExpand (intr);
	    if (intr.isDeepCopy ()) return validateDeepCopy (intr);
	    if (intr.isTypeof ()) {
		auto elem = validateValue (intr.getContent ());
		return elem.to <Value> ().getType ();
	    }
	    if (intr.isSizeof ()) {
		auto elem = validateType (intr.getContent (), true);
		return SizeOf::init (intr.getLocation (), Integer::init (intr.getLocation (), 0, false), elem);
	    }
	    
	    if (intr.isMove ()) {
		Ymir::Error::occur (intr.getLocation (),
				    ExternalError::get (MOVE_ONLY_CLOSURE));				    
	    }
	    
	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Generator::empty ();
	}
       	
	Generator Visitor::validateCopy (const syntax::Intrinsics & intr) {
	    auto content = validateValue (intr.getContent ());

	    if (content.to <Value> ().getType ().is <Array> () || content.to <Value> ().getType ().is <Slice> ()) {
		auto type = content.to<Value> ().getType ();
		type = Type::init (type.to <Type> (), false);
		type = type.to <Type> ().toMutable ();
		
		if (type.is <Array> ()) {
		    if (type.is <Array> ()) {
			type = Type::init (Slice::init (intr.getLocation (), type.to<Array> ().getInners () [0]).to <Type> (), true);
		    }
		    
		    content = Aliaser::init (intr.getLocation (), type, content);
		}
		
		// The copy is done on the first level, so we don't have the right to change the mutability of inner data
		return Copier::init (intr.getLocation (), type, content);
	    } else {
		Ymir::Error::occur (
		    intr.getLocation (),
		    ExternalError::get (NO_COPY_EXIST),
		    content.to<Value> ().getType ().to <Type> ().getTypeName ()
		    );
		
		return Generator::empty ();		
	    }	    
	}

	Generator Visitor::validateAlias (const syntax::Intrinsics & intr) {
	    auto content = validateValue (intr.getContent ());

	    if (content.to <Value> ().getType ().is <Array> () || content.to <Value> ().getType ().is <Slice> ()) {
		auto type = content.to <Value> ().getType ();
		if (type.is <Array> ()) {
		    type = Type::init (Slice::init (intr.getLocation (), type.to<Array> ().getInners () [0]).to <Type> (), 
				       content.to <Value> ().getType ().to <Type> ().isMutable ());
		}
		return Aliaser::init (intr.getLocation (), type, content);
	    }
	    
	    auto type = content.to <Value> ().getType ();
	    if (!type.to <Type> ().isMutable ())
	    Ymir::Error::occur (content.getLocation (),
				ExternalError::get (NOT_A_LVALUE)
		);

	    this-> verifyLockAlias (content);
	    
	    return Aliaser::init (intr.getLocation (), type, content);	    	    
	}

	Generator Visitor::validateDeepCopy (const syntax::Intrinsics & intr) {
	    auto inner = validateValue (intr.getContent ());
	    syntax::Expression call (syntax::Expression::empty ());
	    auto loc = intr.getLocation ();
	    if (inner.to <Value> ().getType ().is <ClassPtr> ()) {
		auto trait = createVarFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (DUPLICATION_MODULE), CoreNames::get (DCOPY_TRAITS)});		
		auto impl = validateType (trait);
		
		verifyClassImpl (intr.getLocation (), inner.to <Value> ().getType (), impl);		
		call = syntax::MultOperator::init (
		    lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		    syntax::Binary::init (lexing::Word::init (loc, Token::DOT),
					  TemplateSyntaxWrapper::init (inner.getLocation (), inner), 	       
					  syntax::Var::init (lexing::Word::init (loc, global::CoreNames::get (DCOPY_OP_OVERRIDE))),
					  syntax::Expression::empty ()
			),
		    {}, false
		    );
	    } else {
		auto func = createVarFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (DUPLICATION_MODULE), CoreNames::get (DCOPY_OP_OVERRIDE)});
		
		call = syntax::MultOperator::init (
		    lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		    func,
		    {TemplateSyntaxWrapper::init (inner.getLocation (), inner)}	       
		    );
	    }

	    auto val = validateValue (call);
	    return Aliaser::init (intr.getLocation (), val.to <Value> ().getType (), val);
	}
	
	Generator Visitor::validateExpand (const syntax::Intrinsics & intr) {
	    std::vector <Generator> values;
	    auto content = validateValue (intr.getContent ());
	    auto type = content.to <Value> ().getType ();
	    auto rref = UniqValue::init (intr.getLocation (), type, content);
	    
	    if (content.to<Value> ().getType ().is<Tuple> ()) {
		auto type = Void::init (intr.getLocation ());
		std::vector <Generator> expanded;
		auto & tu_inners = content.to <Value> ().getType ().to<Tuple> ().getInners ();
		for (auto it : Ymir::r (0, tu_inners.size ())) {
		    auto type = tu_inners [it];
		    if (content.to<Value> ().isLvalue () &&
			content.to <Value> ().getType ().to <Type> ().isMutable () &&
			type.to <Type> ().isMutable ())
		    type = Type::init (type.to <Type> (), true);
		    else type = Type::init (type.to<Type> (), false);
		    
		    expanded.push_back (TupleAccess::init (intr.getLocation (), type, rref, it));
		}
		return List::init (intr.getLocation (), type, expanded);
	    } else {
		return content;
	    }
	}

    }

}
