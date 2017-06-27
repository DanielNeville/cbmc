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
 *
 * This initial implementation only implements support for integers.
 */

#include "interval.h"

#include <util/std_expr.h>
#include <linking/zero_initializer.cpp>
#include <util/symbol_table.h>


typet intervalt::get_type() const
{
  if(!set())
  {
    return nil_typet();
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
  return intervalt(zero()).subtract(*this);
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
//  // Positive / positive
//  exprt lower = plus_exprt(get_lower(), o.get_lower());
//  exprt upper = plus_exprt(get_upper(), o.get_upper());
//
//  return simplified_interval(lower, upper);
  mult_exprt operation;
  return get_extremes(*this, o, operation);
}

intervalt intervalt::divide(const intervalt& o) const
{
  div_exprt operation;
  return get_extremes(*this, o, operation);
}

intervalt intervalt::modulo(const intervalt& o) const
{
  mod_exprt operation;
  return get_extremes(*this, o, operation);

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

//bool intervalt::operator <(const intervalt& lhs, const intervalt& rhs)
//{
//  return intervalt();
//
//}
//
//bool intervalt::operator >(const intervalt& lhs, const intervalt& rhs)
//{
//  return intervalt();
//
//}
//
//bool intervalt::operator <=(const intervalt& lhs, const intervalt& rhs)
//{
//  return intervalt();
//
//}
//
//bool intervalt::operator >=(const intervalt& lhs, const intervalt& rhs)
//{
//  return intervalt();
//
//}
//
//bool intervalt::operator ==(const intervalt& lhs, const intervalt& rhs)
//{
//  return intervalt();
//
//}
//
//bool intervalt::operator !=(const intervalt& lhs, const intervalt& rhs)
//{
//  return intervalt();
//
//}
//
//intervalt intervalt::operator +(const intervalt& lhs, const intervalt& rhs)
//{
//  return intervalt();
//
//}
//
//intervalt intervalt::operator -(const intervalt& lhs, const intervalt& rhs)
//{
//  return intervalt();
//
//}
//
//intervalt intervalt::operator /(const intervalt& lhs, const intervalt& rhs)
//{
//  return intervalt();
//
//}
//
//intervalt intervalt::operator *(const intervalt& lhs, const intervalt& rhs)
//{
//  return intervalt();
//
//}
//
//intervalt intervalt::operator %(const intervalt& lhs, const intervalt& rhs)
//{
//  return intervalt();
//
//}
//
//intervalt intervalt::operator &(const intervalt& lhs, const intervalt& rhs)
//{
//  return intervalt();
//
//}
//
//intervalt intervalt::operator |(const intervalt& lhs, const intervalt& rhs)
//{
//  return intervalt();
//
//}
//
//intervalt intervalt::operator ^(const intervalt& lhs, const intervalt& rhs)
//{
//  return intervalt();
//
//}
//
//intervalt intervalt::operator <<(const intervalt& lhs, const intervalt& rhs)
//{
//  return intervalt();
//
//}
//
//intervalt intervalt::operator >>(const intervalt& lhs, const intervalt& rhs)
//{
//  return intervalt();
//
//}

#include <iostream>

intervalt intervalt::get_extremes(
    const intervalt &a,
    const intervalt &b,
    const exprt operation)
{
  // There are four pair-wise results with arbitrary operation X.
  // 1) L1 X L2, 2) L1 X U2, 3) U1 X L2, 4) U1 X U2
  // 1) a.l X b.l
  // 2) a.l X b.u
  // 3) a.u X b.l
  // 4) a.u X b.u
  // We must return a pair with {min, max} of the above under operation X.

  std::vector<exprt> results(4);

  // Generate all results
  {
    exprt op = operation;
    op.copy_to_operands(a.get_lower(), b.get_lower());
    results.push_back(simplified_expr(op));
  }

  {
    exprt op = operation;
    op.copy_to_operands(a.get_lower(), b.get_upper());
    results.push_back(simplified_expr(op));
  }

  {
    exprt op = operation;
    op.copy_to_operands(a.get_upper(), b.get_lower());
    results.push_back(simplified_expr(op));
  }

  {
    exprt op = operation;
    op.copy_to_operands(a.get_upper(), b.get_upper());
    results.push_back(simplified_expr(op));
  }

  exprt min = get_extreme(results, true);
  exprt max = get_extreme(results, false);

  return simplified_interval(min, max);
}

exprt intervalt::zero() const
{
  symbol_tablet symbol_table;
  const namespacet ns(symbol_table);
  return zero_initializer(get_type(), get_lower().source_location(), ns);
}


exprt intervalt::get_extreme(const std::vector<exprt>& values, bool min)
{
  dstringt op_string = min ? ID_le : ID_ge;
  symbol_tablet symbol_table;
  namespacet ns(symbol_table); // Empty

  if(values.size() == 0)
  {
    return nil_exprt();
  }

  if(values.size() == 1)
  {
    return *(values.begin());
  }

  for(auto left: values)
  {
    bool all_left_OP_right = true;

    for(auto right: values)
    {
      binary_relation_exprt op(left, op_string, right);
      simplify(op, ns);

      if(op.is_true())
      {
        continue;
        // left OP right
      }

      all_left_OP_right = false;
      break;
    }

    if(all_left_OP_right)
    {
      return left;
    }
  }

  return nil_exprt();
}
