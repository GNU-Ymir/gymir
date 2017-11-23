#include <ymir/ast/Program.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/utils/Mangler.hh>
#include <string>

namespace syntax {

    using namespace semantic;    

    void IProgram::declare () {
	std::string name = LOCATION_FILE (this-> locus.getLocus ());
	auto dot = name.find_last_of ('.');
	if (dot != name.npos && name.substr (dot, name.length () - dot) == ".yr") {
	    name = name.substr (0, dot);
	}

	Table::instance ().setCurrentSpace ({Mangler::mangle_file (name)});
	Table::instance ().programNamespace () = Table::instance ().globalNamespace ();
	Table::instance ().addForeignModule (Table::instance ().globalNamespace ());

	for (auto it : this-> decls) {
	    it-> declare ();
	}
	
    }    

}
