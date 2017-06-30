#include "interval.h"

#include <util/std_expr.h>
#include <util/symbol_table.h>
#include <util/namespace.h>
#include <util/arith_tools.h>
#include <ostream>
#include <sstream>

typet intervalt::get_type() const
{
  return type;
}

/* we don't simplify in the constructor otherwise */
intervalt intervalt::simplified_interval(exprt &l, exprt &r)
{
  return intervalt(simplified_expr(l), simplified_expr(r));
}

exprt intervalt::simplified_expr(exprt expr)
{
  symbol_tablet symbol_table;
  const namespacet ns(symbol_table);

  assert(!contains_extreme(expr));

  return simplify_expr(expr, ns);
}

/* Don't allow different types in upper and lower */
typet intervalt::calculate_type(const exprt &l, const exprt &u) const
{
  if(u.type() != l.type())
  {
    std::cout << "ERROR!\n1)\n" << l.pretty() << "\n2)\n" << u.pretty() << "\n";
    assert(0 && "Cannot support mixed types.");
  }

  assert(u.type() == l.type());

  return u.type();
}


constant_exprt intervalt::zero(const typet& type)
{
  constant_exprt zero = from_integer(mp_integer(0), type);
  assert(zero.is_zero()); // NOT is_zero(zero) (inf. recursion
  return zero;
}

constant_exprt intervalt::zero(const exprt& expr)
{
  return zero(expr.type());
}

constant_exprt intervalt::zero() const
{
  return zero(get_type());
}

min_exprt intervalt::min() const
{
  return min_exprt(get_type());
}

max_exprt intervalt::max() const
{
  return max_exprt(get_type());
}

intervalt intervalt::swap(intervalt &i)
{
  return intervalt(i.get_upper(), i.get_lower());
}

intervalt intervalt::swap() const
{
  return intervalt(get_lower(), get_upper());
}

/* Helpers */

bool intervalt::is_int() const
{
  return is_int(get_type());
}

bool intervalt::is_float() const
{
  return is_float(get_type());
}

bool intervalt::is_numeric(const typet &type)
{
  return is_int(type) || is_float(type);
}

bool intervalt::is_numeric() const
{
  return is_numeric(get_type());
}

bool intervalt::is_numeric(const exprt &expr)
{
  return is_numeric(expr.type());
}

bool intervalt::is_numeric(const intervalt &interval)
{
  return interval.is_numeric();
}

bool intervalt::is_int(const typet &type)
{
  return (is_signed(type) || is_unsigned(type));
}

bool intervalt::is_float(const typet &src)
{
  return src.id() == ID_floatbv;
}

bool intervalt::is_int(const exprt &expr)
{
  return is_int(expr.type());
}

bool intervalt::is_int(const intervalt &interval)
{
  return interval.is_int();
}

bool intervalt::is_float(const exprt &expr)
{
  return is_float(expr.type());
}

bool intervalt::is_bitvector(const typet &t)
{
  return t.id() == ID_bv || t.id() == ID_signedbv || t.id() == ID_unsignedbv
      || t.id() == ID_pointer || t.id() == ID_bool;
}

bool intervalt::is_signed(const typet &t)
{
  return t.id() == ID_signedbv;
}

bool intervalt::is_unsigned(const typet &t)
{
  return t.id() == ID_bv || t.id() == ID_unsignedbv || t.id() == ID_pointer
      || t.id() == ID_bool;
}

bool intervalt::is_signed(const intervalt &interval)
{
  return is_signed(interval.get_type());
}

bool intervalt::is_unsigned(const intervalt &interval)
{
  return is_unsigned(interval.get_type());
}

bool intervalt::is_bitvector(const intervalt &interval)
{
  return is_bitvector(interval.get_type());
}

bool intervalt::is_signed(const exprt &expr)
{
  return is_signed(expr.type());
}

bool intervalt::is_unsigned(const exprt &expr)
{
  return is_unsigned(expr.type());
}

bool intervalt::is_bitvector(const exprt &expr)
{
  return is_bitvector(expr.type());
}

