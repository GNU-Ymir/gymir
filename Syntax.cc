#include "Syntax.hh"

namespace Syntax {

    Ast::Ast (Lexical::TokenPtr token) : token (token) {
	AstGC::instance().addInfo (this);
    }
    
    void AstGC::addInfo (Ast * elem) {
	this->table.push_back (elem);
    }

    void AstGC::empty() {
	for (auto & it : this->table) {
	    delete it;
	}
	this->table.clear();
    }
    
    AstGC * AstGC::inst = NULL;


    void Instruction::print(int nb) {
	printf("%*c<Instruction> ", nb, ' ');
	token->print();
	printf ("\n");
    }

    void Expression::print (int nb) {
	printf("%*c<Expression> ", nb, ' ');
	token -> print ();
	printf ("\n");
    }

    void Declaration::print (int nb) {
	printf("%*c<Declaration> ", nb, ' ');
	token -> print ();
	printf ("\n");
    }

    void Var::print (int nb) {
	printf("%*c<Var> ", nb, ' ');
	token -> print ();
	if (templates.size () > 0) {
	    printf ("!(");
	    for (int i = 0; i < (int)templates.size (); i++) {
		if (templates[i] != NULL)
		    templates[i]->print (-1);
		if (i < (int)templates.size () - 1) printf(", ");
	    }
	    printf (")");
	}
	if (nb != -1)
	    printf ("\n");
    }

    void TypedVar::print (int nb) {
	printf("%*c<TypedVar> ", nb, ' ');
	token -> print ();
	if (type != NULL) type->print ();
    }

    void Binary::print (int nb) {
	printf("%*c<Binary> ", nb, ' ');
	token -> print ();
	printf ("\n");
	if (left != NULL)
	    left->print (nb + 4);
	if (right != NULL)
	    right->print (nb + 4);
    }

    void AfUnary::print (int nb) {
	printf("%*c<AfUnary> ", nb, ' ');
	token -> print ();
	printf ("\n");
	if (elem != NULL)
	    elem->print (nb + 4);
    }

    void BefUnary::print (int nb) {
	printf("%*c<BefUnary> ", nb, ' ');
	token -> print ();
	printf ("\n");
	if (elem != NULL)
	    elem->print (nb + 4);
    }

    void Cast::print (int nb) {
	printf("%*c<Cast> ", nb, ' ');
	token -> print ();
	printf ("\n");
	if (type != NULL)
	    type->print (nb + 4);
	if (inside != NULL)
	    inside->print (nb + 4);
    }

    void New::print (int nb) {
	printf("%*c<New> ", nb, ' ');
	token -> print ();
	printf ("\n");
	if (type != NULL)
	    type->print (nb + 4);
	if (size != NULL)
	    size->print (nb + 4);
    }

