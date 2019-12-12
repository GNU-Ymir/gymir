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
	    value = UniqValue::init (value.getLocation (), value.to <Value> ().getType (), value);

	    Generator result (Generator::empty ());
	    
	    auto & matchers = expression.getMatchers ();
	    auto & actions  = expression.getActions ();
	    Generator type (Generator::empty ());
	    for (auto it_ : Ymir::r (0, matchers.size ())) {
		auto it = matchers.size () - 1 - it_; // We get them in the reverse order to have the tests in the right order
		Generator test (Generator::empty ());
		std::vector <std::string> errors;
		this-> _context.enterBlock ();
		{
		    TRY (
			test = this-> validateMatch (value, matchers [it]);
		    ) CATCH (ErrorCode::EXTERNAL) {
			GET_ERRORS_AND_CLEAR (msgs);
			errors = msgs;
		    } FINALLY;
		}
		
		if (!test.isEmpty ()) { // size == 2, if succeed
		    auto content = this-> _context.validateValue (actions [it]);
		    auto local_type = content.to <Value> ().getType ();
		    if (type.isEmpty ()) type = local_type;
		    else this-> _context.verifyCompatibleType (type, local_type);
		    
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
	    }
	    
	    return result;
	}

	Generator MatchVisitor::validateMatch (const Generator & value, const Expression & matcher) {
	    match (matcher) {
		of (syntax::Var, var,
		    if (var.getName () == Keys::UNDER)
			return BoolValue::init (value.getLocation (), Bool::init (value.getLocation ()), true);
		);
	    }

	    auto binVisitor = BinaryVisitor::init (this-> _context);
	    auto fakeBinary = syntax::Binary::init ({matcher.getLocation (), Token::DEQUAL}, TemplateSyntaxWrapper::init (value.getLocation (), value), matcher, Expression::empty ());
	    return binVisitor.validate (fakeBinary.to <syntax::Binary> ());
	}
	
    }
    
}
