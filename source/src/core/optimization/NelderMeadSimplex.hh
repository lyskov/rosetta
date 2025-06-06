// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   core/optimization/Minimizer.hh
/// @brief  Simple low-level minimizer class
/// @author Phil Bradley


#ifndef INCLUDED_core_optimization_NelderMeadSimplex_hh
#define INCLUDED_core_optimization_NelderMeadSimplex_hh

// Package headers
#include <core/optimization/types.hh>
#include <core/optimization/MinimizerOptions.hh>
#include <core/optimization/Multifunc.fwd.hh>


namespace core {
namespace optimization {

class NelderMeadSimplex {
public:
	NelderMeadSimplex( Multifunc & func_in, MinimizerOptions const & options_in );

	Real
	run(
		Multivec & phipsi_inout,
		Multivec const& upperbound
	);

	Multifunc & func_;
	MinimizerOptions options_;
}; // Minimizer

} // namespace optimization
} // namespace core


#endif // INCLUDED_core_optimization_Minimizer_HH
