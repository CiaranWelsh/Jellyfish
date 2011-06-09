/*  This file is part of Jellyfish.

    Jellyfish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Jellyfish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Jellyfish.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <jellyfish/err.hpp>
#include <jellyfish/file_parser.hpp>
#include <jellyfish/dbg.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>

int jellyfish::file_parser::file_peek(const char *path, char *peek) {
  int fd = open(path, O_RDONLY);
  if(fd == -1)
    eraise(FileParserError) << "Error opening file '" 
                            << path << "'" << err::no;
      
  if(read(fd, peek, 1) <= 0)
    eraise(FileParserError) << "Empty input file '" << path << "'";
  
  return fd;
}

jellyfish::file_parser::file_parser(int fd, const char *path, 
                                    const char *str, size_t len, char pbase) : 
  _fd(fd), _base(pbase), _pbase(pbase) {
  struct stat stat_buf;
  if(fstat(fd, &stat_buf) == -1)
    eraise(FileParserError) << "Can't fstat '" << path << "'" << err::no;
  _size       = stat_buf.st_size;
  if(_do_mmap) {
    _buffer     = (char *)mmap(0, _size , PROT_READ, MAP_SHARED, fd, 0);
    _is_mmapped = _buffer != MAP_FAILED;
  } else {
    _is_mmapped  = false;
  }
  if(_is_mmapped) {
    _end_buffer = _buffer + _size;
    _data       = _buffer;
    _end_data   = _end_buffer;
    close(_fd);
  } else {
    if(_force_mmap)
      eraise(FileParserError) << "Cannot mmap file '" << path 
                              << "' as required";
    _buffer     = new char[_buff_size];
    _end_buffer = _buffer + _buff_size;
    _data       = _end_data = _buffer;
    // What if len > _buff_size!!!
    if(str && len) {
      memcpy(_buffer, str, len);
      _end_data   = _buffer + len;
    }        
  }
}

jellyfish::file_parser::~file_parser() {
  if(_is_mmapped) {
    DBG << "unmapping " << V((void*)_buffer) << V(_size);
    munmap(_buffer, _size);
  } else {
    delete _buffer;
    close(_fd);
  }
}

// Get next character in "stream"
int jellyfish::file_parser::sbumpc() {
  _pbase = _base;
  if(_data >= _end_data) {
    if(_is_mmapped)
      return (_base = EOF);
    ssize_t gcount = read(_fd, _buffer, _buff_size);
    if(gcount <= 0)
      return (_base = EOF);
    _data     = _buffer;
    _end_data = _buffer + gcount;
  }
  //    std::cerr << "sbumpc " << *_data << std::endl;
  return (_base = *_data++);
}

// Get next character in "stream" without moving
// XXX code duplication! Refactor
int jellyfish::file_parser::speekc() {
  if(_data >= _end_data) {
    if(_is_mmapped)
      return (_base = EOF);
    ssize_t gcount = read(_fd, _buffer, _buff_size);
    if(gcount <= 0)
      return (_base = EOF);
    _data     = _buffer;
    _end_data = _buffer + gcount;
  }
  //    std::cerr << "sbumpc " << *_data << std::endl;
  return *_data;
}

bool jellyfish::file_parser::do_mmap() { return _do_mmap; }
bool jellyfish::file_parser::do_mmap(bool new_value) {
  bool old_value = _do_mmap;
  _do_mmap = new_value;
  return old_value;
}

bool jellyfish::file_parser::_do_mmap = true;
bool jellyfish::file_parser::_force_mmap = false;
