// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file ./protocols/moves/DsspMover.cc
/// @brief performs dssp and set secondary structure to pose
/// @author Nobuyasu Koga ( nobuyasu@uw.edu )

// Unit headers
#include <protocols/moves/DsspMover.hh>
#include <protocols/moves/DsspMoverCreator.hh>

// Project Headers
#include <core/scoring/dssp/Dssp.hh>
#include <protocols/moves/Mover.hh>
#include <basic/Tracer.hh>
#include <utility/tag/Tag.hh>

// Parser headers


#include <string>

// XSD XRW Includes
#include <utility/tag/XMLSchemaGeneration.hh>
#include <protocols/moves/mover_schemas.hh>


static basic::Tracer TR( "protocols.DsspMover" );

namespace protocols {
namespace moves {

DsspMover::DsspMover():
	Mover( DsspMover::mover_name() ),
	reduced_IG_as_L_(false),
	asu_only_(false)
{}

DsspMover::~DsspMover() = default;

/// @brief clone this object
DsspMover::MoverOP DsspMover::clone() const {

	return utility::pointer::make_shared< DsspMover >( *this );
}

/// @brief create this type of object
DsspMover::MoverOP DsspMover::fresh_instance() const {
	return utility::pointer::make_shared< DsspMover >();
}

/// @details virtual main
void DsspMover::apply( Pose & pose )
{
	if ( !reduced_IG_as_L_ ) {
		core::scoring::dssp::Dssp dssp( pose, asu_only_ );
		dssp.insert_ss_into_pose( pose, false );
		TR << dssp.get_dssp_secstruct() << std::endl;
	} else {
		TR << "reduce IG as L" << std::endl;
		core::scoring::dssp::Dssp dssp( pose, asu_only_ );
		dssp.insert_ss_into_pose_no_IG_helix( pose, false );
		TR << dssp.get_dssp_secstruct() << std::endl;
	}
}

/// @brief parse xml
void
DsspMover::parse_my_tag(
	TagCOP const tag,
	basic::datacache::DataMap &
)
{
	reduced_IG_as_L_ = tag->getOption<bool>( "reduced_IG_as_L" , false );
	//Symmetry options
	set_asu_only( tag->getOption< bool >( "asu_only", false ) );
	TR << "DsspMover loaded." << std::endl;
}

std::string DsspMover::get_name() const {
	return mover_name();
}

std::string DsspMover::mover_name() {
	return "Dssp";
}

void DsspMover::provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd )
{
	using namespace utility::tag;
	AttributeList attlist;
	attlist
		+ XMLSchemaAttribute::attribute_w_default( "reduced_IG_as_L", xs_integer, "if set to be 1, will convert IG from Dssp definition as L rather than H", "0" )
		+ XMLSchemaAttribute::attribute_w_default( "asu_only", xsct_rosetta_bool, "if true, only performs DSSP on asymmetric unit.", "false" );

	protocols::moves::xsd_type_definition_w_attributes(
		xsd, mover_name(),
		"Calculate secondary structures based on dssp algorithm, "
		"and load the information onto Pose",
		attlist );
}

std::string DsspMoverCreator::keyname() const {
	return DsspMover::mover_name();
}

protocols::moves::MoverOP
DsspMoverCreator::create_mover() const {
	return utility::pointer::make_shared< DsspMover >();
}

void DsspMoverCreator::provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd ) const
{
	DsspMover::provide_xml_schema( xsd );
}

void DsspMover::set_asu_only( bool const asu_only )
{
	asu_only_ = asu_only;
}

}  // namespace moves
}  // namespace protocols
