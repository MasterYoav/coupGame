// Email: realyoavperetz@gmail.com
// Implementation of CoupException constructor.

#include "exceptions.hpp"

namespace coup {

CoupException::CoupException(const std::string &msg) : std::logic_error(msg) {}

}
