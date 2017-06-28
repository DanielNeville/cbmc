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
#include <ostream>
#include <sstream>


//make clean -s && make -j 7 CXX="/usr/local/bin/ccache g++" -s && ./unit_tests

typet intervalt::get_type() const
{
  return type;
}

intervalt intervalt::add() const
{
  return *this;
}

intervalt intervalt::minus() const
{
  exprt lower;
  exprt upper;

  if(is_max())
  {
    lower=min();
  }
  else
  {
    lower = simplified_expr(unary_minus_exprt(get_upper()));
  }

  if(is_min())
  {
    upper=max();
  }
  else
  {
    upper = simplified_expr(unary_minus_exprt(get_lower()));
  }

  return intervalt(lower, upper);
}

intervalt intervalt::add(const intervalt& o) const
{
  exprt lower = nil_exprt();
  exprt upper = nil_exprt();

  if(is_max(get_upper()) || is_max(o.get_upper()))
  {
    upper = max_exprt(get_type());
  }
  else
  {
    assert(!is_max(get_upper()) && !is_max(o.get_upper()));
    upper = simplified_expr(plus_exprt(get_upper(), o.get_upper()));
  }

  if(is_min(get_lower()) || is_min(o.get_lower()))
  {
    lower = min_exprt(get_type());
  }
  else
  {
    assert(!is_min(get_lower()) && !is_min(o.get_lower()));
    lower = simplified_expr(plus_exprt(get_lower(), o.get_lower()));
  }

  return simplified_interval(lower, upper);
}

intervalt intervalt::subtract(const intervalt& o) const
{
  // e.g. [t.u - o.l, t.l - o.u]
  return add(o.minus().swap());
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
//  if((!a.upper_set && !a.lower_set) || (!b.upper_set && !b.lower_set))
//  {
//    return intervalt(a.get_type());
//  }

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
  if(!is_extreme(a.get_lower()) && !is_extreme(b.get_lower()) && !is_extreme(a.get_upper()) && !is_extreme(b.get_upper()))
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


std::string intervalt::to_string() const
{
  std::stringstream out;

  out << dstringt("[");

  if(!is_min())
  {
    out << get_lower().get(ID_value);
  }
  else
  {
    if(is_signed(get_lower()))
    {
      out << dstringt("MIN");
    }
    else
    {
      out << dstringt("0");
    }
  }

  out << dstringt(",");

  if(!is_max())
  {
    out << get_upper().get(ID_value);
  }
  else
    out << dstringt("MAX");

  out << dstringt("]");

  return out.str();
}

std::ostream& operator <<(std::ostream& out,
    const intervalt& i)
{
  out << i.to_string();

  return out;
}


