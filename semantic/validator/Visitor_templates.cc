#include <ymir/semantic/validator/_.hh>
#include <ymir/syntax/visitor/Keys.hh>
#include <ymir/semantic/declarator/Visitor.hh>
#include <ymir/semantic/generator/Mangler.hh>
#include <ymir/utils/map.hh>
#include <ymir/global/Core.hh>
#include <ymir/utils/Path.hh>
#include <ymir/global/State.hh>
#include <string>
#include <algorithm>

using namespace global;

namespace semantic {

    namespace validator {

	using namespace generator;
	using namespace Ymir;

	void Visitor::validateTemplateNoSpecialization (const semantic::Symbol & sym) {
	    auto visitor = TemplateVisitor::init (*this);
	    auto mapper = visitor.inferArguments (sym.to <semantic::Template> ().getParams ());
	    if (mapper.succeed) {
		int score = 0; // useless, but it is need inside the following method
		auto sol = visitor.applyMapperOnTemplate (sym.getName (), sym, mapper, score);
		match (sol) {
		    s_of_u (TemplatePreSolution) {
			this-> validateTemplatePreSolution (sol, Generator::empty (), true, true);
		    }
		}
	    }
	}

	void Visitor::validateTemplateSymbol (const semantic::Symbol & sym, const Generator & gen) {
	    std::list <Ymir::Error::ErrorMsg> errors;	    
	    pushReferent (sym, "validateTemplateSymbol");
	    
	    enterForeign ();
	    try {
		if (gen.is <MethodTemplateRef> () && sym.is <TemplateSolution> ()) {
		    this-> validateTemplateSolutionMethod (sym, gen.to <MethodTemplateRef> ().getSelf ());
		} else {
		    this-> validate (sym, false);
		}
	    } catch (Error::ErrorList &list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    }
	    exitForeign ();
	    popReferent ("validateTemplateSymbol");
	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }
	}