bool intervalt::is_signed() const
{
  return is_signed(get_type());
}

bool intervalt::is_unsigned() const
{
  return is_unsigned(get_type());
}

bool intervalt::is_bitvector() const
{
  return is_bitvector(get_type());
}

bool intervalt::is_extreme(const exprt &expr)
{
  return (is_max(expr) || is_min(expr));
}

bool intervalt::is_extreme(const exprt &expr1, const exprt &expr2)
{
  return is_extreme(expr1) || is_extreme(expr2);
}

bool intervalt::is_max() const
{
  return is_max(get_upper());
}

bool intervalt::is_min() const
{
  return is_min(get_lower());
}

bool intervalt::is_max(const exprt &expr)
{
  return expr.id() == ID_max;
}

bool intervalt::is_min(const exprt &expr)
{
  return expr.id() == ID_min;
}

bool intervalt::is_positive(const exprt &expr)
{
  symbol_tablet symbol_table;
  namespacet ns(symbol_table);

  exprt simplified=simplify_expr(expr, ns);

  if(expr.is_nil() || !simplified.is_constant() || expr.get(ID_value) == "")
  {
    return false;
  }

  if(is_max(expr))
  {
    return true;
  }

  if(is_min(expr))
  {
    return false;
  }

  assert(!is_max(expr) && !is_min(expr));

  binary_relation_exprt op(expr, ID_gt, from_integer(0, expr.type()));
  simplify(op, ns);

  return op.is_true();
}

