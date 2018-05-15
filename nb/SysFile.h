//==============================================================================
//
//  SysFile.h
//
//  Copyright (C) 2017  Greg Utas
//
//  This file is part of the Robust Services Core (RSC).
//
//  RSC is free software: you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software
//  Foundation, either version 3 of the License, or (at your option) any later
//  version.
//
//  RSC is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
//  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
//  details.
//
//  You should have received a copy of the GNU General Public License along
//  with RSC.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef SYSFILE_H_INCLUDED
#define SYSFILE_H_INCLUDED

#include <iosfwd>
#include <memory>
#include <string>

//------------------------------------------------------------------------------

namespace NodeBase
{
//  Virtual base class for file iteration.  An instance is created with
//  SysFile::GetFileList.
//
class FileList
{
public:
   //  Virtual to allow subclassing.
   //
   virtual ~FileList() { }

   //  Sets fileName to the name of the current file (without a path prefix).
   //  Clears fileName if NextFile has returned false because no files remain.
   //
   virtual void GetName(std::string& fileName) const = 0;

   //  Returns true if the current file is a subdirectory.
   //
   virtual bool IsSubdir() const = 0;

   //  Returns true if the end of the list has been reached.
   //
   virtual bool AtEnd() const = 0;

   //  Advances to the next file in the list.  Returns false if there are no
   //  more files in the list.
   //
   virtual bool Advance() = 0;
protected:
   //  Protected because this class is virtual.
   //
   FileList() { }
};

//  For wrapping input and output streams and iterating over files.
//
typedef std::unique_ptr< std::istream > istreamPtr;
typedef std::unique_ptr< std::ostream > ostreamPtr;
typedef std::unique_ptr< FileList > FileListPtr;

//  Operating system abstraction layer: file I/O and directory navigation.
//
namespace SysFile
{
   //  Opens an existing file for input.  Returns nullptr if the file is
   //  empty or does not exist.
   //
   istreamPtr CreateIstream(const char* fileName);

   //  Creates a file for output.  If the file already exists, output is
   //  appended to it unless TRUNC is false.
   //
   ostreamPtr CreateOstream(const char* fileName, bool trunc = false);

   //  Sets dirName to the default directory.  On an error, dirName is
   //  cleared.
   //
   void GetDir(std::string& dirName);

   //  Sets the default directory.  Returns false if the directory does
   //  not exist.
   //
   bool SetDir(const char* dirName);

   //  Iterates over files whose name matches fileSpec (which can include
   //  wildcard characters_ in the directory specified by dirName.  If
   //  dirName is nullptr, the default directory is searched.  Returns
   //  nullptr if dirName does not exist or no files matched fileSpec.
   //
   FileListPtr GetFileList(const char* dirName, const char* fileSpec);
}
}
#endif
