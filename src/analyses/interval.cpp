/*
 * interval.cpp
 *
 *  Created on: 16 Jun 2017
 *      Author: dan
 */

/*
 *
 * Types:  Should we store a type for the entire interval?
 * IMO: I think YES (for the case where we have -inf -> inf, we don't know otherwise
 */

#include "interval.h"

typet intervalt::get_type() const
{
  if(!set())
  {
    return nil_typet;
  }

  assert(set());
  assert(get_lower().type() == get_upper().type());

  return get_lower().type();
}

intervalt intervalt::add() const
{
  return *this;
}

intervalt intervalt::minus() const
{
  assert(set());
  return intervalt(zero).subtract(*this);
}

intervalt intervalt::add(const intervalt& o) const
{
  exprt lower = plus_exprt(get_lower(), o.get_lower());
  exprt upper = plus_exprt(get_upper(), o.get_upper());

  return simplified_interval(lower, upper);
}

intervalt intervalt::subtract(const intervalt& o) const
{
  // e.g. [t.u - o.l, t.l - o.u]
  exprt lower = minus_exprt(get_lower(), o.get_upper());
  exprt upper = minus_exprt(get_upper(), o.get_lower());

  return simplified_interval(lower, upper);
}

intervalt intervalt::multiply(const intervalt& o) const
{
  // Positive / positive
  exprt lower = plus_exprt(get_lower(), o.get_lower());
  exprt upper = plus_exprt(get_upper(), o.get_upper());

  return simplified_interval(lower, upper);
}

intervalt intervalt::divide(const intervalt& o) const
{

  return intervalt();
}

intervalt intervalt::modulo(const intervalt& o) const
{
  return intervalt();

}

intervalt intervalt::left_shift(const intervalt& o) const
{
  return intervalt();

}

intervalt intervalt::right_shift(const intervalt& o) const
{
  return intervalt();

}

intervalt intervalt::bitwise_xor(const intervalt& o) const
{
  return intervalt();

}

intervalt intervalt::bitwise_or(const intervalt& o) const
{
  return intervalt();

}

intervalt intervalt::bitwise_and(const intervalt& o) const
{
  return intervalt();

}

intervalt intervalt::bitwise_not(const intervalt& o) const
{
  return intervalt();

}

intervalt intervalt::less_than(const intervalt& o) const
{
  return intervalt();

}

intervalt intervalt::greater_than(const intervalt& o) const
{
  return intervalt();

}

intervalt intervalt::less_than_or_equal(const intervalt& o) const
{
  return intervalt();

}

intervalt intervalt::greater_than_or_equal(const intervalt& o) const
{
  return intervalt();

}

intervalt intervalt::equal(const intervalt& o) const
{
  return intervalt();

}

intervalt intervalt::not_equal(const intervalt& o) const
{
  return intervalt();

}

intervalt intervalt::increment(const intervalt& o) const
{
  return intervalt();

}

intervalt intervalt::decrement(const intervalt& o) const
{
  return intervalt();

}

bool intervalt::operator <(const intervalt& lhs, const intervalt& rhs)
{
  return intervalt();

}

bool intervalt::operator >(const intervalt& lhs, const intervalt& rhs)
{
  return intervalt();

}

bool intervalt::operator <=(const intervalt& lhs, const intervalt& rhs)
{
  return intervalt();

}

bool intervalt::operator >=(const intervalt& lhs, const intervalt& rhs)
{
  return intervalt();

}

bool intervalt::operator ==(const intervalt& lhs, const intervalt& rhs)
{
  return intervalt();

}

bool intervalt::operator !=(const intervalt& lhs, const intervalt& rhs)
{
  return intervalt();

}

intervalt intervalt::operator +(const intervalt& lhs, const intervalt& rhs)
{
  return intervalt();

}

intervalt intervalt::operator -(const intervalt& lhs, const intervalt& rhs)
{
  return intervalt();

}

intervalt intervalt::operator /(const intervalt& lhs, const intervalt& rhs)
{
  return intervalt();

}

intervalt intervalt::operator *(const intervalt& lhs, const intervalt& rhs)
{
  return intervalt();

}

intervalt intervalt::operator %(const intervalt& lhs, const intervalt& rhs)
{
  return intervalt();

}

intervalt intervalt::operator &(const intervalt& lhs, const intervalt& rhs)
{
  return intervalt();

}

intervalt intervalt::operator |(const intervalt& lhs, const intervalt& rhs)
{
  return intervalt();

}

intervalt intervalt::operator ^(const intervalt& lhs, const intervalt& rhs)
{
  return intervalt();

}

intervalt intervalt::operator <<(const intervalt& lhs, const intervalt& rhs)
{
  return intervalt();

}

intervalt intervalt::operator >>(const intervalt& lhs, const intervalt& rhs)
{
  return intervalt();

}
