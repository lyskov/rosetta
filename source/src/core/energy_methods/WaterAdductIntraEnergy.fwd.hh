// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   core/scoring/methods/WaterAdductIntraEnergy.hh
/// @brief  Energetic offset/cost for placing a water adduct on an amino or nucleic acid
/// @author Jim Havranek


#ifndef INCLUDED_core_energy_methods_WaterAdductIntraEnergy_fwd_hh
#define INCLUDED_core_energy_methods_WaterAdductIntraEnergy_fwd_hh

#include <utility/pointer/owning_ptr.hh>

namespace core {
namespace energy_methods {


class WaterAdductIntraEnergy;

typedef utility::pointer::shared_ptr< WaterAdductIntraEnergy > WaterAdductIntraEnergyOP;

} // scoring
} // core


#endif // INCLUDED_core_energy_methods_WaterAdductIntraEnergy_FWD_HH
