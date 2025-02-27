// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington UW TechTransfer, email: license@u.washington.edu.

/// @file    core/scoring/epr_deer/metrics/DEERDistanceDistribution.cc
/// @brief   Container for DEER experimental data and dataype-specific scoring function
/// @detail  This class stores the data as a probability distribution and
///        tries to maximize some function (see derived types).
/// @author   Diego del Alamo ( del.alamo@vanderbilt.edu )

// Unit headers
#include <core/scoring/epr_deer/metrics/DEERDistanceDistribution.hh>
#include <core/scoring/epr_deer/util.hh>

// Project headers
#include <core/types.hh>

// Basic headers
#include <basic/Tracer.hh>

// Utility headers
#include <utility/excn/Exceptions.hh>
#include <utility/vector1.hh>

// Numeric headers

// Basic headers

// C++ headers
#include <map>

#include <cmath> // MANUAL IWYU

namespace core {
namespace scoring {
namespace epr_deer {
namespace metrics {

/// @brief Tracer used for error messages
/// @details Global to avoid re-instantiating tracer with every new object
static basic::Tracer TR( "core.scoring.epr_deer.metrics.DEERDistanceDistribution" );

/// @brief  Virtual function to evaluate score given a distribution
/// @param  sim_histr: Simulated DEER distribution
/// @return Freshly computed score
/// @detail This is the default method when using DEER distance distributions.
///    It computes the cross entropy of the distribution, also
///    called the negative log-likelihood. It contains three loops.
///    First, it iterates over each bin in either the simulated
///    or the experimental DEER distance distribution. Second,
///    it iterates over a series of simulated DEER distribution
///    generated by the taking into account the possibility of
///    backbone dynamics. Finally, it iterates over a range of
///    possible experimental P(r) values obtained from confidence
///    bands (a range of 2-sigma, or 95% confidence, is assumed)
///    Specialized methods can be used instead by declaring in
///    the file being read. See DEERIO.cc for details
Real
DEERDistanceDistribution::get_score(
	std::map< Size, Real > const & sim_histr
) {

	// If backbone dynamics are being modeled and have not been determined
	if ( exp_stdev_ == 0.0 && bb_ ) {
		exp_stdev_ = avg_stdev( distr_ )[ 2 ];
	}

	// Backbone dynamics are modeled using a series of DEER distributions
	//  See notes for the function expand_hists() for details
	auto hists = ( bb_ ) ? expand_hists( sim_histr )
		: utility::vector1< std::map< Size, Real > >{ sim_histr };

	// Set the score
	Real cross_entropy = 0.0;

	// Outermost loop
	// Iterate across the bins in the widest experimental distribution
	for ( auto const & bin_amp : ( ( rev_ ) ? hists.back() : distr_ ) ) {
		// Set the bin
		Size const & bin = bin_amp.first;

		// Initialize the probability of the model given the bin
		Real bin_prob = 0.0;

		// Second loop
		// Iterate over all sub-distributions (if bb_ is set to true)
		for ( auto const & hist : hists ) {

			// Amplitude of simulated DEER distribution at this point
			Real const & sim_pr =
				( hist.find( bin ) != hist.end() ) ? hist.at( bin ) : 0.0;

			// Innermost loop
			// Get range of P(r) values from confidence bands
			auto const exp_prs = get_prs( bin );

			for ( auto const & exp_pr : exp_prs ) {
				// Compute probability of this observation
				Real const prob = ( rev_ )
					? pow( exp_pr, sim_pr ) : pow( sim_pr, exp_pr );

				// Add it to the total bin probability
				bin_prob += prob / ( exp_prs.size() * hists.size() );
			}
		}

		// Add the natural logarithm of this bin probability to the score
		cross_entropy += -1 * ln( bin_prob );

	}

	// Return the score
	return cross_entropy;
}

/// @brief Expand distance distribution into multiple distributions
/// @param distr: Simulated distribution to expand
/// @return Vector of distributions
/// @detail Used for Bayesian analysis when backbone dynamics are not known
///    and several possibilities must be taken into account
utility::vector1< std::map< Size, Real > >
DEERDistanceDistribution::expand_hists(
	std::map< Size, Real > const & distr
) {

	// Get standard deviation of the simulated distribution
	Real const sim_stdev = avg_stdev( distr )[ 2 ];

	// Get the minumum backbone dynamics required to reproduce experimental
	//  distribution
	Real const min_sig = sqrt( std::max( 0.0,
		pow( exp_stdev_, 2 ) - pow( sim_stdev, 2 ) ) );

	// Generate the distributions
	utility::vector1< std::map< Size, Real > > output;
	for ( Real sig = min_sig; sig <= exp_stdev_; sig += 0.1 ) {
		output.push_back( convolute( distr, sig ) );
	}

	// Return the original distribution if the output was empty
	return ( !output.size() ) ? output
		: utility::vector1< std::map< Size, Real > >{ distr };
}

/// @brief Get range of possible P(r) values for experimental distribution
/// @param  bin: Value of r
/// @return Vector of P(r) for Bayesian scoring with confidence bands
utility::vector1< Real >
DEERDistanceDistribution::get_prs(
	Size const & bin
) const {

	// Check if the bin is in fact in the experimental distribution
	if ( distr_.find( bin ) != distr_.end() ) {

		// If we are using confidence bands when calculating score:
		if ( bounds_ ) {

			// Make sure all these vectors are the same size
			if ( distr_.size() != lower_bound_.size()
					|| distr_.size() != upper_bound_.size()
					) {
				throw CREATE_EXCEPTION( utility::excn::RangeError,
					"DEERDistanceDistribution::distr_.size() != "
					"DEERDistanceDistribution::lower_bound_.size() or "
					"DEERDistanceDistribution::distr_.size() != "
					"DEERDistanceDistribution::upper_bound_.size()" );
			}

			// The eventual output
			utility::vector1< Real > exp_prs;

			// Iterate across probit function from 0.025 to 0.975 in stages
			// First, use the lower probability for 0.025 to 0.475 (inclusive)
			// Here it is 0.48 so that minor floating point calculation
			// errors don't prevent 0.475 from being read
			Real const d1 = distr_.at( bin ) - lower_bound_.at( bin );
			for ( Real x = 0.025; x <= 0.48; x += 0.025 ) {

				// Figure out how many std deviations away given probit,
				//  multiply by that value, and deduct from best fit P(r)
				Real const val = distr_.at( bin ) + probit( x ) * d1 / 2;

				// Push back, or if it is less than zero than just use zero
				exp_prs.push_back( std::max( 0.0, val ) );
			}

			// Second, append the best fit (for x = 0.50, or at the mean)
			exp_prs.push_back( distr_.at( bin ) );

			// Third, use the upper probability for 0.525 to 0.975 (inclusive)
			// Here it is 0.98 so that minor floating point calculation
			// errors don't prevent 0.975 from being read
			Real const d2 = upper_bound_.at( bin ) - distr_.at( bin );
			for ( Real x = 0.525; x <= 0.98; x += 0.025 ) {

				// Push back
				exp_prs.push_back( distr_.at( bin ) + probit( x ) * d2 / 2 );
			}

			// Return output
			return exp_prs;

			// If this method for P(r) is not being used, return the best fit
		} else {
			return utility::vector1< Real >{ distr_.at( bin ) };
		}

		// If this bin hasn't even been defined, return zero
	} else {
		return { 0.0 };
	}
}

/// @brief Sets the lower bound/confidence band for the distance distribution
/// @param val: Lower bound or confidence band. Note: Can be negative!
void
DEERDistanceDistribution::lower_bound(
	std::map< Size, Real > const & val
) {
	lower_bound_ = val;
}

/// @brief Sets the line of best fit
/// @param val: Best fit of DEER distribution
void
DEERDistanceDistribution::best_fit(
	std::map< Size, Real > const & val
) {
	distr_ = val;
}

/// @brief Sets the upper bound/confidence band for the distance distribution
/// @param val: Lower bound or confidence band.
void
DEERDistanceDistribution::upper_bound(
	std::map< Size, Real > const & upper_bound
) {
	upper_bound_ = upper_bound;
}

/// @brief Set whether confidence bands will be used when calculating score
/// @param  val: Whether to use confidence bands
void
DEERDistanceDistribution::bounds(
	bool const & val
) {
	bounds_ = val;
}

/// @brief Set whether to compute the reverse metric
/// @param  val: Whether to compute the reverse metric
void
DEERDistanceDistribution::reverse(
	bool const & val
) {
	rev_ = val;
}

/// @brief Set whether to use confidence bands
/// @param  val: Whether to use confidence bands
void
DEERDistanceDistribution::bb(
	bool const & val
) {
	bb_ = val;
}

/// @brief Set whether to use a single distance
/// @param  val: Whether to use a single distance
void
DEERDistanceDistribution::singleval(
	bool const & val
) {
	singleval_ = val;
}

/// @brief Set whether to compute the integral
/// @param  val: Whether to compute the integral
void
DEERDistanceDistribution::integral(
	bool const & val
) {
	integral_ = val;
}

/// @brief  Returns the lower bound/confidence band for the distribution
/// @return The lower bound/confidence band for the distribution
std::map< Size, Real > const &
DEERDistanceDistribution::lower_bound() const {
	return lower_bound_;
}

/// @brief Returns the best fit of the distance distribution
/// @brief The best fit of the distance distribution
std::map< Size, Real > const &
DEERDistanceDistribution::best_fit() const {
	return distr_;
}

/// @brief  Returns the upper bound/confidence band for the distribution
/// @return The upper bound/confidence band for the distribution
std::map< Size, Real > const &
DEERDistanceDistribution::upper_bound() const {
	return upper_bound_;
}

/// @brief  Returns whether confidence bands are being used
/// @return Whether confidence bands are being used
bool
DEERDistanceDistribution::bounds() const {
	return bounds_;
}

/// @brief  Returns whether the reverse metric is being used
/// @return Whether the reverse metric is being used
bool
DEERDistanceDistribution::reverse() const {
	return rev_;
}

/// @brief  Returns whether backbone expansion is being used
/// @return Whether backbone expansion is being used
bool
DEERDistanceDistribution::bb() const {
	return bb_;
}

/// @brief  Returns whether calculation of a single distance is being used
/// @return Whether calculation of a single distance is being used
bool
DEERDistanceDistribution::singleval() const {
	return singleval_;
}

/// @brief  Returns whether calculation of the integral is being used
/// @return Whether calculation of the integral is being used
bool
DEERDistanceDistribution::integral() const {
	return integral_;
}

} // namespace metrics
} // namespace epr_deer
} // namespace scoring
} // namespace core
