#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

class BigInteger {
private:
    static const int kBase_ = 1000000000;
    static const int kDigitLength_ = 9;

    std::vector<int> digits_;
    bool is_negative_ = false;

public:
    BigInteger();
    BigInteger(long long value);
    BigInteger(std::string& str);

    void makeUnsigned();
    void popZeros();
    void addPositive(const BigInteger& other);
    void subtractPositive(BigInteger other);
    std::string toString() const;
    const std::vector<int>& data() const;
    bool is_negative() const;

    BigInteger& operator+=(const BigInteger& other);
    BigInteger& operator-=(const BigInteger& other);
    BigInteger& operator*=(const BigInteger& other);
    BigInteger& operator/=(const BigInteger& other);
    BigInteger& operator%=(const BigInteger& other);

    BigInteger& operator++();
    BigInteger operator++(int);
    BigInteger& operator--();
    BigInteger operator--(int);
    BigInteger operator-() const;

    explicit operator int() const;
    explicit operator bool() const;

    friend std::istream& operator>>(std::istream& istream, BigInteger& number);
};

void swap(BigInteger& first, BigInteger& second);

BigInteger operator""_bi(const char* string, size_t);
BigInteger operator""_bi(unsigned long long value);

bool operator<(const BigInteger& a, const BigInteger& b);
bool operator<=(const BigInteger& a, const BigInteger& b);
bool operator>(const BigInteger& a, const BigInteger& b);
bool operator>=(const BigInteger& a, const BigInteger& b);
bool operator==(const BigInteger& a, const BigInteger& b);
bool operator!=(const BigInteger& a, const BigInteger& b);

BigInteger operator+(const BigInteger& left, const BigInteger& other);
BigInteger operator-(const BigInteger& left, const BigInteger& other);
BigInteger operator*(const BigInteger& left, const BigInteger& other);
BigInteger operator/(const BigInteger& left, const BigInteger& other);
BigInteger operator%(const BigInteger& left, const BigInteger& other);

std::ostream& operator<<(std::ostream& ostream, const BigInteger& number);


BigInteger::BigInteger() {}

BigInteger::BigInteger(long long value) {
    is_negative_ = bool(value < 0);
    value = (value < 0) ? -value : value;
    if (value == 0) { digits_ = {0}; }
    while (value > 0) {
        digits_.push_back(value % kBase_);
        value /= kBase_;
    }
}

BigInteger::BigInteger(std::string& str) {
    if (str[0] == '-') {
        is_negative_ = true;
        str.erase(str.begin());
    }
    std::string support;
    for (int i = static_cast<int>(str.length()) - 1; i >= 0; --i) {
        if (support.length() == kDigitLength_) {
            std::reverse(support.begin(), support.end());
            digits_.push_back(std::stoi(support));
            support.clear();
        }
        support += str[i];
    }
    if (!support.empty()) {
        std::reverse(support.begin(), support.end());
        digits_.push_back(std::stoi(support));
    }
}

BigInteger::operator int() const {
    std::string str = toString();
    return std::stoi(str);
}

BigInteger::operator bool() const {
    return (*this == 1);
}

BigInteger operator""_bi(unsigned long long value) {
    return BigInteger(value);
}

const std::vector<int>& BigInteger::data() const {
    return digits_;
}

bool BigInteger::is_negative() const {
    return is_negative_;
}

void BigInteger::makeUnsigned() {
    is_negative_ = false;
}

void BigInteger::popZeros() {
    while (digits_.size() > 1 && digits_[digits_.size() - 1] == 0) {
        digits_.pop_back();
    }
}

void swap(BigInteger& first, BigInteger& second) {
    BigInteger tmp = first;
    first = second;
    second = tmp;
}

