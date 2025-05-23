// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   protocols/parser/ScoringGridLoader.hh
/// @brief  Declartion of the XML parser's ScoringGridLoader class for adding named ScoringGrids to the basic::datacache::DataMap
/// @author Sam DeLuca
/// @author Andrew Leaver-Fay (aleaverfay@gmail.com) -- moved here from DockDesignParser.cc

#ifndef INCLUDED_protocols_qsar_scoring_grid_ScoringGridLoader_HH
#define INCLUDED_protocols_qsar_scoring_grid_ScoringGridLoader_HH

// Package Headers
#include <protocols/qsar/scoring_grid/ScoringGridLoader.fwd.hh>
#include <protocols/parser/DataLoader.hh>

// Project Headers

// Utility Headers
#include <utility/tag/Tag.fwd.hh>
#include <utility/tag/XMLSchemaGeneration.fwd.hh>

#include <string>


namespace protocols {
namespace qsar {
namespace scoring_grid {

/// @brief A class for loading ScoringGrids into the XML parser's basic::datacache::DataMap.
class ScoringGridLoader : public protocols::parser::DataLoader
{
public:
	ScoringGridLoader();
	~ScoringGridLoader() override;

	/// @brief The ScoringGridLoader will create named ScoringGrids and load them into the basic::datacache::DataMap
	void load_data(
		utility::tag::TagCOP tag,
		basic::datacache::DataMap & data
	) const override;

	/// @brief Utility function for load_data(), which only takes tags for a single GridSet
	void parse_gridset_tag(
		utility::tag::TagCOP tag,
		utility::tag::TagCOP parent,
		basic::datacache::DataMap & data ) const;

	static std::string loader_name();
	static std::string scoring_grid_loader_ct_namer( std::string const & element_name );
	static void provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd );


};

} //namespace scoring_grid
} //namespace qsar
} //namespace protocols

#endif //INCLUDED_protocols_qsar_scoring_grid_ScoringGridLoader_HH
