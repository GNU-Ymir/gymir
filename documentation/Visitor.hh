#pragma once


#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/symbol/_.hh>
#include <ymir/semantic/generator/value/MethodProto.hh>
#include <ymir/semantic/generator/value/Class.hh>

namespace documentation {


    class Visitor {
    private :

	/// The context is used to retreive the type, prototypes, etc. of each symbol to dump 
	semantic::validator::Visitor & _context; 
	
    private : 

	Visitor (semantic::validator::Visitor & context);
	
    public :

	/**
	 * Create an empty visitor
	 */
	static Visitor init (semantic::validator::Visitor & context);

	/**
	 * Dump the documentation for the symbol `sym`
	 * \param sym a semantically validated symbol (basically a module)
	 */
	std::string dump (const semantic::Symbol & sym);
	
	/**
	 * Dump the documentation of a module
	 * \return the doc
	 */
	std::string dumpModule (const semantic::Module & md);

	/**
	 * Dump the documentation of a function
	 * \return the doc
	 */
	std::string dumpFunction (const semantic::Function & func);

	/**
	 * Dump the documentation of a vardecl
	 * \return the doc
	 */
	std::string dumpVarDecl (const semantic::VarDecl & decl);

	/**
	 * Dump the documentation of an alias 
	 * \return the doc
	 */
	std::string dumpAlias (const semantic::Alias & al);

	/**
	 * Dump the documentation of a struct
	 * \return the doc
	 */
	std::string dumpStruct (const semantic::Struct & str);

	/**
	 * Dump the documentation of an enumeration
	 * \return the doc
	 */
	std::string dumpEnum (const semantic::Enum & en);

	/**
	 * Dump the documentation of a class
	 * \return the doc
	 */
	std::string dumpClass (const semantic::Class & cl);

	/**
	 * Dump the documentation of a method
	 * \return the doc
	 */
	std::string dumpMethodProto (const semantic::generator::MethodProto & pt, const semantic::generator::Class::MethodProtection & prot);

	/**
	 * Dump the documentation of a trait
	 * \return the doc
	 */
	std::string dumpTrait (const semantic::Trait & tr);
	
    };
    

}
