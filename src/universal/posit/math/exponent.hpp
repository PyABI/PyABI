#pragma once
// exponent.hpp: exponent functions for posits
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace sw {	namespace unum {

// the current shims are NON-COMPLIANT with the posit standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// Base-e exponential function
template<size_t nbits, size_t es>
posit<nbits,es> exp(posit<nbits,es> x) {
	if (isnar(x)) return x;
	posit<nbits, es> p;
	double d = std::exp(double(x));
	if (d == 0.0) {
		minpos<nbits, es>(p);
	}
	else {
		p = d;
	}
	return p;
}

// Base-2 exponential function
template<size_t nbits, size_t es>
posit<nbits,es> exp2(posit<nbits,es> x) {
	if (isnar(x)) return x;
	posit<nbits, es> p;
	double d = std::exp2(double(x));
	if (d == 0.0) {
		minpos<nbits, es>(p);
	}
	else {
		p = d;
	}
	return p;
}

// Base-10 exponential function
template<size_t nbits, size_t es>
posit<nbits, es> exp10(posit<nbits, es> x) {
	return posit<nbits, es>(std::pow(10.0, double(x)));
}

// Base-e exponential function exp(x)-1
template<size_t nbits, size_t es>
posit<nbits,es> expm1(posit<nbits,es> x) {
	return posit<nbits,es>(std::expm1(double(x)));
}


}}  // namespace sw::unum
