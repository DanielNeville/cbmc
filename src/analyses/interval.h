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
#include <util/symbol_table.h>
#include <util/namespace.h>

#include "interval_template.h"

/*
 *
 * Upper set = Specific max (otherwise INF)
 * Lower set = Specific min (otherwise INF)
 */

class intervalt:
    public interval_templatet<exprt>
{
public:
  intervalt()
      : interval_templatet<exprt>()
  {
  }

  explicit intervalt(const exprt &x)
      : interval_templatet<exprt>(x)
  {
  }

  intervalt(const exprt &l, const exprt &u)
      :
        interval_templatet<exprt>(l, u)
  {
    set_type(l, u);
  }



  /* Decide names later */

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


  bool valid()
  {
    return set();
  }

  intervalt top()
  {
    return intervalt();
  }


  /* Private? */

  static intervalt get_extremes(const intervalt &lhs, const intervalt &rhs, const exprt operation);

  static exprt get_extreme(const std::vector<exprt> &values, bool min = true);

private:
  typet type;

  /* Don't allow different types in upper and lower */
  typet get_type() const;
  void set_type() { type=nil_typet(); }
  void set_type(exprt &e) { type=e.type(); }
  // More flexible in future?
  void set_type(const exprt &l, const exprt &u) { assert(l.type() == u.type()); type=u.type(); }


  bool set() const
  {
    return lower_set && upper_set;
  }

  /* we don't simplify in the constructor otherwise */
  static intervalt simplified_interval(exprt &l, exprt &r)
  {
    return intervalt(simplified_expr(l), simplified_expr(r));
  }

  static exprt simplified_expr(exprt &expr)
  {
    symbol_tablet symbol_table;
    const namespacet ns(symbol_table);

    return simplify_expr(expr, ns);
  }

  exprt zero() const;

  bool is_int() const
  {
    return is_int(get_type());
  }

  bool is_float() const
  {
    return is_float(get_type());
  }

  static bool is_int(const typet &src)
  {
    return src.id()==ID_signedbv || src.id()==ID_unsignedbv;
  }

  static bool is_float(const typet &src)
  {
    return src.id()==ID_floatbv;
  }

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

