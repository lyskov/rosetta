// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   core/scoring/dna/DNAChiEnergy.hh
/// @brief  Energy term for simple DNA chi torsion
/// @author Jim Havranek


#ifndef INCLUDED_core_energy_methods_DNAChiEnergy_HH
#define INCLUDED_core_energy_methods_DNAChiEnergy_HH

// Unit headers
#include <core/energy_methods/DNAChiEnergy.fwd.hh>
#include <core/scoring/dna/DNABFormPotential.fwd.hh>

// Package headers
#include <core/scoring/methods/ContextIndependentOneBodyEnergy.hh>

// Project headers
#include <core/pose/Pose.fwd.hh>


namespace core {
namespace energy_methods {


class DNAChiEnergy : public core::scoring::methods::ContextIndependentOneBodyEnergy {
public:
	typedef core::scoring::methods::ContextIndependentOneBodyEnergy parent;
public:

	/// @brief ctor
	DNAChiEnergy();

	/// @brief dtor
	~DNAChiEnergy() override;

	/// clone
	core::scoring::methods::EnergyMethodOP
	clone() const override;

	/////////////////////////////////////////////////////////////////////////////
	// methods for ContextIndependentOneBodyEnergies
	/////////////////////////////////////////////////////////////////////////////


	void
	residue_energy(
		conformation::Residue const & rsd,
		pose::Pose const & pose,
		core::scoring::EnergyMap & emap
	) const override;


	virtual
	Real
	eval_dof_derivative(
		id::DOF_ID const & dof_id,
		id::TorsionID const & tor_id,
		pose::Pose const & pose,
		core::scoring::ScoreFunction const & sfxn,
		core::scoring::EnergyMap const & weights
	) const;

	/// @brief DunbrackEnergy is context independent; indicates that no
	/// context graphs are required
	void indicate_required_context_graphs( utility::vector1< bool > & ) const override;


	core::Size version() const override;

	// data
private:
	// Will probably need to store data here, although I may hard-wire this for the first pass
	core::scoring::dna::DNABFormPotential const & potential_;

};

} // scoring
} // core


#endif // INCLUDED_core_scoring_dna_DNAChiEnergy_HH
