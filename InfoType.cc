#include <ymir/semantic/tree/Tree.hh>
#include "semantic/types/InfoType.hh"
#include "syntax/Word.hh"
#include <ymir/syntax/Keys.hh>
#include "ast/Expression.hh"
#include "ast/Var.hh"
#include "errors/Error.hh"
#include <ymir/semantic/utils/OperatorUtils.hh>
#include <ymir/semantic/pack/InternalFunction.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/value/_.hh>

namespace semantic {

    std::map <std::string, InfoType> IInfoType::__alias__;
    
    IApplicationScore::IApplicationScore () :
	score (0)
    {}

    IApplicationScore::IApplicationScore (Word tok, bool isVariadic) :
	score (0),
	token (tok),
	isVariadic (isVariadic)
    {}
        
    IInfoType::IInfoType (bool isConst) :
	_isConst (isConst)
    {}   


    Ymir::Tree IInfoType::buildBinaryOp (Word locus, InfoType type, syntax::Expression left, syntax::Expression right) {
	return this-> binopFoo (locus, type, left, right);
    }

    Ymir::Tree IInfoType::buildCastOp (Word locus, InfoType _type, syntax::Expression elem, syntax::Expression type) {
	return this-> binopFoo (locus, _type, elem, type);
    }
    
    Ymir::Tree IInfoType::buildUnaryOp (Word locus, InfoType _type, syntax::Expression elem) {
	return this-> unopFoo (locus, _type, elem);
    }

    Ymir::Tree IInfoType::buildMultOp (Word locus, InfoType type, syntax::Expression left, syntax::Expression rights) {
	return this-> multFoo (locus, type, left, rights);
    }
    
    Ymir::Tree IInfoType::buildApplyOp (Word locus, std::vector <syntax::Var> & vars, syntax::Block bl, syntax::Expression expr) {
	return this-> applyFoo (locus, vars, bl, expr);
    }
    
    InfoType IInfoType::factory (Word word, const std::vector<syntax::Expression> & templates) {
	auto it = (InfoType (*)(Word, const std::vector<syntax::Expression> &)) (
	    Creators::instance ().find (word.getStr ())
	);
	if (it != NULL) {
	    return (*it) (word, templates);
	}
	auto _it_ = __alias__.find (word.getStr ());
	if (_it_ != __alias__.end ()) return _it_-> second-> clone ();
	Ymir::Error::unknownType (word);
	return NULL;
    }

    
    ulong& IInfoType::toGet () {
	return this-> _toGet;
    }
    
    bool IInfoType::isConst () {
	return this-> _isConst;
    }

    void IInfoType::isConst (bool is) {
	this-> _isConst = is;
    }
    
    bool& IInfoType::isStatic () {
	return this-> _isStatic;
    }

    bool IInfoType::isScopable () {
	return false;
    }

    bool IInfoType::isType () {
	return this-> _isType;
    }

    void IInfoType::isType (bool is) {
	this-> _isType = is;
    }

    Value& IInfoType::value () {
	return this-> _value;
    }
    
    std::string IInfoType::typeString () {
	if (this-> isConst ()) {
	    return std::string ("const(") + this-> innerTypeString () + ")";
	} else return this-> innerTypeString ();
    }

    std::string IInfoType::simpleTypeString () {
	if (this-> isConst ()) {
	    return std::string ("x") + this-> innerSimpleTypeString ();
	} else return this-> innerSimpleTypeString ();
    }
    
    InfoType IInfoType::BinaryOp (Word w, InfoType info) {
	auto expr = new (Z0) syntax::IExpression (w);
	expr-> info = new (Z0)  ISymbol (w, info);
	return this-> BinaryOp (w, expr);
    }
    
    InfoType IInfoType::BinaryOp (Word, syntax::Expression) {
	return NULL;
    }

    InfoType IInfoType::BinaryOpRight (Word w, InfoType info) {
	auto expr = new (Z0) syntax::IExpression (w);
	expr-> info = new (Z0)  ISymbol (w, info);
	return this-> BinaryOpRight (w, expr);
    }
    
    InfoType IInfoType::BinaryOpRight (Word, syntax::Expression) {
	return NULL;
    }

    ApplicationScore IInfoType::CallOp (Word, syntax::ParamList) {
	return NULL;
    }

    ApplicationScore IInfoType::CallOp (Word, const std::vector<InfoType> &) {
	return NULL;
    }
    
    InfoType IInfoType::ApplyOp (const std::vector<::syntax::Var> &) {
	return NULL;
    }

    InfoType IInfoType::UnaryOp (Word) {
	return NULL;
    }

    InfoType IInfoType::AccessOp (Word, syntax::ParamList, std::vector <InfoType> &) {
	return NULL;
    }

