// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file protocols/simple_filters/ResiduesInInterfaceFilter.cc
/// @brief
/// @author Sarel Fleishman (sarelf@u.washington.edu), Jacob Corn (jecorn@u.washington.edu)

#include <protocols/simple_filters/ResiduesInInterfaceFilter.hh>
#include <protocols/simple_filters/ResiduesInInterfaceFilterCreator.hh>

#include <core/types.hh>
#include <core/pose/Pose.hh>
#include <basic/Tracer.hh>
#include <protocols/scoring/Interface.hh>
#include <core/scoring/ScoreFunction.hh>
#include <core/scoring/ScoreFunctionFactory.hh>
#include <utility/tag/Tag.hh>
//#include <protocols/moves/ResidueMover.hh>
//#include <protocols/simple_pose_metric_calculators/BuriedUnsatisfiedPolarsCalculator.hh>


//Objectxxxx header

// C++ headers

// XSD XRW Includes
#include <utility/tag/XMLSchemaGeneration.hh>
#include <protocols/filters/filter_schemas.hh>

#include <core/conformation/Residue.hh> // AUTO IWYU For Pose::Residue


namespace protocols {
namespace simple_filters {

static basic::Tracer residues_in_interface_tracer( "protocols.simple_filters.ResiduesInInterfaceFilter" );



ResiduesInInterfaceFilter::~ResiduesInInterfaceFilter() = default;

/// @details a utility for docking_filter
core::Size ResiduesInInterfaceFilter::compute( core::pose::Pose const & pose ) const
{
	core::Size interface_counter( 0 );

	runtime_assert( rb_jump_ <= pose.num_jump() );

	core::pose::Pose temp_pose( pose );
	core::scoring::ScoreFunctionOP scorefxn(
		core::scoring::get_score_function() );
	(*scorefxn)(temp_pose);

	protocols::scoring::Interface interface(rb_jump_);
	interface.calculate( temp_pose );

	for ( core::Size i = 1; i <= temp_pose.size(); i++ ) {
		if ( !temp_pose.residue(i).is_protein() ) continue;
		if ( interface.is_interface( i ) ) interface_counter++;
	}
	return( interface_counter );
}

void
ResiduesInInterfaceFilter::parse_my_tag(
	utility::tag::TagCOP tag,
	basic::datacache::DataMap &
) {
	residues_in_interface_threshold_ = tag->getOption<core::Size>( "residues", 20 );
	rb_jump_ = tag->getOption<core::Size>( "jump_number", 1 );

	residues_in_interface_tracer<<"residues in interface filter over jump number " << rb_jump_ << " with threshold "<<residues_in_interface_threshold_<<std::endl;
}

bool
ResiduesInInterfaceFilter::apply( core::pose::Pose const & pose ) const {
	core::Size const interface_res( compute( pose ));
	residues_in_interface_tracer<<"There are "<<interface_res<<" residues in the interface.";
	if ( interface_res <= residues_in_interface_threshold_ ) {
		residues_in_interface_tracer<<" Breaking out."<<std::endl;
		return( false );
	} else residues_in_interface_tracer<<std::endl;
	return( true );
}

void
ResiduesInInterfaceFilter::report( std::ostream & out, core::pose::Pose const & pose ) const {
	core::Size const interface_res( compute( pose ));
	out<<"residues_in_interface "<<interface_res<<'\n';
}

core::Real
ResiduesInInterfaceFilter::report_sm( core::pose::Pose const & pose ) const {
	core::Size const interface_res( compute( pose ));
	return( (core::Real) interface_res );
}

std::string ResiduesInInterfaceFilter::name() const {
	return class_name();
}

std::string ResiduesInInterfaceFilter::class_name() {
	return "ResInInterface";
}

void ResiduesInInterfaceFilter::provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd )
{
	using namespace utility::tag;
	AttributeList attlist;
	attlist + XMLSchemaAttribute::attribute_w_default("residues", xsct_non_negative_integer, "Threshold number of residues in the interface.", "20")
		+ XMLSchemaAttribute::attribute_w_default("jump_number", xsct_non_negative_integer, "jump number", "1");

	protocols::filters::xsd_type_definition_w_attributes( xsd, class_name(), "Filters based upon the number of residues in the interface.", attlist );
}

std::string ResiduesInInterfaceFilterCreator::keyname() const {
	return ResiduesInInterfaceFilter::class_name();
}

protocols::filters::FilterOP
ResiduesInInterfaceFilterCreator::create_filter() const {
	return utility::pointer::make_shared< ResiduesInInterfaceFilter >();
}

void ResiduesInInterfaceFilterCreator::provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd ) const
{
	ResiduesInInterfaceFilter::provide_xml_schema( xsd );
}


}
}