bool intervalt::is_zero(const exprt &expr)
{
  if(is_min(expr))
  {
    if(is_unsigned(expr))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  if(is_max(expr))
  {
    return false;
  }

  assert(!is_max(expr) && !is_min(expr));

  if(expr.is_zero())
  {
    return true;
  }

  return equal(expr, zero(expr));
}

bool intervalt::is_negative(const exprt &expr)
{
  symbol_tablet symbol_table;
  namespacet ns(symbol_table);

  exprt simplified=simplify_expr(expr, ns);

  if(expr.is_nil() || !simplified.is_constant() || expr.get(ID_value) == "")
  {
    return false;
  }

  if(is_min(expr))
  {
    if(is_signed(expr))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  if(is_max(expr))
  {
    return false;
  }

  assert(!is_max(expr) && !is_min(expr));

  binary_relation_exprt op(expr, ID_lt, from_integer(0, expr.type()));
  simplify(op, ns);

  return op.is_true();
}

bool intervalt::equal(const exprt& a, const exprt& b)
{
  if(a == b)
  {
    return true;
  }

  if(!is_numeric(a) || !is_numeric(b))
  {
    // Best we can do now is a==b?, but this is covered by the above, so always false.
    assert(!(a == b));
    return false;
  }

  if(is_max(a) && is_max(b))
  {
    return true;
  }

  exprt l=(is_min(a) && is_unsigned(a)) ? zero(a) : a;
  exprt r=(is_min(b) && is_unsigned(b)) ? zero(b) : b;

  if((is_min(a) && is_min(b)))
  {
    return true;
  }

  if(is_extreme(a, b))
  {
    assert(((is_max(a) && is_min(b)) || (is_min(b) && is_max(a))));
    // If they're still extreme, they're opposites.
    return false;
  }

  return simplified_expr(equal_exprt(a, b)).is_true();
}

// TODO: Signed/unsigned comparisons.
bool intervalt::less_than(const exprt& a, const exprt& b)
{
  if(!is_numeric(a) || !is_numeric(b))
  {
    return false;
  }

  exprt l=(is_min(a) && is_unsigned(a)) ? zero(a) : a;
  exprt r=(is_min(b) && is_unsigned(b)) ? zero(b) : b;

  if(is_min(l) && !is_min(r))
  {
    return true;
  }

  if((is_max(l) && is_max(r)) || (is_min(l) && is_min(r)))
  {
    return false;
  }

  if((is_min(l) || is_zero(l)) && is_max(r))
  {
    return true;
  }

  assert(!is_extreme(l, r));

  return simplified_expr(binary_relation_exprt(l, ID_lt, r)).is_true();
}

bool intervalt::greater_than(const exprt &a, const exprt &b)
{
  if(!is_numeric(a) || !is_numeric(b))
  {
    return false;
  }

  exprt l=(is_min(a) && is_unsigned(a)) ? zero(a) : a;
  exprt r=(is_min(b) && is_unsigned(b)) ? zero(b) : b;

  if(is_max(l) && !is_max(r))
  {
    return true;
  }

  if((is_max(l) && is_max(r)) || (is_min(l) && is_min(r)))
  {
    return false;
  }

  if(is_min(l) && is_max(r))
  {
    return false;
  }

  assert(!is_extreme(l) && !is_extreme(r));

  return simplified_expr(binary_relation_exprt(l, ID_gt, r)).is_true();
}

bool intervalt::less_than_or_equal(const exprt& a, const exprt& b)
{
  return less_than(a, b) || equal(a, b);
}

bool intervalt::greater_than_or_equal(const exprt& a, const exprt& b)
{
  return greater_than(a, b) || equal(a, b);
}

bool intervalt::not_equal(const exprt &a, const exprt &b)
{
  return !equal(a, b);
}

bool intervalt::contains(const intervalt& interval) const
{
  // [a, b] Contains [c, d] If c >= a and d <= b.
  return(
      less_than_or_equal(interval.get_upper(), get_upper())
      &&
      greater_than_or_equal(interval.get_lower(), get_lower())
  );
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

tvt operator< (const intervalt &lhs, const intervalt &rhs)
{
  return lhs.less_than(rhs);
}

tvt operator> (const intervalt &lhs, const intervalt &rhs)
{
  return lhs.greater_than(rhs);
}

tvt operator<=(const intervalt &lhs, const intervalt &rhs)
{
  return lhs.less_than_or_equal(rhs);
}

tvt operator>=(const intervalt &lhs, const intervalt &rhs)
{
  return lhs.greater_than(rhs);
}

tvt operator==(const intervalt &lhs, const intervalt &rhs)
{
  return lhs.equal(rhs);
}

tvt operator!=(const intervalt &lhs, const intervalt &rhs)
{
  return lhs.not_equal(rhs);
}

intervalt operator+(const intervalt &lhs, const intervalt &rhs)
{
  return lhs.add(rhs);
}

intervalt operator-(const intervalt &lhs, const intervalt &rhs)
{
  return lhs.subtract(rhs);
}

intervalt operator/(const intervalt &lhs, const intervalt &rhs)
{
  return lhs.divide(rhs);
}

intervalt operator*(const intervalt &lhs, const intervalt &rhs)
{
  return lhs.multiply(rhs);
}

intervalt operator%(const intervalt &lhs, const intervalt &rhs)
{
  return lhs.modulo(rhs);
}

intervalt operator&(const intervalt &lhs, const intervalt &rhs)
{
  return lhs.bitwise_and(rhs);
}

intervalt operator|(const intervalt &lhs, const intervalt &rhs)
{
  return lhs.bitwise_or(rhs);
}

intervalt operator^(const intervalt &lhs, const intervalt &rhs)
{
  return lhs.bitwise_xor(rhs);
}

intervalt operator!(const intervalt &lhs)
{
  return lhs.bitwise_not();
}

intervalt operator<<(const intervalt &lhs, const intervalt &rhs)
{
  return lhs.left_shift(rhs);
}

intervalt operator>>(const intervalt &lhs, const intervalt &rhs)
{
  return lhs.right_shift(rhs);
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

bool intervalt::contains_extreme(const exprt expr1, const exprt expr2)
{
  return contains_extreme(expr1) || contains_extreme(expr2);
}

bool intervalt::is_empty() const
{
  if(greater_than(get_lower(), get_upper()))
  {
    return false;
  }

  return true;
}

bool intervalt::is_constant() const
{
  return equal(get_lower(), get_upper());
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
