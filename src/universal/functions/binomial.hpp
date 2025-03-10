#pragma once
// binomial.hpp: definition of recursive binomial coefficient function
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
namespace function {

// BinomialCoefficient calculates the binomial coefficience recursively
// (n over k) = (n-1 over k-1) + (n-1 over k)
template<typename Scalar>
Scalar binomial(const Scalar& n, const Scalar& k) {
	Scalar coef;
	if (k == 0 || k == n || n == 0) {
		coef = Scalar(1);
	}
	else {
		coef = binomial(n - 1, k - 1) + binomial(n - 1, k);
	}
	return coef;
}

/*
	// this is not an appropriate algorithm to calculate binomial coefficients

	uint64_t factorial(uint64_t n) {
		return (n == 0 || n == 1) ? 1 : factorial(n - 1) * n;
	}

	// Calculate (n over k) via the relationship (n over k) = n! / k!(n-k)!
	// this is computationally difficult due to the large numbers
	// generated by the factorials
	template<typename Scalar>
	Scalar BinomialCoefficient_factorial(uint64_t n, uint64_t k) {
		Scalar numerator = Scalar(factorial(n));
		Scalar denominator = Scalar(factorial(k)*factorial(n - k));
		Scalar coef = numerator / denominator;
		//	std::cout << numerator << " / " << denominator << " = " << coef << std::endl;
		if (coef * denominator != numerator) std::cout << "FAIL: (" << n << " over " << k << ")  coef = " << coef << " ( " << numerator << "/" << denominator << ") error = " << numerator - (coef * denominator) << std::endl;
		return coef;
	}
*/

}  // namespace function
}  // namespace sw
