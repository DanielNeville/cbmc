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
#include <util/symbol_table.h>
#include <util/namespace.h>
#include <util/arith_tools.h>


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

  // Signed vs unsigned handled via subtract.
  exprt zero = from_integer(mp_integer(0), get_type());
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
//  // Positive / positive
//  exprt lower = plus_exprt(get_lower(), o.get_lower());
//  exprt upper = plus_exprt(get_upper(), o.get_upper());
//
//  return simplified_interval(lower, upper);

//  if(upper_set && o.upper_set && lower_set && o.lower_set)
//  {
    mult_exprt operation;
    return get_extremes(*this, o, operation);
//  }

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

bool is_infinity(const exprt &expr)
{
  return expr.is_nil();
}

bool is_positive(const exprt &expr)
{
  symbol_tablet symbol_table;
  namespacet ns(symbol_table);

  exprt simplified = simplify_expr(expr, ns);

  if(expr.is_nil() || !simplified.is_constant() || expr.get(ID_value) == "")
  {
    return false;
  }

  binary_relation_exprt op(expr, ID_gt, from_integer(0, expr.type()));
  simplify(op, ns);

  return op.is_true();
}

bool is_zero(const exprt &expr)
{
  return expr.is_zero();
}

bool is_negative(const exprt &expr)
{
  symbol_tablet symbol_table;
  namespacet ns(symbol_table);

  exprt simplified = simplify_expr(expr, ns);

  if(expr.is_nil() || !simplified.is_constant() || expr.get(ID_value) == "")
  {
    return false;
  }

  binary_relation_exprt op(expr, ID_lt, from_integer(0, expr.type()));
  simplify(op, ns);

  return op.is_true();
}

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

  intervalt result;

  exprt min = nil_exprt();
  exprt max = nil_exprt();

  /* Positive infinities */
  if((!a.upper_set && !a.lower_set) || (!b.upper_set && !b.lower_set))
  {
    return intervalt(a.get_type());
  }

  // both have at most one infinity.


//  if(!b.upper_set)
//  {
//    assert(b.lower_set);
//
//    if(is_positive(b.lower))
//    {
//
//    }
//  }

  // Generate all results
  if(a.upper_set && b.upper_set && a.lower_set && b.lower_set)
  {
    std::vector<exprt> results;
    results.reserve(4);

    {
      exprt op1 = operation;
      op1.type() = a.get_type();
      op1.copy_to_operands(a.get_lower(), b.get_lower());
      results.push_back(simplified_expr(op1));
    }

    {
      exprt op2 = operation;
      op2.type() = a.get_type();
      op2.copy_to_operands(a.get_lower(), b.get_upper());
      results.push_back(simplified_expr(op2));
    }

    {
      exprt op3 = operation;
      op3.type() = a.get_type();
      op3.copy_to_operands(a.get_upper(), b.get_lower());
      results.push_back(simplified_expr(op3));
    }

    {
      exprt op4 = operation;
      op4.type() = a.get_type();
      op4.copy_to_operands(a.get_upper(), b.get_upper());
      results.push_back(simplified_expr(op4));
    }

    exprt min = get_extreme(results, true);
    exprt max = get_extreme(results, false);

    return simplified_interval(min, max);
  }


  return intervalt();
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
