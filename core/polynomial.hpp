#ifndef OS2CX_POLYNOMIAL_HPP_
#define OS2CX_POLYNOMIAL_HPP_

#include <iostream>
#include <map>
#include <set>

namespace os2cx {

/* Polynomial represents a multivariate polynomial over an arbitrary number of
variables, indexed by arbitrary integers. */
class Polynomial
{
public:
    class Variable {
    public:
        bool operator<(Variable other) const {
            return index < other.index;
        }
        bool operator==(Variable other) const {
            return index == other.index;
        }
        int index;
    };

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
            for (const auto &var : pair.first) {
                product *= evaluator(var);
            }
            sum += product;
        }
        return sum;
    }

    int num_terms() const { return terms.size(); }

private:
    friend Polynomial operator*(
        const Polynomial &left, const Polynomial &right);
    friend bool operator==(const Polynomial &left, const Polynomial &right);
    friend std::ostream &operator<<(
        std::ostream &stream, const Polynomial &poly);

    void add_term(const std::multiset<Variable> key, double value);
    void prune_zero_terms();

    /* Each entry in the map is a term in the polynomial. The key describes the
    variables in the term; repeated elements of the multiset represent variables
    raised to higher powers. The value is the coefficient of the term. The value
    should never be zero. */
    std::map<std::multiset<Variable>, double> terms;
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
