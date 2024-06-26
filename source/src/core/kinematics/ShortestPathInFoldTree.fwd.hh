// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   core/kinematics/ShortestPathInFoldTree.fwd.hh
/// @brief  kinematics::ShortestPathInFoldTree forward declarations header
/// @author Oliver Lange


#ifndef INCLUDED_core_kinematics_ShortestPathInFoldTree_fwd_hh
#define INCLUDED_core_kinematics_ShortestPathInFoldTree_fwd_hh


// Utility headers
#include <utility/pointer/access_ptr.hh>
#include <utility/pointer/owning_ptr.hh>


namespace core {
namespace kinematics {


// Forward
class ShortestPathInFoldTree;


// Types
typedef  utility::pointer::weak_ptr< ShortestPathInFoldTree >  ShortestPathInFoldTreeAP;
typedef  utility::pointer::shared_ptr< ShortestPathInFoldTree >  ShortestPathInFoldTreeOP;
typedef  utility::pointer::shared_ptr< ShortestPathInFoldTree const >  ShortestPathInFoldTreeCOP;


} // namespace kinematics
} // namespace core

#endif
