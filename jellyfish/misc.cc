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

#include <config.h>
#include "misc.hpp"

void die(const char *msg, ...)
{
  va_list ap;

  va_start(ap, msg);
  vfprintf(stderr, msg, ap);
  va_end(ap);

  exit(1);
}

uint64_t bogus_sum(void *data, size_t len) {
  uint64_t res = 0, tmp = 0;
  uint64_t *ptr = (uint64_t *)data;

  while(len >= sizeof(uint64_t)) {
    res ^= *ptr++;
    len -= sizeof(uint64_t);
  }
  
  if(len > 0) {
    memcpy(&tmp, ptr, len);
    res ^= tmp;
  }
  return res;
}

int parse_long(char *arg, std::ostream *err, unsigned long *res)
{
  char *endptr;
  char error[200];
  errno = 0;
  *res = strtoul(arg, &endptr, 0);
  if(errno) {
    error[0] = '\0';
    strerror_r(errno, error, sizeof(error));
    (*err) << "Error parsing integer string '" << arg << "': " << error << std::endl;
    return errno;
  }
  if(*arg == '\0' || *endptr != '\0') {
    (*err) << "Invalid integer argument '" << arg << "'" << std::endl;;
    return EINVAL;
  }
  return 0;
}

std::string stringf(const char *fmt, va_list _ap)
{
  char *buf = NULL;
  int olength;
  int length = 5;   // Initial guess                                                             
  va_list ap;

  do {
    olength = length + 1;
    buf = (char *)realloc(buf, olength);
    va_copy(ap, _ap);
    length = vsnprintf(buf, olength, fmt, ap);
    if(length < 0) { // What should we do? Throw?                                                
      strerror_r(errno, buf, olength);
      return std::string(buf, olength);
    }
    va_end(ap);
  } while(length > olength);
  std::string res(buf, length);
  free(buf);
  return res;
}

std::string stringf(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  std::string res = stringf(fmt, ap);
  va_end(ap);

  return res;
}