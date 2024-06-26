// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file protocols/stepwise/monte_carlo/mover/StepWiseMasterMover.hh
/// @brief
/// @details
/// @author Rhiju Das, rhiju@stanford.edu


#ifndef INCLUDED_protocols_stepwise_monte_carlo_mover_StepWiseMasterMover_HH
#define INCLUDED_protocols_stepwise_monte_carlo_mover_StepWiseMasterMover_HH

#include <protocols/moves/Mover.hh>
#include <protocols/stepwise/monte_carlo/mover/StepWiseMasterMover.fwd.hh>
#include <protocols/stepwise/monte_carlo/mover/AddMover.fwd.hh>
#include <protocols/stepwise/monte_carlo/mover/DeleteMover.fwd.hh>
#include <protocols/stepwise/monte_carlo/mover/FromScratchMover.fwd.hh>
#include <protocols/stepwise/monte_carlo/mover/AddOrDeleteMover.fwd.hh>
#include <protocols/stepwise/monte_carlo/mover/ResampleMover.fwd.hh>
#include <protocols/stepwise/monte_carlo/mover/VaryLoopLengthMover.fwd.hh>
#include <protocols/stepwise/monte_carlo/options/StepWiseMonteCarloOptions.fwd.hh>
#include <protocols/stepwise/monte_carlo/mover/StepWiseMoveSelector.fwd.hh>
#include <protocols/stepwise/monte_carlo/mover/StepWiseMove.fwd.hh>
#include <protocols/stepwise/monte_carlo/submotif/SubMotifLibrary.fwd.hh>
#include <protocols/stepwise/modeler/StepWiseModeler.fwd.hh>
#include <core/scoring/ScoreFunction.fwd.hh>
#include <core/types.hh>

namespace protocols {
namespace stepwise {
namespace monte_carlo {
namespace mover {

/// @brief Central mover of stepwise Monte Carlo protocols, handing work off to special move types.
/// @details It's worth noting that one function that for historical reasons is a method of this
/// class is capable of spawning threads. Notably, that method is not called by this Mover's apply
/// function (rather, it calls this Mover's apply function!) and so you don't have to worry. If
/// you are studying this class as an archetypical example of a Mover, for one thing, don't do that
/// because it isn't; for another thing, don't multithread your apply function as a misguided
/// inference from how this Mover has a multithreaded function that's not apply.
class StepWiseMasterMover: public protocols::moves::Mover {

public:

	//constructor
	StepWiseMasterMover( core::scoring::ScoreFunctionCOP scorefxn,
		protocols::stepwise::monte_carlo::options::StepWiseMonteCarloOptionsCOP options );

	//destructor
	~StepWiseMasterMover() override;

public:

	using Mover::apply;
	void apply( Pose & ) override;

	std::string get_name() const override { return "StepWiseMasterMover"; }

	StepWiseMasterMover( StepWiseMasterMover const & src );

	protocols::moves::MoverOP clone() const override;

	void
	apply( core::pose::Pose & pose,
		StepWiseMove const & stepwise_move,
		bool const figure_out_all_possible_moves = true );

	void
	do_the_move( StepWiseMove const & move, core::pose::Pose & pose );

	bool apply_legacy( Pose & );

	void
	initialize( core::scoring::ScoreFunctionCOP scorefxn, protocols::stepwise::monte_carlo::options::StepWiseMonteCarloOptionsCOP options );

	void initialize_pose_if_empty( core::pose::Pose & pose );

	void
	set_minimize_single_res( bool const minimize_single_res );

	bool
	do_test_move( StepWiseMove const & move,
		core::pose::Pose & pose );

	void
	preminimize_pose( core::pose::Pose & pose );

	void
	set_options( protocols::stepwise::monte_carlo::options::StepWiseMonteCarloOptionsCOP options ); // forces reinitalization.

	void
	set_scorefxn( core::scoring::ScoreFunctionCOP scorefxn ){ scorefxn_ = scorefxn; }

	std::string move_type_string() const { return move_type_string_; }

	bool success() const { return success_; }

	core::Real const & proposal_density_ratio() const { return proposal_density_ratio_; }

	void
	resample_full_model( core::pose::Pose const & start_pose, core::pose::Pose & output_pose, bool const checkpointing_breadcrumbs );

	void
	resample_full_model( core::pose::Pose const & start_pose, core::pose::Pose & output_pose, bool const checkpointing_breadcrumbs,
		utility::vector1< core::Size > const & residues_to_resample, core::Size const resample_round, core::Size const nstruct );

	void
	build_full_model( core::pose::Pose const & start_pose, core::pose::Pose & full_model_pose, bool const & choose_random = false );

	void set_submotif_library( monte_carlo::submotif::SubMotifLibraryCOP setting ) { submotif_library_ = setting; }

	StepWiseMoveSelectorCOP stepwise_move_selector() const { return stepwise_move_selector_; }

private:

	void initialize();

	bool minimize_single_res_;

	bool
	test_all_moves( core::pose::Pose & pose );

	void
	test_all_moves_recursively( core::pose::Pose & pose );

	// Used only by resample_full_model
	utility::vector1< StepWiseMove >
	moves_for_pose(
		core::pose::Pose const & start_pose,
		utility::vector1< core::Size > const & residues_to_rebuild
	);

private:

	core::scoring::ScoreFunctionCOP scorefxn_;
	protocols::stepwise::monte_carlo::options::StepWiseMonteCarloOptionsCOP options_;

	modeler::StepWiseModelerOP stepwise_modeler_;
	DeleteMoverOP delete_mover_;
	AddMoverOP add_mover_;
	FromScratchMoverOP from_scratch_mover_;
	AddOrDeleteMoverOP add_or_delete_mover_;
	ResampleMoverOP resample_mover_;
	VaryLoopLengthMoverOP vary_loop_length_mover_;
	monte_carlo::submotif::SubMotifLibraryCOP submotif_library_;
	StepWiseMoveSelectorOP stepwise_move_selector_;

	bool success_;
	std::string move_type_string_;
	core::Real proposal_density_ratio_;

	core::Size num_tested_moves_;
};

// AMW TODO: truly, this is a utility function that could go anywhere.
modeler::StepWiseModelerOP
setup_unified_stepwise_modeler( protocols::stepwise::monte_carlo::options::StepWiseMonteCarloOptionsCOP options, core::scoring::ScoreFunctionCOP scorefxn );


} //mover
} //monte_carlo
} //stepwise
} //protocols

#endif
