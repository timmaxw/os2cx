#include "polynomial.hpp"

#include <math.h>

#include <algorithm>
#include <vector>

namespace os2cx {

inline Polynomial::Key::Key() {
    for (int i = 0; i < Polynomial::max_order; ++i) {
        vars[i] = Variable::invalid();
    }
}

inline bool Polynomial::Key::operator==(Key other) const {
    for (int i = 0; i < Polynomial::max_order; ++i) {
        if (vars[i] != other.vars[i]) return false;
    }
    return true;
}

inline bool Polynomial::Key::operator!=(Key other) const {
    return !(*this == other);
}

inline bool Polynomial::Key::operator<(Key other) const {
    for (int i = 0; i < Polynomial::max_order; ++i) {
        if (vars[i] < other.vars[i]) return true;
        if (other.vars[i] < vars[i]) return false;
    }
    return false;
}

inline int Polynomial::Key::count(Variable var) const {
    int c = 0;
    for (int i = 0; i < Polynomial::max_order; ++i) {
        if (vars[i] == var) {
            ++c;
        }
    }
    return c;
}

inline Polynomial::Key Polynomial::Key::insert(Variable var) const {
    assert(vars[Polynomial::max_order - 1] == Variable::invalid());
    Key res;
    int this_ix = 0, res_ix = 0;
    for (; this_ix < Polynomial::max_order; ++this_ix, ++res_ix) {
        if (var < vars[this_ix]) {
            break;
        } else {
            res.vars[res_ix] = vars[this_ix];
        }
    }
    res.vars[res_ix++] = var;
    for (; this_ix < Polynomial::max_order; ++this_ix, ++res_ix) {
        if (vars[this_ix] == Variable::invalid()) {
            break;
        }
        res.vars[res_ix] = vars[this_ix];
    }
    return res;
}

inline Polynomial::Key Polynomial::Key::merge(Key other) const {
    Key res;
    int this_ix = 0, other_ix = 0, res_ix = 0;
    while (true) {
        Variable this_var = (this_ix == Polynomial::max_order)
            ? Variable::invalid() : vars[this_ix];
        Variable other_var = (other_ix == Polynomial::max_order)
            ? Variable::invalid() : other.vars[other_ix];
        if (this_var == Variable::invalid() &&
                other_var == Variable::invalid()) {
            break;
        }
        assert(res_ix < Polynomial::max_order);
        if (this_var < other_var) {
            res.vars[res_ix++] = this_var;
            ++this_ix;
        } else {
            res.vars[res_ix++] = other_var;
            ++other_ix;
        }
    }
    return res;
}

inline Polynomial::Key Polynomial::Key::erase_one(Variable var) const {
    Key res;
    int res_ix = 0;
    for (int this_ix = 0; this_ix < Polynomial::max_order; ++this_ix) {
        if (vars[this_ix] != var || this_ix != res_ix) {
            res.vars[res_ix++] = vars[this_ix];
        }
    }
    return res;
}

inline Polynomial::Key Polynomial::Key::erase_all(Variable var) const {
    Key res;
    int res_ix = 0;
    for (int this_ix = 0; this_ix < Polynomial::max_order; ++this_ix) {
        if (vars[this_ix] != var) {
            res.vars[res_ix++] = vars[this_ix];
        }
    }
    return res;
}

Polynomial::Polynomial()
{
}

Polynomial::Polynomial(double coeff) {
    terms[Key()] = coeff;
    prune_zero_terms();
}

Polynomial::Polynomial(Variable var) {
    assert(var != Variable::invalid());
    Key key;
    key.vars[0] = var;
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
    assert(var != Variable::invalid());
    Polynomial result;
    for (const auto &pair : terms) {
        Polynomial x;
        x.add_term(pair.first.erase_all(var), pair.second);

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
        int c = pair.first.count(var);
        if (c == 0) continue;
        double new_value = pair.second * c;
        result.add_term(pair.first.erase_one(var), new_value);
    }
    return result;
}

Polynomial Polynomial::antidifferentiate(Variable var) const {
    Polynomial result;
    for (const auto &pair : terms) {
        Key new_key = pair.first.insert(var);
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
        Key new_key = pair.first.erase_all(var);
        for (int from_or_to = 0; from_or_to <= 1; ++from_or_to) {
            /* Populate the 'pows' cache */
            while (static_cast<int>(pows[from_or_to].size()) <= var_power + 1) {
                pows[from_or_to].push_back(
                    pows[from_or_to].back() * pows[from_or_to][1]);
            }

            for (const auto &pair2 : pows[from_or_to][var_power + 1].terms) {
                Key key = pair2.first.merge(new_key);
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

void Polynomial::add_term(Key key, double value) {
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
            Polynomial::Key new_key = lpair.first.merge(rpair.first);
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
        for (int i = 0; i < Polynomial::max_order; ++i) {
            Polynomial::Variable var = pair.first.vars[i];
            if (var == Polynomial::Variable::invalid()) {
                break;
            }
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
