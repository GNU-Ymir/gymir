#pragma once

#include <vector>
#include <string>

namespace Ymir {

    class Path {

	std::vector <std::string> _files;
	
    private : 

	Path (const std::vector <std::string> & files);
	
    public : 

	/**
	 * \param path the path to parse
	 * \param file_sep the separator char used
	 */
	Path (const std::string & path, const std::string & file_sep = "/");

	/**
	 * \brief create a new path
	 */
	static Path build (const std::string & begin, const std::string & end);

	/**
	 * \brief create a new path
	 */
	static Path build (const Path & begin, const std::string & end);
	
	/**
	 * \brief Remove the extension of the file
	 * \return the same path without file extension
	 */
	Path stripExtension () const;
	
	/**
	 * \return true if this path is a relative path of absolute path from anywhere
	 */
	bool isRelativeFrom (const Path & absolute) const;

	/**
	 * \return the path without the file 
	 */
	Path baseName () const;

	/**
	 * \return only the file name
	 */
	Path fileName () const;

	/**
	 * \return the list of file and directory composing the path
	 */
	std::vector <std::string> getFiles () const;
	
	/**
	 * \brief Transform the path into a string
	 * \param file_sep the string to use between files
	 */
	std::string toString (const std::string & file_sep = "/") const;
	
    };
    

    bool file_exists (const std::string& name);
    
}

