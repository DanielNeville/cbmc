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

  /* Naming scheme
   *      is_[X]?  Returns bool / tvt
   *      get_[X]? Returns relevant object
   *      [X]      Generator of some object.
   *      generate_[X]  generator used for evaluation
   */


  /** SET OF ARITHMETIC OPERATORS */

  /* Unary arithmetic */
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

  /* Unary bitwise */
  intervalt bitwise_not() const;

  /* Binary bitwise */
  intervalt bitwise_xor(const intervalt &o) const;
  intervalt bitwise_or(const intervalt &o) const;
  intervalt bitwise_and(const intervalt &o) const;

  tvt less_than(const intervalt &o) const;
  tvt greater_than(const intervalt &o) const;
  tvt less_than_or_equal(const intervalt &o) const;
  tvt greater_than_or_equal(const intervalt &o) const;
  tvt equal(const intervalt &o) const;
  tvt not_equal(const intervalt &o) const;

  intervalt increment() const;
  intervalt decrement() const;

  bool is_empty() const;
  bool is_constant() const;

//  tvt contains(intervalt &o) const;

  /* SET OF EXPR COMPARATORS */
  static bool equal(const exprt &a, const exprt &b);
  static bool not_equal(const exprt &a, const exprt &b);
  static bool less_than(const exprt &a, const exprt &b);
  static bool less_than_or_equal(const exprt &a, const exprt &b);
  static bool greater_than(const exprt &a, const exprt &b);
  static bool greater_than_or_equal(const exprt &a, const exprt &b);

  const exprt &get_lower() const
  {
    return lower;
  }

  const exprt &get_upper() const
  {
    return upper;
  }

  friend tvt operator< (const intervalt &lhs, const intervalt &rhs);
  friend tvt operator> (const intervalt &lhs, const intervalt &rhs);
  friend tvt operator<=(const intervalt &lhs, const intervalt &rhs);
  friend tvt operator>=(const intervalt &lhs, const intervalt &rhs);
  friend tvt operator==(const intervalt &lhs, const intervalt &rhs);
  friend tvt operator!=(const intervalt &lhs, const intervalt &rhs);

  friend intervalt operator+(const intervalt &lhs, const intervalt &rhs);
  friend intervalt operator-(const intervalt &lhs, const intervalt &rhs);
  friend intervalt operator/(const intervalt &lhs, const intervalt &rhs);
  friend intervalt operator*(const intervalt &lhs, const intervalt &rhs);
  friend intervalt operator%(const intervalt &lhs, const intervalt &rhs);
  friend intervalt operator!(const intervalt &lhs);
  friend intervalt operator&(const intervalt &lhs, const intervalt &rhs);
  friend intervalt operator|(const intervalt &lhs, const intervalt &rhs);
  friend intervalt operator^(const intervalt &lhs, const intervalt &rhs);
  friend intervalt operator<<(const intervalt &lhs, const intervalt &rhs);
  friend intervalt operator>>(const intervalt &lhs, const intervalt &rhs);

  friend std::ostream& operator<< (std::ostream& out, const intervalt &i);
  std::string to_string() const;

  bool valid()
  {
    return true;
  }

  bool is_top() const;
  bool is_bottom() const;
  static intervalt top(const typet &type);
  static intervalt bottom(const typet &type);
  intervalt top() const;
  intervalt bottom() const;


  /* Generate min and max exprt according to current type */
  bool is_max() const;
  static bool is_max(const exprt &expr);
  bool is_min() const;
  static bool is_min(const exprt &expr);
  min_exprt min() const;
  max_exprt max() const;

  static constant_exprt zero(const typet &type);
  static constant_exprt zero(const intervalt &interval);
  static constant_exprt zero(const exprt &expr);
  constant_exprt zero() const;


  /* Private? */
  static intervalt get_extremes(const intervalt &lhs, const intervalt &rhs, const exprt operation);
  static exprt get_extreme(std::vector<exprt> values, bool min = true);

  static exprt generate_expression(const exprt &a, const exprt &b, const exprt &operation);

  static exprt generate_multiply_expression(const exprt &a, const exprt &b, exprt operation);
  static exprt generate_multiply_expression_max(const exprt &expr);
  static exprt generate_multiply_expression_min(const exprt &min, const exprt &other);

  static exprt generate_division_expression(const exprt &a, const exprt &b, exprt operation);
  static exprt generate_modulo_expression(const exprt &a, const exprt &b, exprt operation);


  /* we don't simplify in the constructor otherwise */
  static intervalt simplified_interval(exprt &l, exprt &r);
  static exprt simplified_expr(exprt expr);

  /* Don't allow different types in upper and lower */
  typet get_type() const;
  typet calculate_type(const exprt &l, const exprt &u) const;


  /* Swap lower and upper! */
  static intervalt swap(intervalt &i);
  intervalt swap() const;


  /* Helpers */
  /* Four common params: self, static: type, expr, interval */

  bool is_numeric() const;
  static bool is_numeric(const typet &type);
  static bool is_numeric(const exprt &expr);
  static bool is_numeric(const intervalt &interval);

  bool is_int() const;
  static bool is_int(const typet &type);
  static bool is_int(const exprt &expr);
  static bool is_int(const intervalt &interval);

  bool is_float() const;
  static bool is_float(const typet &type);
  static bool is_float(const exprt &expr);
  static bool is_float(const intervalt &interval);

  bool is_bitvector() const;
  static bool is_bitvector(const typet &type);
  static bool is_bitvector(const intervalt &interval);
  static bool is_bitvector(const exprt &expr);

  bool is_signed() const;
  static bool is_signed(const typet &type);
  static bool is_signed(const exprt &expr);
  static bool is_signed(const intervalt &interval);

  bool is_unsigned() const;
  static bool is_unsigned(const typet &type);
  static bool is_unsigned(const exprt &expr);
  static bool is_unsigned(const intervalt &interval);

  static bool contains_extreme(const exprt expr);
  static bool contains_extreme(const exprt expr1, const exprt expr2);

  bool contains_zero() const;
  bool contains(const intervalt &interval) const;

  static bool is_extreme(const exprt &expr);
  static bool is_extreme(const exprt &expr1, const exprt &expr2);

  static bool is_positive(const exprt &expr);
  static bool is_zero(const exprt &expr);
  static bool is_negative(const exprt &expr);

  static bool is_positive(const intervalt &interval);
  static bool is_zero(const intervalt &interval);
  static bool is_negative(const intervalt &interval);

  bool is_positive() const;
  bool is_zero() const;
  bool is_negative() const;
private:
  /* This is the entirety */
  const exprt lower;
  const exprt upper;
  const typet type;
};

#endif /* SRC_ANALYSES_INTERVAL_H_ */
