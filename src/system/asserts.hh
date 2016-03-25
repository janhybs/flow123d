/*!
 *
﻿ * Copyright (C) 2015 Technical University of Liberec.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 3 as published by the
 * Free Software Foundation. (http://www.gnu.org/licenses/gpl-3.0.en.html)
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 *
 * @file    asserts.hh
 * @brief   Definitions of ASSERTS.
 */

#ifndef ASSERTS_HH
#define ASSERTS_HH

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>

namespace feal {

/**
 * @brief Class defining debugging messages.
 *
 * Allows define assert, warning etc.
 */
class Assert {
public:
	/// Constructor.
	Assert(const std::string& expression)
	: FEAL_ASSERT_A (*this),
	  FEAL_ASSERT_B (*this),
	  expression_(expression) {}

	Assert& FEAL_ASSERT_A; ///< clever macro A
	Assert& FEAL_ASSERT_B; ///< clever macro B

	/// Adds name and value of variable
	template <typename T>
	Assert& add_value(T var_current_val, const char* var_name) {
		std::stringstream ss;
		ss << var_name << " : '" << var_current_val << "'";
		current_val_.push_back(ss.str());

		return *this;
	}

	/// Stores values for printing out line number, function, etc
	Assert& set_context(const char* file_name, const char* function, const int line)
	{
		file_name_ = file_name;
		function_ = function;
		line_ = line;

		return *this;
	}

	/// Generate error
	void error() {
		std::cout << "Program Error: Violated Assert! " << std::endl;
		print();
		//abort();
	}

	/// Generate warning
	void warning() {
		std::cout << "Warning: " << std::endl;
		print();
	}

protected:
	/// Print formatted assert message.
	void print()
	{
		std::cout << "> In file: " << file_name_ << "(" << line_ << "): Throw in function " << function_ << std::endl;
		std::cout << "> Expression: \'" << expression_ << "\'" << std::endl;
		if (current_val_.size()) {
			std::cout << "> Values:" << std::endl;
			for (auto val : current_val_) {
				std::cout << "  " << val << std::endl;
			}
		}
	}

	std::string expression_;                  ///< Assertion expression
	const char* file_name_;                   ///< Actual file.
	const char* function_;                    ///< Actual function.
	int line_;                                ///< Actual line.
	std::vector< std::string > current_val_;  ///< Formated strings of names and values of given variables.

};

/// Make an assertion
static Assert make_assert(const std::string& expression) {
	return Assert(expression);
}

} // namespace feal

// Must define the macros afterwards
/// Clever macro A
#define FEAL_ASSERT_A(x) FEAL_ASSERT_OP(x, B)
/// Clever macro B
#define FEAL_ASSERT_B(x) FEAL_ASSERT_OP(x, A)
/// Clever macro recursion
#define FEAL_ASSERT_OP(x, next) \
    FEAL_ASSERT_A.add_value((x), #x).FEAL_ASSERT_ ## next


#ifdef FLOW123D_DEBUG_ASSERTS
/// High-level macro
#define FEAL_ASSERT( expr) \
if ( (expr) ) ; \
else feal::make_assert( #expr).set_context( __FILE__, __func__, __LINE__).FEAL_ASSERT_A
#else
#define FEAL_ASSERT( expr)
#endif

/**
 * Sources:
 * http://www.drdobbs.com/cpp/enhancing-assertions/184403745
 * https://gist.github.com/hang-qi/5308285
 * https://beliefbox.googlecode.com/svn-history/r825/trunk/src/core/SmartAssert.h
 */

#endif // ASSERTS_HH
