#include "big_integer.h"
//
// Created by nikita on 12.04.19.
//

#include "big_integer.h"

#include "string.h"
#include <iterator>
#include <iostream>
#include <algorithm>

const uint64_t big_integer::NEGATIVE_SIGN = ~UINT64_C(0);
const uint64_t big_integer::POSITIVE_SIGN = 0;
const size_t big_integer::INITIAL_SIZE = 1;

std::string to_string(big_integer bi, int radix) {
    std::string res;
    bool negative = false;
    negative = bi.isNegative();
    if (negative)
        bi.invert();
    do {
        uint64_t remainder;
        bi.div_rem(radix, remainder);
        res.push_back(static_cast<char>(remainder + '0'));
    } while (!bi.isZero());
    if (negative)
        res.push_back('-');
    std::reverse(res.begin(), res.end());
    return res;
}

void big_integer::clear() {
    size_t signBit = this->isNegative() ? big_integer::NEGATIVE_SIGN : big_integer::POSITIVE_SIGN;
    for (int i = this->size - 1; i >= 0 && this->digits[i] == signBit && size > 1; i--) {
        size--;
    }
    if (isNegative() != (static_cast<int64_t>(signBit) < 0)) {
        size++;
    }
    if (size == 1)
        return;
}

bool big_integer::isZero() {
    for (size_t i = 0; i < size; ++i) {
        if (digits[i] != 0) return false;
    }
    return true;
}

bool big_integer::isNegative() const {
    return static_cast<int64_t>(digits[size - 1]) < 0;
}

// return: this >= 0
bool big_integer::isPositive() const {
    return !isNegative();
}

big_integer::big_integer() {
    this->digits = nullptr;
    resize(INITIAL_SIZE);
    std::fill_n(digits, INITIAL_SIZE, UINT64_C(0));
}

big_integer::big_integer(int64_t number) : big_integer() {
    this->digits[0] = number;
    expandSign(0);
}

big_integer::big_integer(const big_integer &bi) {
    int size = bi.size;
    this->digits = nullptr;
    resize(size);
    std::copy(bi.digits, bi.digits + bi.size, this->digits);
}

big_integer &big_integer::operator=(const big_integer& right) {
    if ((*this) == right)
        return *this;
    if (this->size < right.size) {
        resize(right.size);
    }
    std::copy(right.digits, right.digits + right.size, digits);
    expandSign(right.size - 1);
    this->size = right.size;
    return *this;
}

void swap(big_integer &lhs, big_integer &rhs) {
    std::swap(lhs.size, rhs.size);
    std::swap(lhs.capacity, rhs.capacity);
    std::swap(lhs.digits, rhs.digits);
}

big_integer::big_integer(std::string s, unsigned int radix) : big_integer() {
    size_t i = 0;
    bool isNegative = false;
    if (s[0] == '-') {
        isNegative = (s != "-0");
        ++i;
    }
    for ( ; i < s.size(); ++i) {
        uint64_t digit;
        if ('0' <= s[i] && s[i] <= '9')
            digit = static_cast<uint64_t>(s[i] - '0');
        else if ('a' <= s[i] && s[i] <= 'f')
            digit = static_cast<uint64_t>(s[i] - 'a' + 10);

        (*this) = this->mulShort(radix).addShort(digit);
    }
    if (isNegative)
        invert();
    setSign(!isNegative);
}

big_integer::~big_integer() {
    free(this->digits);
}

uint64_t big_integer::addWithCarry(uint64_t &res, uint64_t a, uint64_t b) {
    uint64_t carry = 0;
    asm volatile ("add %2, %1\n"
                  "adc $0, %0"
                    : "+r" (carry), "+r" (a)
                    : "r" (b));
    res = a;
    return carry;
}

uint64_t big_integer::mulWithCarry(uint64_t &res, uint64_t a, uint64_t b) {
    uint64_t carry = 0;
    asm volatile ("mov %2, %%rax \n"
                  "mul %1 \n"
                  "mov %%rax, %1 \n"
                  "mov %%rdx, %0"
    : "+r" (carry), "+r" (a)
    : "r" (b)
    : "rdx", "rax");
    res = a;
    return carry;
}

