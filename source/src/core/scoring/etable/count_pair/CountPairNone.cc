// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   core/scoring/etable/count_pair/CountPairNone.cc
/// @brief  Count pair for residues where all atom pairs should be counted.
/// @author Andrew Leaver-Fay (aleaverfay@gmail.com)


#include <core/scoring/etable/count_pair/CountPairNone.hh>

//Auto Headers
namespace core {
namespace scoring {
namespace etable {
namespace count_pair {

CountPairNone::CountPairNone() = default;

CountPairNone::~CountPairNone() = default;

bool
CountPairNone::count(
	int const at1,
	int const at2,
	Real & w,
	Size & path_dist
) const
{
	return operator() ( at1, at2, w, path_dist );
}

void
CountPairNone::residue_atom_pair_energy(
	conformation::Residue const &,// res1,
	conformation::Residue const &,// res2,
	etable::TableLookupEvaluator const &,// etable_energy,
	EnergyMap &// emap
) const
{
	return;
	//inline_residue_atom_pair_energy( res1, res2, etable_energy, *this, emap );
}


void
CountPairNone::residue_atom_pair_energy_sidechain_backbone(
	conformation::Residue const &,// res1,
	conformation::Residue const &,// res2,
	etable::TableLookupEvaluator const &,// etable_energy,
	EnergyMap &// emap
) const
{
	return;
	//inline_residue_atom_pair_energy_sidechain_backbone( res1, res2, etable_energy, *this, emap );
}


void
CountPairNone::residue_atom_pair_energy_sidechain_whole(
	conformation::Residue const &,// res1,
	conformation::Residue const &,// res2,
	etable::TableLookupEvaluator const &,// etable_energy,
	EnergyMap &// emap
) const
{
	return;
	//inline_residue_atom_pair_energy_sidechain_whole( res1, res2, etable_energy, *this, emap );
}

void
CountPairNone::residue_atom_pair_energy_backbone_backbone(
	conformation::Residue const &,
	conformation::Residue const &,
	etable::TableLookupEvaluator const &,
	EnergyMap &
) const
{}


void
CountPairNone::residue_atom_pair_energy_sidechain_sidechain(
	conformation::Residue const &,
	conformation::Residue const &,
	etable::TableLookupEvaluator const &,
	EnergyMap &
) const
{}

void
CountPairNone::residue_atom_pair_energy(
	conformation::Residue const &,// res1,
	conformation::Residue const &,// res2,
	etable::AnalyticEtableEvaluator const &,// etable_energy,
	EnergyMap &// emap
) const
{
	return;
	//inline_residue_atom_pair_energy( res1, res2, etable_energy, *this, emap );
}


void
CountPairNone::residue_atom_pair_energy_sidechain_backbone(
	conformation::Residue const &,// res1,
	conformation::Residue const &,// res2,
	etable::AnalyticEtableEvaluator const &,// etable_energy,
	EnergyMap &// emap
) const
{
	return;
	//inline_residue_atom_pair_energy_sidechain_backbone( res1, res2, etable_energy, *this, emap );
}


void
CountPairNone::residue_atom_pair_energy_sidechain_whole(
	conformation::Residue const &,// res1,
	conformation::Residue const &,// res2,
	etable::AnalyticEtableEvaluator const &,// etable_energy,
	EnergyMap &// emap
) const
{
	return;
	//inline_residue_atom_pair_energy_sidechain_whole( res1, res2, etable_energy, *this, emap );
}
void
CountPairNone::residue_atom_pair_energy_backbone_backbone(
	conformation::Residue const &,
	conformation::Residue const &,
	etable::AnalyticEtableEvaluator const &,
	EnergyMap &
) const
{}


void
CountPairNone::residue_atom_pair_energy_sidechain_sidechain(
	conformation::Residue const &,
	conformation::Residue const &,
	etable::AnalyticEtableEvaluator const &,
	EnergyMap &
) const
{}


} // namespace count_pair
} // namespace etable
} // namespace scoring
} // namespace core
