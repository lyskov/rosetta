// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file protocols/simple_moves/sidechain_moves/SidechainMCMover.cc
/// @brief implementation of SidechainMCMover class and functions
/// @author Oliver Lange


#include <devel/coupled_sidechains/CoupledSidechainProtocol2.hh>
//#include <devel/coupled_sidechains/CoupledSidechainProtocolCreator.hh>


// Core Headers
#include <core/types.hh>

#include <core/pack/task/TaskFactory.hh>
#include <core/pack/task/operation/TaskOperations.hh>



#include <core/scoring/ScoreFunctionFactory.hh>
#include <core/scoring/ScoreFunction.fwd.hh>

#include <core/pose/Pose.hh>

#include <basic/Tracer.hh>
//Auto Headers
#include <basic/options/option_macros.hh>
#include <basic/options/keys/run.OptionKeys.gen.hh>
#include <basic/options/keys/score.OptionKeys.gen.hh>
#include <basic/options/keys/packing.OptionKeys.gen.hh>

#include <protocols/canonical_sampling/MetropolisHastingsMover.hh>
#include <protocols/canonical_sampling/SidechainMetropolisHastingsMover.hh>
#include <protocols/moves/MonteCarlo.hh>
#include <protocols/canonical_sampling/SilentTrajectoryRecorder.hh>
#include <protocols/canonical_sampling/SimulatedTempering.hh>
#include <protocols/simple_moves/sidechain_moves/PerturbRotamerSidechainMover.hh>
#include <protocols/simple_moves/sidechain_moves/JumpRotamerSidechainMover.hh>
#include <protocols/simple_moves/sidechain_moves/PerturbChiSidechainMover.hh>

#include <utility/tag/Tag.fwd.hh>

#include <basic/options/option.hh> // AUTO IWYU For OptionCollection, option

// Numeric Headers

// Auto-header: duplicate removed #include <basic/Tracer.hh>

// C++ Headers

#ifdef WIN_PYROSETTA
#include <protocols/canonical_sampling/ThermodynamicObserver.hh>
#endif


using namespace core;
using namespace core::pose;
using namespace basic;
using namespace protocols::moves;
using namespace protocols;

static basic::Tracer tr( "devel.coupled_sidechains.CoupledSidechainProtocol" );

OPT_1GRP_KEY(Integer,rotamers,observer_stride)
OPT_1GRP_KEY(Real,rotamers,unif)
OPT_1GRP_KEY(Real,rotamers,pert)
OPT_1GRP_KEY(Real,rotamers,within)
OPT_1GRP_KEY(Real,rotamers,pert_magnitude)


bool devel::coupled_sidechains::CoupledSidechainProtocol::options_registered_( false );

void devel::coupled_sidechains::CoupledSidechainProtocol::register_options() {
	using namespace basic::options;
	using namespace OptionKeys;
	if ( options_registered_ ) return;
	options_registered_ = true;
	protocols::canonical_sampling::SimulatedTempering::register_options();
	protocols::canonical_sampling::SilentTrajectoryRecorder::register_options();
	// protocols::canonical_sampling::SilentTrajectoryRecorder::register_options();
	//protocols::canonical_sampling::SilentTrajectoryRecorder::register_options();
	OPT( score::weights );
	OPT( score::patch );
	OPT( run::n_cycles );
	OPT( packing::resfile ); // is handled directly by SidechainMoverBase
	NEW_OPT( rotamers::observer_stride," how often should the entire score-line be written to file", 100);

	NEW_OPT( rotamers::unif,"",0.1);
	NEW_OPT( rotamers::pert,"",0);
	NEW_OPT( rotamers::pert_magnitude, "deviation for perturb_chi moves in degree", 10 );
	NEW_OPT( rotamers::within,"",0);
}

