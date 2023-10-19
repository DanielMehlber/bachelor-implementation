#include "common/uuid/UuidGenerator.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace common::uuid;

std::string UuidGenerator::Random() noexcept {
  return boost::uuids::to_string(boost::uuids::random_generator()());
}
