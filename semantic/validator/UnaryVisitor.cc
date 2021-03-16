#include <ymir/semantic/validator/UnaryVisitor.hh>
#include <ymir/semantic/validator/CallVisitor.hh>
#include <ymir/semantic/validator/FunctionVisitor.hh>
#include <ymir/semantic/generator/value/_.hh>
#include <ymir/global/Core.hh>

namespace semantic {

    namespace validator {

	using namespace Ymir;
	using namespace generator;
	using namespace global;

	UnaryVisitor::UnaryVisitor (Visitor & context) :
	    _context (context)
	{}

	UnaryVisitor UnaryVisitor::init (Visitor & context) {
	    return UnaryVisitor (context);
	}

	Generator UnaryVisitor::validate (const syntax::Unary & expression) {
	    auto op = toOperator (expression.getOperator ());
	    // If the operator is &, we want to have access to the adress of the function,
	    // so it is forbidden to fast call any function (even if it is a dot templateCall)
	    auto operand = this-> _context.validateValue (expression.getContent (), false, op == Unary::Operator::ADDR);

	    if (op == Unary::Operator::ADDR) { // Pointer
		match (operand) {
		    of_u (FrameProto)
			return validateFunctionPointer (expression, operand);
		    elfo {
			if (operand.is <MultSym> ()) {
			    auto gen = validateDelegatePointer (expression, operand);
			    if (!gen.isEmpty ()) return gen;
			}
			
			if (operand.to <Value> ().isLvalue ()) {
			    auto inner = operand.to<Value> ().getType ();
			    inner = Type::init (inner.to <Type> (), inner.to <Type> ().isMutable (), false);
			    auto type = Pointer::init (operand.getLocation (), inner);
			    type = Type::init (type.to<Type> (), true);
			    return Addresser::init (expression.getLocation (), type, operand);
			} else {
			    auto note = Ymir::Error::createNote (expression.getLocation (), ExternalError::get (OF), operand.prettyString ());
			    Ymir::Error::occurAndNote (operand.getLocation (), note,
						       ExternalError::get (NOT_A_LVALUE)
			    );
			}
		    }
		}
	    } else {	    	
		match (operand.to <Value> ().getType ()) {
		    s_of_u (Bool)
			return validateBool (expression, operand);		    
		    
		    s_of_u (Float)
			return validateFloat (expression, operand);		    

		    s_of_u (Integer)
			return validateInt (expression, operand);
		    
		    s_of_u (Pointer)
			return validatePointer (expression, operand);
		    		    
		    s_of_u (ClassPtr)
			return validateClass (expression, operand);
		    
		}
	    }
	    
	    UnaryVisitor::error (expression, operand);	
	    return Generator::empty ();	    
	}

	Generator UnaryVisitor::validateBool (const syntax::Unary & un, const Generator & operand) {
	    auto op = toOperator (un.getOperator ());
	    if (op == Unary::Operator::NOT)
		return UnaryBool::init (un.getLocation (), op, operand.to <Value> ().getType (), operand);
	    
	    error (un, operand);
	    return Generator::empty ();
	}       

	Generator UnaryVisitor::validateFloat (const syntax::Unary & un, const Generator & operand) {
	    auto op = toOperator (un.getOperator ());
	    if (op == Unary::Operator::MINUS)
		return UnaryFloat::init (un.getLocation (), op, operand.to <Value> ().getType (), operand);
	    
	    error (un, operand);
	    return Generator::empty ();
	}

       	Generator UnaryVisitor::validateInt (const syntax::Unary & un, const Generator & operand) {
	    auto op = toOperator (un.getOperator ());
	    if (op == Unary::Operator::MINUS)
		return UnaryInt::init (un.getLocation (), op, operand.to <Value> ().getType (), operand);
	    
	    error (un, operand);
	    return Generator::empty ();
	}

