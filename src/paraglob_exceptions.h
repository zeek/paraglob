// See the file "COPYING" in the main distribution directory for copyright.
// Collection of exceptions thrown by paraglob.
// These allow more specific error handling when dealing with paraglob.
#ifndef PARAGLOB_EXCEPTIONS_H
#define PARAGLOB_EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace paraglob {
  /* Indicates that less data was found than expected. */
  struct underflow_error : public std::underflow_error {
    underflow_error(std::string msg) : std::underflow_error(msg) {}
  };
  /* Indicates that more data was found than expected. */
  struct overflow_error : public std::overflow_error {
    overflow_error(std::string msg) : std::overflow_error(msg) {}
  };
  /* Thrown when a paraglob fails to add a pattern. */
  struct add_error : public std::runtime_error {
    add_error(std::string msg) : std::runtime_error(msg) {}
  };
}

#endif
