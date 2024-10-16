// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file core/energy_methods/DumpTrajectoryEnergy.cc
/// @brief An EnergyMethod that dumps trajectories to file.
/// @details Dumps trajectories of the minimizer and packer to file when the dump_trajectory
/// ScoreType is enable. Output may be controlled through the dump_trajectory:* flags.
/// @author Brian Coventry (bcov@uw.edu). - dump minimizer
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org) - Added support for dumping during packer trajectories.

// Unit headers
#include <core/energy_methods/DumpTrajectoryEnergy.hh>
#include <core/energy_methods/DumpTrajectoryEnergyCreator.hh>

// Package headers
#include <core/scoring/methods/EnergyMethodOptions.hh>
#include <core/conformation/Residue.hh>
#include <core/pose/Pose.hh>
#include <core/io/pdb/pdb_writer.hh>

// Options system

// Utility headers
#include <utility/sys_util.hh>

// Other Headers
#include <basic/Tracer.hh>
#include <utility/vector1.hh>
#include <utility/pointer/owning_ptr.hh>

#include <core/io/StructFileRepOptions.hh> // AUTO IWYU For StructFileRepOptions
#include <iomanip> // AUTO IWYU For operator<<, setfill, setw

namespace core {
namespace energy_methods {

static basic::Tracer TR( "core.energy_methods.DumpTrajectoryEnergy" );

/// @brief This must return a fresh instance of the DumpTrajectoryEnergy class, never an instance already in use.
///
core::scoring::methods::EnergyMethodOP
DumpTrajectoryEnergyCreator::create_energy_method( core::scoring::methods::EnergyMethodOptions const & options ) const
{
	return utility::pointer::make_shared< DumpTrajectoryEnergy >( options );
}

/// @brief Defines the score types that this energy method calculates.
///
core::scoring::ScoreTypes
DumpTrajectoryEnergyCreator::score_types_for_method() const
{
	using namespace core::scoring;
	ScoreTypes sts;
	sts.push_back( dump_trajectory );
	return sts;
}

/// @brief Options constructor.
///
DumpTrajectoryEnergy::DumpTrajectoryEnergy ( core::scoring::methods::EnergyMethodOptions const &options ) :
	parent1( utility::pointer::make_shared< DumpTrajectoryEnergyCreator >() ),
	parent2( ),
	state_( IDLE ),
	dump_filename_( "" ),
	dumped_frames_( 0 ),
	dump_prefix_( options.dump_trajectory_prefix() ),
	dump_as_gz_( options.dump_trajectory_gz() ),
	dump_on_every_nth_call_( options.dump_trajectory_stride() ),
	call_count_(0)
{}

/// @brief Copy constructor.
/// @details Must move to IDLE before we duplicate.
DumpTrajectoryEnergy::DumpTrajectoryEnergy( DumpTrajectoryEnergy const &src ) :
	parent1( utility::pointer::make_shared< DumpTrajectoryEnergyCreator >() ),
	parent2( src ) {

	// Move to IDLE first. This class abuses mutable so we must do this first.
	src.try_move_to_idle();
	assert( src.state_ == IDLE );

	state_ = src.state_;
	dump_filename_ = src.dump_filename_;
	dumped_frames_ = src.dumped_frames_;
	dump_prefix_ = src.dump_prefix_;
	dump_as_gz_ = src.dump_as_gz_;
	dump_on_every_nth_call_ = src.dump_on_every_nth_call_;
	call_count_ = 0; //New object has not yet been called.
}

/// @brief Default destructor.
///
DumpTrajectoryEnergy::~DumpTrajectoryEnergy() {}

/// @brief Clone: create a copy of this object, and return an owning pointer
/// to the copy.
core::scoring::methods::EnergyMethodOP DumpTrajectoryEnergy::clone() const {
	return utility::pointer::make_shared< DumpTrajectoryEnergy >(*this);
}

/// @brief DumpTrajectoryEnergy is context-independent and thus indicates that no context graphs need to be maintained by
/// class Energies.
void DumpTrajectoryEnergy::indicate_required_context_graphs( utility::vector1< bool > & /*context_graphs_required*/ ) const
{
	//Do nothing.
	return;
}

/// @brief DumpTrajectoryEnergy is version 2 right now.
///
core::Size DumpTrajectoryEnergy::version() const
{
	return 2;
}


/********************************************************************
Minimizing
********************************************************************/

/// @brief This is where the state_ moves to SCORING.
void
DumpTrajectoryEnergy::setup_for_scoring(
	core::pose::Pose &,//pose,
	core::scoring::ScoreFunction const &//sfxn
) const {
	if ( state_ == IDLE ) {
		call_count_ = 0; //Reset the call count if we're not already scoring.
		start_new_activity(SCORING);
	}
}

/// @brief Dump out a pose whenever we evaluate the energy.
void
DumpTrajectoryEnergy::finalize_total_energy(
	pose::Pose & pose,
	core::scoring::ScoreFunction const &,// sfxn,
	core::scoring::EnergyMap &//total_energy
) const {
	if ( state_ == SCORING ) { //i.e. we're not in another sort of trajectory
		dump_pose(pose);
	}
}

/// @brief This is where the state_ moves to MINIMIZING.
///
void
DumpTrajectoryEnergy::setup_for_minimizing( pose::Pose & , core::scoring::ScoreFunction const & , kinematics::MinimizerMapBase const &) const {
	start_new_activity( MINIMIZING );

}

/// @brief Reset the state to IDLE
void
DumpTrajectoryEnergy::finalize_after_minimizing(
	core::pose::Pose &pose
) const {
	runtime_assert( state_ == MINIMIZING );
	if ( (call_count_ - 1) % dump_on_every_nth_call_ != 0 ) {
		dump_pose(pose, true); //Force a write of the end of the trajectory if we didn't write on the last function call.
	}
	state_ = IDLE;
}

/// @brief This is where the minimizer frames are dumped.
/// @details Assumption: All minimizers must evaluate derivatives repeatedly while minimizing.
/// Although these won't be equally spaced, dumping them is the best we can do without modifying the minimizers.
void
DumpTrajectoryEnergy::setup_for_derivatives( pose::Pose & pose, core::scoring::ScoreFunction const & ) const {
	dump_pose( pose );
}


/********************************************************************
Packing
********************************************************************/


/// @brief This is where the state_ moves to PACKING.
///
void
DumpTrajectoryEnergy::set_up_residuearrayannealableenergy_for_packing (
	core::pose::Pose &,
	core::pack::rotamer_set::RotamerSets const &,
	core::scoring::ScoreFunction const &/*sfxn*/
) {
	start_new_activity( PACKING );
}

/// @brief Switch state back to IDLE.
void
DumpTrajectoryEnergy::clean_up_residuearrayannealableenergy_after_packing(
	core::pose::Pose &pose
) {
	runtime_assert( state_ == PACKING );
	if ( (call_count_ - 1) % dump_on_every_nth_call_ != 0 ) {
		dump_pose(pose, true); //Force a write of the end of the trajectory if we didn't write on the last function call.
	}
	state_ = IDLE;
}

/// @brief Should be possible to dump the packer from here. Not implemented yet though
///
core::Real
DumpTrajectoryEnergy::calculate_energy(
	utility::vector1< core::conformation::ResidueCOP > const &residues,
	utility::vector1< core::Size > const &,
	core::Size const /*substitution_position = 0*/
) const {
	if ( call_count_ % dump_on_every_nth_call_ == 0 ) {
		core::pose::Pose pose;
		for ( core::Size i(1), imax(residues.size()); i<=imax; ++i ) {
			if ( i == 1 || !residues[i]->is_polymer() || !residues[i]->is_polymer_bonded(i-1) )  {
				pose.append_residue_by_jump( *(residues[i]), 1, "", "", true );
			} else {
				pose.append_residue_by_bond( *(residues[i]), false, 0, i-1, 0, false, false );
			}
		}
		dump_pose(pose);
	} else {
		++call_count_;
	}
	return 0;
}


/********************************************************************
Dumping
********************************************************************/

/// @brief Ensure that the previous activity has finished before starting a new one
/// @details This function is to be called when starting a new activity to catch invalid states.
/// This throws an exception when an invalid state is detected.
void
DumpTrajectoryEnergy::try_move_to_idle() const {
	switch ( state_ ) {
	case IDLE : {
		break;
	}
	case SCORING : {
		// Can always move scoring to idle.
		break;
	}
	case MINIMIZING : {
		utility_exit_with_message( "Attempted to switch state of DumpTrajectoryEnergy in the middle of a minimization trajectory!" );
		break;
	}
	case PACKING : {
		utility_exit_with_message( "Attempted to switch state of DumpTrajectoryEnergy in the middle of a packing trajectory!" );
		break;
	}
	}
	// no one threw an exception. So switching to IDLE must be ok.
	state_ = IDLE;
}


/// @brief Perform initial setup related to an activity
/// @details This ensure the switch is valid based on the current state_.
void
DumpTrajectoryEnergy::start_new_activity( DumpState new_state ) const {
	try_move_to_idle();

	state_ = new_state;

	std::stringstream filename;
	filename << dump_prefix_;
	filename << "_" << utility::timestamp_millis_short();

	switch ( state_ ) {
	case IDLE : {
		break;
	}
	case SCORING : {
		if ( TR.visible() ) {
			TR << "Dumping scoring trajectory to: ";
		}
		filename << "_score";
		break;
	}
	case MINIMIZING : {
		if ( TR.visible() ) {
			TR << "Dumping minimization trajectory to: ";
		}
		filename << "_min";
		call_count_ = 0;
		break;
	}
	case PACKING : {
		if ( TR.visible() ) {
			TR << "Dumping packer trajectory to: ";
		}
		filename << "_pack";
		call_count_ = 0;
	}
	}

	filename << ".pdb";
	if ( dump_as_gz_ ) {
		filename << ".gz";
	}
	dump_filename_ = filename.str();
	dumped_frames_ = 0;

	if ( TR.visible() && state_ != IDLE ) {
		TR << dump_filename_ << std::endl;
	}
}


void
DumpTrajectoryEnergy::dump_pose(
	core::pose::Pose const & pose,
	bool const force /*= false*/
) const {
	if ( force || (call_count_ % dump_on_every_nth_call_ == 0) ) {

		if ( state_ == IDLE ) {
			utility_exit_with_message("Tried to dump pose from IDLE state!!! Email bcov@uw.edu");
		}

		dumped_frames_++;
		std::stringstream model_tag;
		model_tag << std::setfill('0') << std::setw( 6 ) << dumped_frames_;

		core::io::StructFileRepOptionsOP options( utility::pointer::make_shared< core::io::StructFileRepOptions >() );
		options->set_output_pose_energies_table( false );         // this causes problems during the minimizer

		io::pdb::add_to_multimodel_pdb( pose, dump_filename_, model_tag.str(), options );
	}

	++call_count_;
}


} // scoring
} // core
