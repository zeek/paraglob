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
    using std::underflow_error::underflow_error;
  };
  /* Indicates that more data was found than expected. */
  struct overflow_error : public std::overflow_error {
    using std::overflow_error::overflow_error;
  };
  /* Thrown when a paraglob fails to add a pattern. */
  struct add_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
  };
}

#endif
