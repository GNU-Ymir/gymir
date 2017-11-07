#include "Binary.hh"
#include "Error.hh"
#include "Symbol.hh"
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
	if (aux-> left == NULL || aux-> right == NULL) return NULL;
	
	if (aux-> left-> type == AstEnums::TYPE) return varUndef (aux-> left);
	if (aux-> right-> type == AstEnums::TYPE) return varUndef (aux-> right);

	if (aux-> right-> getType ()-> type-> Is (Semantic::UNDEF)) return varUndef (aux-> right);
	else if (aux-> left-> getType ()-> type-> Is (Semantic::UNDEF)) return varUndef (aux-> left);
	else {
	    auto type = aux-> left-> getType ()-> type-> binaryOp(aux-> token, aux-> right);
	    if (type == NULL) {
		type = aux-> right-> getType ()-> type-> binaryOpRight (aux-> token, aux-> left);
		if (type == NULL) return opUndef (aux);
		else aux-> isRight = true;
	    }
	    aux-> sym = Semantic::SymbolPtr (new Semantic::Symbol(aux-> token, type));
	}
	return aux;	
    }

    ExpressionPtr Binary::affect () {
	auto aux = new Binary (this->token, this->left, this->right);
	aux-> left = aux-> left-> expression ();
	aux-> right = aux-> right-> expression ();
	if (aux-> left == NULL || aux-> right == NULL) return NULL;

	if (aux-> left-> type == AstEnums::TYPE) return varUndef (aux-> left);
	if (aux-> right-> sym-> type-> Is(Semantic::UNDEF)) return varUndef (aux-> right);
	else if (aux-> right-> type == AstEnums::TYPE
		 && aux-> left-> sym-> type-> Is (Semantic::UNDEF)) {
	    this-> declaration = true;
	    auto type = aux-> left-> sym-> type-> typeOp (aux-> right);
	    if (type == NULL) {
		return opUndef (aux);
	    } else aux-> sym = Semantic::SymbolPtr (new Semantic::Symbol (aux-> token, type));	 
   		
	} else if (aux-> left-> sym-> type-> Is (Semantic::UNDEF)) {
	    this-> declaration = true;
	    aux-> left-> sym-> type = (aux-> right-> sym-> type-> clone ());
	}

	auto type = aux-> left-> sym-> type-> binaryOp (this-> token, aux-> right);
	if (type == NULL) {
	    return opUndef (aux);
	} else aux-> sym = Semantic::SymbolPtr (new Semantic::Symbol (aux-> token, type));
	return aux;
    }

    
    Ymir::Tree Binary::statement () {
	if (this-> isInstruction ()) {
	    if (this-> sym-> isVoid ()) {
		Ymir::Error::fatal (this-> token-> getLocus (),
				    "Erreur interne");
		return Ymir::Tree ();
	    } else {
		return this-> sym-> type-> buildBinaryOp (this-> left, this-> right);
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
			     "Type de la variable non definis '%s'",
			     exp-> token-> getCstr ());
	return NULL;
    }
    
    ExpressionPtr Binary::opUndef (Binary* elem) {
	Ymir::Error::append (elem-> token-> getLocus (),
			     "Operateur '%s', non definis entre les types '%s' et '%s'",
			     elem-> token-> getCstr (),
			     elem-> left-> getType ()-> type-> typeToString ().c_str (),
			     elem-> right-> getType ()-> type-> typeToString ().c_str ());

	return NULL;			     
    }
    
    Ymir::Tree Binary::treeExpr () {
	return Ymir::Tree ();
    }
       
    
};