void big_integer::div128(uint64_t &res, uint64_t &remainder, uint64_t d1, uint64_t d2, uint64_t divider) {
    uint64_t r = 0;
    uint64_t rem = 0;
    asm volatile ("mov %2, %%rdx \n"
                  "mov %3, %%rax \n"
                  "mov %4, %%rcx \n"
                  "div %%rcx \n"
                  "mov %%rax, %0 \n"
                  "mov %%rdx, %1"
    : "=r" (r), "=r" (rem)
    : "r" (d1), "r" (d2), "r" (divider)
    : "rdx", "rax", "rcx");
    res = r;
    remainder = rem;
}

const big_integer& big_integer::operator++() {
    uint64_t carry = 1;
    bool sign = isPositive();

    for (size_t i = 0; carry != 0 && i < size; ++i) {
        carry = big_integer::addWithCarry(digits[i], digits[i], carry);
    }
    if (!carry)
        setSign(sign);
    return *this;
}

big_integer big_integer::operator-() const {
    big_integer res(*this);
    res.invert();
    return res;
}

big_integer big_integer::operator+() const {
    return (*this);
}

void big_integer::invert() {
    for (size_t i = 0; i < size; ++i) {
        digits[i] = ~digits[i];
    }
    this->operator++();
}

big_integer big_integer::addShort(uint64_t right) {
    big_integer res(*this);
    uint64_t carry = right;
    for (size_t i = 0; i < res.size; ++i) {
        carry = big_integer::addWithCarry(res.digits[i], res.digits[i], carry);
    }
    if (carry) {
        printf("+:resize");
        resize(res.size + 1);
        res.digits[res.size - 1] = ~UINT64_C(0);
    }
    return res;
}

big_integer operator+(big_integer lhs, big_integer const &rhs) {
    return lhs += rhs;
}

big_integer operator-(big_integer lhs, big_integer const &rhs) {
    return lhs -= rhs;
}

big_integer operator*(big_integer lhs, big_integer const &rhs) {
    return lhs *= rhs;
}

big_integer operator/(big_integer lhs, big_integer const &rhs) {
    return lhs /= rhs;
}

big_integer operator%(big_integer lhs, big_integer const &rhs) {
    return lhs %= rhs;
}

big_integer big_integer::mulShort(uint64_t right) {
    big_integer res(*this);
    bool positiveResult = right < 0 == res.isNegative();
    if (res.isNegative())
        res.invert();
    uint64_t carry = 0;
    for (size_t i = 0; i < res.size; ++i) {
        uint64_t c = big_integer::mulWithCarry(res.digits[i], res.digits[i], right);
        carry = big_integer::addWithCarry(res.digits[i], res.digits[i], carry);
        carry += c;
    }
    if (carry) {
        res.resize(res.size + 1);
        res.digits[res.size - 1] = carry;
    }
    if (!positiveResult)
        res.invert();
    res.setSign(positiveResult);
    return res;
}


big_integer& big_integer::operator+=(big_integer const &rhs) {
    uint64_t carry, newCarry;
    carry = newCarry = 0;
    int resultSign;
    if (rhs.isNegative() && isNegative())
        resultSign = -1;
    else if (!rhs.isNegative() && !isNegative())
        resultSign = 1;
    else
        resultSign = 0;
    resize(rhs.size);
    size_t len = std::min(size, rhs.size);
    size_t i;
    for (i = 0; i < len; ++i) {
        newCarry = big_integer::addWithCarry(digits[i], digits[i], rhs.digits[i]);
        newCarry += big_integer::addWithCarry(digits[i], digits[i], carry);
        carry = newCarry;
    }
    uint64_t highBit = rhs.isNegative() ? big_integer::NEGATIVE_SIGN : big_integer::POSITIVE_SIGN;
    while (i < size) {
        newCarry = big_integer::addWithCarry(digits[i], digits[i], carry);
        newCarry += big_integer::addWithCarry(digits[i], digits[i], highBit);
        carry = newCarry;
        i++;
    }
    if (resultSign) {
        setSign(resultSign == 1);
    }
    clear();
    return (*this);
}

big_integer& big_integer::operator-=(big_integer const &rhs) {
    this->invert();                 // -lhs
    this->operator+=(rhs);           // rhs - lhs
    this->invert();                 // lhs - rhs
    return (*this);
}

