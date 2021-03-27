#ifndef NET_RATIONAL_HPP
#define NET_RATIONAL_HPP
#include <numeric>
#include <ostream>

namespace net {

	struct rational {
		int numerator = 0;
		int denominator = 1;

		rational() = default;
		rational(const int & num);
		rational(const int & num, const int & denom);
		rational(const rational &) = default;
		~rational() = default;

		double to_double() const;
		int to_int() const;
	};

	inline rational::rational(const int & num) {
		numerator = num;
		denominator = 1;
	}

	inline rational::rational(const int & num, const int & denom) {
		int gcd_val;
		if (num == 0 && denom == 0) {
			numerator = 0;
			denominator = 0;
		} else if (num == 0 && denom > 0) {
			numerator = 0;
			denominator = 1;
		} else if (num == 0 && denom < 0) {
			numerator = 0;
			denominator = -1;
		} else if (num > 0 && denom == 0) {
			numerator = 1;
			denominator = 0;
		} else if (num < 0 && denom == 0) {
			numerator = -1;
			denominator = 0;
		} else {
			gcd_val = std::gcd(num, denom);
			if (denominator < 0) {
				numerator = -num / gcd_val;
				denominator = -denom / gcd_val;
			} else {
				numerator = num / gcd_val;
				denominator = denom / gcd_val;
			}
		}
	}

	inline double rational::to_double() const {
		return double(numerator) / double(denominator);
	}
	inline int rational::to_int() const {
		return numerator / denominator;
	}

	inline bool operator<(const rational & a, const rational & b) {
		return a.numerator * b.denominator < a.denominator * b.numerator;
	}

	inline bool operator<=(const rational & a, const rational & b) {
		return a.numerator * b.denominator <= a.denominator * b.numerator;
	}

	inline bool operator>(const rational & a, const rational & b) {
		return a.numerator * b.denominator > a.denominator * b.numerator;
	}

	inline bool operator>=(const rational & a, const rational & b) {
		return a.numerator * b.denominator >= a.denominator * b.numerator;
	}

	inline bool operator==(const rational & a, const rational & b) {
		return a.numerator * b.denominator == a.denominator * b.numerator;
	}

	inline bool operator!=(const rational & a, const rational & b) {
		return a.numerator * b.denominator != a.denominator * b.numerator;
	}

	inline std::ostream & operator<<(std::ostream & os, const rational & a) {
		return os << a.numerator << '/' << a.denominator;
	}

	inline rational operator+(const rational & a, const rational & b) {
		return rational(a.numerator * b.denominator + a.denominator * b.numerator, a.denominator * b.denominator);
	}

	inline rational operator-(const rational & a, const rational & b) {
		return rational(a.numerator * b.denominator - a.denominator * b.numerator, a.denominator * b.denominator);
	}

	inline rational operator-(const rational & a) {
		return rational(-a.numerator, a.denominator);
	}

	inline rational operator*(const rational & a, const rational & b) {
		return rational(a.numerator * b.numerator, a.denominator * b.denominator);
	}

	inline rational operator/(const rational & a, const rational & b) {
		return rational(a.numerator * b.denominator, a.denominator * b.numerator);
	}

	inline rational & operator+=(rational & a, const rational & b) {
		a = a + b;
		return a;
	}

	inline rational & operator-=(rational & a, const rational & b) {
		a = a - b;
		return a;
	}

	inline rational & operator*=(rational & a, const rational & b) {
		a = a * b;
		return a;
	}

	inline rational & operator/=(rational & a, const rational & b) {
		a = a / b;
		return a;
	}

} // namespace net
#endif