/*
 * interval.h
 *
 *  Created on: 16 Jun 2017
 *      Author: dan
 */

#ifndef SRC_ANALYSES_INTERVAL_H_
#define SRC_ANALYSES_INTERVAL_H_

#include <util/expr.h>
#include <util/simplify_expr.h>
#include <util/std_types.h>
#include <util/std_expr.h>
#include <util/symbol_table.h>
#include <util/namespace.h>
#include <util/arith_tools.h>

#include "interval_template.h"

#include <ostream>
#include <sstream>
#include <iostream>

/*
 *
 * Upper set = Specific max (otherwise INF)
 * Lower set = Specific min (otherwise INF)
 */

class max_exprt:public exprt
{
public:
  explicit max_exprt(const typet &_type):
    exprt(ID_max, _type)
  {
  }

  explicit max_exprt(const exprt &_expr):
    exprt(ID_max, _expr.type())
  {
  }
};

class min_exprt:public exprt
{
public:
  explicit min_exprt(const typet &_type):
    exprt(ID_min, _type)
  {
  }

  explicit min_exprt(const exprt &_expr):
    exprt(ID_min, _expr.type())
  {
  }
};



class intervalt
{
public:
  intervalt():
    lower(min_exprt(nil_typet())),
    upper(max_exprt(nil_typet())),
    type(nil_typet())
  {
  }

  explicit intervalt(const exprt &x):
    lower(x),
    upper(x),
    type(x.type())
  {

  }

  intervalt(const typet type_):
    lower(min_exprt(type_)),
    upper(max_exprt(type_)),
    type(type_)
  {
  }

  intervalt(const exprt &l, const exprt &u):
    lower(l),
    upper(u)
  {
    set_type(l, u);
  }



  /* Unary */
  intervalt add() const;
  intervalt minus() const;

  /* Binary */
  intervalt add(const intervalt &o) const;
  intervalt subtract(const intervalt &o) const;
  intervalt multiply(const intervalt &o) const;
  intervalt divide(const intervalt &o) const;
  intervalt modulo(const intervalt &o) const;

  /* Binary shifts */
  intervalt left_shift(const intervalt &o) const;
  intervalt right_shift(const intervalt &o) const;

  /* Binary bitwise */
  intervalt bitwise_xor(const intervalt &o) const;
  intervalt bitwise_or(const intervalt &o) const;
  intervalt bitwise_and(const intervalt &o) const;
  intervalt bitwise_not(const intervalt &o) const;

  intervalt less_than(const intervalt &o) const;

  intervalt greater_than(const intervalt &o) const;
  intervalt less_than_or_equal(const intervalt &o) const;
  intervalt greater_than_or_equal(const intervalt &o) const;
  intervalt equal(const intervalt &o) const;
  intervalt not_equal(const intervalt &o) const;

  intervalt increment(const intervalt &o) const;
  intervalt decrement(const intervalt &o) const;


  const exprt &get_lower() const
  {
    return lower;
  }

  const exprt &get_upper() const
  {
    return upper;
  }

//  bool operator< (const intervalt &lhs, const intervalt &rhs);
//  bool operator> (const intervalt &lhs, const intervalt &rhs);
//  bool operator<=(const intervalt &lhs, const intervalt &rhs);
//  bool operator>=(const intervalt &lhs, const intervalt &rhs);
//  bool operator==(const intervalt &lhs, const intervalt &rhs);
//  bool operator!=(const intervalt &lhs, const intervalt &rhs);

//  intervalt operator+(const intervalt &lhs, const intervalt &rhs);
//  intervalt operator-(const intervalt &lhs, const intervalt &rhs);
//  intervalt operator/(const intervalt &lhs, const intervalt &rhs);
//  intervalt operator*(const intervalt &lhs, const intervalt &rhs);
//  intervalt operator%(const intervalt &lhs, const intervalt &rhs);
//  intervalt operator&(const intervalt &lhs, const intervalt &rhs);
//  intervalt operator|(const intervalt &lhs, const intervalt &rhs);
//  intervalt operator^(const intervalt &lhs, const intervalt &rhs);
//  intervalt operator<<(const intervalt &lhs, const intervalt &rhs);
//  intervalt operator>>(const intervalt &lhs, const intervalt &rhs);

  friend std::ostream& operator<< (std::ostream& out, const intervalt &i);

  std::string to_string() const;

  bool valid()
  {
    return true;
  }

  static intervalt top()
  {
    return intervalt();
  }

  /* Private? */

  static intervalt get_extremes(const intervalt &lhs, const intervalt &rhs, const exprt operation);

  static exprt get_extreme(const std::vector<exprt> &values, bool min = true);

