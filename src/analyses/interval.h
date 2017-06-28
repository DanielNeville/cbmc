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
    upper(u),
    type(calculate_type(l, u))
  {
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
  static intervalt simplified_interval(exprt &l, exprt &r);
  static exprt simplified_expr(exprt expr);

  /* Don't allow different types in upper and lower */
  typet get_type() const;
  typet calculate_type(const exprt &l, const exprt &u) const;

  min_exprt min() const;
  max_exprt max() const;

  static intervalt swap(intervalt &i);
  intervalt swap() const;


  /* Helpers */
  /* Four common params: self, type, expr, interval */

  bool is_int() const;
  bool is_float() const;
  static bool is_int(const typet &type);
  static bool is_float(const typet &src);

  static bool is_bitvector(const typet &t);
  static bool is_signed(const typet &t);
  static bool is_unsigned(const typet &t);

  static bool is_signed(const intervalt &interval);
  static bool is_unsigned(const intervalt &interval);
  static bool is_bitvector(const intervalt &interval);

  static bool is_signed(const exprt &expr);
  static bool is_unsigned(const exprt &expr);
  static bool is_bitvector(const exprt &expr);

  bool is_signed() const;
  bool is_unsigned() const;
  bool is_bitvector() const;

  static bool is_extreme(const exprt &expr);
  static bool is_max(const exprt &expr);
  static bool is_min(const exprt &expr);

  bool is_max() const;
  bool is_min() const;

  static bool is_positive(const exprt &expr);
  static bool is_zero(const exprt &expr);
  static bool is_negative(const exprt &expr);
private:

  /* This is the entirety */
  const exprt lower;
  const exprt upper;
  const typet type;
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

