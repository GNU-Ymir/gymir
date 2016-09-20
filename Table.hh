#pragma once


namespace Semantic {

    struct Table {
	
	static Table & instance () {
	    return inst;	    
	}

	void enterBlock ();

	void quitBlock ();

	void enterFrame ();

	void quitFrame ();
	
	void addImport (Package pck);

	void insert (std::string name, Symbol sym);

	Symbol get (std::string name);	

    private:
	
	Table () {}
	Table(const Table&);
	Table & operator = (const Table &);
	
	static Table inst;
	Package imported;       	
	Scope global_scope;
	std::list <FrameScope> frameTable;
	
    };
       
}