  /* we don't simplify in the constructor otherwise */
  static intervalt simplified_interval(exprt &l, exprt &r)
  {
    return intervalt(simplified_expr(l), simplified_expr(r));
  }


  static exprt simplified_expr(exprt expr)
  {
    symbol_tablet symbol_table;
    const namespacet ns(symbol_table);

    return simplify_expr(expr, ns);
  }

  /* Don't allow different types in upper and lower */
  typet get_type() const;

  void set_type() { type=nil_typet(); }
  void set_type(const typet type_) { type=type_; }

  void set_type(const exprt &e) { type=e.type(); }
  // More flexible in future?
  void set_type(const exprt &l, const exprt &u)
  {
    if(u.type() != l.type())
    {
      std::cout << "ERROR!\n1)\n" << l.pretty() << "\n2)\n" << u.pretty() << "\n";
      assert(0 && "Cannot support mixed types." );
    }

    assert(u.type() == l.type());

    type = u.type();
  }

  min_exprt min() const
  {
    return min_exprt(get_type());
  }

  max_exprt max() const
  {
    return max_exprt(get_type());
  }

  static intervalt swap(intervalt &i)
  {
    return intervalt(i.get_upper(), i.get_lower());
  }

  intervalt swap() const
  {
    return intervalt(get_lower(), get_upper());
  }


  /* Helpers */

  bool is_int() const
  {
    return is_int(get_type());
  }

  bool is_float() const
  {
    return is_float(get_type());
  }

  static bool is_int(const typet &type)
  {
    return (is_signed(type) || is_unsigned(type));
  }

  static bool is_float(const typet &src)
  {
    return src.id()==ID_floatbv;
  }

  static bool is_bitvector(const typet &t)
  {
    return t.id()==ID_bv ||
           t.id()==ID_signedbv ||
           t.id()==ID_unsignedbv ||
           t.id()==ID_pointer ||
           t.id()==ID_bool;
  }

  static bool is_signed(const typet &t)
  {
    return t.id()==ID_signedbv;
  }

  static bool is_unsigned(const typet &t)
  {
    return t.id()==ID_bv ||
           t.id()==ID_unsignedbv ||
           t.id()==ID_pointer ||
           t.id()==ID_bool;
  }

  static bool is_signed(const intervalt &interval)
  {
    return is_signed(interval.get_type());
  }

  static bool is_unsigned(const intervalt &interval)
  {
    return is_unsigned(interval.get_type());
  }

  static bool is_bitvector(const intervalt &interval)
  {
    return is_bitvector(interval.get_type());
  }

  static bool is_signed(const exprt &expr)
  {
    return is_signed(expr.type());
  }

  static bool is_unsigned(const exprt &expr)
  {
    return is_unsigned(expr.type());
  }

  static bool is_bitvector(const exprt &expr)
  {
    return is_bitvector(expr.type());
  }

  bool is_signed() const
  {
    return is_signed(get_type());
  }

  bool is_unsigned() const
  {
    return is_unsigned(get_type());
  }

  bool is_bitvector() const
  {
    return is_bitvector(get_type());
  }


  static bool is_extreme(const exprt &expr)
  {
    return (expr.id() == ID_max || expr.id() == ID_min);
  }

  bool is_max() const
  {
    return is_max(get_upper());
  }

  bool is_min() const
  {
    return is_min(get_lower());
  }

  static bool is_max(const exprt &expr)
  {
    return expr.id() == ID_max;
  }

  static bool is_min(const exprt &expr)
  {
    return expr.id() == ID_min;
  }

  static bool is_positive(const exprt &expr)
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

  static bool is_zero(const exprt &expr)
  {
    return expr.is_zero();
  }

  static bool is_negative(const exprt &expr)
  {
    symbol_tablet symbol_table;
    namespacet ns(symbol_table);

    exprt simplified = simplify_expr(expr, ns);

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


private:

  /* This is the entirety */
  exprt lower;
  exprt upper;
  typet type;
};

#endif /* SRC_ANALYSES_INTERVAL_H_ */




//  void transform(const exprt &o);
//
//  // Unary minus, not subtract
//  void minus(const exprt &o);
//
//  void add(const exprt &o);
//  void subtract(const exprt &o);
//  void multiply(const exprt &o);
//  void divide(const exprt &o);
//  void modulo(const exprt &o);
//
//  void left_shift(const exprt &o);
//  void right_shift(const exprt &o);
//
//  void bitwise_xor(const exprt &o);
//  void bitwise_or(const exprt &o);
//  void bitwise_and(const exprt &o);
//  void bitwise_not(const exprt &o);
/*+a
-a
a + b
a - b
a * b
a / b
a % b
~a
a & b
a | b
a ^ b
a << b
a >> b*/


  /*
   * a == b
a != b
a < b
a > b
a <= b
a >= b */