    void InsideIf::print (int nb) {
	printf("%*c<InsideIf> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (test != NULL)
	    test->print (nb + 4);
	if (if_bl != NULL)
	    if_bl -> print(nb + 4);
	if (else_bl != NULL)
	    else_bl->print (nb + 4);
    }

    void InsideFor::print (int nb) {
	printf("%*c<InsideFor> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (var != NULL)
	    var -> print (nb + 4);
	if (iter != NULL)
	    iter -> print (nb + 4);
	if (value != NULL)
	    value -> print (nb + 4);
    }

    void Int::print (int nb) {
	printf("%*c<Int> ", nb, ' ');
	token -> print ();
	printf ("\n");	
    }

    void Float::print (int nb) {
	printf("%*c<Float> ", nb, ' ');
	token -> print ();	
	if (suite == "") {
	    printf("0.%s", token->getCstr ());
	    printf ("\n");
	} else {
	    printf ("%s.%s", token->getCstr (), suite.c_str());
	    printf ("\n");
	}
    }

    void Bool::print (int nb) {
	printf("%*c<Bool> ", nb, ' ');
	token -> print ();
	printf ("\n");	
    }

    void Null::print (int nb) {
	printf("%*c<Null> ", nb, ' ');
	token -> print ();
	printf ("\n");	
    }

    void Char::print (int nb) {
	printf("%*c<Char> %i", nb, ' ', value);
	token -> print ();
	printf ("\n");	
    }
    
    void String::print (int nb) {
	printf("%*c<String> %s", nb, ' ', content.c_str());
	token -> print ();
	printf ("\n");	
    }

    void ParamList::print (int nb) {
	printf("%*c<ParamList> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	for (int i = 0; i < (int)params.size (); i++) {
	    if (params[i] != NULL)
		params[i]->print(nb + 4);
	}
    }

    void Par::print (int nb) {
	printf("%*c<Par> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (left != NULL)
	    left -> print (nb + 4);
	if (right != NULL)
	    right -> print (nb + 4);
    }

    void Access::print (int nb) {
	printf("%*c<Access> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (left != NULL)
	    left -> print (nb + 4);
	if (right != NULL)
	    right -> print (nb + 4);
    }

    void Dot::print (int nb) {
	printf("%*c<Dot> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (left != NULL)
	    left -> print (nb + 4);
	if (right != NULL)
	    right -> print (nb + 4);
    }

    void Block::print (int nb) {
	printf("%*c<Block> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	for (auto & it : decls) {
	    if (it != NULL)
		it -> print (nb + 4);
	}
	for (auto & it : instructions) {
	    if (it != NULL)
		it -> print (nb + 4);
	}	
    }

    void If::print (int nb) {
	printf("%*c<If> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (test != NULL)
	    test->print (nb + 4);
	if (block != NULL)
	    block -> print (nb + 4);
	if (else_bl != NULL)
	    else_bl -> print (nb + 4);
    }

    void ElseIf::print (int nb) {
	printf("%*c<Else> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (if_bl != NULL)
	    if_bl -> print (nb + 4);
    }

    void Else::print (int nb) {
	printf("%*c<Else> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (block != NULL)
	    block -> print (nb + 4);
    }

    void Return::print (int nb) {
	printf("%*c<Return> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (ret != NULL)
	    ret -> print (nb + 4);
    }

    void For::print (int nb) {
	printf("%*c<For> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	for (auto & it : inits) {
	    if (it != NULL)
		it -> print (nb + 4);
	}
	if (test != NULL)
	    test -> print (nb + 4);
	for (auto & it : iter)
	    if (it != NULL) it -> print (nb + 4);
	if (block != NULL)
	    block -> print(nb + 4);
    }

    void Foreach::print (int nb) {
	printf("%*c<Foreach> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if(var != NULL) var -> print (nb + 4);
	if (iter != NULL) iter->print (nb + 4);
	if (block != NULL) block -> print (nb + 4);
    }

    void While::print (int nb) {
	printf("%*c<While> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (test != NULL)
	    test -> print (nb + 4);
	if (block != NULL)
	    block -> print (nb + 4);
    }

    void Break::print (int nb) {
	printf("%*c<Break> ", nb, ' ');
	token -> print ();
	printf ("\n");	
    }

    void Delete::print (int nb) {
	printf("%*c<Delete> ", nb, ' ');
	token -> print ();
	printf ("\n");	
	if (elem != NULL)
	    elem -> print (nb + 4);
    }

    void Function::print (int nb) {
	printf ("%*c", nb, ' ');
	printf("<Function> ");
	token -> print ();	
	if (type != NULL)
	    type -> print (-1);
	printf ("\n");
	for (auto it : params)
	    if (it != NULL) it -> print (nb + 4);
	if (block != NULL)
	    block -> print (nb + 4);
    }
    

    void Program::print (int nb) {
	printf("%*c<Program> ", nb, ' ');
	token -> print ();
	printf ("\n");
	for (auto & it : elems) {
	    if (it != NULL)
		it -> print (nb + 4);
	}
    }
    
};
