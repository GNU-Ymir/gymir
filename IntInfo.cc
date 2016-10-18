#include "IntInfo.hh"
#include "Expression.hh"
#include "IntUtils.hh"
#include "BoolInfo.hh"

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
	case Lexical::AFFECT : return Affect (right-> getType ()-> type);
	case Lexical::PLUS_AFF : return PlusAff (right-> getType ()-> type);
	case Lexical::MUL_AFF : return MulAff (right-> getType ()-> type);
	case Lexical::SUB_AFF : return SubAff (right-> getType ()-> type);
	case Lexical::DIV_AFF : return DivAff (right-> getType ()-> type);
	case Lexical::INF : return Inf (right-> getType ()-> type);
	case Lexical::SUP : return Sup (right-> getType ()-> type);
	case Lexical::EQUALS : return Equals (right-> getType ()-> type);
	case Lexical::SUP_EQUALS : return SupEquals (right-> getType ()-> type);
	case Lexical::INF_EQUALS : return InfEquals (right-> getType ()-> type);
	case Lexical::NOT_EQUALS : return NotEquals (right-> getType ()-> type);
	case Lexical::PLUS : return Plus (right-> getType ()-> type);
	case Lexical::SUB : return Sub (right-> getType ()-> type);
	case Lexical::OR_BIT : return OrBit (right-> getType ()-> type);
	case Lexical::LEFT_BIT : return LeftBit (right-> getType ()-> type);
	case Lexical::RIGHT_BIT : return RightBit (right-> getType ()-> type);
	case Lexical::XOR : return Xor (right-> getType ()-> type);
	case Lexical::MUL : return Mul (right-> getType ()-> type);
	case Lexical::DIV : return Div (right-> getType ()-> type);
	case Lexical::AND_BIT : return AndBit (right-> getType ()-> type);
	case Lexical::MODULO : return Modulo (right-> getType ()-> type);
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

    TypeInfo * IntInfo::clone () {
	return new IntInfo ();
    }
    
    TypeInfo * IntInfo::Affect (TypeInfo * right) {
	if (right->Is(INT)) return new IntInfo (&IntUtils::InstAffInt);
	else if (right->Is(CHAR)) return new IntInfo (&IntUtils::InstAffChar);
	else if (right->Is(FLOAT)) return new IntInfo (&IntUtils::InstAffFloat);
	else return NULL;
    }

    TypeInfo * IntInfo::PlusAff (TypeInfo * right) {
	if (right-> Is (INT)) return new IntInfo (&IntUtils::InstPlusAffInt);
	else if (right-> Is (CHAR)) return new IntInfo (&IntUtils::InstPlusAffChar);
	else if (right-> Is (FLOAT)) return new IntInfo (&IntUtils::InstPlusAffFloat);
	else return NULL;
    }

    TypeInfo * IntInfo::MulAff (TypeInfo * right) {
	if (right-> Is (INT)) return new IntInfo (&IntUtils::InstMulAffInt);
	else if (right-> Is (CHAR)) return new IntInfo (&IntUtils::InstMulAffChar);
	else if (right-> Is (FLOAT)) return new IntInfo (&IntUtils::InstMulAffFloat);
	else return NULL;
    }
    
    TypeInfo * IntInfo::SubAff (TypeInfo * right) {
	if (right-> Is (INT)) return new IntInfo (&IntUtils::InstSubAffInt);
	else if (right-> Is (CHAR)) return new IntInfo (&IntUtils::InstSubAffChar);
	else if (right-> Is (FLOAT)) return new IntInfo (&IntUtils::InstSubAffFloat);
	else return NULL;
    }

    TypeInfo * IntInfo::DivAff (TypeInfo * right) {
	if (right-> Is (INT)) return new IntInfo (&IntUtils::InstDivAffInt);
	else if (right-> Is (CHAR)) return new IntInfo (&IntUtils::InstDivAffChar);
	else if (right-> Is (FLOAT)) return new IntInfo (&IntUtils::InstDivAffFloat);
	else return NULL;
    }	

    TypeInfo * IntInfo::Inf (TypeInfo * right) {
	if (right-> Is (INT)) return new BoolInfo (&IntUtils::InstInfInt);
	else if (right-> Is (CHAR)) return new BoolInfo (&IntUtils::InstInfChar);
	else if (right-> Is (FLOAT)) return new BoolInfo (&IntUtils::InstInfFloat);
	else return NULL;
    }

    TypeInfo * IntInfo::Sup (TypeInfo * right) {
	if (right-> Is (INT)) return new BoolInfo (&IntUtils::InstSupInt);
	else if (right-> Is (CHAR)) return new BoolInfo (&IntUtils::InstSupChar);
	else if (right-> Is (FLOAT)) return new BoolInfo (&IntUtils::InstSupFloat);
	else return NULL;
    }

    TypeInfo * IntInfo::Equals (TypeInfo * right) {
	if (right-> Is (INT)) return new BoolInfo (&IntUtils::InstEqualsInt);
	else if (right-> Is (CHAR)) return new BoolInfo (&IntUtils::InstEqualsChar);
	else if (right-> Is (FLOAT)) return new BoolInfo (&IntUtils::InstEqualsFloat);
	else return NULL;
    }

    TypeInfo * IntInfo::SupEquals (TypeInfo * right) {
	if (right-> Is (INT)) return new BoolInfo (&IntUtils::InstSupEqualsInt);
	else if (right-> Is (CHAR)) return new BoolInfo (&IntUtils::InstSupEqualsChar);
	else if (right-> Is (FLOAT)) return new BoolInfo (&IntUtils::InstSupEqualsFloat);
	else return NULL;
    }

    TypeInfo * IntInfo::InfEquals (TypeInfo * right) {
	if (right-> Is (INT)) return new BoolInfo (&IntUtils::InstInfEqualsInt);
	else if (right-> Is (CHAR)) return new BoolInfo (&IntUtils::InstInfEqualsChar);
	else if (right-> Is (FLOAT)) return new BoolInfo (&IntUtils::InstInfEqualsFloat);
	else return NULL;
    }
    
    TypeInfo * IntInfo::NotEquals (TypeInfo * right) {
	if (right-> Is (INT)) return new BoolInfo (&IntUtils::InstNotEqualsInt);
	else if (right-> Is (CHAR)) return new BoolInfo (&IntUtils::InstNotEqualsChar);
	else if (right-> Is (FLOAT)) return new BoolInfo (&IntUtils::InstNotEqualsFloat);
	else return NULL;
    }

    TypeInfo * IntInfo::Plus (TypeInfo * right) {
	if (right-> Is (INT)) return new IntInfo (&IntUtils::InstPlusInt);
	else return NULL;
    }

    TypeInfo * IntInfo::Sub (TypeInfo * right) {
	if (right-> Is (INT)) return new IntInfo (&IntUtils::InstSubInt);
	else return NULL;
    }

    TypeInfo * IntInfo::OrBit (TypeInfo * right) {
	if (right-> Is (INT)) return new IntInfo (&IntUtils::InstOrBitInt);
	else return NULL;
    }

    TypeInfo * IntInfo::LeftBit (TypeInfo * right) {
	if (right-> Is (INT)) return new IntInfo (&IntUtils::InstLeftBitInt);
	else return NULL;
    }

    TypeInfo * IntInfo::RightBit (TypeInfo * right) {
	if (right-> Is (INT)) return new IntInfo (&IntUtils::InstRightBitInt);
	else return NULL;
    }

    TypeInfo * IntInfo::Xor (TypeInfo * right) {
	if (right-> Is (INT)) return new IntInfo (&IntUtils::InstXorInt);
	else return NULL;
    }

    TypeInfo * IntInfo::Mul (TypeInfo * right) {
	if (right-> Is (INT)) return new IntInfo (&IntUtils::InstMulInt);
	else return NULL;
    }

    TypeInfo * IntInfo::Div (TypeInfo * right) {
	if (right-> Is (INT)) return new IntInfo (&IntUtils::InstDivInt);
	else return NULL;
    }

    TypeInfo * IntInfo::AndBit (TypeInfo * right) {
	if (right-> Is (INT)) return new IntInfo (&IntUtils::InstAndBitInt);
	else return NULL;
    }

    TypeInfo * IntInfo::Modulo (TypeInfo * right) {
	if (right-> Is (INT)) return new IntInfo (&IntUtils::InstModuloInt);
	else return NULL;
    }
    
    std::string IntInfo::typeToString () const {
	return "int";
    }
    
    bool IntInfo::Is (TypeEnum info) {
	return info == INT;
    }


}