	semantic::Symbol Visitor::validateTemplatePreSolution (const semantic::Symbol & sol, const Generator & gen, bool validate, bool inModule) {
	    auto tmplVisitor = TemplateVisitor::init (*this);
	    auto preSol = sol.to <semantic::TemplatePreSolution> ();	    
	    auto final_syntax = tmplVisitor.replaceAll (preSol.getDeclaration (), preSol.getMapping (), preSol.getTemplateReferent ());
		    
	    auto visit = declarator::Visitor::init ();
	    visit.setWeak ();
	    visit.pushReferent (preSol.getTemplateReferent ());
		    
	    auto soluce = TemplateSolution::init (sol.getName (), sol.getComments (), preSol.getTemplateParams (), preSol.getMapping (), preSol.getNameOrder (), true);
	    visit.pushReferent (soluce);
	    visit.visit (final_syntax);
	    auto glob = visit.popReferent ();
	    glob.setReferent (visit.getReferent ());
		    
	    auto already = tmplVisitor.getTemplateSolution (visit.getReferent (), soluce);
	    if (already.isEmpty ()) {
		visit.getReferent ().insertTemplate (glob);
	    } else glob = already;

	    if (validate) {
		if (gen.isEmpty ()) {
		    this-> validate (glob, inModule);
		} else {
		    this-> validateTemplateSymbol (glob, gen);
		}
	    }
	    return glob;
	}
	
	
	void Visitor::validateTemplateSolution (const semantic::Symbol & sol, bool inModule) {
	    Visitor::__TEMPLATE_NB_RECURS__ += 1;
	    std::list <Error::ErrorMsg> errors;
	    try {
		if (Visitor::__TEMPLATE_NB_RECURS__ >= VisitConstante::LIMIT_TEMPLATE_RECUR) {
		    Ymir::Error::occur (sol.getName (), ExternalError::get (TEMPLATE_RECURSION), Visitor::__TEMPLATE_NB_RECURS__);
		}
		const std::vector <Symbol> & syms = sol.to <TemplateSolution> ().getAllLocal ();
		bool dovalidate = true;
		if (!inModule) {
		    auto visitor = TemplateVisitor::init (*this);
		    auto mapper = visitor.inferArguments (sol.to <TemplateSolution> ().getTempls ());
		    if (mapper.succeed) dovalidate = false;
		}

		if (dovalidate) {
		    for (auto & it : syms) {
			validate (it, true);
		    }
		}
	    } catch (Error::ErrorList &lst) {
		errors = lst.errors;
	    }
	    
	    Visitor::__TEMPLATE_NB_RECURS__ -= 1;
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }
	}

	Generator Visitor::validateTemplateSolutionMethod (const semantic::Symbol & sol, const Generator & self) {
	    std::list <Ymir::Error::ErrorMsg> errors;	    
	    const std::vector <Symbol> & syms = sol.to <TemplateSolution> ().getAllLocal ();
	    
	    auto classType = sol.getReferent ().to <semantic::Class> ().getGenerator ().to <generator::Class> ().getClassRef ();
	    if (insertTemplateSolution (sol, errors)) { // If it is the first time the solution is presented
		if (errors.size () != 0) {
		    if (!global::State::instance ().isVerboseActive ()) {
			Ymir::Error::occur (sol.getName (), ExternalError::get (INCOMPLETE_TEMPLATE), sol.to <TemplateSolution> ().getSolutionName ());
		    }
			    
		    throw Error::ErrorList {errors};
		}
		
		pushReferent (sol, "validateTemplateSolutionMethod");
		enterForeign ();
		try {
		    Visitor::__TEMPLATE_NB_RECURS__ += 1;
		    if (Visitor::__TEMPLATE_NB_RECURS__ >= VisitConstante::LIMIT_TEMPLATE_RECUR) {
			Ymir::Error::occur (sol.getName (), ExternalError::get (TEMPLATE_RECURSION), Visitor::__TEMPLATE_NB_RECURS__);
		    }

		    auto visitor = FunctionVisitor::init (*this);
		    visitor.validateMethod (syms [0].to <semantic::Function> (), classType);
		} catch (Error::ErrorList &list) {
		    list.errors.insert (list.errors.begin (), Ymir::Error::createNoteOneLine ("% -> %", syms [0].getName (), syms[0].getRealName ()));
		    list.errors.insert (list.errors.begin (), Ymir::Error::createNote (syms [0].getName (), ExternalError::get (IN_TEMPLATE_DEF)));
		    
		    errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
		    removeTemplateSolution (sol); // If there is an error, we don't want to store the solution anymore
		    insertErrorTemplateSolution (sol, errors);
		}
		
		Visitor::__TEMPLATE_NB_RECURS__ -= 1;
		exitForeign ();
		popReferent ("validateTemplateSolutionMethod");
		if (errors.size () != 0) {
		    throw Error::ErrorList {errors};
		}
	    }
	    
	    if (syms [0].to <semantic::Function> ().isOver ()) {
		Ymir::Error::occur (syms [0].getName (), ExternalError::get (NOT_OVERRIDE), syms [0].getName ().getStr ());
	    }

	    pushReferent (syms [0], "validateTemplateSolutionMethod");
	    Generator proto (Generator::empty ());
	    try {
		proto = FunctionVisitor::init (*this).validateMethodProto (syms [0].to <semantic::Function> (), classType, Generator::empty ());
	    } catch (Error::ErrorList &list) {
		errors = list.errors;
	    }
	    
	    popReferent ("validateTemplateSolutionMethodProto");
	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }
	    
	    return proto;
	}


	Generator Visitor::validateTypeTemplateCall (const syntax::TemplateCall & call) {
	    auto left = call.getContent ();
	    match (left) {
		of (syntax::Var, var) {
		    Generator innerType (Generator::empty ());
		    if (call.getParameters ().size () == 1) {
			innerType = validateType (call.getParameters ()[0], true);
		    }
		    
		    if (var.getName ().getStr () == Range::NAME) {
			return Range::init (var.getName (), innerType);
		    }
		} fo;
	    }
	    
	    return Generator::empty ();
	}

	Generator Visitor::validateTemplateChecker (const syntax::TemplateChecker & check) {
	    std::vector<Generator> params;
	    std::list <Ymir::Error::ErrorMsg> errors;
	    for (auto & it : check.getCalls ()) {
		bool succeed = true;
		std::list <Ymir::Error::ErrorMsg> locErrors;
		try {
		    params.push_back (validateType (it, true));
		} catch (Error::ErrorList &list) {
		    locErrors.insert (locErrors.begin (), list.errors.begin (), list.errors.end ());
		    succeed = false;
		}

		if (!succeed) {
		    succeed = true;
		    try {
			auto val = validateValue (it);
			auto rvalue = retreiveValue (val);
			params.push_back (rvalue);
		    } catch (Error::ErrorList &list) {
			succeed = false;
		    }
		}
		
		if (!succeed)
		errors.insert (errors.end (), locErrors.begin (), locErrors.end ());
	    }

	    if (errors.size () != 0)
	    throw Error::ErrorList {errors};

	    bool succeed = false; // again, due to longjmp
	    try {
		auto visitor = TemplateVisitor::init (*this);
		int consumed = 0;
		auto mapper = visitor.validateFromExplicit (check.getParameters (), params, consumed);
		succeed = mapper.succeed && consumed == (int) params.size ();
	    } catch (Error::ErrorList &list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    }

	    return BoolValue::init (check.getLocation (), Bool::init (check.getLocation ()), succeed);
	}


	Generator Visitor::validateTemplateTest (const Symbol & context, const syntax::Expression & expr) {
	    Generator value (Generator::empty ());
	    std::list <Ymir::Error::ErrorMsg> errors;
	    pushReferent (context, "validateTemplateTest");
	    enterForeign ();
	    try {
		value = this-> validateValue (expr);
	    } catch (Error::ErrorList &list) {
		errors.insert (errors.end (), list.errors.begin (), list.errors.end ());
	    }

	    exitForeign ();
	    popReferent ("validateTemplateTest");
	    
	    if (errors.size () != 0) {
		throw Error::ErrorList {errors};
	    }
	    return value;
	}

	Generator Visitor::validateTemplateCall (const syntax::TemplateCall & tcl) {
	    auto value = this-> validateValue (tcl.getContent (), false, true);

	    std::list <Ymir::Error::ErrorMsg> errors;
	    std::vector <Generator> params;
	    for (auto & it : tcl.getParameters ()) {
		bool succeed = true;
		std::list <Ymir::Error::ErrorMsg> locErrors;
		try {
		    params.push_back (validateType (it, true));
		} catch (Error::ErrorList &list) {
		    locErrors.insert (locErrors.begin (), list.errors.begin (), list.errors.end ());
		    succeed = false;
		}

		if (!succeed) {
		    succeed = true;
		    try {
			auto val = validateValue (it);
			auto rvalue = retreiveValue (val);
			params.push_back (rvalue);
		    } catch (Error::ErrorList &list) {
			auto note = Ymir::Error::createNoteOneLine (ExternalError::get (TEMPLATE_VALUE_TRY));
			for (auto & it : list.errors)
			note.addNote (it);
			
			locErrors.back ().addNote (note);
			succeed = false;
		    }
		}
		
		if (!succeed)
		errors.insert (errors.end (), locErrors.begin (), locErrors.end ());
	    }
	    
	    if (errors.size () != 0)
	    throw Error::ErrorList {errors};
	    
	    if (value.is <TemplateRef> ()) {
		value = MultSym::init (value.getLocation (), {value});
	    }
	    
	    if (value.is<MultSym> ()) {
		int all_score = -1;
		Symbol final_sym (Symbol::empty ());
		std::map <int, std::vector <Symbol>> loc_scores;
		std::map <int, std::vector <Generator>> loc_elem;

		for (auto & elem : value.to <MultSym> ().getGenerators ()) {
		    if (elem.is<TemplateRef> ()) {
			int local_score = 0;
			Symbol local_sym (Symbol::empty ());
			bool succeed = true;
			try {
			    auto templateVisitor = TemplateVisitor::init (*this);
			    local_sym = templateVisitor.validateFromExplicit (elem.to <TemplateRef> (), params, local_score);
			    if (!local_sym.isEmpty ()) {
				loc_scores [local_score].push_back (local_sym);
				loc_elem [local_score].push_back (elem);
			    }
			} catch (Error::ErrorList &list) {
			    auto note = Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), elem.getLocation (), elem.prettyString ());
			    for (auto & it : list.errors) {
				note.addNote (it);
			    }
			    errors.push_back (note);			    
			    succeed = false;
			}

			if (succeed) {
			    if (local_score > all_score) {
				all_score = local_score;
				final_sym = local_sym;
			    } else if (local_sym.isEmpty ()) {
				errors.push_back (Ymir::Error::createNoteOneLine (ExternalError::get (CANDIDATE_ARE), elem.getLocation (), elem.prettyString ()));
			    }
			}
		    }
		}

		
		if (loc_scores.size () != 0) {
		    errors = {};
		    Generator ret (Generator::empty ());
		    auto &element_on_scores = loc_scores.find ((int) all_score)-> second;
		    auto &location_elems = loc_elem.find ((int) all_score)-> second;
		    std::vector <Symbol> syms;
		    std::vector <Generator> aux;
		    for (auto it : Ymir::r (0, element_on_scores.size ())) {
			std::vector <Generator> types;
			Visitor::__CALL_NB_RECURS__ += 1;
			try {
			    Symbol sym (Symbol::empty ());
			    if (element_on_scores [it].is <TemplatePreSolution> ()) sym = this-> validateTemplatePreSolution (element_on_scores [it], loc_elem.find ((int) all_score)-> second [it]);
			    if (location_elems [it].is<MethodTemplateRef> ()) {
				if (!sym.isEmpty () && sym.is <TemplateSolution> ()) {
				    auto self = location_elems [it].to <MethodTemplateRef> ().getSelf ();
				    auto proto = this-> validateTemplateSolutionMethod (sym, self);
				    auto delType = Delegate::init (proto.getLocation (), proto);
				    aux.push_back (
					DelegateValue::init (proto.getLocation (),
							     delType,
							     proto.to<MethodProto> ().getClassType (),
							     self,
							     proto
					    )
					);
				} else { // Not finalized template call
				    auto & tmpRef = location_elems [it];
				    aux.push_back (
					MethodTemplateRef::init (tmpRef.getLocation (), element_on_scores [it], tmpRef.to <MethodTemplateRef> ().getSelf ())
					);
				}
			    } else {
				syms.push_back (element_on_scores [it]);
			    }
			} catch (Error::ErrorList &list) {
			    list.errors.insert (list.errors.begin (), Ymir::Error::createNoteOneLine ("% -> %", element_on_scores [it].getName (), element_on_scores [it].getRealName ()));
			    list.errors.insert (list.errors.begin (), Ymir::Error::createNote (tcl.getLocation (), ExternalError::get (IN_TEMPLATE_DEF)));
			    std::vector<std::string> names;
			    for (auto & it : params) {
				names.push_back (it.prettyString ());
			    }

			    std::string leftName = value.getLocation ().getStr ();
			    list.errors = {Ymir::Error::makeOccurAndNote (
				    tcl.getLocation (),
				    list.errors,
				    ExternalError::get (UNDEFINED_TEMPLATE_OP),
				    leftName,
				    names
				    )};
				
			    errors = std::move (list.errors);
			}
			Visitor::__CALL_NB_RECURS__ -= 1;
		    }
		    		    
		    if (aux.size () != 0 || syms.size () != 0) {
			
			if (syms.size () != 0) {
			    ret = this-> validateMultSym (value.getLocation (), syms);
			    if (ret.is <MultSym> ())
			    aux.insert (aux.end (), ret.to <MultSym> ().getGenerators ().begin (), ret.to <MultSym> ().getGenerators ().end ());
			    else aux.push_back (ret);
			}
			
			if (aux.size () == 1) {
			    ret = aux [0];
			} else ret = MultSym::init (value.getLocation (), aux);
		    		    
			if (!ret.isEmpty ())
			return ret;
		    }
		}
	    }

	    std::vector<std::string> names;
	    for (auto & it : params) {
		names.push_back (it.prettyString ());
	    }
		    
	    std::string leftName = value.getLocation ().getStr ();
	    auto err = Ymir::Error::makeOccurAndNote (
		tcl.getLocation (),
		errors,
		ExternalError::get (UNDEFINED_TEMPLATE_OP),
		leftName,
		names
		);
	    err.setWindable (true);
	    throw Error::ErrorList {{err}};
	}

	bool Visitor::insertTemplateSolution (const Symbol & sol, std::list <Error::ErrorMsg> & errors) {
	    for (auto & it : this-> _templateSolutions) {
		if (it.equals (sol)) return false;
	    }
	    
	    for (auto & it : this-> _errorTemplateSolutions) {
		if (it.first.equals (sol)) {
		    errors = it.second;
		    return true;
		}
	    }
	    
	    this-> _templateSolutions.push_back (sol);
	    return true;
	}

	void Visitor::insertErrorTemplateSolution (const Symbol & sol, const std::list <Error::ErrorMsg> & errors) {
	    this-> _errorTemplateSolutions.push_back (std::pair <Symbol, std::list <Error::ErrorMsg> > (sol, errors));
	}

	void Visitor::removeTemplateSolution (const Symbol & sol) {
	    std::vector <Symbol> res;
	    bool found = false;
	    for (auto & it : this-> _templateSolutions) {
		if (found) res.push_back (it);
		else if (!it.equals (sol)) res.push_back (it);
		else found = true;
	    }

	    this-> _templateSolutions = res;
	}

	void Visitor::removeErrorTemplateSolution (const Symbol & sol) {
	    std::vector <std::pair <Symbol, std::list <Error::ErrorMsg> > > res;
	    bool found = false;
	    for (auto & it : this-> _errorTemplateSolutions) {
		if (found) res.push_back (it);
		else if (!it.first.equals (sol)) res.push_back (it);
		else found = true;
	    }

	    this-> _errorTemplateSolutions = res;
	}

	Generator Visitor::validateListTemplate (const TemplateSyntaxList & lst) {
	    std::vector <syntax::Expression> exprs;
	    for (auto & it : lst.getContents ())
	    exprs.push_back (TemplateSyntaxWrapper::init (it.getLocation (), it));
	    return validateTuple (syntax::List::init (lst.getLocation (), lst.getLocation (), exprs).to <syntax::List> ());
	}
	
    }
   
}
