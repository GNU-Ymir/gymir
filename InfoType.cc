#include <ymir/semantic/tree/Tree.hh>
#include "semantic/types/InfoType.hh"
#include "syntax/Word.hh"
#include "ast/Expression.hh"
#include "ast/Var.hh"
#include "errors/Error.hh"
#include <ymir/semantic/utils/OperatorUtils.hh>

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


    Ymir::Tree IInfoType::buildBinaryOp (Word locus, syntax::Expression left, syntax::Expression right) {
	return this-> binopFoo (locus, left, right);
    }

    Ymir::Tree IInfoType::buildUnaryOp (Word locus, syntax::Expression elem) {
	return this-> unopFoo (locus, elem);
    }

    Ymir::Tree IInfoType::buildMultOp (Word locus, syntax::Expression left, syntax::Expression rights) {
	return this-> multFoo (locus, left, rights);
    }

    
    InfoType IInfoType::factory (Word word, std::vector <syntax::Expression> templates) {
	auto it = (InfoType (*)(Word, std::vector<syntax::Expression>)) (
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
    
    bool& IInfoType::isConst () {
	return this-> _isConst;
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

    std::string IInfoType::typeString () {
	if (this-> isConst ()) {
	    return std::string ("const(") + this-> innerTypeString () + ")";
	} else return this-> innerTypeString ();
    }

    InfoType IInfoType::BinaryOp (Word w, InfoType info) {
	auto expr = new syntax::IExpression (w);
	expr-> info = new ISymbol (w, info);
	return this-> BinaryOp (w, expr);
    }
    
    InfoType IInfoType::BinaryOp (Word, syntax::Expression) {
	return NULL;
    }

    InfoType IInfoType::BinaryOpRight (Word, syntax::Expression) {
	return NULL;
    }

    ApplicationScore IInfoType::CallOp (Word, syntax::ParamList) {
	return NULL;
    }

    ApplicationScore IInfoType::CallOp (Word, std::vector <InfoType>) {
	return NULL;
    }
    
    InfoType IInfoType::ApplyOp (std::vector <::syntax::Var>) {
	return NULL;
    }

    InfoType IInfoType::UnaryOp (Word) {
	return NULL;
    }

    InfoType IInfoType::AccessOp (Word, syntax::ParamList) {
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

    InfoType IInfoType::TempOp (std::vector <syntax::Expression>) {
	return NULL;
    }

    InfoType IInfoType::cloneOnExit () {
	auto ret = this-> clone ();
	//if (ret) ret-> _value = NULL;
	return ret;
    }

    InfoType IInfoType::cloneConst () {
	auto ret = this-> clone ();
	ret-> isConst () = true;
	return ret;
    }

    InfoType IInfoType::getTemplate (ulong) {
	return NULL;
    }

    Ymir::Tree IInfoType::toGeneric () {
	Ymir::Error::assert ((std::string ("TODO") + this-> getId ()).c_str ());
	return NULL;
    }

    Ymir::Tree IInfoType::getInitFnPtr () {
	Ymir::Error::assert ((std::string ("TODO getInitFnPtr") + this-> getId ()).c_str ());
	return NULL;
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
	    return NOP_EXPR;
	}
    
    }
}
