#include <ymir/semantic/validator/UnaryVisitor.hh>
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
		    of (FrameProto, proto ATTRIBUTE_UNUSED,
			return validateFunctionPointer (expression, operand);
		    ) else {
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
		    of (Bool, b ATTRIBUTE_UNUSED,
			return validateBool (expression, operand);
		    );

		    of (Float, f ATTRIBUTE_UNUSED,
			return validateFloat (expression, operand);
		    );

		    of (Integer, i ATTRIBUTE_UNUSED,
			return validateInt (expression, operand);
		    );

		    of (Pointer, p ATTRIBUTE_UNUSED,			
			return validatePointer (expression, operand);
		    );

		    of (ClassPtr, p ATTRIBUTE_UNUSED,
			return validateClass (expression, operand);
		    );
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
	    return Addresser::init (un.getLocation (), this-> _context.validateFunctionType (proto), proto);
	}
	
	void UnaryVisitor::error (const syntax::Unary & un, const generator::Generator & left) {
	    std::string leftName;
	    match (left) {
		of (FrameProto, proto, leftName = proto.getName ())
		else of (generator::Struct, str, leftName = str.getName ())
		else of (MultSym,    sym,   leftName = sym.getLocation ().getStr ())
		else of (Value,      val,   leftName = val.getType ().to <Type> ().getTypeName ());
	    }
	    
	    Ymir::Error::occur (un.getLocation (),
				ExternalError::get (UNDEFINED_UN_OP),
				un.getLocation ().getStr (), 
				leftName
	    );
	}	

	Unary::Operator UnaryVisitor::toOperator (const lexing::Word & loc) {
	    string_match (loc.getStr ()) {
		eq (Token::NOT, return Unary::Operator::NOT;);
		eq (Token::MINUS, return Unary::Operator::MINUS;);
		eq (Token::STAR, return Unary::Operator::UNREF;);
		eq (Token::AND, return Unary::Operator::ADDR;);
	    }	    

	    Ymir::Error::halt ("%(r) - reaching impossible point", "Critical");
	    return Unary::Operator::LAST_OP;
	}
	
    }
    
}
