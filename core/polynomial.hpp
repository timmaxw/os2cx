#ifndef OS2CX_POLYNOMIAL_HPP_
#define OS2CX_POLYNOMIAL_HPP_

#include <assert.h>

#include <iostream>
#include <limits>
#include <map>

namespace os2cx {

/* Polynomial represents a multivariate polynomial over an arbitrary number of
variables, indexed by arbitrary integers. */
class Polynomial
{
public:
    class Variable {
    public:
        static Variable invalid() {
            Variable v;
            v.index = std::numeric_limits<int8_t>::max();
            return v;
        }
        static Variable from_index(int index) {
            Variable v;
            v.index = static_cast<int8_t>(index);
            assert(static_cast<int>(v.index) == index);
            return v;
        }
        bool operator<(Variable other) const {
            return index < other.index;
        }
        bool operator==(Variable other) const {
            return index == other.index;
        }
        bool operator!=(Variable other) const {
            return index != other.index;
        }
        int8_t index;
    };

    static const int max_order = 16;

    Polynomial();
    explicit Polynomial(double coeff);
    explicit Polynomial(Variable var);

    Polynomial operator-() const;
    Polynomial &operator+=(const Polynomial &right);
    Polynomial &operator-=(const Polynomial &right);
    Polynomial &operator*=(const Polynomial &right);
    Polynomial &operator/=(double right);

    Polynomial evaluate_partial(Variable var, const Polynomial &value) const;
    Polynomial differentiate(Variable var) const;
    Polynomial antidifferentiate(Variable var) const;
    Polynomial integrate(
        Variable var,
        const Polynomial &from_value,
        const Polynomial &to_value) const;

    template<class Callable>
    double evaluate(const Callable &evaluator) const {
        double sum = 0.0;
        for (const auto &pair : terms) {
            double product = pair.second;
            for (int i = 0; i < Polynomial::max_order; ++i) {
                Variable var = pair.first.vars[i];
                if (var != Variable::invalid()) {
                    product *= evaluator(var);
                }
            }
            sum += product;
        }
        return sum;
    }

    int num_terms() const { return terms.size(); }

private:
    /* Key is a multiset of Variable. It's represented as a fixed-size array
    with unused elements filled by Variable::invalid(). */
    class Key {
    public:
        Key();

        bool operator==(Key other) const;
        bool operator!=(Key other) const;
        bool operator<(Key other) const;

        /* Counts how many times 'var' appears in the key */
        int count(Variable var) const;

        /* This key plus 'var'. */
        Key insert(Variable var) const;

        /* This key plus another key. */
        Key merge(Key other) const;

        /* This key minus the first instance of 'var', if any. */
        Key erase_one(Variable var) const;

        /* This key minus all instances of 'var'. */
        Key erase_all(Variable var) const;

        Variable vars[Polynomial::max_order];
    };

    friend Polynomial operator*(
        const Polynomial &left, const Polynomial &right);
    friend bool operator==(const Polynomial &left, const Polynomial &right);
    friend std::ostream &operator<<(
        std::ostream &stream, const Polynomial &poly);

    void add_term(Key key, double value);
    void prune_zero_terms();

    /* Each entry in the map is a term in the polynomial. The key describes the
    variables in the term; repeated elements of the multiset represent variables
    raised to higher powers. The value is the coefficient of the term. The value
    should never be zero. */
    std::map<Key, double> terms;
};

Polynomial operator+(const Polynomial &left, const Polynomial &right);
Polynomial operator-(const Polynomial &left, const Polynomial &right);
Polynomial operator*(const Polynomial &left, const Polynomial &right);
Polynomial operator/(const Polynomial &left, double right);
Polynomial pow(const Polynomial &left, int power);

void jacobian(
    Polynomial (*const values)[3],
    Polynomial::Variable variables[3],
    Polynomial (*matrix_out)[3][3]);
Polynomial determinant(
    Polynomial (*const matrix)[3][3]);

bool operator==(const Polynomial &left, const Polynomial &right);
bool operator!=(const Polynomial &left, const Polynomial &right);

std::ostream &operator<<(std::ostream &stream, const os2cx::Polynomial &poly);

} /* namespace os2cx */

#endif
