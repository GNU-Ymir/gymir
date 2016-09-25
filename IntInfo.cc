#include "IntInfo.hh"
#include "Expression.hh"
#include "IntUtils.hh"

namespace Semantic {

    IntInfo::_init IntInfo::_initializer;
    

    IntInfo::IntInfo () {}
    
    IntInfo::IntInfo (TypeInfo::BinopLint lint) {
	this->binopFoo = lint;
    }

    IntInfo::IntInfo (TypeInfo::UnopLint lint) {
	this->unopFoo = lint;
    }    
    
    TypeInfo* IntInfo::binaryOp (Lexical::TokenPtr tok, Syntax::Expression* right) {
	switch (tok->getId ()) {
	case Lexical::AFFECT : return Affect (right->getType ());
	    /*case PLUS_AFF : return PlusAff (right);
	case MUL_AFF : return MulAff (right);
	case SUB_AFF : return SubAff (right);
	case DIV_AFF : return DivAff (right);
	case INF : return Inf (right);
	case SUP : return Sup (right);
	case EQUALS : return Equals (right);
	case SUP_EQUALS : return SupEquals (right);
	case INF_EQUALS : return InfEquals (right);
	case NOT_EQUALS : return NotEquals (right);
	case PLUS : return Plus (right);
	case SUB : return Sub (right);
	case OR_BIT : return OrBit (right);
	case LEFT_BIT : return LeftBit (right);
	case RIGHT_BIT : return RightBit (right);
	case XOR : return Xor (right);
	case MUL : return Mul (right);
	case DIV : return Div (right);
	case AND_BIT : return AndBit (right);
	case MODULO : return Modulo (right);*/
	default : return NULL;
	}
    }

    TypeInfo * IntInfo::binaryOpRight (Lexical::TokenPtr, Syntax::Expression *) {
	return NULL;
    }

    TypeInfo * IntInfo::unaryOp (Lexical::TokenPtr) {
	return NULL;
    }

    TypeInfo * IntInfo::multOp (Lexical::TokenPtr, Syntax::Expression *) {
	return NULL;
    }
    
    TypeInfo * IntInfo::Affect (TypeInfo * right) {
	if (right->Is(INT)) return new IntInfo (&IntUtils::InstAffInt);
	else if (right->Is(CHAR)) return new IntInfo (&IntUtils::InstAffChar);
	else if (right->Is(FLOAT)) return new IntInfo (&IntUtils::InstAffFloat);
	else return NULL;
    }

    std::string IntInfo::typeToString () const {
	return "int";
    }
    
    bool IntInfo::Is (TypeEnum info) {
	return info == INT;
    }


}
