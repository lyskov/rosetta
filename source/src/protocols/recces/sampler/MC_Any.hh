// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file protocols/recces/sampler/MC_Any.hh
/// @brief Chooses at random between samplers (rather than running all of them)
/// @detailed
/// @author Rhiju Das, rhiju@stanford.edu


#ifndef INCLUDED_protocols_recces_sampler_MC_Any_HH
#define INCLUDED_protocols_recces_sampler_MC_Any_HH

#include <protocols/recces/sampler/MC_Comb.hh>
#include <protocols/recces/sampler/MC_Any.fwd.hh>

namespace protocols {
namespace recces {
namespace sampler {

class MC_Any: public MC_Comb {

public:

	//constructor
	MC_Any();

	//destructor
	~MC_Any() override;

public:

	/// @brief Initialization
	void init() override;

	/// @brief Move to next rotamer
	void operator++() override;

	/// @brief Apply the current rotamer to pose
	void apply( core::pose::Pose & pose ) override;

	/// @brief Type of class (see enum in SamplerPlusPlusTypes.hh)
	toolbox::SamplerPlusPlusType type() const override { return toolbox::MC_ANY; }

	core::Size curr_id() const { return curr_id_; }

	MC_SamplerCOP rotamer() const { return rotamer_list_[ curr_id_ ]; }

	/// @brief output summary of class
	void show( std::ostream & out, core::Size const indent = 0 ) const override;

protected:
	core::Size curr_id_;
};

} //sampler
} //recces
} //protocols

#endif
