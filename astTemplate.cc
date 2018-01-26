#include <ymir/ast/_.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/syntax/Token.hh>

using namespace std;

namespace syntax {

    Expression IAccess::templateExpReplace (const map <string, Expression>& values) {
	auto params = (ParamList) this-> params-> templateExpReplace (values);
	auto left = this-> left-> templateExpReplace (values);
	return new (Z0)  IAccess (this-> token, this-> end, left, params);
    }

    Expression IArrayAlloc::templateExpReplace (const map <string, Expression>& values) {
	auto type = this-> type-> templateExpReplace (values);
	auto size = this-> size-> templateExpReplace (values);
	return new (Z0)  IArrayAlloc (this-> token, type, size);
    }

    Instruction IAssert::templateReplace (const map <string, Expression>& values) {
	if (this-> msg != NULL) {
	    return new (Z0)  IAssert (this-> token,
				     this-> expr-> templateExpReplace (values),
				     this-> msg-> templateExpReplace (values),
				     this-> isStatic
	    );
	} else {
	    return new (Z0)  IAssert (this-> token,
				     this-> expr-> templateExpReplace (values),
				     NULL,
				     this-> isStatic
	    );
	}
    }

    Expression IBinary::templateExpReplace (const map <string, Expression>& values) {
	auto left = this-> left-> templateExpReplace (values);
	auto right = this-> right-> templateExpReplace (values);
	return new (Z0)  IBinary (this-> token, left, right);
    }

    Instruction IBlock::templateReplace (const map <string, Expression>& values) {
	vector <Declaration> decls;
	vector <Instruction> insts;
	for (auto it : this-> decls) 
	    decls.push_back (it-> templateDeclReplace (values));
	
	for (auto it : this-> insts)
	    insts.push_back (it-> templateReplace (values)); 
	
	return new (Z0)  IBlock (this-> token, decls, insts);
    }

    Instruction IBreak::templateReplace (const map <string, Expression>&) {
	return new (Z0)  IBreak (this-> token, this-> ident);
    }

    Expression ICast::templateExpReplace (const map <string, Expression>& values) {
	auto type = this-> type-> templateExpReplace (values);
	auto expr = this-> expr-> templateExpReplace (values);
	return new (Z0)  ICast (this-> token, type, expr);
    }

    Expression IFixed::templateExpReplace (const map <string, Expression>&) {
	auto ret = new (Z0)  IFixed (this-> token, this-> type);
	ret-> setUValue (this-> uvalue);
	ret-> setValue (this-> value);
	return ret;
    }

    Expression IChar::templateExpReplace (const map <string, Expression>&) {
	return new (Z0)  IChar (this-> token, this-> code);
    }

    Expression IFloat::templateExpReplace (const map <string, Expression>&) {
	auto ret = new (Z0)  IFloat (this-> token, this-> suite);
	ret-> _type = this-> _type;
	ret-> totale = this-> totale;
	return ret;
    }

    Expression IString::templateExpReplace (const map <string, Expression>&) {
	return new (Z0)  IString (this-> token, this-> content);	
    }

    Expression IBool::templateExpReplace (const map <string, Expression>&) {
	return new (Z0)  IBool (this-> token);
    }

    Expression INull::templateExpReplace (const map <string, Expression>&) {
	return new (Z0)  INull (this-> token);
    }

    Expression IIgnore::templateExpReplace (const map <string, Expression>&) {
	return new (Z0)  IIgnore (this-> token);
    }

    Expression IConstArray::templateExpReplace (const map <string, Expression>& values) {
	vector <Expression> params;
	for (auto it : this-> params)
	    params.push_back (it-> templateExpReplace (values));
	return new (Z0)  IConstArray (this-> token, params);
    }
    
    Expression IConstRange::templateExpReplace (const map <string, Expression>& values) {
	auto left = this-> left-> templateExpReplace (values);
	auto right = this-> right-> templateExpReplace (values);
	return new (Z0)  IConstRange (this-> token, left, right);
    }

    Expression IDColon::templateExpReplace (const map <string, Expression>& values) {
	auto left = this-> left-> templateExpReplace (values);
	auto right = this-> right-> templateExpReplace (values);
	return new (Z0)  IDColon (this-> token, left, right);
    }

    Declaration IDeclaration::templateDeclReplace (const map <string, Expression>&)  {
	Ymir::Error::assert ("TODO");
	return NULL;
    }
    
    Expression IDot::templateExpReplace (const map <string, Expression>& values) {
	auto left = this-> left-> templateExpReplace (values);
	auto right = this-> right-> templateExpReplace (values);
	return new (Z0)  IDot (this-> token, left, right);
    }

