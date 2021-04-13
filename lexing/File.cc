#include <ymir/lexing/File.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/errors/Error.hh>

namespace lexing {

    std::string StringFile::__filename__ = "(string file)";
    std::string File::__empty_filename__ = "(null)";

    
    IFile::IFile () {}

    IFile::~IFile () {}
    
    void IFile::close () {}
    
    RealFile::RealFile () :
	_filename (""),
	_ptr (nullptr)
    {}

    RealFile::RealFile (const char * filename, FILE * ptr) :
	_filename (filename),
	_ptr (ptr)
    {
	if (this-> _ptr == nullptr) {
	    Ymir::Error::halt ("%(r) no such file : %(y)", "Critical", this-> _filename);
	}
    }
    
    File RealFile::init (const char * filename) {
	return File {new (NO_GC) RealFile (filename, fopen (filename, "r"))};
    }

    File RealFile::init (const std::string & filename) {
	return File {new (NO_GC) RealFile (filename.c_str (), fopen (filename.c_str (), "r"))};
    }
    
    bool RealFile::isEof () const {
	if (this-> _ptr != nullptr) {
	    return feof (this-> _ptr);
	} else {
	    Ymir::Error::halt ("%(r) Reading in closed file", "Critical");
	    return true;
	}
    }

    ulong RealFile::tell () const {
	if (this-> _ptr != nullptr) {
	    return ftell (this-> _ptr);
	} else {
	    Ymir::Error::halt ("%(r) Reading in closed file", "Critical");
	    return 0;
	} 
    }
    
    void RealFile::seek (ulong i) {
	if (this-> _ptr != nullptr) {
	    fseek (this-> _ptr, i, SEEK_SET);
	} else {
	    Ymir::Error::halt ("%(r) Reading in closed file", "Critical");
	} 
    }
	
    std::string RealFile::readln () {
	if (this-> _ptr != nullptr) {
	    std::string all = "";
	    char buf [255];
	    while (1) {
		char * aux = fgets(buf, 255, this-> _ptr);
		if (aux == NULL) {
		    return all;
		}

		std::string ret = std::string (buf);
		all += ret;
		// String is null terminated, so the line return is at -2, not -1
		if (ret [ret.size () - 2] != '\n') {
		    return all;
		}
	    }
	} else {
	    Ymir::Error::halt ("%(r) Reading in closed file", "Critical");
	    return "";
	}
    }

    std::string RealFile::readAll ()  {
	if (this-> _ptr != nullptr) {
	    auto tell = ftell (this-> _ptr);
	    fseek (this-> _ptr, 0, SEEK_END);
	    auto length = ftell (this-> _ptr) - tell;
	    char * buf = new char [length];
	    fseek (this-> _ptr, tell, SEEK_SET);
	
	    auto size = fread (buf, 1, length, this-> _ptr);
	    buf [size] = '\0';
	    std::string ret = std::string (buf);
	    delete buf;
	    return ret;
	} else {
	    Ymir::Error::halt ("%(r) Reading in closed file", "Critical");
	    return "";
	}
    }    

    const std::string & RealFile::getFilename () const {
	return this-> _filename;
    }

    void RealFile::close () {
	if (this-> _ptr != nullptr) {
	    fclose (this-> _ptr);
	    this-> _ptr = nullptr;	    
	}
    }

    bool RealFile::isClosed () const {
	return this-> _ptr == nullptr;
    }
    
    RealFile::~RealFile () {
	this-> close ();
    }

    File RealFile::clone () const {
	auto file = RealFile::init (this-> _filename);	
	return file;
    }
    
    StringFile::StringFile () :
	_content (""),
	_cursor (0)
    {}
    
    StringFile::StringFile (const std::string & content) :
	_content (content),
	_cursor (0)
    {}
    
    File StringFile::init (const std::string & content) {
	return File {new (NO_GC) StringFile (content)};
    }

    File StringFile::clone () const {
	return File {new (NO_GC) StringFile (*this)};
    }
    
    bool StringFile::isEof () const {
	return this-> _cursor >= this-> _content.length ();
    }

    ulong StringFile::tell () const {
	return this-> _cursor;
    }
    
    void StringFile::seek (ulong i) {
	this-> _cursor = i;
    }
	
    std::string StringFile::readln ()  {
	auto start = this-> _cursor;
	
	while (this-> _cursor < this-> _content.length () && this-> _content [this-> _cursor] != '\n') {
	    this-> _cursor ++;
	}
	this-> _cursor += 1;
	
	if (start < this-> _content.length ()) {
	    char * buf = new char [this-> _cursor - start + 1];
	    for (unsigned long i = start ; i < this-> _cursor ; i++) {
		buf [i - start] = this-> _content [i];
	    }
	    
	    buf[this-> _cursor - start] = '\0';	    
	    std::string ret (buf);
	    
	    delete buf;
	    return ret;
	}
	
	return "";
    }

    std::string StringFile::readAll ()  {	
	auto tell = this-> _cursor;;
	auto length = this-> _content.length () - tell;
	
	char * buf = new char [length];
	while (this-> _cursor < this-> _content.length ()) {
	    buf [this-> _cursor - tell] = this-> _content [this-> _cursor];
	    this-> _cursor += 1;
	}
	
	std::string ret = std::string (buf);
	delete buf;

	return ret;
    }    

    const std::string & StringFile::getFilename () const {
	return StringFile::__filename__; 
    }    

    void StringFile::close () {}

    bool StringFile::isClosed () const {
	return false;
    }
    
    File::File () : RefProxy <IFile, File> (nullptr)
    {}

    File::File (IFile* f) : RefProxy <IFile, File> (f)
    {}

    File File::empty () {
	return File (nullptr);
    }

    File File::clone () const {
	if (this-> _value != nullptr)
	    return this-> _value-> clone ();
	else return *this;
    }
    
    bool File::isEmpty () const {
	return this-> _value == nullptr;
    }

    bool File::isEof () const {
	if (this-> _value == nullptr) {
	    return true;
	} else {
	    return this-> _value-> isEof ();
	}
    }

    ulong File::tell () const {
	if (this-> _value == nullptr) {
	    return 0;
	} else {
	    return this-> _value-> tell ();
	}
    }

    std::string File::readln () {
	if (this-> _value == nullptr) {
	    return "";
	} else {
	    return this-> _value-> readln ();
	}
    }

    void File::seek (ulong i) {
	if (this-> _value != nullptr) {
	    return this-> _value-> seek (i);
	}
    }

    std::string File::readAll () {
	if (this-> _value == nullptr) {
	    return "";
	} else {
	    return this-> _value-> readAll ();
	}
    }

    const std::string & File::getFilename () const {
	if (this-> _value == nullptr) {
	    return File::__empty_filename__;
	} else {
	    return this-> _value-> getFilename ();
	}
    }    

    
    void File::close () {
	if (this-> _value != nullptr) {
	    this-> _value-> close ();
	}
    }

    bool File::isClosed () const {
	if (this-> _value == nullptr)
	    return true;
	else return this-> _value-> isClosed ();
    }
        
    
}