big_integer& big_integer::operator*=(big_integer rhs) {
    big_integer res;
    big_integer &left = *this;
    bool resultSign = (rhs.isNegative() == isNegative());
    if (rhs.isNegative())
        rhs.invert();
    if (isNegative())
        left.invert();
    resize(rhs.size);

    size_t len = std::min(size, rhs.size);
    size_t i;
    size_t shift = 0;

    for (i = 0; i < len; ++i) {
        res += left.mulShort(rhs.digits[i]) << shift;
        shift += 64;
    }
    if(!resultSign)
        res.invert();
    setSign(resultSign);
    swap(res, *this);
    this->clear();
    return (*this);
}

big_integer& big_integer::operator/=(big_integer rhs) {
    big_integer res;
    big_integer &left = *this;
    bool resultSign = (rhs.isNegative() == isNegative());
    if (rhs.isNegative())
        rhs.invert();
    if (isNegative())
        left.invert();
    if (rhs.size == 1) {
        uint64_t rem;
        this->div_rem(rhs.digits[0], rem);
        if(!resultSign)
            this->invert();
        return (*this);
    }
    if (*this < rhs) {
        *this = big_integer();
        return (*this);
    } else if (*this == rhs) {
        *this = big_integer(1);
        if (!resultSign)
            this->invert();
        return (*this);
    }

    uint64_t m = left.size - rhs.size;
    uint64_t shift = m;
    res.resize(m + 1);

    size_t i;
    for (i = rhs.size; i--; ) {
        if (rhs.digits[i] != 0) {
            break;
        }
    }
    if (i == -1) {
        throw std::runtime_error("Division by zero");
    }
    uint64_t k = rhs.digits[i];
    uint64_t sh = 0;
    while (static_cast<int64_t>(k) >= 0) {
        sh++;
        k <<= 1;
    }
    shift = (i) * 64 + sh;
    rhs <<= shift;
    left <<= shift;
    rhs.setSign(true);
    left.setSign(true);

    int realSizeRight = rhs.size - 1;
    while (realSizeRight >= 0 && rhs.digits[realSizeRight] == (rhs.isNegative() ? big_integer::NEGATIVE_SIGN : big_integer::POSITIVE_SIGN))
        realSizeRight--;
    realSizeRight++;

    int realSizeLeft = left.size - 1;
    while (realSizeLeft >= 0 && left.digits[realSizeLeft] == (left.isNegative() ? big_integer::NEGATIVE_SIGN : big_integer::POSITIVE_SIGN))
        realSizeLeft--;
    realSizeLeft++;

    big_integer highPart = rhs << 64 * (realSizeLeft - realSizeRight);
    if (left >= highPart) {
        res = 1;
        left -= highPart;
    }
    big_integer partLeft, partRight;

    partRight.resize(2);

    for (int i = 0; i < 2; i++) {
        partRight.digits[i] = 0;
    }
    for (int i = std::min(2, realSizeRight) - 1; i >= 0; --i) {
        partRight.digits[i] = rhs.digits[rhs.size - std::min(2, realSizeRight) + i - 1];
    }

    for (int j = realSizeLeft - realSizeRight - 1; j >= 0; j--) {
        partLeft.resize(3);
        for (int i = 0; i < 3; i++) {
            partLeft.digits[i] = 0;
        }

        int realSizeLeft = left.size - 1;
        while (realSizeLeft >= 0 && left.digits[realSizeLeft] == (left.isNegative() ? big_integer::NEGATIVE_SIGN : big_integer::POSITIVE_SIGN))
            realSizeLeft--;
        realSizeLeft++;
        for (int i = std::min(3, realSizeLeft) - 1; i >= 0; --i) {
            partLeft.digits[i] = left.digits[realSizeLeft - std::min(3, realSizeLeft) + i];
        }
        partLeft.setSign(true);
        partRight.setSign(true);
        uint64_t l, r, mid;
        l = 0;
        r = UINT64_MAX;

        while (r != l) {
            mid = r - (r - l) / 2;
            big_integer n = partRight.mulShort(mid);

            if (n < partLeft) {
                l = mid;
            } else if (n > partLeft) {
                r = mid - 1;
            } else {
                l = r = mid;
            }
        }

        uint64_t shift = std::max((realSizeLeft - realSizeRight - 1) * 64, 0);
        big_integer temp = rhs.mulShort(l) << shift;
        if (temp > left) {
            temp -= rhs << shift;
            l--;
        }
        left -= temp;
        res <<= 64;
        res = res.addShort(l);
    }
    res.setSign(true);
    if (!resultSign) {
        res.invert();
    }
    swap(*this, res);
    return (*this);
}

