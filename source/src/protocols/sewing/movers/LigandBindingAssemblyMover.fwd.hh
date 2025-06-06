// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file protocols/sewing/movers/LigandBindingAssemblyMover.fwd.hh
/// @brief a subclass of AppendAssemlbyMover for building an assembly with new ligand contacts
/// @author Sharon Guffy (guffy@email.unc.edu)


#ifndef INCLUDED_protocols_sewing_movers_LigandBindingAssemblyMover_fwd_hh
#define INCLUDED_protocols_sewing_movers_LigandBindingAssemblyMover_fwd_hh

// Utility headers
#include <utility/pointer/owning_ptr.hh>



// Forward
namespace protocols {
namespace sewing {
namespace movers {

class LigandBindingAssemblyMover;

typedef utility::pointer::shared_ptr< LigandBindingAssemblyMover > LigandBindingAssemblyMoverOP;
typedef utility::pointer::shared_ptr< LigandBindingAssemblyMover const > LigandBindingAssemblyMoverCOP;



} //protocols
} //sewing
} //movers


#endif //INCLUDED_protocols_sewing_movers_LigandBindingAssemblyMover_fwd_hh





