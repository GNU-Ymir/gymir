#include <ymir/ast/Function.hh>
#include <ymir/ast/TypedVar.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/pack/FrameTable.hh>
#include <ymir/semantic/pack/PureFrame.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/types/FunctionInfo.hh>

namespace syntax {

    using namespace semantic;
    
    IFunction::IFunction (Word ident, std::vector <Var> params, std::vector <Expression> tmps, Expression test, Block block) :
	ident (ident),
	type (NULL),
	params (params),
	tmps (tmps),
	block (block),
	test (test)	    
    {}
    
	
    IFunction::IFunction (Word ident, Var type, std::vector <Var> params, std::vector <Expression> tmps, Expression test, Block block) :
	ident (ident),
	type (type),
	params (params),
	tmps (tmps),
	block (block),
	test (test)	    
    {}

    Word IFunction::getIdent () {
	return this-> ident;
    }

    std::vector <Var> IFunction::getParams () {
	return this-> params;
    }

    Var IFunction::getType () {
	return this-> type;
    }

    std::string IFunction::name () {
	return this-> ident.getStr ();
    }
    
    Block IFunction::getBlock () {
	return this-> block;
    }
    
    void IFunction::declare ()  {
	if (this-> ident == Keys::MAIN) {
	    FrameTable::instance ().insert (new IPureFrame (Table::instance ().space (), this));							    
	} else {
	    auto fr = verifyPure (Table::instance ().space ());
	    auto space = Table::instance ().space ();
	    auto it = Table::instance ().getLocal (this-> ident.getStr ());
	    if (it != NULL) {
		if (!it-> type-> is<IFunctionInfo> ()) {
		    Ymir::Error::append (ident,
					 Ymir::ShadowingVar
		    );
		    Ymir::Error::note (it-> sym,
				       Ymir::Here
		    );
		}		
	    }
	    auto fun = new IFunctionInfo (space, this-> ident.getStr ());
	    fun-> set (fr);
	    Table::instance ().insert (new ISymbol (this-> ident, fun));
	}
    }

    Frame IFunction::verifyPure (Namespace space) {
	if (this-> tmps.size () != 0) {
	    Ymir::Error::assert ("TODO");
	    return NULL;//
	    // auto isPure = verifyTemplates ();
	    // auto ret = new TemplateFrame (space, this);
	    // if (!isPure) return ret;
	    // for (auto it : this-> params) {
	    // 	if (!it-> is<ITypedVar> ()) return ret;
	    // }

	    // ret-> isPure = true;
	    // FrameTable::instance ().insert (ret);
	    // return ret;
	}

	for (auto it : this-> params) {
	    if (!it-> is<ITypedVar> ()) {
		Ymir::Error::assert ("TODO");
		return NULL;//new IUnPureFrame (space, this);
	    }
	}
	
	auto fr = new IPureFrame (space, this);
	FrameTable::instance ().insert (fr);
	return fr;
    }
    
    void IFunction::print (int nb) {
	printf ("\n%*c<Function> %s ",
		nb, ' ',
		this-> ident.toString ().c_str ()
	);
	if (this-> type) {
	    printf ("\n%*c ->", nb, ' ');
	    this-> type-> print (nb + 4);
	}

	if (this-> test) {
	    printf ("\n%*c if (", nb + 2, ' ');
	    this-> test-> print (nb + 4);
	    printf ("\n%*c )", nb + 2, ' ');
	}
	    
	printf ("\n%*c !( ", nb + 2, ' ');
	for (auto it : this-> tmps) {
	    it-> print (nb + 4);
	}
	printf ("\n%*c )", nb + 2, ' ');

	printf ("\n%*c (", nb + 2, ' ');
	for (auto it : this-> params) {
	    it-> print (nb + 4);
	}
	printf ("\n%*c )", nb + 2, ' ');

	this-> block-> print (nb + 6);	    	    
    }



    
}
