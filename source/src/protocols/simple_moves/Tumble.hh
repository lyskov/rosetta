// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   Tumble.hh
///
/// @brief
/// @author Yifan Song

#ifndef INCLUDED_protocols_docking_Tumble_hh
#define INCLUDED_protocols_docking_Tumble_hh

// Unit Headers
//#include <protocols/docking/Tumble.fwd.hh>

// Package Headers

// Project Headers
#include <core/types.hh>
#include <core/select/residue_selector/ResidueSelector.fwd.hh>
#include <core/pose/Pose.fwd.hh>


#include <protocols/moves/Mover.hh>



// Utility Headers

// Numeric Headers

// ObjexxFCL Headers

// C++ headers

namespace protocols {
namespace simple_moves {

class Tumble: public protocols::moves::Mover
{
public:
	/// @brief
	///  empty constructor fills values with the expected defaults
	Tumble();

	//destructor
	~Tumble() override;

	numeric::xyzVector<core::Real> center_of_mass(core::pose::Pose const & pose);

	void apply( core::pose::Pose & pose ) override;

	void
	parse_my_tag(
		TagCOP tag,
		basic::datacache::DataMap & datamap
	) override;

	protocols::moves::MoverOP clone() const override;

	protocols::moves::MoverOP fresh_instance() const override;


	std::string
	get_name() const override;

	static
	std::string
	mover_name();

	static
	void
	provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd );



private:
	core::select::residue_selector::ResidueSelectorCOP residue_list_;
};

} // simple_moves
} // protocols

#endif
