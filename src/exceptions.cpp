// Email: realyoavperetz@gmail.com


#include "exceptions.hpp"

namespace coup {

CoupException::CoupException(const std::string &msg) : std::logic_error(msg) {}

}