	Generator UnaryVisitor::validatePointer (const syntax::Unary & un, const Generator & operand) {
	    auto op = toOperator (un.getOperator ());
	    // This is unsafe, we don't want to do that in ymir right ?
	    // This will throw SegFault exception, sometimes
	    
	    if (op == Unary::Operator::UNREF) {
	    	auto type = operand.to <Value> ().getType ().to <Type> ().getInners ()[0];
		
	    	if (!operand.to <Value> ().getType ().to <Type> ().isMutable ()) {
	    	    type = Type::init (type.to<Type> (), false);
		}

		auto ret = UnaryPointer::init (un.getLocation (),
					       op,
					       type,
					       operand
	    	);

		auto loc = un.getLocation ();
		// It might seg fault
		auto syntaxType = this-> _context.createClassTypeFromPath (loc, {CoreNames::get (CORE_MODULE), CoreNames::get (EXCEPTION_MODULE), CoreNames::get (SEG_FAULT_TYPE)});
		auto segFaultType = Generator::init (un.getLocation (), this-> _context.validateType (syntaxType));
		
		auto thrs = ret.getThrowers ();
		thrs.push_back (segFaultType);
		ret.setThrowers (thrs);
		
		return ret;
	    }

	    error (un, operand);
	    return Generator::empty ();
	}

	Generator UnaryVisitor::validateClass (const syntax::Unary & expression, const Generator & operand) {
	    auto loc = expression.getLocation ();
	    auto leftSynt = TemplateSyntaxWrapper::init (loc, operand);
	    auto templ = syntax::TemplateCall::init (
		loc,
		{syntax::String::init (loc, loc, loc, lexing::Word::eof ())},
		syntax::Binary::init (
		    lexing::Word::init (loc, Token::DOT),
		    leftSynt,		    
		    syntax::Var::init (lexing::Word::init (loc, CoreNames::get (UNARY_OP_OVERRIDE))),
		    syntax::Expression::empty ()
		)
	    );

	    auto call = syntax::MultOperator::init (
		lexing::Word::init (loc, Token::LPAR), lexing::Word::init (loc, Token::RPAR),
		templ,
		{}, false
	    );

	    try {
		return this-> _context.validateValue (call);
	    } catch (Error::ErrorList list) {
		auto note = Ymir::Error::createNoteOneLine (ExternalError::get (VALIDATING), call.prettyString ());
		list.errors.back ().addNote (note);
		throw list;
	    }
	}
	
