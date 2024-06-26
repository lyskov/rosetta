// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   protocols/simple_filters/PoolEvaluatorCreator.hh
/// @brief  Header for PoolEvaluatorCreator
/// @author Matthew O'Meara

// Unit Headers
#include <protocols/simple_filters/PoolEvaluatorCreator.hh>

// Package Headers

// Package Headers
#include <protocols/evaluation/PoseEvaluator.fwd.hh>
#include <protocols/evaluation/PoseEvaluator.hh>
#include <protocols/canonical_sampling/mc_convergence_checks/Pool_ConvergenceCheck.hh>


// ObjexxFCL Headers

// Utility headers
#include <utility/file/FileName.hh>

#include <basic/options/option.hh>
#include <basic/Tracer.hh>

// due to template function

// option key includes
#include <basic/options/option_macros.hh>
#include <basic/options/keys/evaluation.OptionKeys.gen.hh>

#ifdef WIN32
#include <core/scoring/constraints/Constraint.hh>
#endif


static basic::Tracer tr( "protocols.evalution.PoolEvaluatorCreator" );

namespace protocols {
namespace simple_filters {

PoolEvaluatorCreator::~PoolEvaluatorCreator() = default;

void PoolEvaluatorCreator::register_options() {
	using namespace basic::options;
	if ( options_registered_ ) return;
	options_registered_ = true;

	OPT( evaluation::pool );

}

void PoolEvaluatorCreator::add_evaluators( evaluation::MetaPoseEvaluator & eval ) const {
	using namespace core;
	using namespace basic::options;
	using namespace basic::options::OptionKeys;
	using protocols::evaluation::PoseEvaluatorOP;


	if ( option[ OptionKeys::evaluation::pool ].user() ) {
		using namespace protocols::canonical_sampling::mc_convergence_checks;
		Pool_RMSD_OP pool_ptr( new Pool_RMSD( option[ OptionKeys::evaluation::pool ]() ) );
		eval.add_evaluation( utility::pointer::make_shared< Pool_Evaluator >( pool_ptr ) );
	}


}

std::string PoolEvaluatorCreator::type_name() const {
	return "PoolEvaluatorCreator";
}

} //namespace
} //namespace
