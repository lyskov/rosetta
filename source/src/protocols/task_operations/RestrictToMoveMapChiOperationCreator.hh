// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file protocols/antibody_design/RestrictToMoveMapChiOperationCreator.hh
/// @brief
/// @author Jared Adolf-Bryfogle (jadolfbr@gmail.com)


#ifndef INCLUDED_protocols_task_operations_RestrictToMoveMapChiOperationCreator_hh
#define INCLUDED_protocols_task_operations_RestrictToMoveMapChiOperationCreator_hh




namespace protocols {
namespace task_operations {

/* Undefined, commenting out to fix PyRosetta build
class RestrictToMoveMapChiOperationCreator : public core::pack::task::operation::TaskOperationCreator {
public:
virtual core::pack::task::operation::TaskOperationOP create_task_operation() const;
virtual std::string keyname() const { return "RestrictToMoveMapChiOperation"; }
};
*/

} //namespace protocols
} //namespace task_operations

#endif //INCLUDED_protocols_antibody_design_RestrictToMoveMapChiOperationCreator.hh
