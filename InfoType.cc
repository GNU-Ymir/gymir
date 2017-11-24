#include "semantic/types/InfoType.hh"
#include "syntax/Word.hh"
#include "ast/Expression.hh"
#include "ast/Var.hh"
#include "errors/Error.hh"

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

    InfoType IInfoType::factory (Word word, std::vector <syntax::Expression> templates) {
	auto it = (InfoType (*)(Word, std::vector<syntax::Expression>)) (
	    Creators::instance ().find (word.getStr ())
	);
	if (it != NULL) {
	    return (*it) (word, templates);
	}
	auto _it_ = __alias__.find (word.getStr ());
	if (_it_ != __alias__.end ()) return _it_-> second-> clone ();
	Ymir::Error::append (word,
			     "Type inconnu '%s%s%s'",
			     Ymir::Error::YELLOW,
			     word.getStr ().c_str (),
			     Ymir::Error::RESET);
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
	return NULL;
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

    std::vector <InfoType> IInfoType::getTemplate (ulong bef, ulong) {
	return {this-> getTemplate (bef)};
    }
}
