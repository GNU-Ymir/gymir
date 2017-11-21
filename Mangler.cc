#include <ymir/utils/Mangler.hh>
#include <ymir/semantic/pack/FrameProto.hh>
#include <ymir/semantic/types/InfoType.hh>
#include <string>
#include <ymir/utils/OutBuffer.hh>

namespace Mangler {
    using namespace Ymir;
    
    std::string mangle_file (std::string & in) {
	Ymir::OutBuffer ss;
	for (auto it : in) {
	    if (it == '/') ss.write ('.');
	    else ss.write ((char) it);
	}
	return ss.str ();
    }

    std::string replace (std::string ph, std::vector <char> from, std::vector <char> to) {
	OutBuffer buf;
	for (ulong i = 0 ; i < ph.length () ; i++) {
	    for (ulong j = 0 ; j < from.size () ; j++) {
		if (ph [i] == from [j]) {
		    buf.write (to [j]);
		} else buf.write (ph [i]);
	    }
	}
	return buf.str ();
    }
    
    std::string mangle_var (std::string name) {
	auto res = replace (name, {'(', ')', ',', '\''}, {'N', 'N', 'U', 'G'});
	OutBuffer fin;
	for (auto it : res) {
	    if ((it < 'a' || it > 'z') &&
		(it < 'A' || it > 'Z') &&
		(it < '0' || it > '9') &&
		(it != '_')
	    ) {
		fin.write((short) it);
	    } else fin.write (it);
	}
	
	OutBuffer ss;
	ss.write (fin.str ().length (), replace (fin.str (), {'!'}, {'T'}));
	return ss.str ();
    }
    
    std::string mangle_type (std::string name) {
	auto res = replace (name, {'(', ')', ',', '\''}, {'N', 'N', 'U', 'G'});
	OutBuffer fin;
	for (auto it : res) {
	    if ((it < 'a' || it > 'z') &&
		(it < 'A' || it > 'Z') &&
		(it < '0' || it > '9') &&
		(it != '_')
	    ) {
		fin.write ((short) it);
	    } else fin.write (it);
	}
	return fin.str ();
    }

    std::string mangle_namespace (std::string name) {
	OutBuffer ss;
	while (true) {
	    auto index = name.find (".");
	    if (index != name.npos) {
		auto curr = mangle_var (name.substr (0, index));
		name = name.substr (index + 1, name.length () - (index + 1));
		ss.write (curr);
	    } else {
		ss.write (mangle_var (name));
		break;
	    }
	}
	return ss.str ();
    }
    
    std::string mangle_function (std::string& name, ::semantic::FrameProto frame) {
	auto space = frame-> space.toString ();
	OutBuffer ss;
	ss.write ("_Y", mangle_namespace (space), mangle_namespace (name), "F");
	for (auto it : frame-> vars) {
	    ss.write (mangle_type (it-> info-> type-> simpleTypeString ()));
	}
	ss.write ("Z", mangle_type (frame-> type-> simpleTypeString ()));
	return ss.str ();
    }

    std::string mangle_functionv (std::string& name, ::semantic::FrameProto frame) {
	auto space = frame-> space.toString ();
	OutBuffer ss;
	ss.write  ("_Y", mangle_namespace (space), mangle_namespace (name), "VF");
	for (auto it : frame-> vars) {
	    ss.write (mangle_type (it-> info-> type-> simpleTypeString ()));
	}
	ss.write ("Z", mangle_type (frame-> type-> simpleTypeString ()));
	return ss.str ();
    }

}