big_integer& big_integer::operator%=(const big_integer &rhs) {
    big_integer tmp(*this);
    *this -= (tmp / rhs * rhs);
    return *this;
}


big_integer& big_integer::operator&=(big_integer const &rhs) {
    if (size < rhs.size) {
        resize(rhs.size);
    }
    size_t i = 0;
    for ( ; i < rhs.size; ++i) {
        digits[i] &= rhs.digits[i];
    }
    uint64_t highBit = rhs.isNegative() ? big_integer::NEGATIVE_SIGN : big_integer::POSITIVE_SIGN;
    for ( ; i < size; ++i) {
        digits[i] &= highBit;
    }
    this->clear();

    return (*this);
}


big_integer& big_integer::operator|=(big_integer const &rhs) {
    if (size < rhs.size) {
        resize(rhs.size);
    }
    size_t i = 0;
    for ( ; i < rhs.size; ++i) {
        digits[i] |= rhs.digits[i];
    }
    uint64_t highBit = rhs.isNegative() ? big_integer::NEGATIVE_SIGN : big_integer::POSITIVE_SIGN;
    for ( ; i < size; ++i) {
        digits[i] |= highBit;
    }
    this->clear();

    return (*this);
}


big_integer& big_integer::operator^=(big_integer const &rhs) {
    if (size < rhs.size) {
        resize(rhs.size);
    }
    size_t i = 0;
    for ( ; i < rhs.size; ++i) {
        digits[i] ^= rhs.digits[i];
    }
    uint64_t highBit = rhs.isNegative() ? big_integer::NEGATIVE_SIGN : big_integer::POSITIVE_SIGN;
    for ( ; i < size; ++i) {
        digits[i] ^= highBit;
    }
    this->clear();

    return (*this);
}


void big_integer::div_rem(uint64_t divider, uint64_t& remainder) {
    uint64_t rem = 0;
    for (size_t i = size; i--; ) {
        big_integer::div128(digits[i], rem, rem, digits[i], divider);
    }
    remainder = rem;
}

// return: sign(lhs - rhs)
int big_integer::cmp(big_integer const &lhs, big_integer const &rhs) {
    size_t len = std::min(lhs.size, rhs.size);
    if (lhs.isNegative() && !rhs.isNegative()) {
        return -1;
    } else if (!lhs.isNegative() && rhs.isNegative()) {
        return 1;
    }

    for (size_t i = std::max(rhs.size, lhs.size) - 1; i >= len; i--) {
        uint64_t leftDigit, rightDigit;
        leftDigit = (i >= lhs.size) ? (lhs.isNegative() ? big_integer::NEGATIVE_SIGN : big_integer::POSITIVE_SIGN) : lhs.digits[i];
        rightDigit = (i >= rhs.size) ? (rhs.isNegative() ? big_integer::NEGATIVE_SIGN : big_integer::POSITIVE_SIGN) : rhs.digits[i];

        if (leftDigit != rightDigit) {
            int res = (leftDigit > rightDigit) ? 1 : -1;
            return res;
        }
    }

    for (size_t i = len; i--; ) {
        uint64_t leftDigit, rightDigit;
        leftDigit = lhs.digits[i];
        rightDigit = rhs.digits[i];

        if (leftDigit != rightDigit) {
            int res = (leftDigit > rightDigit) ? 1 : -1;
            return res;
        }
    }
    return 0;
}

bool operator!=(const big_integer &left, const big_integer &right) {
    return big_integer::cmp(left, right) != 0;
}

bool operator==(const big_integer &left, const big_integer &right) {
    return big_integer::cmp(left, right) == 0;
}

bool operator<(const big_integer &left, const big_integer &right) {
    return big_integer::cmp(left, right) < 0;
}

bool operator>(const big_integer &left, const big_integer &right) {
    return big_integer::cmp(left, right) > 0;
}

bool operator<=(const big_integer &left, const big_integer &right) {
    return big_integer::cmp(left, right) <= 0;
}

bool operator>=(const big_integer &left, const big_integer &right) {
    return big_integer::cmp(left, right) >= 0;
}