    Expression IExpand::templateExpReplace (const map <string, Expression>& values) {
	auto expr = this-> expr-> templateExpReplace (values);
	return new (Z0)  IExpand (this-> token, expr);
    }

    Instruction IFor::templateReplace (const map <string, Expression>& values) {
	vector <Var> vars;
	for (auto it : this-> var)
	    vars.push_back ((Var) it-> templateExpReplace (values));
	
	auto iter = this-> iter-> templateExpReplace (values);
	auto block = (Block) this-> block-> templateReplace (values);
	return new (Z0)  IFor (this-> token, this-> id, vars, iter, block);
    }

    Expression IFuncPtr::templateExpReplace (const map <string, Expression>& values) {
	vector <Var> params;
	for (auto it : this-> params) 
	    params.push_back ((Var) it-> templateExpReplace (values));

	auto ret = this-> ret-> templateExpReplace (values);
	if (this-> expr) {
	    auto expr = this-> expr-> templateExpReplace (values);
	    return new (Z0)  IFuncPtr (this-> token, params, (Var) ret, expr);
	} else
	    return new (Z0)  IFuncPtr (this-> token, params, (Var) ret);
    }

    Declaration IFunction::templateDeclReplace (const map <string, Expression>& tmps) {
	return this-> templateReplace (tmps);
    }
	
    Function IFunction::templateReplace (const map <string, Expression>& values) {
	Var type = NULL;
	if (this-> type)
	    type = (Var) this-> type-> templateExpReplace (values);

	vector <Var> params;
	for (auto it : this-> params)
	    params.push_back ((Var) it-> templateExpReplace (values));

	Expression test = NULL;
	if (this-> test)
	    test = this-> test-> templateExpReplace (values);

	vector <Expression> tmps;
	for (auto it : this-> tmps) 
	    tmps.push_back (it-> templateExpReplace (values));

	auto block = (Block) this-> block-> templateReplace (values);
	return new (Z0)  IFunction (this-> ident, type, params, tmps, test, block);	
    }

    Instruction IIf::templateReplace (const map <string, Expression>& values) {
	Expression test = NULL;
	if (this-> test)
	    test = this-> test-> templateExpReplace (values);

	auto block = (Block) this-> block-> templateReplace (values);
	If _else = NULL;
	if (this-> else_)
	    _else = (If) this-> else_-> templateReplace (values);
	
	return new (Z0)  IIf (this-> token, test, block, _else, this-> isStatic);
    }

    Expression IIs::templateExpReplace (const map <string, Expression>& values) {
	auto left = this-> left-> templateExpReplace (values);
	if (this-> type) {
	    auto right = this-> type-> templateExpReplace (values);
	    return new (Z0)  IIs (this-> token, left, right);
	} else {
	    return new (Z0)  IIs (this-> token, left, this-> expType);
	}	    
    }

    Expression ILambdaFunc::templateExpReplace (const map <string, Expression>& values) {
	vector <Var> var;
	for (auto it : this-> params)
	    var.push_back ((Var) it-> templateExpReplace (values));

	Var ret = NULL;
	if (this-> ret)
	    ret = (Var) this-> ret-> templateExpReplace (values);
	
	if (this-> block) {
	    auto block = (Block) this-> block-> templateReplace (values);
	    return new (Z0)  ILambdaFunc (this-> token, var, ret, block);
	} else if (this-> expr) {
	    auto expr = this-> expr-> templateExpReplace (values);
	    return new (Z0) ILambdaFunc (this-> token, var, expr);
	} else {
	    return new (Z0) ILambdaFunc (this-> token, this-> frame);
	}
    }
    
    Expression IMatch::templateExpReplace (const map <string, Expression>& values) {
	auto expr = this-> expr-> templateExpReplace (values);
	vector <Expression> auxValues;
	vector <Block> auxBlock;
	for (auto it : this-> values)
	    auxValues.push_back (it-> templateExpReplace (values));

	for (auto it : this-> block)
	    auxBlock.push_back ((Block) it-> templateReplace (values));

	Block def = NULL;
	if (this-> default_)
	    def = (Block) this-> default_-> templateReplace (values);
	return new (Z0)  IMatch (this-> token, expr, auxValues, auxBlock, def);
    }

    Expression IMixin::templateExpReplace (const map <string, Expression>& values) {
	auto inside = this-> inside-> templateExpReplace (values);
	return new (Z0)  IMixin (this-> token, inside);
    }

