// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file LegacyResidueRetentionRequirement.hh
///
/// @brief A container for all LEGACY_SEWING requirements
/// @author Tim Jacobs

#ifndef INCLUDED_protocols_legacy_sewing_sampling_requirements_LegacyResidueRetentionRequirement_hh
#define INCLUDED_protocols_legacy_sewing_sampling_requirements_LegacyResidueRetentionRequirement_hh

//Unit headers
#include <protocols/legacy_sewing/sampling/requirements/LegacyResidueRetentionRequirement.fwd.hh>
#include <protocols/legacy_sewing/sampling/requirements/LegacyGlobalRequirement.hh>
#include <core/types.hh>

//Package headers
#include <protocols/legacy_sewing/conformation/Assembly.fwd.hh>

//Utility headers
#include <set>
#include <string>

#include <utility/tag/XMLSchemaGeneration.fwd.hh> // AUTO IWYU For XMLSchemaDefinition

namespace protocols {
namespace legacy_sewing  {
namespace sampling {
namespace requirements {

class LegacyResidueRetentionRequirement : public LegacyGlobalRequirement {

public:

	///@brief default constructor
	LegacyResidueRetentionRequirement();

	LegacyResidueRetentionRequirement(
		int model_id
	);

	void
	model_id(
		int model_id
	);

	void
	required_resnums(
		std::set<core::Size> required_resnums
	);

	void
	add_resnum(
		core::Size resnum
	);

	///@brief Inverse of violated for this requirement
	bool
	satisfies(
		AssemblyCOP assembly
	) const override;

	///@brief Has the Assembly removed required residues
	///for the specified model
	bool
	violates(
		AssemblyCOP assembly
	) const override;

	void
	parse_my_tag(
		utility::tag::TagCOP tag,
		basic::datacache::DataMap & /*data*/
	) override;

	void
	show(
		std::ostream & out
	) const override;

	static std::string
	class_name();

	static void
	provide_xml_schema( utility::tag::XMLSchemaDefinition & );

private:

	int model_id_;
	std::set<core::Size> required_resnums_;

};


} //requirements namespace
} //sampling namespace
} //legacy_sewing namespace
} //protocols namespace

#endif
