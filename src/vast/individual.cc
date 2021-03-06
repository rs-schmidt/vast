#include "vast/individual.h"
#include "vast/serialization.h"

namespace vast {

individual::individual(uuid id)
  : id_(id)
{
}

uuid const& individual::id() const
{
  return id_;
}

void individual::id(uuid id)
{
  id_ = std::move(id);
}

void individual::serialize(serializer& sink) const
{
  sink << id_;
}

void individual::deserialize(deserializer& source)
{
  source >> id_;
}

bool operator<(individual const& x, individual const& y)
{
  return x.id() < y.id();
}

bool operator==(individual const& x, individual const& y)
{
  return x.id() == y.id();
}

} // namespace vast