    InfoType IInfoType::CastOp (InfoType) {
	return NULL;
    }

    InfoType IInfoType::CompOp (InfoType) {
	return NULL;
    }

    InfoType IInfoType::ConstVerif (InfoType) {
	return this;
    }
    
    InfoType IInfoType::CastTo (InfoType) {
	return NULL;
    }

    InfoType IInfoType::DotOp (::syntax::Var) {
	return NULL;
    }

    InfoType IInfoType::DotExpOp (syntax::Expression) {
	return NULL;
    }

    InfoType IInfoType::DColonOp (::syntax::Var) {
	return NULL;
    }

    InfoType IInfoType::DotOp (std::string) {
	return NULL;
    }

    InfoType IInfoType::TempOp (const std::vector<syntax::Expression> &) {
	return NULL;
    }

    InfoType IInfoType::StringOf () {
	auto str = new (Z0)  IStringInfo (true);
	str-> value () = new (Z0)  IStringValue (this-> typeString ().c_str ());
	return str;
    }	

    bool IInfoType::isImmutable () {
	return false;
    }
    
    InfoType IInfoType::clone () {
	auto ret = this-> onClone ();
	if (ret) {
	    ret-> value () = this-> value ();
	}
	return ret;
    }

    InfoType IInfoType::cloneOnExit () {
	auto ret = this-> clone ();
	if (ret) ret-> value () = NULL;
	return ret;
    }

    InfoType IInfoType::cloneConst () {
	auto ret = this-> clone ();
	ret-> isConst (true);
	return ret;
    }

    InfoType IInfoType::onlyInMod (Module mod) {
	auto ret = this-> clone ();
	ret-> _onlyInMe = mod;
	return ret;
    }

    
    InfoType IInfoType::getTemplate (ulong) {
	return NULL;
    }

    ulong IInfoType::nbTemplates () {
	return 0;
    }
    
    Ymir::Tree IInfoType::toGeneric () {
	Ymir::Error::assert ((std::string ("TODO") + this-> getId ()).c_str ());
	return NULL;
    }

    Ymir::Tree IInfoType::getInitFnPtr () {
	return InternalFunction::getYInitType (this-> innerTypeString ().c_str ());
    }
    
    std::vector <InfoType> IInfoType::getTemplate (ulong bef, ulong) {
	return {this-> getTemplate (bef)};
    }

    namespace OperatorUtils {

	tree_code toGeneric (Word op) {
	    if (op == Token::EQUAL) return MODIFY_EXPR;
	    if (op == Token::DIV_AFF) return TRUNC_DIV_EXPR;
	    if (op == Token::AND_AFF) return BIT_AND_EXPR;
	    if (op == Token::PIPE_EQUAL) return BIT_IOR_EXPR;
	    if (op == Token::MINUS_AFF) return MINUS_EXPR;
	    if (op == Token::PLUS_AFF) return PLUS_EXPR;
	    if (op == Token::LEFTD_AFF) return LSHIFT_EXPR;
	    if (op == Token::RIGHTD_AFF) return RSHIFT_EXPR;
	    if (op == Token::STAR_EQUAL) return MULT_EXPR;
	    if (op == Token::PERCENT_EQUAL) return TRUNC_MOD_EXPR;
	    if (op == Token::XOR_EQUAL) return BIT_XOR_EXPR;
	    if (op == Token::DAND) return TRUTH_ANDIF_EXPR;
	    if (op == Token::DPIPE) return TRUTH_ORIF_EXPR;
	    if (op == Token::INF) return LT_EXPR;
	    if (op == Token::SUP) return GT_EXPR;
	    if (op == Token::INF_EQUAL) return LE_EXPR;
	    if (op == Token::SUP_EQUAL) return GE_EXPR;
	    if (op == Token::NOT_EQUAL) return NE_EXPR;
	    if (op == Token::DEQUAL) return EQ_EXPR;
	    if (op == Token::PLUS) return PLUS_EXPR;
	    if (op == Token::MINUS) return MINUS_EXPR;
	    if (op == Token::DIV) return TRUNC_DIV_EXPR;
	    if (op == Token::STAR) return MULT_EXPR;
	    if (op == Token::PIPE) return BIT_IOR_EXPR;
	    if (op == Token::AND) return BIT_AND_EXPR;
	    if (op == Token::LEFTD) return LSHIFT_EXPR;
	    if (op == Token::XOR) return BIT_XOR_EXPR;
	    if (op == Token::RIGHTD) return RSHIFT_EXPR;
	    if (op == Token::PERCENT) return TRUNC_MOD_EXPR;
	    if (op == Keys::NOT_IS) return NE_EXPR;
	    if (op == Keys::IS) return EQ_EXPR;
	    return NOP_EXPR;
	}
	