	Generator UnaryVisitor::validateFunctionPointer (const syntax::Unary & un, const Generator & proto) {
	    // Proto is a frameproto
	    if (proto.getThrowers ().size () != 0) {
		std::list <Ymir::Error::ErrorMsg> notes;
		for (auto &it : proto.getThrowers ())
		    notes.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (THROWS), it.prettyString ()));
		Ymir::Error::occurAndNote (un.getLocation (), notes, ExternalError::get (ADDR_MIGHT_THROW), proto.prettyString ());
	    }
	    return Addresser::init (un.getLocation (), FunctionVisitor::init (this-> _context).validateFunctionType (proto), proto);
	}

	Generator UnaryVisitor::validateDelegatePointer (const syntax::Unary & un, const Generator & gen) {
	    // All this is probably over kill, there are two cases actually mutable and immutable, and there is no possibility that one works if the other does
	    // So, I put that as it is, because it is thus totally safe if there is modification in the future, but a big optimization pass should be done 
	    std::list <Error::ErrorMsg> errors;
	    std::map <int, std::vector <Generator> > results;
	    int highest = 0;
	    for (auto & del : gen.to <MultSym> ().getGenerators ()) {
		try {
		    if (!del.is <DelegateValue> ()) {
			auto note = Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), del.to <Value> ().getType ().getLocation (), del.prettyString ());
			throw Error::ErrorList {{note}};
		    }
	    	     
		    if (del.getThrowers ().size () != 0) {
			std::list <Ymir::Error::ErrorMsg> notes;
			for (auto &it : del.getThrowers ())
			    notes.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (THROWS), it.prettyString ()));
			Ymir::Error::occurAndNote (un.getLocation (), notes, ExternalError::get (ADDR_MIGHT_THROW), del.prettyString ());	
		    }
	    
		    auto funcPtr = del.to <DelegateValue> ().getFuncPtr ();
		    auto proto = del.to <Value> ().getType ().to <Type> ().getInners ()[0];
		    if (proto.is <MethodProto> ()) {
			auto meth = proto.to <MethodProto> ();
			Generator type (Generator::empty ());
			if (meth.isMutable ()) {
			    type = meth.getClassType ().to <Type> ().toDeeplyMutable ();
			} else type = Type::init (meth.getClassType ().to <Type> (), meth.isMutable ());

			try {
			    // Verify that the delegate won't implicitly alias the closure
			    this-> _context.verifyImplicitAlias (un.getLocation (), type, del.to <DelegateValue> ().getClosure ());
			    this-> _context.verifyMemoryOwner (un.getLocation (), type, del.to <DelegateValue> ().getClosure (), true);
			} catch (Error::ErrorList list) {
			    auto note = Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), del.to <Value> ().getType ().getLocation (), proto.prettyString ());

			    for (auto & i : list.errors)
				note.addNote (i);
			    throw Error::ErrorList {{note}};
			}

			auto llevel = del.to <DelegateValue> ().getClosure ().to <Value> ().getType ().to <Type> ().mutabilityLevel ();
			auto rlevel = type.to <Type> ().mutabilityLevel () + 1;
			auto score = rlevel - llevel;
			if (score > highest) highest = score;
			
			auto delType = Delegate::init (un.getLocation (), FunctionVisitor::init (this-> _context).validateFunctionType (proto));
			if (results.find (score) != results.end ()) {
			    results[score].push_back (Value::init (del.to <Value> (), delType));
			} else results [score] = {Value::init (del.to <Value> (), delType)};
		    }
		} catch (Error::ErrorList list) {
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		}
	    }

	    auto elements = results.find (highest);
	    if (elements == results.end ()) {
		this-> error (un, gen, errors);
	    }

	    if (elements-> second.size () == 1) return elements-> second [0];
	    else {
		std::list <Error::ErrorMsg> notes;
		for (auto & it : elements-> second)
		    notes.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), CallVisitor::realLocation (it), CallVisitor::prettyName (it)));
		
		Ymir::Error::occurAndNote (un.getLocation (),
					   notes,
					   ExternalError::get (SPECIALISATION_WORK_WITH_BOTH_PURE),
					   gen.prettyString ());
	    }
	    
	    return Generator::empty ();
	}
	
	void UnaryVisitor::error (const syntax::Unary & un, const generator::Generator & left) {
	    std::string leftName;
	    match (left) {
		of (FrameProto, proto) leftName = proto.getName ();
		elof (generator::Struct, str) leftName = str.getName ();
		elof (MultSym,    sym)   leftName = sym.getLocation ().getStr ();
		elof (Value,      val)   leftName = val.getType ().to <Type> ().getTypeName ();
		fo;
	    }
	    
	    Ymir::Error::occur (un.getLocation (),
				ExternalError::get (UNDEFINED_UN_OP),
				un.getLocation ().getStr (), 
				leftName
	    );
	}	

	void UnaryVisitor::error (const syntax::Unary & un, const generator::Generator & left, const std::list <Error::ErrorMsg> & errors) {
	    std::string leftName;
	    match (left) {
		of (FrameProto, proto) leftName = proto.getName ();
		elof (generator::Struct, str) leftName = str.getName ();
		elof (MultSym,    sym)   leftName = sym.getType ().prettyString ();
		elof (Value,      val)   leftName = val.getType ().to <Type> ().getTypeName ();
		fo;
	    }
	    
	    Ymir::Error::occurAndNote (un.getLocation (),
				       errors,
				       ExternalError::get (UNDEFINED_UN_OP),
				       un.getLocation ().getStr (), 
				       leftName
	    );
	}	

	
	Unary::Operator UnaryVisitor::toOperator (const lexing::Word & loc) {
	    string_match (loc.getStr ()) {
		eq (Token::NOT) return Unary::Operator::NOT;
		eq (Token::MINUS) return Unary::Operator::MINUS;
		eq (Token::STAR) return Unary::Operator::UNREF;
		eq (Token::AND) return Unary::Operator::ADDR;
	    }	    

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Unary::Operator::LAST_OP;
	}
	
    }
    
}
