// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   protocols/simple_task_operations/RestrictToLoopsAndNeighbors.cc
/// @brief
/// @author Brian D. Weitzner (brian.weitzner@gmail.com)

// Unit headers
#include <protocols/simple_task_operations/RestrictToLoopsAndNeighbors.hh>
#include <protocols/simple_task_operations/RestrictToLoopsAndNeighborsCreator.hh>

// Package headers
#include <protocols/simple_task_operations/RestrictToLoops.hh>

#include <protocols/loops/Loops.hh>

// Project headers

// Utility headers
#include <utility/tag/Tag.hh>
#include <utility/tag/XMLSchemaGeneration.hh>
#include <core/pack/task/operation/task_op_schemas.hh>

namespace protocols {
namespace simple_task_operations {

using namespace core::pack::task::operation;
using namespace utility::tag;

using core::Size;
using core::Real;
using core::pack::task::PackerTask;
using core::pack::task::operation::TaskOperationOP;
using utility::tag::TagCOP;

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// BOILER PLATE CODE //////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

//@brief clone operator, calls the copy constructor
TaskOperationOP
RestrictToLoopsAndNeighbors::clone() const
{
	return utility::pointer::make_shared< RestrictToLoopsAndNeighbors >( *this );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// END OF BOILER PLATE CODE //////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////


void RestrictToLoopsAndNeighbors::parse_tag(TagCOP tag, DataMap & data)
{
	parent::parse_tag(tag, data);

	set_include_neighbors(
		tag->getOption<bool>("include_neighbors", include_neighbors()));

	set_design_neighbors(
		tag->getOption<bool>("design_neighbors", design_neighbors()));

	set_cutoff_distance(
		tag->getOption<Real>("cutoff_dist", cutoff_distance()));


}

void RestrictToLoopsAndNeighbors::provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd )
{
	AttributeList attributes;

	// From parent.
	RestrictToLoops::provide_attributes( attributes );
	attributes
		+ XMLSchemaAttribute( "include_neighbors", xsct_rosetta_bool , "Should we include neighbors?" )
		+ XMLSchemaAttribute( "design_neighbors", xsct_rosetta_bool , "Should we design neighbors?  Default False." )
		+ XMLSchemaAttribute( "cutoff_dist", xsct_real , "Distance for neighbor detection. default 10.0" );

	task_op_schema_w_attributes( xsd, keyname(), attributes, "A TaskOp that restricts packing and/or design to the loops specified and optionally the neighbors." );
}

void RestrictToLoopsAndNeighbors::apply( Pose const & pose, PackerTask & task ) const
{
	apply_helper(pose, task, include_neighbors(), cutoff_distance(), design_neighbors());
}

bool RestrictToLoopsAndNeighbors::include_neighbors() const
{
	return include_neighbors_;
}

void RestrictToLoopsAndNeighbors::set_include_neighbors( bool include_neighbors )
{
	include_neighbors_ = include_neighbors;
}

bool RestrictToLoopsAndNeighbors::design_neighbors() const
{
	return design_neighbors_;
}

void RestrictToLoopsAndNeighbors::set_design_neighbors(bool design_neighbors)
{
	design_neighbors_ = design_neighbors;
}

core::Real RestrictToLoopsAndNeighbors::cutoff_distance() const
{
	return cutoff_distance_;
}

void RestrictToLoopsAndNeighbors::set_cutoff_distance( core::Real cutoff_distance )
{
	if ( cutoff_distance >= 0.0 && cutoff_distance <= 10.0 ) {
		cutoff_distance_ = cutoff_distance;
	}
}

TaskOperationOP RestrictToLoopsAndNeighborsCreator::create_task_operation() const
{
	return utility::pointer::make_shared< RestrictToLoopsAndNeighbors >();
}

void RestrictToLoopsAndNeighborsCreator::provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd ) const
{
	RestrictToLoopsAndNeighbors::provide_xml_schema( xsd );
}

std::string RestrictToLoopsAndNeighborsCreator::keyname() const
{
	return RestrictToLoopsAndNeighbors::keyname();
}

} //namespace simple_task_operations
} //namespace protocols