std::string BigInteger::toString() const {
    std::string str;
    if (is_negative_) { str += "-"; }
    str += std::to_string(digits_[digits_.size() - 1]);
    for (int i = static_cast<int>(digits_.size()) - 2; i >= 0; --i) {
        std::string support = std::to_string(digits_[i]);
        std::reverse(support.begin(), support.end());
        while (support.length() < kDigitLength_) {
            support.push_back('0');
        }
        std::reverse(support.begin(), support.end());
        str += support;
    }
    return str;
}

bool operator<(const BigInteger& a, const BigInteger& b) {
    if (a.is_negative() + b.is_negative() == 1) {
        return a.is_negative();
    }
    if (a.is_negative()) {
        if (a.data().size() != b.data().size()) {
            return (b.data().size() < a.data().size());
        }
        for (int i = static_cast<int>(a.data().size()) - 1; i >= 0; --i) {
            if (a.data()[i] != b.data()[i]) {
                return (b.data()[i] < a.data()[i]);
            }
        }
    } else {
        if (a.data().size() != b.data().size()) {
            return (a.data().size() < b.data().size());
        }
        for (int i = static_cast<int>(a.data().size()) - 1; i >= 0; --i) {
            if (a.data()[i] != b.data()[i]) {
                return (a.data()[i] < b.data()[i]);
            }
        }
    }
    return false;
}

bool operator<=(const BigInteger& a, const BigInteger& b) {
    return !(b < a);
}

bool operator>(const BigInteger& a, const BigInteger& b) {
    return (b < a);
}

bool operator>=(const BigInteger& a, const BigInteger& b) {
    return !(a < b);
}

