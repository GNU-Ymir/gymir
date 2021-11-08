#pragma once


#include <ymir/semantic/validator/Visitor.hh>
#include <ymir/semantic/symbol/_.hh>
#include <ymir/semantic/generator/value/MethodProto.hh>
#include <ymir/semantic/generator/value/Class.hh>
#include <ymir/syntax/declaration/_.hh>
#include <ymir/documentation/Json.hh>

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
	 * Dump the dependecies for the symbol `sym`
	 */
	json::JsonValue dumpDependency (semantic::Symbol & sym);
	
	/**
	 * Dump the documentation for the symbol `sym`
	 * \param sym a semantically validated symbol (basically a module)
	 */
	json::JsonValue dump (const semantic::Symbol & sym);

	/**
	 * Dump the documentation for the symbol `sym`
	 */
	json::JsonValue dumpUnvalidated (const semantic::Symbol & sym);
	
	/**
	 * Dump the documentation for the declaration `sym`
	 */
	json::JsonValue dumpUnvalidated (const syntax::Declaration & sym, bool pub = false, bool prot = false);
	
	/**
	 * Dump the documentation of a module
	 * \return the doc
	 */
	json::JsonValue dumpModule (const semantic::Module & md);

	/**
	 * Dump the documentation of a module
	 * \return the doc
	 */
	json::JsonValue dumpModuleUnvalidated (const syntax::Module & md, bool pub, bool prot);

	/**
	 * Dump the documentation of a decl block
	 * \return the doc
	 */
	json::JsonValue dumpDeclBlockUnvalidated (const syntax::DeclBlock & dl, bool pub, bool prot);

	/**
	 * Dump the documentation of an external block
	 * \return the doc
	 */
	json::JsonValue dumpExternBlockUnvalidated (const syntax::ExternBlock & ex, bool pub, bool prot);
	
	/**
	 * Dump the documentation of a condition block
	 * \return the doc
	 */
	json::JsonValue dumpCondBlockUnvalidated (const syntax::CondBlock & bl, bool pub, bool prot);
	
	/**
	 * Dump the documentation of a function
	 * \return the doc
	 */
	json::JsonValue dumpFunction (const semantic::Symbol & func);

	/**
	 * Dump the documentation of a function
	 * \return the doc
	 */
	json::JsonValue dumpFunctionUnvalidated (const syntax::Function & func, bool pub, bool prot);

	/**
	 * Dump the documentation of a vardecl
	 * \return the doc
	 */
	json::JsonValue dumpVarDecl (const semantic::VarDecl & decl);

	/**
	 * Dump the documentation of a global var declaration 
	 * \return the doc
	 */
	json::JsonValue dumpGlobalUnvalidated (const syntax::Global & gl, bool pub, bool prot);
	

	/**
	 * Dump the documentation of an alias 
	 * \return the doc
	 */
	json::JsonValue dumpAka (const semantic::Aka & al);

	/**
	 * Dump the documentation of an unvalidated alias 
	 * \return the doc
	 */
	json::JsonValue dumpAkaUnvalidated (const syntax::Aka & al, bool pub, bool prot);

	/**
	 * Dump the documentation of a struct
	 * \return the doc
	 */
	json::JsonValue dumpStruct (const semantic::Struct & str);

	/**
	 * Dump the documentation of a structure
	 * \return the doc
	 */
	json::JsonValue dumpStructUnvalidated (const syntax::Struct & str, bool pub, bool prot);
	
	/**
	 * Dump the documentation of an enumeration
	 * \return the doc
	 */
	json::JsonValue dumpEnum (const semantic::Enum & en);

	/**
	 * Dump the documentation of an enumeration
	 * \return the doc
	 */
	json::JsonValue dumpEnumUnvalidated (const syntax::Enum & en, bool pub, bool prot);
	
	/**
	 * Dump the documentation of a class
	 * \return the doc
	 */
	json::JsonValue dumpClass (const semantic::Class & cl);

	/**
	 * Dump the documentation of a class
	 * \return the doc
	 */
	json::JsonValue dumpClassUnvalidated (const syntax::Class & cl, bool pub, bool prot);

	/**
	 * Dump the documentation of the inner declaration of an unvalidated class (template)
	 */
	void dumpInnerClassUnvalidated (std::vector <json::JsonValue> & fields, std::vector <json::JsonValue> & cstrs, std::vector <json::JsonValue> & methods, std::vector <json::JsonValue> & others, const std::vector <syntax::Declaration> & decls, bool prv, bool prot, bool pub);

	json::JsonValue dumpConstructorUnvalidated (const syntax::Constructor & cstr, bool prv, bool prot, bool pub);
	
	/**
	 * Dump the documentation of a method
	 * \return the doc
	 */
	json::JsonValue dumpMethodProto (const semantic::generator::MethodProto & pt, const semantic::generator::Class::MethodProtection & prot, bool isOver);

	/**
	 * Dump the documentation of a trait
	 * \return the doc
	 */
	json::JsonValue dumpTrait (const semantic::Trait & tr);

	/**
	 * Dump the documentation of a trait
	 * \return the doc
	 */
	json::JsonValue dumpTraitUnvalidated (const syntax::Trait & tr, bool pub, bool prot);
	
	/**
	 * Dump the documentation of a template
	 * \return the doc
	 */	
	json::JsonValue dumpTemplate (const semantic::Template & tm);

	/**
	 * Dump the documentation  of a template
	 * \return the doc
	 */
	json::JsonValue dumpTemplateUnvalidated (const syntax::Template & tm, bool pub, bool prot);	
	
	/**
	 * Dump the documentation of a macro
	 * \return the doc
	 */
	json::JsonValue dumpMacro (const semantic::Macro & x);

	/**
	 * Dump the documentation of a macro
	 * \return the doc
	 */
	json::JsonValue dumpMacroUnvalidated (const syntax::Macro & x, bool pub, bool prot);

	/**
	 * Dump a type
	 * \return the doc of the type
	 */
	json::JsonValue dumpType (const semantic::generator::Generator & type, bool forceMut = false);

	/**
	 * Dump a type
	 * \return the doc of the type
	 */
	json::JsonValue dumpType (const syntax::Expression & type);
	
    private :

	/**
	 * Dump the location, comments and protection of a symbol
	 */
	void dumpStandard (const semantic::ISymbol & sym, std::map <std::string, json::JsonValue> & val);

	/**
	 * Dump the location, comments and protection of a symbol
	 */
	void dumpStandard (const syntax::IDeclaration & sym, bool pub, bool prot, std::map <std::string, json::JsonValue> & val);
	
	
    };
    

}
