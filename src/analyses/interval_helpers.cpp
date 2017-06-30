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
bool intervalt::is_numeric() const
{
  return is_int(get_type()) || is_float(get_type());
}

bool intervalt::is_int() const
{
  return is_int(get_type());
}

bool intervalt::is_float() const
{
  return is_float(get_type());
}

bool intervalt::is_numeric(const typet &type) const
{
  return is_int(type) || is_float(type);
}

bool intervalt::is_int(const typet &type)
{
  return (is_signed(type) || is_unsigned(type));
}

bool intervalt::is_float(const typet &src)
{
  return src.id() == ID_floatbv;
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
  return expr.is_zero();
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
  if(is_max(a) && is_max(b))
  {
    return true;
  }

  if(is_min(a) && is_min(b))
  {
    return true;
  }

  if(is_extreme(a) || is_extreme(b))
  {
    // If they're still extreme, they're opposites.
    return false;
  }

  return simplified_expr(equal_exprt(a, b)).is_true();
}

bool intervalt::less_than(const exprt& a, const exprt& b)
{
  if((is_max(a) && is_max(b)) || (is_min(a) && is_min(b)))
  {
    return false;
  }

  if(is_min(a) && is_max(b))
  {
    return true;
  }

  if(is_extreme(a) || is_extreme(b))
  {
    return false;
  }

  return simplified_expr(binary_relation_exprt(a, ID_lt, b)).is_true();
}

bool intervalt::more_than(const exprt& a, const exprt& b)
{
  if((is_max(a) && is_max(b)) || (is_min(a) && is_min(b)))
  {
    return false;
  }

  if(is_min(a) && is_max(b))
  {
    return true;
  }

  if(is_extreme(a) || is_extreme(b))
  {
    return false;
  }

  return simplified_expr(binary_relation_exprt(a, ID_gt, b)).is_true();
}

bool intervalt::less_than_or_equal(const exprt& a, const exprt& b)
{
  return less_than(a, b) || equal(a, b);
}

bool intervalt::more_than_or_equal(const exprt& a, const exprt& b)
{
  return more_than(a, b) || equal(a, b);
}

bool intervalt::not_equal(const exprt &a, const exprt &b)
{
  return !equal(a, b);
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