namespace devel {
namespace coupled_sidechains {
/*
std::string
CoupledSidechainProtocolCreator::keyname() const {
return CoupledSidechainProtocolCreator::mover_name();
}

protocols::moves::MoverOP
CoupledSidechainProtocolCreator::create_mover() const {
return new CoupledSidechainProtocol;
}

std::string
CoupledSidechainProtocolCreator::mover_name() {
return "SidechainMC";
}
*/
CoupledSidechainProtocol::CoupledSidechainProtocol()  : protocols::moves::Mover()
{
	// set_defaults();
	init_from_options();
	setup_objects();
}

protocols::moves::MoverOP
CoupledSidechainProtocol::clone() const {
	return( utility::pointer::make_shared< CoupledSidechainProtocol >( *this ) );
}

protocols::moves::MoverOP
CoupledSidechainProtocol::fresh_instance() const {
	return (utility::pointer::make_shared< CoupledSidechainProtocol >());
}

void
CoupledSidechainProtocol::init_from_options() {
	if ( !options_registered_ ) return;

	using namespace options;
	using namespace options::OptionKeys;
	// core::Real unif = option[ rotamers::unif ]; // Unused variable causes warning.
	core::Real pert = option[ rotamers::pert ];
	core::Real within = option[ rotamers::within ];
	ntrials_=option[ run::n_cycles ]();


	prob_withinrot_ = within;
	prob_pert_chi_ = pert;
	prob_jump_rot_ = 1.0-pert-within;
	stride_ = option[ rotamers::observer_stride ];
	pert_magnitude_ = option[ rotamers::pert_magnitude ];

}

void
CoupledSidechainProtocol::setup_objects() {

	scoring::ScoreFunctionOP scorefunction( core::scoring::get_score_function() );

	core::pack::task::TaskFactoryOP task_factory( new core::pack::task::TaskFactory );
	task_factory->push_back( utility::pointer::make_shared< core::pack::task::operation::RestrictToRepacking >() );

	protocols::canonical_sampling::SimulatedTemperingOP tempering( new protocols::canonical_sampling::SimulatedTempering() );
	moves::MonteCarloOP mc_object( new moves::MonteCarlo( *scorefunction, 0.6 ) );
	tempering->set_monte_carlo( mc_object );

	sampler_ = utility::pointer::make_shared< protocols::canonical_sampling::SidechainMetropolisHastingsMover >( stride_ );
	sampler_->set_monte_carlo( mc_object );
	sampler_->set_tempering( tempering );
	sampler_->add_observer( utility::pointer::make_shared< protocols::canonical_sampling::SilentTrajectoryRecorder >() );
	sampler_->set_ntrials( ntrials_ );
	if ( prob_withinrot_ > 0.0 ) {
		protocols::simple_moves::sidechain_moves::SidechainMoverBaseOP mover( new protocols::simple_moves::sidechain_moves::PerturbRotamerSidechainMover );
		//  mover->set_task_factory( task_factory );
		sampler_->add_mover( mover, prob_withinrot_ );
	}

	if ( prob_pert_chi_ > 0.0 ) {
		protocols::simple_moves::sidechain_moves::PerturbChiSidechainMoverOP mover( new protocols::simple_moves::sidechain_moves::PerturbChiSidechainMover() );
		mover->set_magnitude( pert_magnitude_ );
		//  mover->set_task_factory( task_factory );
		sampler_->add_mover( mover, prob_pert_chi_ );
	}

	if ( prob_jump_rot_ > 0.0 ) {
		protocols::simple_moves::sidechain_moves::SidechainMoverBaseOP mover( new protocols::simple_moves::sidechain_moves::JumpRotamerSidechainMover() );
		//  mover->set_task_factory( task_factory );
		sampler_->add_mover( mover, prob_jump_rot_ );
	}

}

/// @details
void
CoupledSidechainProtocol::apply(
	Pose & pose
)
{
	sampler_->apply( pose );
}

std::string
CoupledSidechainProtocol::get_name() const {
	return "CoupledSidechainProtocol";
}

void
CoupledSidechainProtocol::parse_my_tag( utility::tag::TagCOP const /*tag*/, basic::datacache::DataMap & /*data*/ ) {
	/* ntrials_ = tag->getOption<core::Size>( "ntrials", 10000 );
	set_prob_uniform( tag->getOption<core::Real>( "prob_uniform", 0.0 ) );
	set_prob_withinrot( tag->getOption<core::Real>( "prob_withinrot", 0.0 ) );
	set_prob_random_pert_current( tag->getOption<core::Real>( "prob_random_pert_current", 0.0 ) );
	core::Real between_rot = 1.0 - prob_uniform() - prob_withinrot () - prob_random_pert_current();
	set_scorefunction(
	protocols::rosetta_scripts::parse_score_function(tag, data)->clone() );
	*/
}

} // moves
} // protocols