big_integer operator~(big_integer left) {
    for (size_t i = 0; i < left.size; ++i) {
        left.digits[i] = ~left.digits[i];
    }
    return left;
}

big_integer operator&(big_integer left, big_integer const& right) {
    return left &= right;
}

big_integer operator|(big_integer left, big_integer const& right) {
    return left |= right;
}

big_integer operator^(big_integer left, big_integer const& right) {
    return left ^= right;
}

uint64_t shl(uint64_t number, uint64_t shift) {
    return shift == 64 ? 0 : number << shift;
}

uint64_t shr(uint64_t number, uint64_t shift) {
    return shift == 64 ? 0 : number >> shift;
}

big_integer &big_integer::operator<<=(uint64_t shift) {
    if (shift == 0)
        return (*this);
    bool sign = isPositive();
    uint64_t full = shift / 64;
    uint64_t red = shift % 64;
    size_t oldSize = size;
    resize(full + size + 1);


    uint64_t highPart, lowPart;
    highPart = lowPart = 0;
    highPart = shr(digits[oldSize - 1], (64 - red));
    if (isNegative()) {
        highPart |= ~UINT64_C(0) << red;
    }
    digits[size - 1] = highPart;
    for (int i = oldSize - 1; i >= 1; i--) {
        highPart = shr(digits[i - 1], (64 - red));
        lowPart = digits[i] << red;
        lowPart |= highPart;
        digits[i + full] = lowPart;
    }
    digits[full] = digits[0] << red;
    std::fill(digits, digits + full, UINT64_C(0));
    this->setSign(sign);
    this->clear();
    return (*this);
}


big_integer &big_integer::operator>>=(uint64_t shift) {
    bool sign = isPositive();
    uint64_t full = shift / 64;
    uint64_t red = shift % 64;

    if (full > size) {
        big_integer tmp;
        swap(*this, tmp);
        return (*this);
    }
    uint64_t highPart, lowPart;
    highPart = lowPart = 0;
    for (int i = 0; i < size - full - 1; ++i) {
        highPart = shl(digits[i + 1], 64 - red);
        lowPart = shr(digits[i], red);
        lowPart |= highPart;
        digits[i] = lowPart;
    }
    lowPart = shr(digits[size - full - 1], red);
    if (isNegative()) {
        lowPart |= shl(~UINT64_C(0), 64 - red);
    }
    digits[size - full - 1] = lowPart;
    std::fill(digits, digits + full, UINT64_C(0));
    this->setSign(sign);
    this->clear();
    return (*this);
}

big_integer operator<<(big_integer left, uint64_t shift) {
    return left <<= shift;
}

big_integer operator>>(big_integer left, uint64_t shift) {
    return left >>= shift;
}

void big_integer::resize(size_t new_size) {
    if (!this->digits) {
        this->digits = allocate(new_size);
        this->size = this->capacity = new_size;
    }
    if (size >= new_size)
        return;
    uint64_t *tmp;
    size_t sign = isNegative() ? big_integer::NEGATIVE_SIGN : big_integer::POSITIVE_SIGN;
    if (capacity > new_size && new_size > 1) {
        tmp = digits;
    } else {
        tmp = allocate(new_size);
        std::copy(this->digits, this->digits + this->size, tmp);
        this->capacity = new_size;
        free(this->digits);
    }
    std::fill(tmp + this->size, tmp + new_size, sign);
    this->digits = tmp;
    this->size = new_size;
}

void big_integer::print(std::string s) const {
    printf("%s --> ", s.c_str());
    for (size_t i = size; i--; ) {
        printf("0x%016lx ", digits[i]);
    }
    std::cout << std::endl;
}

void big_integer::expandSign(size_t highestBit) {
    bool negative = static_cast<int64_t>(digits[highestBit]) < 0;
    std::fill(digits + highestBit + 1, digits + size, negative ? ~UINT64_C(0) : UINT64_C(0));
}

uint64_t *big_integer::allocate(size_t nElements) {
    uint64_t size = nElements * sizeof(uint64_t);
    return static_cast<uint64_t*>(malloc(size));
}

void big_integer::setSign(bool positive) {
    if (isNegative() == !positive) {
        return;
    }
    resize(size + 1);
    digits[size - 1] = positive ? big_integer::POSITIVE_SIGN : big_integer::NEGATIVE_SIGN;
}