    Expression IOfVar::templateExpReplace (const map <string, Expression>& values) {
	auto type = (Var) this-> type-> templateExpReplace (values);
	return new (Z0)  IOfVar (this-> token, type);
    }

    Expression IParamList::templateExpReplace (const map <string, Expression>& values) {
	vector <Expression> params;
	for (auto it : this-> params)
	    params.push_back (it-> templateExpReplace (values));
	return new (Z0)  IParamList (this-> token, params);
    }

    Expression IPar::templateExpReplace (const map <string, Expression>& values) {
	auto params = (ParamList) this-> params-> templateExpReplace (values);
	auto left = this-> _left-> templateExpReplace (values);
	return new (Z0)  IPar (this-> token, this-> end, left, params);
    }

    Instruction IReturn::templateReplace (const map <string, Expression>& values) {
	if (this-> elem == NULL) return new (Z0)  IReturn (this-> token);
	return new (Z0)  IReturn (this-> token, this-> elem-> templateExpReplace (values));
    }

    Instruction ITupleDest::templateReplace (const map <string, Expression>& values) {
	vector <Var> decls;
	for (auto it : this-> decls)
	    decls.push_back ((Var) it-> templateExpReplace ({}));

	auto right = this-> right-> templateExpReplace (values);
	return new (Z0)  ITupleDest (this-> token, this-> isVariadic, decls, right);
    }
    
    Expression IConstTuple::templateExpReplace (const map <string, Expression>& values) {
	vector <Expression> exprs;
	for (auto it : this-> params)
	    exprs.push_back (it-> templateExpReplace (values));

	return new (Z0)  IConstTuple (this-> token, this-> end, exprs);
    }

    Expression ITypedVar::templateExpReplace (const map <string, Expression>& values) {
	if (this-> type) {
	    auto type = (Var) this-> type-> templateExpReplace (values);
	    return new (Z0)  ITypedVar (this-> token, type, this-> deco);
	} else {
	    auto type = this-> expType-> templateExpReplace (values);
	    return new (Z0)  ITypedVar (this-> token, type, this-> deco);
	}
    }

    Expression IType::templateExpReplace (const map <string, Expression>&) {
	return new (Z0)  IType (this-> token, this-> _type);
    }
    
    Expression ITypeOf::templateExpReplace (const map <string, Expression>& values) {
	auto left = this-> expr-> templateExpReplace (values);
	return new (Z0)  ITypeOf (this-> token, left);
    }

    Expression IUnary::templateExpReplace (const map <string, Expression>& values) {
	auto elem = this-> elem-> templateExpReplace (values);
	return new (Z0)  IUnary (this-> token, elem);
    }

    Instruction IVarDecl::templateReplace (const map <string, Expression>& values) {
	vector <Expression> insts;
	vector <Var> decls;
	vector <Word> decos;
	for (auto it : this-> decls)
	    decls.push_back ((Var) it-> templateExpReplace ({}));

	for (auto it : this-> decos)
	    decos.push_back (it);

	for (auto it : this-> insts)
	    insts.push_back (it-> templateExpReplace (values));
	
	return new (Z0)  IVarDecl (this-> token, decos, decls, insts);
    }

    Expression IVar::templateExpReplace (const map <string, Expression>& values) {
	Var ret = NULL;
	for (auto it : values) {
	    if (it.first == this-> token.getStr ()) {
		auto clo = it.second-> templateExpReplace ({});
		clo-> token.setLocus (this-> token.getLocus ());
		if (auto v = clo-> to <IVar> ()) {
		    v-> deco = this-> deco;
		    ret = v;
		    break;		
		} else if (this-> templates.size () == 0) {
		    return clo;
		} else {
		    break;
		}
	    }
	}

	vector <Expression> tmps;
	for (auto it : this-> templates) 
	    tmps.push_back (it-> templateExpReplace (values));

	if (ret == NULL)
	    ret = new (Z0)  IVar (this-> token, tmps);
	else ret-> templates = tmps;
	
	ret-> deco = this-> deco;
	return ret;	
    }
    
    Expression IArrayVar::templateExpReplace (const map <string, Expression>& values) {
	auto cont = this-> content-> templateExpReplace (values);
	return new (Z0)  IArrayVar (this-> token, cont);
    }

    Instruction IWhile::templateReplace (const map <string, Expression>& values) {
	auto test = this-> test-> templateExpReplace (values);
	auto block = (Block) this-> block-> templateReplace (values);
	return new (Z0)  IWhile (this-> token, test, block);
    }

}
