// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   core/energy_methods/CentroidDisulfideEnergy.fwd.hh
/// @brief  Centroid Disulfide Energy class forward declaration
/// @author Spencer Bliven <blivens@u.washington.edu>
/// @date   2/4/09

#ifndef INCLUDED_core_scoring_disulfides_CentroidDisulfideEnergy_fwd_hh
#define INCLUDED_core_scoring_disulfides_CentroidDisulfideEnergy_fwd_hh

// Utility headers
#include <utility/pointer/owning_ptr.hh>

namespace core {
namespace energy_methods {

class CentroidDisulfideEnergy;

typedef utility::pointer::shared_ptr< CentroidDisulfideEnergy > CentroidDisulfideEnergyOP;
typedef utility::pointer::shared_ptr< CentroidDisulfideEnergy const > CentroidDisulfideEnergyCOP;

} // namespace energy_methods
} // namespace core

#endif
