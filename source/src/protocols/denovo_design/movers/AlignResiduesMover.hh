// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file protocols/denovo_design/movers/AlignResiduesMover.hh
/// @brief Aligns one residue onto another
/// @author Tom Linsky (tlinsky@uw.edu)

#ifndef INCLUDED_protocols_denovo_design_movers_AlignResiduesMover_hh
#define INCLUDED_protocols_denovo_design_movers_AlignResiduesMover_hh

// Unit headers
#include <protocols/denovo_design/movers/AlignResiduesMover.fwd.hh>
#include <protocols/moves/Mover.hh>

// Protocol headers
#include <protocols/denovo_design/components/StructureData.fwd.hh>

// Core headers
#include <core/pose/Pose.fwd.hh>
#include <core/select/residue_selector/ResidueSelector.fwd.hh>
#include <core/select/residue_selector/ResidueVector.fwd.hh>

// Basic/Utility headers
#include <basic/datacache/DataMap.fwd.hh>

#include <utility/vector1.hh> // AUTO IWYU For vector1

namespace protocols {
namespace denovo_design {
namespace movers {

///@brief Aligns one residue onto another
class AlignResiduesMover : public protocols::moves::Mover {
public:
	typedef utility::vector1< core::select::residue_selector::ResidueSelectorCOP > ResidueSelectorCOPs;
	typedef utility::vector1< core::select::residue_selector::ResidueVector > ResidueVectors;

public:
	AlignResiduesMover();

	// destructor (important for properly forward-declaring smart-pointer members)
	~AlignResiduesMover() override;


public:
	// mover virtual API
	void
	apply( core::pose::Pose & pose ) override;

	void
	show( std::ostream & output = std::cout ) const override;


	/// @brief parse XML tag (to use this Mover in Rosetta Scripts)
	void
	parse_my_tag(
		utility::tag::TagCOP tag,
		basic::datacache::DataMap & data
	) override;

	/// @brief required in the context of the parser/scripting scheme
	protocols::moves::MoverOP
	fresh_instance() const override;

	/// @brief required in the context of the parser/scripting scheme
	protocols::moves::MoverOP
	clone() const override;

public:
	/// @brief Sets the ID of this mover
	void
	set_id( std::string const & id );

	/// @brief Adds residue selector that selects the template (i.e. the reference residues that we are aligning to)
	/// @details Clones the residue selector
	void
	add_template_selector( core::select::residue_selector::ResidueSelector const & selector );

	/// @brief Adds residue selector that selects the target (i.e. the residues that we are moving to align with the template)
	/// @details Clones the residue selector
	void
	add_target_selector( core::select::residue_selector::ResidueSelector const & selector );

	/// @brief Adds residue selector that selects the template (i.e. the reference residues that we are aligning to)
	/// @details Directly stores the ResidueSelectorCOP
	void
	add_template_selector( core::select::residue_selector::ResidueSelectorCOP selector );

	/// @brief Adds residue selector that selects the target (i.e. the residues that we are moving to align with the template)
	/// @details Directly stores the ResidueSelectorCOP
	void
	add_target_selector( core::select::residue_selector::ResidueSelectorCOP selector );

	/// @brief clears the list of template residue selectors
	void
	clear_template_selectors() { template_selectors_.clear(); }

	/// @brief clears the list of target residue selectors
	void
	clear_target_selectors() { target_selectors_.clear(); };

	std::string
	get_name() const override;

	static
	std::string
	mover_name();

	static
	void
	provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd );


private:
	/// @brief aligns residues from template and target subsets
	/// @returns template resid that was aligned
	core::Size
	align_residues(
		core::pose::Pose & pose,
		core::Size const align_count,
		core::select::residue_selector::ResidueVector const & template_subset,
		core::select::residue_selector::ResidueVector const & target_subset ) const;

	void
	align_residues(
		core::pose::Pose & pose,
		core::Size const align_count,
		core::Size const template_resid,
		core::Size const target_resid ) const;

	void
	align_residues(
		core::pose::Pose & pose,
		core::Size const jump_idx,
		core::Size const template_resid,
		core::Size const target_resid,
		core::Size const res_with_torsions ) const;

	void
	copy_residue(
		core::pose::Pose & pose,
		core::Size const resid_has_info,
		core::Size const resid_wants_info ) const;

	void
	add_metadata(
		components::StructureData & sd,
		core::Size const align_count,
		std::string const & template_segment,
		std::string const & target_segment,
		core::Size const target_resid ) const;

	ResidueVectors
	compute_target_residues( core::pose::Pose const & pose ) const;

	ResidueVectors
	compute_template_residues( core::pose::Pose const & pose ) const;

	void
	delete_residues(
		core::pose::Pose & pose,
		core::select::residue_selector::ResidueVector const & resids ) const;

private:
	std::string id_;
	ResidueSelectorCOPs template_selectors_;
	ResidueSelectorCOPs target_selectors_;

};

std::ostream &
operator<<( std::ostream & os, AlignResiduesMover const & mover );

} //protocols
} //denovo_design
} //movers

#endif //protocols/denovo_design/movers_AlignResiduesMover_hh