bool operator==(const BigInteger& a, const BigInteger& b) {
    if (a.data().size() == b.data().size()) {
        for (int i = 0, j = 0; i < static_cast<int>(a.data().size()); i++, j++) {
            if (a.data()[i] != b.data()[i]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool operator!=(const BigInteger& a, const BigInteger& b) {
    return !(a == b);
}

BigInteger operator+(const BigInteger& left, const BigInteger& other) {
    BigInteger value = left;
    value += other;
    return value;
}

BigInteger operator-(const BigInteger& left, const BigInteger& other) {
    BigInteger value = left;
    value -= other;
    return value;
}

BigInteger operator*(const BigInteger& left, const BigInteger& other) {
    BigInteger value = left;
    value *= other;
    return value;
}

BigInteger operator/(const BigInteger& left, const BigInteger& other) {
    BigInteger value = left;
    value /= other;
    return value;
}

BigInteger operator%(const BigInteger& left, const BigInteger& other) {
    BigInteger value = left;
    value %= other;
    return value;
}

BigInteger& BigInteger::operator++() {
    *this += 1;
    return *this;
}

BigInteger BigInteger::operator++(int) {
    BigInteger copy = *this;
    ++*this;
    return copy;
}

BigInteger& BigInteger::operator--() {
    *this -= 1;
    return *this;
}

BigInteger BigInteger::operator--(int) {
    BigInteger copy = *this;
    --*this;
    return copy;
}

BigInteger BigInteger::operator-() const {
    BigInteger copy = *this;
    if (copy != 0_bi) {
        copy.is_negative_ = !copy.is_negative();
    }
    return copy;
}

void BigInteger::addPositive(const BigInteger& other) {
    while (digits_.size() <= other.data().size()) {
        digits_.push_back(0);
    }
    for (int i = 0; i < static_cast<int>(other.data().size()); ++i) {
        if (digits_[i] + other.data()[i] < kBase_) {
            digits_[i] += other.data()[i];
        } else {
            digits_[i] = (digits_[i] + other.data()[i]) % kBase_;
            digits_[i + 1] += 1;
        }
    }
    popZeros();
}

void BigInteger::subtractPositive(BigInteger other) {
    makeUnsigned();
    other.makeUnsigned();
    if (*this < other) {
        swap(*this, other);
        is_negative_ = true;
    }
    while (other.data().size() < digits_.size()) {
        other.digits_.push_back(0);
    }
    for (int i = 0; i < static_cast<int>(digits_.size()); ++i) {
        if (digits_[i] - other.data()[i] >= 0) {
            digits_[i] -= other.data()[i];
        } else {
            digits_[i] = kBase_ + (digits_[i] - other.data()[i]);
            digits_[i + 1] -= 1;
        }
    }
    popZeros();
}

BigInteger& BigInteger::operator+=(const BigInteger& other) {
    if (this == &other) {
        BigInteger copy = *this;
        *this += copy;
        return *this;
    }
    if (is_negative_ == other.is_negative()) {
        addPositive(other);
        return *this;
    }
    if (is_negative_) {
        subtractPositive(other);
        if (*this != 0) {
            is_negative_ = !(is_negative_);
        }
    } else {
        subtractPositive(other);
    }
    return *this;
}

BigInteger& BigInteger::operator-=(const BigInteger& other) {
    if (this == &other) {
        *this = 0_bi;
        return *this;
    }
    if (is_negative_ != other.is_negative()) {
        addPositive(other);
        return *this;
    }
    if (is_negative_) {
        subtractPositive(other);
        if (*this != 0) {
            is_negative_ = !(is_negative_);
        }
    } else {
        subtractPositive(other);
    }
    return *this;
}

BigInteger& BigInteger::operator*=(const BigInteger& other) {
    BigInteger result = 0;
    BigInteger other_copy = other;
    bool res_is_negative = (is_negative_ != other_copy.is_negative());
    other_copy.makeUnsigned();
    makeUnsigned();
    if (*this < other_copy) {
        swap(*this, other_copy);
    }
    BigInteger support;
    int tmp = 0;
    for (int i = 0; i < static_cast<int>(other_copy.data().size()); ++i) {
        support.digits_.resize(digits_.size() + 1 + i, 0);
        for (int k = 0; k < static_cast<int>(digits_.size()); k++) {
            support.digits_[k + i] = static_cast<int>((((static_cast<long long>(digits_[k]) * static_cast<long long>(other_copy.data()[i])) + static_cast<long long>(tmp)) % static_cast<long long>(kBase_)));
            tmp = static_cast<int>((((static_cast<long long>(digits_[k]) * static_cast<long long>(other_copy.data()[i])) + static_cast<long long>(tmp)) / static_cast<long long>(kBase_)));
        }
        support.digits_[digits_.size() + i] = tmp;
        support.popZeros();
        result += support;
        support = 0;
        tmp = 0;
    }
    *this = result;
    popZeros();
    is_negative_ = res_is_negative;
    return *this;
}

BigInteger& BigInteger::operator/=(const BigInteger& other) {
    bool res_is_negative = (is_negative_ != other.is_negative());
    is_negative_ = false;
    BigInteger support = 0, result;
    if ((digits_.empty() == 1) || (other.data().empty() == 1) || (*this < other)) {
        *this = support;
        return *this;
    }
    for (int i = static_cast<int>(digits_.size()); i > 0; --i) {
        support *= kBase_;
        support += digits_[i - 1];
        if (support >= other) {
            int left = 0;
            int right = kBase_;
            while (right - left > 1) {
                int mid = (right + left) >> 1;
                (other.is_negative_) ? (mid * (-other) <= support) ? left = mid : right = mid : (mid * other <= support) ? left = mid : right = mid;
            }
            support -= (other.is_negative_) ? left*(-other) : left * other;
            result.digits_.push_back(left);
        } else {
            result.digits_.push_back(0);
        }
    }
    for (int i = 0; i < static_cast<int>(result.data().size()) / 2; ++i) {
        std::swap(result.digits_[static_cast<int>(result.data().size()) - i - 1], result.digits_[i]);
    }
    *this = result;
    popZeros();
    is_negative_ = res_is_negative;
    return *this;
}

BigInteger& BigInteger::operator%=(const BigInteger& other) {
    *this = *this - (*this / other) * other;
    return *this;
}

std::ostream& operator<<(std::ostream& ostream, const BigInteger& number) {
    ostream << number.toString();
    return ostream;
}

std::istream& operator>>(std::istream& istream, BigInteger& number) {
    std::string support;
    istream >> support;
    number = support;
    return istream;
}

class Rational{
private:
    BigInteger numer_ = 0;
    BigInteger denom_ = 1;
    bool is_negative_ = false;

public:
    Rational();
    Rational(const BigInteger& other);
    Rational(long long value);

    Rational& operator+=(const Rational& other);
    Rational& operator-=(const Rational& other);
    Rational& operator*=(const Rational& other);
    Rational& operator/=(const Rational& other);
    Rational operator-() const;

    bool is_negative() const;
    void makeUnsigned();
    void toReduce();
    std::string toString() const;
    std::string asDecimal(size_t precision = 0) const;
    friend bool operator<(const Rational& a, const Rational& b);

    explicit operator double() const;
};

bool operator<=(const Rational& a, const Rational& b);
bool operator>(const Rational& a, const Rational& b);
bool operator>=(const Rational& a, const Rational& b);
bool operator==(const Rational& a, const Rational& b);
bool operator!=(const Rational& a, const Rational& b);

Rational operator+(const Rational& left, const Rational& other);
Rational operator-(const Rational& left, const Rational& other);
Rational operator*(const Rational& left, const Rational& other);
Rational operator/(const Rational& left, const Rational& other);

BigInteger GCD(BigInteger first, BigInteger second);

Rational::Rational() {}

Rational::Rational(const BigInteger& other) {
    is_negative_ = other.is_negative();
    numer_ = (other < 0) ? -other : other;
    denom_ = 1;
}

Rational::Rational(long long value) {
    is_negative_ = bool(value < 0);
    numer_ = (value < 0) ? BigInteger(-value) : BigInteger(value);
    denom_ = 1;
}

Rational Rational::operator-() const {
    Rational copy = *this;
    if (copy != 0) {
        copy.is_negative_ = !is_negative_;
    }
    return copy;
}

bool Rational::is_negative() const {
    return is_negative_;
}

void Rational::makeUnsigned() {
    is_negative_ = false;
}

BigInteger GCD(BigInteger first, BigInteger second) {
    if (second == 1) {
        return 1;
    }
    if (first.is_negative()) {
        first = -first;
    }
    if (second.is_negative()) {
        second = -second;
    }
    if (first == 0) {
        return second;
    }
    if (second == 0) {
        return first;
    }
    if (first <= second) {
        return GCD(first, (second % first));
    }
    return GCD((first % second), second);
}

void Rational::toReduce() {
    BigInteger a = GCD(BigInteger(numer_), BigInteger(denom_));
    numer_ /= a;
    denom_ /= a;
}

std::string Rational::toString() const {
    std::string s;
    if (is_negative_) {
        s += "-";
    }
    if (denom_ == 1) return s + numer_.toString();
    return s + numer_.toString() + '/' + denom_.toString();
}

std::string Rational::asDecimal(size_t precision) const {
    std::string support = numer_.toString();
    std::string support2(precision, '0');
    support += support2;
    BigInteger other(support);
    other /= denom_;
    if (other == 0) {
        std::string result = "0.";
        std::string sup(precision, '0');
        result += sup;
        return result;
    }
    std::string result;
    if (is_negative_) {
        result = "-";
    }
    std::string f = other.toString();
    if (f.length() <= precision) {
        result += "0.";
        std::string sup(precision - f.length(), '0');
        result += sup;
        result += f;
        return result;
    }
    result += other.toString();
    std::string dot = ".";
    result.insert(result.size() - precision, dot);
    return result;
}

Rational::operator double() const {
    return std::stod(asDecimal(15));
}

bool operator<(const Rational& a, const Rational& b) {
    if (a.is_negative() != b.is_negative()) {
        return (b.is_negative() < a.is_negative());
    }
    if (a == b) {
        return false;
    }
    bool res = (a.numer_ *  b.denom_ < a.denom_ * b.numer_);
    return (a.is_negative_) ? !res : res;
}

bool operator<=(const Rational& a, const Rational& b) {
    return !(b < a);
}

bool operator>(const Rational& a, const Rational& b) {
    return (b < a);
}

bool operator>=(const Rational& a, const Rational& b) {
    return !(a < b);
}

bool operator==(const Rational& a, const Rational& b) {
    return (a.toString() == b.toString());
}

bool operator!=(const Rational& a, const Rational& b){
    return !(a == b);
}

Rational& Rational::operator+=(const Rational& other) {
    if (*this == 0 || other == 0) {
        *this = (*this == 0) ? other : *this;
        return *this;
    }
    if ((is_negative_) && !(other.is_negative_)) {
        is_negative_ = false;
        Rational result = other - *this;
        *this = result;
        return *this;
    }
    if (!(is_negative_) && (other.is_negative_)) {
        Rational result = other;
        result.makeUnsigned();
        *this -= result;
        return *this;
    }
    Rational support = other;
    bool res_is_neg = false;
    if (is_negative_ && support.is_negative_) {
        res_is_neg = true;
        makeUnsigned();
        support.makeUnsigned();
    }
    numer_ = (numer_ * support.denom_ + support.numer_ * denom_);
    denom_ *= support.denom_;
    toReduce();
    is_negative_ = res_is_neg;
    return *this;
}

Rational& Rational::operator-=(const Rational& other){
    if (*this == 0 || other == 0) {
        *this = (*this == 0) ? -other : *this;
        return *this;
    }
    if ((is_negative_) && !(other.is_negative_)) {
        is_negative_ = false;
        Rational result = other + *this;
        *this = result;
        is_negative_ = true;
        return *this;
    }
    if (!(is_negative_) && (other.is_negative_)) {
        Rational result = other;
        result.makeUnsigned();
        *this += result;
        return *this;
    }
    Rational support = other;
    bool res_is_neg = false;
    if (is_negative_ && support.is_negative_) {
        res_is_neg = true;
        makeUnsigned();
        support.makeUnsigned();
    }
    numer_ = (numer_ * support.denom_ - support.numer_ * denom_);
    if (numer_ < 0) {
        res_is_neg = !(res_is_neg);
        numer_.makeUnsigned();
    }
    denom_ *= support.denom_;
    toReduce();
    is_negative_ = res_is_neg;
    return *this;
}

Rational& Rational::operator*=(const Rational& other){
    if (*this == 0 || other == 0) {
        *this = 0;
        return *this;
    }
    Rational support = other;
    bool res_is_neg = false;
    if (((is_negative_) && !(other.is_negative_)) || (!(is_negative_) && (other.is_negative_))) {
        res_is_neg = true;
        makeUnsigned();
        support.makeUnsigned();
    }
    denom_ *= support.denom_;
    toReduce();
    numer_ *= support.numer_;
    toReduce();
    is_negative_ = res_is_neg;
    return *this;
}

Rational& Rational::operator/=(const Rational& other){
    Rational support = other;
    std::swap(support.numer_, support.denom_);
    *this *= support;
    toReduce();
    return *this;
}

Rational operator+(const Rational& left, const Rational& other) {
    Rational value = left;
    value += other;
    return value;
}
Rational operator-(const Rational& left, const Rational& other) {
    Rational value = left;
    value -= other;
    return value;
}
Rational operator*(const Rational& left, const Rational& other){
    Rational value = left;
    value *= other;
    return value;
}
Rational operator/(const Rational& left, const Rational& other){
    Rational value = left;
    value /= other;
    return value;
}
