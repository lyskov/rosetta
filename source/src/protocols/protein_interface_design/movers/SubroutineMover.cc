// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file protocols/protein_interface_design/movers/SubroutineMover.cc
/// @brief
/// @author Sarel Fleishman (sarelf@u.washington.edu)

// Unit headers
#include <protocols/protein_interface_design/movers/SubroutineMover.hh>
#include <protocols/protein_interface_design/movers/SubroutineMoverCreator.hh>
#include <protocols/rosetta_scripts/RosettaScriptsParser.hh>
#include <protocols/rosetta_scripts/ParsedProtocol.hh> // needed for ParsedProtocolOP -> MoverOP conversion

// Project headers
#include <utility/tag/Tag.hh>

#include <protocols/moves/Mover.hh>
#include <basic/Tracer.hh>
#include <basic/datacache/DataMap.fwd.hh>

// XSD XRW Includes
#include <utility/tag/XMLSchemaGeneration.hh>
#include <protocols/moves/mover_schemas.hh>


using namespace protocols::protein_interface_design;

static basic::Tracer TR( "protocols.protein_interface_design.movers.SubroutineMover" );

namespace protocols {
namespace protein_interface_design {
namespace movers {

using namespace protocols::moves;
using namespace core;




protocols::moves::MoverOP
SubroutineMover::clone() const {
	return( utility::pointer::make_shared< SubroutineMover >( *this ) );
}


void
SubroutineMover::apply( core::pose::Pose & pose )
{
	mover_->apply( pose );
	set_last_move_status( mover_->get_last_move_status() );
}

void
SubroutineMover::parse_my_tag( TagCOP const tag,
	basic::datacache::DataMap &
)
{
	std::string const xml_fname( tag->getOption< std::string >( "xml_fname" ) );

	protocols::rosetta_scripts::RosettaScriptsParserOP rsparser( new protocols::rosetta_scripts::RosettaScriptsParser );
	TR<<"Parsing a subroutine xml_file"<<std::endl;
	TR<<"*************WARNING: AT THIS POINT, CONSTRAINTS ADDED TO THE POSE IN A SUBROUTINE WILL BE IGNORED***********"<<std::endl;

	mover_ = rsparser->generate_mover(xml_fname);

}

protocols::moves::MoverOP
SubroutineMover::fresh_instance() const {
	return utility::pointer::make_shared< SubroutineMover >();
}

SubroutineMover::~SubroutineMover()= default;

SubroutineMover::SubroutineMover() :
	Mover( SubroutineMover::mover_name() ),
	mover_( /* NULL */ )
{}


std::string SubroutineMover::get_name() const {
	return mover_name();
}

std::string SubroutineMover::mover_name() {
	return "Subroutine";
}

void SubroutineMover::provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd )
{
	using namespace utility::tag;
	AttributeList attlist;
	attlist + XMLSchemaAttribute( "xml_fname", xs_string, "Filename for the XML to execute" );

	protocols::moves::xsd_type_definition_w_attributes( xsd, mover_name(), "XRW TO DO", attlist );
}

std::string SubroutineMoverCreator::keyname() const {
	return SubroutineMover::mover_name();
}

protocols::moves::MoverOP
SubroutineMoverCreator::create_mover() const {
	return utility::pointer::make_shared< SubroutineMover >();
}

void SubroutineMoverCreator::provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd ) const
{
	SubroutineMover::provide_xml_schema( xsd );
}


} //movers
} //protein_interface_design
} //protocols

