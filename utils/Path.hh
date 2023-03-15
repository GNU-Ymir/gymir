#pragma once

#include <vector>
#include <string>

namespace Ymir {

	class Path {

		std::vector <std::string> _files;

	public :

		Path (const std::vector <std::string> & files);


		/**
		 * \param path the path to parse
		 * \param file_sep the separator char used
		 */
		Path (const std::string & path, const std::string & file_sep = "/");

		/**
		 * \returns that path of the current working directory
		 * */
		static Path getCwd ();

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
		 * \returns an absolute path where current directory is cwd
		 * */
		Path toAbsolute (const Path & cwd) const;

		/**
		 * \returns a simplified path where '..' and '.' are removed (assuming the path is absolute)
		 * */
		Path simplify () const;

		/**
		 * \return true if path has the same start as begin
		 */
		bool startWith (const Path & begin) const;
	
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
		const std::vector <std::string> & getFiles () const;
	
		/**
		 * \brief Transform the path into a string
		 * \param file_sep the string to use between files
		 */
		std::string toString (const std::string & file_sep = "/") const;
	
	};
    

	bool file_exists (const std::string& name);
    
}

