#include <ymir/semantic/validator/MatchVisitor.hh>
#include <ymir/semantic/validator/BinaryVisitor.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/global/State.hh>

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;
	using namespace syntax;

	MatchVisitor::MatchVisitor (Visitor & context) :
	    _context (context)
	{}

	MatchVisitor MatchVisitor::init (Visitor & context) {
	    return MatchVisitor {context};
	}
	
	generator::Generator MatchVisitor::validate (const syntax::Match & expression) {
	    auto value = this-> _context.validateValue (expression.getContent ());
	    // The value will be used in multiple test, we don't want to generate it multiple time though
	    if (!value.is<Referencer> ())
		value = UniqValue::init (value.getLocation (), value.to <Value> ().getType (), value);
	       	    
	    Generator result (Generator::empty ());
	    
	    auto & matchers = expression.getMatchers ();
	    auto & actions  = expression.getActions ();
	    Generator type (Generator::empty ());
	    bool isMandatory = false;
	    for (auto it_ : Ymir::r (0, matchers.size ())) {
		auto it = matchers.size () - 1 - it_; // We get them in the reverse order to have the tests in the right order
		Generator test (Generator::empty ());
		std::vector <std::string> errors;
		this-> _context.enterBlock ();
		{
		    TRY (
			bool local_mandatory = false;
			test = this-> validateMatch (value, matchers [it], local_mandatory);
			if (local_mandatory) isMandatory = true;
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors.insert (errors.end (), msgs.begin (), msgs.end ());
		    } FINALLY;
		}
		
		if (!test.isEmpty ()) { // size == 2, if succeed
		    auto content = this-> _context.validateValue (actions [it]);
		    auto local_type = content.to <Value> ().getType ();
		    if (type.isEmpty ()) type = local_type;
		    else this-> _context.verifyCompatibleType (content.getLocation (), type, local_type);
		    
		    result = Conditional::init (matchers [it].getLocation (), type, test, content, result);
		}

		{
		    TRY (
			this-> _context.quitBlock ();
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors.insert (errors.end (), msgs.begin (), msgs.end ());
		    } FINALLY;
		}

		if (errors.size () != 0 && expression.isFinal ())
		    THROW (ErrorCode::EXTERNAL, errors);
	    }
	    
	    
	    if (!isMandatory && !type.is<Void> ()) {
		Ymir::Error::occur (expression.getLocation (), ExternalError::get (MATCH_NO_DEFAULT), type.prettyString ());
	    }
	    
	    return result;
	}

	Generator MatchVisitor::validateMatch (const Generator & value, const Expression & matcher, bool & isMandatory) {
	    match (matcher) {
		of (syntax::Var, var,		    
		    if (var.getName () == Keys::UNDER) {
			isMandatory = true;
			return BoolValue::init (value.getLocation (), Bool::init (value.getLocation ()), true);
		    }
		);
				
		of (syntax::Binary, bin,
		    return validateMatchBinary (value, bin, isMandatory);
		);
		
		of (syntax::VarDecl, decl,
		    return validateMatchVarDecl (value, decl, isMandatory);
		);

		of (syntax::List, lst,
		    return validateMatchList (value, lst, isMandatory);
		);
				
		of (syntax::MultOperator, call, {
			if (call.getEnd () == Token::RPAR) 
			    return validateMatchCall (value, call, isMandatory);
		    }
		);
	    }
	    return validateMatchAnything (value, matcher, isMandatory);
	}

	Generator MatchVisitor::validateMatchVarDecl (const Generator & value_, const syntax::VarDecl & var, bool & isMandatory) {
	    std::vector <std::string> errors;
	    Generator test (Generator::empty ());
	    Generator varDecl (Generator::empty ());
	    TRY (
		Generator value (value_);
		if (var.getName () != Keys::UNDER)
		    this-> _context.verifyShadow (var.getName ());
	    
		Generator varValue (Generator::empty ());
		if (!var.getValue ().isEmpty ()) {
		    varValue = this-> _context.validateValue (var.getValue ());
		}

		Generator varType (Generator::empty ());
		if (!var.getType ().isEmpty ())		
		    varType = this-> _context.validateType (var.getType ());
		else {
		    varType = value.to <Value> ().getType (); // to have a vardecl, we must at least have a type or a value
		    varType.to <Type> ().isRef (false);
		    varType.to <Type> ().isMutable (false);
		}	    
	    
		bool isMutable = false;
		bool isRef = false;
		this-> _context.applyDecoratorOnVarDeclType (var.getDecorators (), varType, isRef, isMutable);

		this-> _context.verifySameType (varType, value.to <Value> ().getType ());
		if (!varValue.isEmpty ()) {
		    this-> _context.verifyMemoryOwner (var.getLocation (), varType, varValue, true);
		    test = validateMatch (value, TemplateSyntaxWrapper::init (varValue.getLocation (), varValue), isMandatory);		
		} else {
		    isMandatory = true;
		    varValue = value;
		    this-> _context.verifyMemoryOwner (var.getLocation (), varType, varValue, true);
		    test = BoolValue::init (value.getLocation (), Bool::init (value.getLocation ()), true);
		}
	    
		varDecl = generator::VarDecl::init (var.getLocation (), var.getName ().str, varType, varValue, isMutable);
		if (var.getName () != Keys::UNDER) {
		    this-> _context.insertLocal (var.getName ().str, varDecl);
		}
	    ) CATCH (ErrorCode::EXTERNAL) {
		GET_ERRORS_AND_CLEAR (msgs);
		errors = msgs;
		errors.insert (errors.begin (), Ymir::Error::createNote (var.getLocation (), ExternalError::get (IN_MATCH_DEF)));
	    } FINALLY;

	    if (errors.size () != 0)
		THROW (ErrorCode::EXTERNAL, errors);
	    
	    return Set::init (var.getLocation (), Bool::init (value_.getLocation ()), {varDecl, test});
	}

	Generator MatchVisitor::validateMatchBinary (const Generator & value, const syntax::Binary & bin, bool & isMandatory) {
	    if (bin.getLocation () == Token::PIPE) {
		auto testLeft = this-> validateMatch (value, bin.getLeft (), isMandatory);
		auto testRight = this-> validateMatch (value, bin.getRight (), isMandatory);
		
		return BinaryBool::init (bin.getLocation (), Binary::Operator::OR, Bool::init (bin.getLocation ()), testLeft, testRight);					 
	    } else if (bin.getLocation () == Token::DDOT || bin.getLocation () == Token::TDOT) {
		auto loc = lexing::Word{bin.getLocation (), Keys::IN};		
		auto  templ = syntax::TemplateCall::init (
		    loc,
		    {syntax::String::init (loc, loc, loc, lexing::Word::eof ())},
		    syntax::Var::init ({loc, BinaryVisitor::BINARY_OP_OVERRIDE})
		);
		
		auto call = syntax::MultOperator::init (
		    {loc, Token::LPAR}, {loc, Token::RPAR},
		    templ,
		    {TemplateSyntaxWrapper::init (value.getLocation (), value), bin.clone ()}
		);
		
		return this-> _context.validateValue (call);
	    }
	    
	    return validateMatchAnything (value, bin.clone (), isMandatory);	    
	}

	Generator MatchVisitor::validateMatchList (const Generator & value, const syntax::List & lst, bool & isMandatory) {
	    if (value.to <Value> ().getType ().is <Tuple> () && lst.getLocation () == Token::LPAR) {
		if (value.to <Value> ().getType ().to <Type> ().getInners ().size () == lst.getParameters ().size ()) {
		    isMandatory = true;
		    Generator globTest (Generator::empty ());
		    for (auto it : Ymir::r (0, value.to <Value> ().getType ().to <Type> ().getInners ().size ())) {
			bool loc_mandatory = false;
			auto acc = TupleAccess::init (lst.getLocation (), value.to <Value> ().getType ().to <Type> ().getInners ()[it], value, it);
			auto loc_test = validateMatch (acc, lst.getParameters () [it], loc_mandatory);
			isMandatory = isMandatory && loc_mandatory;
			if (it == 0)
			    globTest = loc_test;
			else globTest = BinaryBool::init (lst.getLocation (),
							  Binary::Operator::AND,
							  Bool::init (lst.getLocation ()),
							  globTest, loc_test);
		    }
		    
		    if (globTest.isEmpty () && lst.getParameters ().size () == 0) // No params
			return BoolValue::init (value.getLocation (), Bool::init (value.getLocation ()), true);
		    return globTest;
		}
	    } else if (value.to <Value> ().getType ().is <Array> () && lst.getLocation () == Token::LCRO) {
		if (value.to <Value> ().getType ().to <Array> ().getSize () == lst.getParameters ().size ()) {
		    isMandatory = true;
		    Generator globTest (Generator::empty ());
		    auto innerType = value.to <Value> ().getType ().to <Array> ().getInners ()[0];
		    for (auto it : Ymir::r (0, value.to <Value> ().getType ().to <Type> ().getInners ().size ())) {
			bool loc_mandatory = false;
			auto acc = ArrayAccess::init (lst.getLocation (), innerType, value, ufixed (lst.getLocation (), it));
			auto loc_test = validateMatch (acc, lst.getParameters ()[it], loc_mandatory);
			isMandatory = isMandatory && loc_mandatory;
			if (it == 0)
			    globTest = loc_test;
			else globTest = BinaryBool::init (lst.getLocation (),
							  Binary::Operator::AND,
							  Bool::init (lst.getLocation ()),
							  globTest, loc_test);
			if (globTest.isEmpty () && lst.getParameters ().size () == 0) // No params
			    return BoolValue::init (value.getLocation (), Bool::init (value.getLocation ()), true);
			return globTest;
		    }
		}
	    }
	    
	    return validateMatchAnything (value, lst.clone (), isMandatory);
	}

	Generator MatchVisitor::validateMatchCall (const Generator & value, const syntax::MultOperator & call, bool & isMandatory) {
	    if (!value.to <Value> ().getType ().is <StructRef> ()) {
		auto note = Ymir::Error::createNote (call.getLocation (), ExternalError::get (IN_MATCH_DEF));		
		Ymir::Error::occurAndNote (value.getLocation (), note, ExternalError::get (NOT_A_STRUCT), value.to <Value> ().getType ().prettyString ());
	    }
	    std::vector <std::string> errors;
	    TRY (
		if (!call.getLeft ().is <Var> () || call.getLeft ().to <Var> ().getName () != Keys::UNDER) {
		    auto type = this-> _context.validateType (call.getLeft ());
		    this-> _context.verifyCompatibleTypeWithValue (value.getLocation (), type, value);
		}
	    ) CATCH (ErrorCode::EXTERNAL) {
		GET_ERRORS_AND_CLEAR (msgs);
		errors.insert (errors.end (), msgs.begin (), msgs.end ());
		errors.insert (errors.begin (), Ymir::Error::createNote (call.getLocation (), ExternalError::get (IN_MATCH_DEF)));		
	    } FINALLY;
	    
	    if (errors.size () != 0)
		THROW (ErrorCode::EXTERNAL, errors);
	    
	    auto & str = value.to <Value> ().getType ().to <StructRef> ().getRef ().to <semantic::Struct> ().getGenerator ().to <generator::Struct> ();
	    	    
	    isMandatory = true;
	    std::vector <Expression> rights = call.getRights ();
	    std::vector <Expression> params;
	    for (auto it : str.getFields ()) {
		auto param = findParameterStruct (rights, it.to <generator::VarDecl> ());		    		
		params.push_back (param); // We add empties, to have the same amount of params as fields
		// The pattern can be successful even if there is some fields that are not tested
	    }
	    
	    if (rights.size () != 0) {
		std::vector <std::string> names;
		for (auto & it : rights)
		    names.push_back (it.prettyString ());
		
		Ymir::Error::occur (call.getLocation (), call.getEnd (), ExternalError::get (UNUSED_MATCH_CALL_OP), names);				    
	    }		

	    Generator globTest (Generator::empty ());
	    for (auto it : Ymir::r (0, params.size ())) {
		if (!params [it].isEmpty ()) {
		    auto loc_mandatory = false;
		    auto acc = StructAccess::init (value.getLocation (), str.getFieldType (str.getFields() [it].to <generator::VarDecl> ().getName ()), value, str.getFields() [it].to <generator::VarDecl> ().getName ());
		    if (value.is<Referencer> ()) {
			auto type = acc.to <Value> ().getType ();
			type.to <Type> ().isRef (true);
			acc = Referencer::init (acc.getLocation (), type, acc);
		    }
		    
		    auto loc_test = validateMatch (acc, params [it], loc_mandatory);		    
		    isMandatory = loc_mandatory && isMandatory;
		    
		    if (globTest.isEmpty ()) // Not necessarily when it == 0, as params [0] can be empty
			globTest = loc_test;
		    else globTest = BinaryBool::init (value.getLocation (),
						      Binary::Operator::AND,
						      Bool::init (value.getLocation ()),
						      globTest, loc_test);
		}
	    }

	    return globTest;		
	}

	syntax::Expression MatchVisitor::findParameterStruct (std::vector <Expression> & params, const generator::VarDecl & var) {
	    // Cf CallVisitor::findParameterStruct (), it is the same function but on expression instead of Generators
	    for (auto it : Ymir::r (0, params.size ())) {
		if (params [it].is <NamedExpression> ()) {
		    auto name = params [it].to <NamedExpression> ().getLocation ();
		    if (name.str == var.getLocation ().str) {
			auto toRet = params [it].to <NamedExpression> ().getContent ();
			params.erase (params.begin () + it);
			return toRet;
		    }		    
		}
	    }
	    
	    if (!var.getVarValue ().isEmpty ()) return Expression::empty ();
	    for (auto it : Ymir::r (0, params.size ())) {
		if (!params [it].is <NamedExpression> ()) {
		    auto toRet = params [it];
		    params.erase (params.begin () + it);
		    return toRet;
		}
	    }
	    return Expression::empty ();
	}
	
	
	Generator MatchVisitor::validateMatchAnything (const Generator & value, const syntax::Expression & matcher, bool & isMandatory) {	    
	    auto binVisitor = BinaryVisitor::init (this-> _context);
	    auto fakeBinary = syntax::Binary::init ({matcher.getLocation (), Token::DEQUAL}, TemplateSyntaxWrapper::init (value.getLocation (), value), matcher, Expression::empty ());
	    auto ret = binVisitor.validate (fakeBinary.to <syntax::Binary> ());
	    Generator retValue (Generator::empty ());
	    TRY (
		retValue = this-> _context.retreiveValue (ret);
	    ) CATCH (ErrorCode::EXTERNAL) {
	    	GET_ERRORS_AND_CLEAR (msgs);		
	    } FINALLY;
	    
	    if (!retValue.isEmpty ()) {
		isMandatory = retValue.is <BoolValue> () && retValue.to <BoolValue> ().getValue ();
		return value;
	    } else return ret;
	}	
    }
    
}
