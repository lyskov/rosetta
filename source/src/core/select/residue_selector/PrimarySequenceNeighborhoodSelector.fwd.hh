// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   core/select/residue_selector/PrimarySequenceNeighborhoodSelector.fwd.hh
/// @brief  Forward declaration of a class that selects residues using their names
/// @author Tom Linsky (tlinsky@uw.edu)

#ifndef INCLUDED_core_select_residue_selector_PrimarySequenceNeighborhoodSelector_FWD_HH
#define INCLUDED_core_select_residue_selector_PrimarySequenceNeighborhoodSelector_FWD_HH

// Utility Headers
#include <utility/pointer/owning_ptr.hh>
#include <utility/vector1.fwd.hh>


namespace core {
namespace select {
namespace residue_selector {

class PrimarySequenceNeighborhoodSelector;

typedef utility::pointer::shared_ptr< PrimarySequenceNeighborhoodSelector > PrimarySequenceNeighborhoodSelectorOP;
typedef utility::pointer::shared_ptr< PrimarySequenceNeighborhoodSelector const > PrimarySequenceNeighborhoodSelectorCOP;

} //namespace residue_selector
} //namespace select
} //namespace core


#endif
