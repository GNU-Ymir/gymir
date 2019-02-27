#include <ymir/utils/Path.hh>
#include <ymir/utils/OutBuffer.hh>

namespace Ymir {
    
    Path::Path (const std::vector <std::string> & path) :
	_files (path)
    {}
    
    Path::Path (const std::string & path, const std::string & sep) {
	auto aux = path;
	auto pos = std::string::npos;
	do {
	    pos = aux.find (sep);
	    if (pos != std::string::npos) {
		this-> _files.push_back (aux.substr (0, pos));
		aux = aux.substr (pos + sep.size ());
	    } else this-> _files.push_back (aux);
	} while (pos != std::string::npos);
    }

    Path Path::build (const std::string & begin, const std::string & end) {
	auto left = begin.size () >= 1 && begin [begin.length () - 1] == '/';
	auto right = end.size () >= 1 && end [0] == '/';
	if (left && right) {
	    return Path {begin + end.substr (1)};
	} else if (left || right) {
	    return Path {begin + end};
	} else return Path {begin + "/" + end};
    }

    Path Path::build (const Path & begin, const std::string & end) {
	if (end.size () >= 1 && end [0] == '/')
	    return Path {begin.toString () + end};
	else return Path {begin.toString () + '/' + end};
    }


    Path Path::stripExtension () const {
	if (this-> _files.size () == 0) return *this;
	else {
	    auto ret = this-> _files;
	    auto end = this-> _files [this-> _files.size () - 1];
	    auto index = end.find ('.');
	    if (index != std::string::npos) {
		ret [ret.size () - 1] = end.substr (0, index);
	    }
	    return Path {ret};
	}
    }

    bool Path::isRelativeFrom (const Path & absolute) const {
	if (this-> _files.size () > absolute._files.size ())
	    return false;

	for (int i = this-> _files.size () - 1 ; i >= 0 ; i --) {
	    if (this-> _files [i] != absolute._files [i]) return false;
	}
	
	return true;
    }

    Path Path::baseName () const {
	if (this-> _files.size () == 0) return Path (std::vector <std::string> ());
	else {
	    auto ret = this-> _files;
	    ret.pop_back ();
	    return Path {ret};
	}
    }

    Path Path::fileName () const {
	if (this-> _files.size () == 0) return Path {std::vector <std::string> ()};
	else {
	    return Path {{this-> _files[this-> _files.size () - 1]}};
	}
    }    

    std::string Path::toString (const std::string & file_sep) const {
	Ymir::OutBuffer buf;
	int i = 0;
	for (auto & it : this-> _files) {
	    if (i != 0) buf.write (file_sep);
	    buf.write (it);
	    i += 1;
	}
	return buf.str ();
    }
    
}
