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
  mult_exprt operation;
  operation.type()=get_type();
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

intervalt intervalt::bitwise_not() const
{
  return intervalt();

}

tvt intervalt::less_than(const intervalt& o) const
{
  return tvt::unknown();
}

tvt intervalt::greater_than(const intervalt& o) const
{
  return tvt::unknown();

}

tvt intervalt::less_than_or_equal(const intervalt& o) const
{
  return tvt::unknown();

}

tvt intervalt::greater_than_or_equal(const intervalt& o) const
{
  return tvt::unknown();

}

tvt intervalt::equal(const intervalt& o) const
{
  return tvt::unknown();

}

tvt intervalt::not_equal(const intervalt& o) const
{
  return tvt::unknown();

}

intervalt intervalt::increment() const
{
  return add(intervalt(from_integer(mp_integer(1), get_type())));

}

intervalt intervalt::decrement() const
{
  return subtract(intervalt(from_integer(mp_integer(1), get_type())));
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



intervalt intervalt::get_extremes(
    const intervalt &a,
    const intervalt &b,
    const exprt operation)
{
  intervalt result;

  std::vector<exprt> results;

  // If b might be division by zero, set everything to top.
  if(b.contains_zero())
  {
    return intervalt();
  }

  results.push_back(
      generate_expression(a.get_lower(), b.get_lower(), operation));
  results.push_back(
      generate_expression(a.get_lower(), b.get_upper(), operation));
  results.push_back(
      generate_expression(a.get_upper(), b.get_lower(), operation));
  results.push_back(
      generate_expression(a.get_upper(), b.get_upper(), operation));

  exprt min=get_extreme(results, true);
  exprt max=get_extreme(results, false);

  return simplified_interval(min, max);

}

exprt intervalt::get_extreme(std::vector<exprt> values, bool min)
{
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

  typet type=values.begin()->type();

  if(min)
  {
    auto i=values.begin();

    while (i != values.end())
    {
      if(is_min(*i))
      {
        return *i;
      }

      if(is_max(*i))
      {
        i=values.erase(i);
      }
      else
      {
        i++;
      }

      if(values.size() == 0)
      {
        // Everything was max, return max.
        return max_exprt(type);
      }
    }
  }
  else
  {
    auto i=values.begin();

    while (i != values.end())
    {
      if(is_max(*i))
      {
        return *i;
      }

      if(is_min(*i))
      {
        i=values.erase(i);
      }
      else
      {
        i++;
      }

      if(values.size() == 0)
      {
        // Everything was min, return min.
        return min_exprt(type);
      }
    }
  }

  for(auto left: values)
  {
    assert(!is_min(left) && !is_max(left));

    bool all_left_OP_right = true;

    for(auto right: values)
    {
      if((min && less_than_or_equal(left, right)) || (!min && greater_than_or_equal(left, right)))
      {
        continue;
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



exprt intervalt::generate_expression(const exprt& a, const exprt& b, const exprt &operation)
{
  if(operation.id() == ID_mult)
  {
    return generate_multiply_expression(a, b, operation);
  }

  if(operation.id() == ID_div)
  {
    return generate_division_expression(a, b, operation);
  }

  assert(0 && "Not yet implemented!");
}

exprt intervalt::generate_multiply_expression(const exprt& a, const exprt& b,
    exprt operation)
{
  assert(operation.id() == ID_mult);
  assert(operation.type().is_not_nil() && is_int(operation.type()));

  if(is_max(a))
  {
    return generate_multiply_expression_max(b);
  }

  if(is_max(b))
  {
    return generate_multiply_expression_max(a);
  }

  if(is_min(a))
  {
    return generate_multiply_expression_min(a, b);
  }

  if(is_min(b))
  {
    return generate_multiply_expression_min(b, a);
  }

  assert(!is_extreme(a) && !is_extreme(b));

  operation.copy_to_operands(a, b);
  return simplified_expr(operation);
}


exprt intervalt::generate_multiply_expression_max(const exprt &expr)
{
  if(is_max (expr))
  {
    return max_exprt(expr);
  }

  if(is_min (expr))
  {
    if(is_negative(expr))
    {
      return min_exprt(expr);
    }
    else
    {
      assert(!is_positive(expr) && "Min value cannot be >0.");
      assert(is_zero(expr) && "Non-negative MIN must be zero.");

      return expr;
    }
  }

  assert(!is_extreme(expr));

  if(is_negative (expr))
  {
    return min_exprt(expr);
  }

  if(is_zero (expr))
  {
    return expr;
  }

  if(is_positive (expr))
  {
    return max_exprt(expr);
  }

  assert(0 && "Unreachable.");
  return nil_exprt();
}

exprt intervalt::generate_multiply_expression_min(const exprt &min, const exprt &other)
{
  assert(is_min(min));

  if(is_max(other))
  {
    if(is_negative(min))
    {
      return min_exprt(min);
    }
    else
    {
      assert(!is_positive(min) && "Min value cannot be >0.");
      assert(is_zero(min) && "Non-negative MIN must be zero.");

      return min;
    }
  }

  if(is_min(other))
  {
    assert(!is_positive(min) && !is_positive(other)  && "Min value cannot be >0.");
    assert(is_negative(other) || is_zero(other));

    if(is_negative(min) && is_negative(other))
    {
      return max_exprt(min);
    }

    assert(is_zero(min) || is_zero(other));
    return (is_zero(min) ? min : other);
  }

  assert(0 && "Unreachable.");
  return nil_exprt();
}



exprt intervalt::generate_division_expression(const exprt& a, const exprt& b,
    exprt operation)
{
  assert(!is_zero(b));

  if(b.is_one())
  {
    return a;
  }

  if(is_max(a)) {
    if(is_negative(b))
    {
      return min_exprt(a);
    }

    return a;
  }

  if(is_min(a))
  {
    if(is_negative(b))
    {
      return max_exprt(a);
    }

    return a;
  }

  assert(!is_extreme(a));

  if(is_max(b))
  {
    return zero(b);
  }

  if(is_min(b))
  {
    assert(is_signed(b));
    return zero(b);
  }

  assert(!is_extreme(a) && !is_extreme(b));

  operation.copy_to_operands(a, b);
  return simplified_expr(operation);
}

bool intervalt::contains_extreme(const exprt expr)
{
  forall_operands(it, expr)
  {
    if(is_extreme(*it))
    {
      return true;
    }

    if(it->has_operands())
    {
      return contains_extreme(*it);
    }
  }

  return false;
}

bool intervalt::contains_zero() const
{
  if(!is_numeric())
  {
    return false;
  }

  if(get_lower().is_zero() || get_upper().is_zero())
  {
    return true;
  }

  if(is_unsigned() && is_min(get_lower()))
  {
    return true;
  }

  if(less_than_or_equal(get_lower(), zero()) && greater_than_or_equal(get_upper(), zero()))
  {
    return true;
  }

  return false;
}

