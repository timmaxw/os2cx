#include "polynomial.hpp"

#include <math.h>

#include <vector>

namespace os2cx {

Polynomial::Polynomial()
{
}

Polynomial::Polynomial(double coeff) {
    terms[std::multiset<Variable>()] = coeff;
    prune_zero_terms();
}

Polynomial::Polynomial(Variable var) {
    std::multiset<Variable> key;
    key.insert(var);
    add_term(key, 1.0);
}

Polynomial Polynomial::operator-() const {
    Polynomial result;
    for (const auto &pair : terms) {
        result.terms.insert(std::make_pair(pair.first, -pair.second));
    }
    return result;
}

Polynomial &Polynomial::operator+=(const Polynomial &right) {
    for (const auto &pair : right.terms) {
        add_term(pair.first, pair.second);
    }
    prune_zero_terms();
    return *this;
}

Polynomial &Polynomial::operator-=(const Polynomial &right) {
    return (*this += -right);
}

Polynomial &Polynomial::operator*=(const Polynomial &right) {
    *this = *this * right;
    return *this;
}

Polynomial &Polynomial::operator/=(double right) {
    for (auto &pair : terms) {
        pair.second /= right;
    }
    return *this;
}

Polynomial Polynomial::evaluate_partial(Variable var, const Polynomial &value) const {
    Polynomial result;
    for (const auto &pair : terms) {
        std::multiset<Variable> x_key = pair.first;
        x_key.erase(var);
        Polynomial x;
        x.add_term(x_key, pair.second);

        Polynomial y = pow(value, pair.first.count(var));

        /* This is essentially 'result += x * y', but we defer calling
        prune_zero_terms() to avoid quadratic blowup */
        for (const auto &pair : (x * y).terms) {
            result.add_term(pair.first, pair.second);
        }
    }
    result.prune_zero_terms();
    return result;
}

Polynomial Polynomial::differentiate(Variable var) const {
    Polynomial result;
    for (const auto &pair : terms) {
        std::multiset<Variable> new_key = pair.first;
        auto it = new_key.find(var);
        if (it == new_key.end()) {
            continue;
        }
        double new_value = pair.second * new_key.count(var);
        new_key.erase(it);
        result.add_term(new_key, new_value);
    }
    return result;
}

Polynomial Polynomial::antidifferentiate(Variable var) const {
    Polynomial result;
    for (const auto &pair : terms) {
        std::multiset<Variable> new_key = pair.first;
        new_key.insert(var);
        double new_value = pair.second / new_key.count(var);
        result.add_term(new_key, new_value);
    }
    return result;
}

Polynomial Polynomial::integrate(
    Variable var,
    const Polynomial &from_value,
    const Polynomial &to_value
) const {
    /* This function was originally implemented as:
        Polynomial antiderivative = antidifferentiate(var);
        Polynomial res = antiderivative.evaluate_partial(var, to_value) -
            antiderivative.evaluate_partial(var, from_value);
        return res;
    However, this turned out to be too slow, so here's an optimized version that
    constructs a lot fewer temporaries. */

    /* Cache powers of 'from_value' and 'to_value'. Specifically,
    pows[0][n] = pow(from_value, n) and pows[1][n] = pow(to_value, n). */
    std::vector<Polynomial> pows[2];
    pows[0].push_back(Polynomial(1));
    pows[0].push_back(from_value);
    pows[1].push_back(Polynomial(1));
    pows[1].push_back(to_value);

    Polynomial result;
    for (const auto &pair : terms) {
        int var_power = pair.first.count(var);
        for (int from_or_to = 0; from_or_to <= 1; ++from_or_to) {
            /* Populate the 'pows' cache */
            while (static_cast<int>(pows[from_or_to].size()) <= var_power + 1) {
                pows[from_or_to].push_back(
                    pows[from_or_to].back() * pows[from_or_to][1]);
            }

            for (const auto &pair2 : pows[from_or_to][var_power + 1].terms) {
                std::multiset<Variable> key = pair2.first;
                for (Variable v : pair.first) {
                    if (v != var) {
                        key.insert(v);
                    }
                }
                double coeff = pair.second * pair2.second / (var_power + 1);
                if (from_or_to == 0) {
                    coeff = -coeff;
                }
                result.add_term(key, coeff);
            }
        }
    }

    result.prune_zero_terms();

    return result;
}

void Polynomial::add_term(const std::multiset<Variable> key, double value) {
    auto iterator_and_inserted = terms.insert(std::make_pair(key, value));
    if (!iterator_and_inserted.second) {
        iterator_and_inserted.first->second += value;
    }
}

void Polynomial::prune_zero_terms() {
    for (auto it = terms.begin(); it != terms.end();) {
        auto jt = it;
        ++jt;
        if (it->second == 0) {
            terms.erase(it);
        }
        it = jt;
    }
}

Polynomial operator+(const Polynomial &left, const Polynomial &right) {
    Polynomial result = left;
    result += right;
    return result;
}

Polynomial operator-(const Polynomial &left, const Polynomial &right) {
    Polynomial result = left;
    result -= right;
    return result;
}

Polynomial operator*(const Polynomial &left, const Polynomial &right) {
    Polynomial result;
    for (const auto &lpair : left.terms) {
        for (const auto &rpair : right.terms) {
            std::multiset<Polynomial::Variable> new_key = lpair.first;
            new_key.insert(rpair.first.begin(), rpair.first.end());
            result.add_term(new_key, lpair.second * rpair.second);
        }
    }
    result.prune_zero_terms();
    return result;
}

Polynomial operator/(const Polynomial &left, double right) {
    Polynomial result = left;
    result /= right;
    return result;
}

Polynomial pow(const Polynomial &left, int power) {
    Polynomial result(1.0);
    for (int i = 0; i < power; ++i) {
        result *= left;
    }
    return result;
}

void jacobian(
    Polynomial (*const values)[3],
    Polynomial::Variable variables[3],
    Polynomial (*matrix_out)[3][3]
) {
    for (int value_index = 0; value_index < 3; ++value_index) {
        for (int variable_index = 0; variable_index < 3; ++variable_index) {
            (*matrix_out)[value_index][variable_index] =
                (*values)[value_index].differentiate(variables[variable_index]);
        }
    }
}

Polynomial determinant(
    Polynomial (*const matrix)[3][3]
) {
    return (*matrix)[0][0] * (*matrix)[1][1] * (*matrix)[2][2]
         - (*matrix)[0][0] * (*matrix)[1][2] * (*matrix)[2][1]
         + (*matrix)[0][1] * (*matrix)[1][2] * (*matrix)[2][0]
         - (*matrix)[0][1] * (*matrix)[1][0] * (*matrix)[2][2]
         + (*matrix)[0][2] * (*matrix)[1][0] * (*matrix)[2][1]
         - (*matrix)[0][2] * (*matrix)[1][1] * (*matrix)[2][0];
}

bool operator==(const Polynomial &left, const Polynomial &right) {
    return left.terms == right.terms;
}

bool operator!=(const Polynomial &left, const Polynomial &right) {
    return !(left == right);
}

std::ostream &operator<<(std::ostream &stream, const Polynomial &poly) {
    bool is_first_term = true;
    for (const auto &pair : poly.terms) {
        if (pair.second < 0) {
            stream << "-";
        } else if (!is_first_term) {
            stream << "+";
        }
        bool is_first_factor = true;
        if (fabs(pair.second) != 1.0) {
            stream << fabs(pair.second);
            is_first_factor = false;
        }
        for (Polynomial::Variable var : pair.first) {
            if (!is_first_factor) {
                stream << "*";
            }
            stream << "[" << var.index << "]";
            is_first_factor = false;
        }
        if (is_first_factor) {
            stream << "1";
        }
        is_first_term = false;
    }
    if (is_first_term) {
        stream << "0";
    }
    return stream;
}

} /* namespace os2cx */
