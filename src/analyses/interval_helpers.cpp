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

bool intervalt::is_int() const
{
  return is_int(get_type());
}

bool intervalt::is_float() const
{
  return is_float(get_type());
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
  return (expr.id() == ID_max || expr.id() == ID_min);
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

  binary_relation_exprt op(expr, ID_gt, from_integer(0, expr.type()));
  simplify(op, ns);

  return op.is_true();
}

bool intervalt::is_zero(const exprt &expr)
{
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

  if(is_min(expr) && intervalt::is_signed(expr))
  {
    return true;
  }

  binary_relation_exprt op(expr, ID_lt, from_integer(0, expr.type()));
  simplify(op, ns);

  return op.is_true();
}
