#include "Binary.hh"
#include "Error.hh"
#include <stdio.h>

namespace Syntax {

    void Binary::print (int nb) {
	printf("%*c<Binary> ", nb, ' ');
	token -> print ();
	printf ("\n");
	if (left != NULL)
	    left->print (nb + 4);
	if (right != NULL)
	    right->print (nb + 4);
    }

    ExpressionPtr Binary::expression () {
	if (token->getId() == Lexical::AFFECT)
	    return affect ();
	else return normal ();
    }
    
    ExpressionPtr Binary::normal () {
	auto aux = new Binary (this->token, this->left, this->right);
	aux-> left = aux-> left-> expression ();
	aux-> right = aux-> right-> expression ();
	
	if (aux-> left-> type == AstEnums::TYPE) return varUndef (aux-> left);
	if (aux-> right-> type == AstEnums::TYPE) return varUndef (aux-> right);
	
	if (aux-> right-> getType ()-> Is (Semantic::UNDEF)) return varUndef (aux-> right);
	else if (aux-> left-> getType ()-> Is (Semantic::UNDEF)) return varUndef (aux-> left);
	else {
	    auto type = aux-> left-> getType ()-> binaryOp(aux-> token, aux-> right);
	    if (type == NULL) {
		type = aux-> right-> getType ()-> binaryOpRight (aux-> token, aux-> left);
		if (type == NULL) return opUndef (aux);
		else aux-> isRight = true;
	    }
	    aux-> info = type;
	}
	return aux;	
    }

    ExpressionPtr Binary::affect () {
	auto aux = new Binary (this->token, this->left, this->right);
	aux-> left = aux-> left-> expression ();
	aux-> right = aux-> right-> expression ();

	if (aux-> left-> type == AstEnums::TYPE) return varUndef (aux-> left);
	if (aux-> right-> info-> Is(Semantic::UNDEF)) return varUndef (aux-> right);
	else if (aux-> right-> type == AstEnums::TYPE
		 && aux-> left-> info-> Is (Semantic::UNDEF)) {
	    this-> declaration = true;
	    auto type = aux-> left-> info-> typeOp (aux-> right);
	    if (type == NULL) {
		return opUndef (aux);
	    } else aux-> info = type;	    		
	} else if (aux-> left-> info-> Is (Semantic::UNDEF)) {
	    this-> declaration = true;
	    aux-> left-> info = aux-> right-> info-> clone ();
	}

	auto type = aux-> left-> info-> binaryOp (this-> token, aux-> right);
	if (type == NULL) {
	    return opUndef (aux);
	} else aux-> info = type;
	return aux;
    }

    
    Ymir::Tree Binary::statement () {
	if (this-> isInstruction ()) {
	    if (this-> info == NULL) {
		Ymir::Error::fatal (this-> token-> getLocus (),
				    "Erreur interne");
		return Ymir::Tree ();
	    } else {
		return this-> info-> buildBinaryOp (this-> left, this-> right);
	    }
	} else {
	    Expression::statement ();
	    return Ymir::Tree ();
	}
    }

    bool Binary::isInstruction () {
	return (this-> token-> getId () == Lexical::AFFECT ||
		this-> token-> getId () == Lexical::PLUS_AFF ||
		this-> token-> getId () == Lexical::MUL_AFF ||
		this-> token-> getId () == Lexical::SUB_AFF ||
		this-> token-> getId () == Lexical::DIV_AFF);
    }
        
    ExpressionPtr Binary::varUndef (ExpressionPtr exp) {
	Ymir::Error::append (exp-> token-> getLocus (),
			     "Variable non definis '%s'",
			     exp-> token-> getCstr ());
	return NULL;
    }
    
    ExpressionPtr Binary::opUndef (Binary* elem) {
	Ymir::Error::append (elem-> token-> getLocus (),
			     "Operateur '%s', non definis entre les types '%s' et '%s'",
			     elem-> token-> getCstr (),
			     elem-> left-> getType ()-> typeToString ().c_str (),
			     elem-> right-> getType ()-> typeToString ().c_str ());

	return NULL;			     
    }
    

    
};