	tree_code toGenericReal (Word op) {
	    if (op == Token::EQUAL) return MODIFY_EXPR;
	    if (op == Token::DIV_AFF) return RDIV_EXPR;
	    if (op == Token::MINUS_AFF) return MINUS_EXPR;
	    if (op == Token::PLUS_AFF) return PLUS_EXPR;
	    if (op == Token::STAR_EQUAL) return MULT_EXPR;
	    if (op == Token::DAND) return TRUTH_ANDIF_EXPR;
	    if (op == Token::DPIPE) return TRUTH_ORIF_EXPR;
	    if (op == Token::INF) return LT_EXPR;
	    if (op == Token::SUP) return GT_EXPR;
	    if (op == Token::INF_EQUAL) return LE_EXPR;
	    if (op == Token::SUP_EQUAL) return GE_EXPR;
	    if (op == Token::NOT_EQUAL) return NE_EXPR;
	    if (op == Token::DEQUAL) return EQ_EXPR;
	    if (op == Token::PLUS) return PLUS_EXPR;
	    if (op == Token::MINUS) return MINUS_EXPR;
	    if (op == Token::DIV) return RDIV_EXPR;
	    if (op == Token::STAR) return MULT_EXPR;
	    return NOP_EXPR;
	}
	
	tree_code toGeneric (std::string& op) {
	    if (op == Token::EQUAL) return MODIFY_EXPR;
	    if (op == Token::DIV_AFF) return TRUNC_DIV_EXPR;
	    if (op == Token::AND_AFF) return BIT_AND_EXPR;
	    if (op == Token::PIPE_EQUAL) return BIT_IOR_EXPR;
	    if (op == Token::MINUS_AFF) return MINUS_EXPR;
	    if (op == Token::PLUS_AFF) return PLUS_EXPR;
	    if (op == Token::LEFTD_AFF) return LSHIFT_EXPR;
	    if (op == Token::RIGHTD_AFF) return RSHIFT_EXPR;
	    if (op == Token::STAR_EQUAL) return MULT_EXPR;
	    if (op == Token::PERCENT_EQUAL) return TRUNC_MOD_EXPR;
	    if (op == Token::XOR_EQUAL) return BIT_XOR_EXPR;
	    if (op == Token::DAND) return TRUTH_ANDIF_EXPR;
	    if (op == Token::DPIPE) return TRUTH_ORIF_EXPR;
	    if (op == Token::INF) return LT_EXPR;
	    if (op == Token::SUP) return GT_EXPR;
	    if (op == Token::INF_EQUAL) return LE_EXPR;
	    if (op == Token::SUP_EQUAL) return GE_EXPR;
	    if (op == Token::NOT_EQUAL) return NE_EXPR;
	    if (op == Token::DEQUAL) return EQ_EXPR;
	    if (op == Token::PLUS) return PLUS_EXPR;
	    if (op == Token::MINUS) return MINUS_EXPR;
	    if (op == Token::DIV) return TRUNC_DIV_EXPR;
	    if (op == Token::STAR) return MULT_EXPR;
	    if (op == Token::PIPE) return BIT_IOR_EXPR;
	    if (op == Token::AND) return BIT_AND_EXPR;
	    if (op == Token::LEFTD) return LSHIFT_EXPR;
	    if (op == Token::XOR) return BIT_XOR_EXPR;
	    if (op == Token::RIGHTD) return RSHIFT_EXPR;
	    if (op == Token::PERCENT) return TRUNC_MOD_EXPR;
	    return NOP_EXPR;
	}
	
	tree_code toGenericReal (std::string& op) {
	    if (op == Token::EQUAL) return MODIFY_EXPR;
	    if (op == Token::DIV_AFF) return RDIV_EXPR;
	    if (op == Token::MINUS_AFF) return MINUS_EXPR;
	    if (op == Token::PLUS_AFF) return PLUS_EXPR;
	    if (op == Token::STAR_EQUAL) return MULT_EXPR;
	    if (op == Token::DAND) return TRUTH_ANDIF_EXPR;
	    if (op == Token::DPIPE) return TRUTH_ORIF_EXPR;
	    if (op == Token::INF) return LT_EXPR;
	    if (op == Token::SUP) return GT_EXPR;
	    if (op == Token::INF_EQUAL) return LE_EXPR;
	    if (op == Token::SUP_EQUAL) return GE_EXPR;
	    if (op == Token::NOT_EQUAL) return NE_EXPR;
	    if (op == Token::DEQUAL) return EQ_EXPR;
	    if (op == Token::PLUS) return PLUS_EXPR;
	    if (op == Token::MINUS) return MINUS_EXPR;
	    if (op == Token::DIV) return RDIV_EXPR;
	    if (op == Token::STAR) return MULT_EXPR;
	    return NOP_EXPR;
	}

    }
}
