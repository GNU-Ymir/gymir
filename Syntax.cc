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
	printf("%*c<Instruction> ", nb, '\t');
	token->print();
	printf ("\n");
    }

    void Expression::print (int nb) {
	printf("%*c<Expression> ", nb, '\t');
	token -> print ();
	printf ("\n");
    }

    void Declaration::print (int nb) {
	printf("%*c<Declaration> ", nb, '\t');
	token -> print ();
	printf ("\n");
    }

    void Var::print (int nb) {
	printf("%*c<Var> ", nb, '\t');
	token -> print ();
	if (templates.size () > 0) {
	    printf ("!(");
	    for (int i = 0; i < (int)templates.size (); i++) {
		if (templates[i] != NULL)
		    templates[i]->print ();
		if (i < (int)templates.size () - 1) printf(", ");
	    }
	    printf (")");
	}
	printf ("\n");
    }

    void TypedVar::print (int nb) {
	printf("%*c<TypedVar> ", nb, '\t');
	token -> print ();
	if (type != NULL) type->print ();
    }

    void Binary::print (int nb) {
	printf("%*c<Binary> ", nb, '\t');
	token -> print ();
	printf ("\n");
	if (left != NULL)
	    left->print (nb + 1);
	if (right != NULL)
	    right->print (nb + 1);
    }

    void AfUnary::print (int nb) {
	printf("%*c<AfUnary> ", nb, '\t');
	token -> print ();
	printf ("\n");
	if (elem != NULL)
	    elem->print (nb + 1);
    }

    void BefUnary::print (int nb) {
	printf("%*c<BefUnary> ", nb, '\t');
	token -> print ();
	printf ("\n");
	if (elem != NULL)
	    elem->print (nb + 1);
    }

    void Cast::print (int nb) {
	printf("%*c<Cast> ", nb, '\t');
	token -> print ();
	printf ("\n");
	if (type != NULL)
	    type->print (nb + 1);
	if (inside != NULL)
	    inside->print (nb + 1);
    }

    void New::print (int nb) {
	printf("%*c<New> ", nb, '\t');
	token -> print ();
	printf ("\n");
	if (type != NULL)
	    type->print (nb + 1);
	if (size != NULL)
	    size->print (nb + 1);
    }

    void InsideIf::print (int nb) {
	printf("%*c<InsideIf> ", nb, '\t');
	token -> print ();
	printf ("\n");	
	if (test != NULL)
	    test->print (nb + 1);
	if (if_bl != NULL)
	    if_bl -> print(nb + 1);
	if (else_bl != NULL)
	    else_bl->print (nb + 1);
    }

    void InsideFor::print (int nb) {
	printf("%*c<InsideFor> ", nb, '\t');
	token -> print ();
	printf ("\n");	
	if (var != NULL)
	    var -> print (nb + 1);
	if (iter != NULL)
	    iter -> print (nb + 1);
	if (value != NULL)
	    value -> print (nb + 1);
    }

    void Int::print (int nb) {
	printf("%*c<Int> ", nb, '\t');
	token -> print ();
	printf ("\n");	
    }

    void Float::print (int nb) {
	printf("%*c<Float> ", nb, '\t');
	token -> print ();
	printf ("\n");	
	if (suite == "") {
	    printf("0.%s", token->getCstr ());
	    token -> print();
	    printf ("\n");
	} else {
	    printf ("%s.%s", token->getCstr (), suite.c_str());
	    token -> print ();
	    printf ("\n");
	}
    }

    void Bool::print (int nb) {
	printf("%*c<Bool> ", nb, '\t');
	token -> print ();
	printf ("\n");	
    }

    void Null::print (int nb) {
	printf("%*c<Null> ", nb, '\t');
	token -> print ();
	printf ("\n");	
    }

    void Char::print (int nb) {
	printf("%*c<Char> %i", nb, '\t', value);
	token -> print ();
	printf ("\n");	
    }
    
    void String::print (int nb) {
	printf("%*c<String> %s", nb, '\t', content.c_str());
	token -> print ();
	printf ("\n");	
    }

    void ParamList::print (int nb) {
	printf("%*c<ParamList> ", nb, '\t');
	token -> print ();
	printf ("\n");	
	for (int i = 0; i < (int)params.size (); i++) {
	    if (params[i] != NULL)
		params[i]->print(nb + 1);
	}
    }

    void Par::print (int nb) {
	printf("%*c<Par> ", nb, '\t');
	token -> print ();
	printf ("\n");	
	if (left != NULL)
	    left -> print (nb + 1);
	if (right != NULL)
	    right -> print (nb + 1);
    }

    void Access::print (int nb) {
	printf("%*c<Access> ", nb, '\t');
	token -> print ();
	printf ("\n");	
	if (left != NULL)
	    left -> print (nb + 1);
	if (right != NULL)
	    right -> print (nb + 1);
    }

    void Dot::print (int nb) {
	printf("%*c<Dot> ", nb, '\t');
	token -> print ();
	printf ("\n");	
	if (left != NULL)
	    left -> print (nb + 1);
	if (right != NULL)
	    right -> print (nb + 1);
    }

    void Block::print (int nb) {
	printf("%*c<Block> ", nb, '\t');
	token -> print ();
	printf ("\n");	
	for (auto & it : decls) {
	    if (it != NULL)
		it -> print (nb + 1);
	}
	for (auto & it : instructions) {
	    if (it != NULL)
		it -> print (nb + 1);
	}	
    }

    void If::print (int nb) {
	printf("%*c<If> ", nb, '\t');
	token -> print ();
	printf ("\n");	
	if (test != NULL)
	    test->print (nb + 1);
	if (block != NULL)
	    block -> print (nb + 1);
	if (else_bl != NULL)
	    else_bl -> print (nb + 1);
    }

    void ElseIf::print (int nb) {
	printf("%*c<Else> ", nb, '\t');
	token -> print ();
	printf ("\n");	
	if (if_bl != NULL)
	    if_bl -> print (nb + 1);
    }

    void Else::print (int nb) {
	printf("%*c<Else> ", nb, '\t');
	token -> print ();
	printf ("\n");	
	if (block != NULL)
	    block -> print (nb + 1);
    }

    void Return::print (int nb) {
	printf("%*c<Return> ", nb, '\t');
	token -> print ();
	printf ("\n");	
	if (ret != NULL)
	    ret -> print (nb + 1);
    }

    void For::print (int nb) {
	printf("%*c<For> ", nb, '\t');
	token -> print ();
	printf ("\n");	
	for (auto & it : inits) {
	    if (it != NULL)
		it -> print (nb + 1);
	}
	if (test != NULL)
	    test -> print (nb + 1);
	for (auto & it : iter)
	    if (it != NULL) it -> print (nb + 1);
	if (block != NULL)
	    block -> print(nb + 1);
    }

    void Foreach::print (int nb) {
	printf("%*c<Foreach> ", nb, '\t');
	token -> print ();
	printf ("\n");	
	if(var != NULL) var -> print (nb + 1);
	if (iter != NULL) iter->print (nb + 1);
	if (block != NULL) block -> print (nb + 1);
    }

    void While::print (int nb) {
	printf("%*c<While> ", nb, '\t');
	token -> print ();
	printf ("\n");	
	if (test != NULL)
	    test -> print (nb + 1);
	if (block != NULL)
	    block -> print (nb + 1);
    }

    void Break::print (int nb) {
	printf("%*c<Break> ", nb, '\t');
	token -> print ();
	printf ("\n");	
    }

    void Delete::print (int nb) {
	printf("%*c<Delete> ", nb, '\t');
	token -> print ();
	printf ("\n");	
	if (elem != NULL)
	    elem -> print (nb + 1);
    }

    void Function::print (int nb) {
	printf("%*c<Function> ", nb, '\t');
	token -> print ();	
	if (type != NULL)
	    type -> print ();
	printf ("\n");
	for (auto it : params)
	    if (it != NULL) it -> print (nb + 1);
	if (block != NULL)
	    block -> print (nb + 1);
    }
    

    void Program::print (int nb) {
	printf("%*c<Program> ", nb, '\t');
	token -> print ();
	printf ("\n");
	for (auto & it : elems) {
	    if (it != NULL)
		it -> print (nb + 1);
	}
    }
    
};
