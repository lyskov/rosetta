// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   core/energy_methods/RamaPreProEnergy.cc
/// @brief  A variation on the Ramachandran scorefunction that has separate probability tables for residues that precede prolines.
/// @author Frank DiMaio
/// @modified Vikram K. Mulligan (vmullig@uw.edu) -- not the original author, but modified this to work with D-amino acids, BACKBONE_AA
/// amino acids, and cyclic geometry.  Refactored greatly to support arbitrary heteropolymer building blocks with any number of
/// mainchain torsions.

// Unit headers
#include <core/energy_methods/RamaPreProEnergy.hh>
#include <core/energy_methods/RamaPreProEnergyCreator.hh>
#include <core/scoring/RamaPrePro.hh>

// Package headers
#include <core/scoring/EnergyMap.hh>
#include <core/scoring/methods/EnergyMethodOptions.hh>

// Project headers
#include <core/chemical/ResidueType.hh>
#include <core/conformation/Residue.hh>
#include <core/id/DOF_ID.fwd.hh>
#include <core/id/TorsionID.hh>
#include <core/id/PartialAtomID.hh>
#include <core/scoring/ScoringManager.hh>


#include <core/pose/Pose.hh>
#include <basic/Tracer.hh>

// Utility headers

// Numeric headers
#include <numeric/angle.functions.hh>

// C++ headers
#include <utility/vector1.hh>

#include <core/scoring/methods/util.hh> // AUTO IWYU For determine_lo_and_hi_residues, create_long_range_e...
#include <numeric/conversions.hh> // AUTO IWYU For degrees

namespace core {
namespace energy_methods {



static basic::Tracer TR( "core.energy_methods.RamaPreProEnergy" );

//////////////////////
/// EnergyMethod Creator
core::scoring::methods::EnergyMethodOP
RamaPreProEnergyCreator::create_energy_method(
	core::scoring::methods::EnergyMethodOptions const & /*options*/
) const {
	return utility::pointer::make_shared< RamaPreProEnergy >( );
}

core::scoring::ScoreTypes
RamaPreProEnergyCreator::score_types_for_method() const {
	using namespace core::scoring;
	ScoreTypes sts;
	sts.push_back( rama_prepro );
	return sts;
}


RamaPreProEnergy::RamaPreProEnergy( ) :
	parent( utility::pointer::make_shared< RamaPreProEnergyCreator >() ),
	potential_( core::scoring::ScoringManager::get_instance()->get_RamaPrePro() )
{}

core::scoring::methods::EnergyMethodOP
RamaPreProEnergy::clone() const {
	return utility::pointer::make_shared< RamaPreProEnergy >( *this );
}


void
RamaPreProEnergy::setup_for_scoring(
	pose::Pose & pose,
	core::scoring::ScoreFunction const &
) const {
	create_long_range_energy_container( pose, core::scoring::rama_prepro, long_range_type() );
}

bool
RamaPreProEnergy::defines_residue_pair_energy(
	pose::Pose const &pose,
	Size rsd1,
	Size rsd2
) const {
	bool res1_is_lo(false), res2_is_lo(false);
	core::scoring::methods::determine_lo_and_hi_residues( pose, rsd1, rsd2, res1_is_lo, res2_is_lo );
	runtime_assert_string_msg( !(res1_is_lo && res2_is_lo ), "Error in core::scoring::methods::RamaPreProEnergy::defines_residue_pair_energy(): The RamaPrePro term is incompatible with cyclic dipeptides (as is most of the rest of Rosetta)." );

	return (res1_is_lo || res2_is_lo);
}

core::scoring::methods::LongRangeEnergyType
RamaPreProEnergy::long_range_type() const { return core::scoring::methods::ramaprepro_lr; }

void
RamaPreProEnergy::residue_pair_energy(
	conformation::Residue const & rsd1,
	conformation::Residue const & rsd2,
	pose::Pose const & pose,
	core::scoring::ScoreFunction const &,
	core::scoring::EnergyMap & emap
) const {
	using namespace numeric;

	if ( !is_allowed_type( rsd1.type() ) || !is_allowed_type( rsd2.type() ) ) return;
	if ( !rsd1.is_bonded(rsd2) ) return;

	bool res1_is_lo(false), res2_is_lo(false);

	core::scoring::methods::determine_lo_and_hi_residues( pose, rsd1.seqpos(), rsd2.seqpos(), res1_is_lo, res2_is_lo );

	if ( !(res1_is_lo || res2_is_lo) ) return;
	runtime_assert_string_msg( !(res1_is_lo && res2_is_lo ), "Error in core::scoring::methods::RamaPreProEnergy::residue_pair_energy(): The RamaPrePro term is incompatible with cyclic dipeptides (as is most of the rest of Rosetta)." );

	conformation::Residue const &res_lo = (res1_is_lo) ? rsd1 : rsd2;
	conformation::Residue const &res_hi = (res2_is_lo) ? rsd1 : rsd2;

	if ( res_lo.is_terminus() || !res_lo.has_lower_connect() || !res_lo.has_upper_connect() ) return; // Rama not defined.  Note that is_terminus() checks for UPPER_TERMINUS or LOWER_TERMINUS variant types; it knows nothing about sequence position.

	utility::vector1 < core::Real > mainchain_torsions( res_lo.type().mainchain_atoms().size() - 1 );
	for ( core::Size i=1, imax=mainchain_torsions.size(); i<=imax; ++i ) mainchain_torsions[i] = nonnegative_principal_angle_degrees( res_lo.mainchain_torsion(i) );
	//utility::vector1 < core::Real > gradient; //Dummy argument for below.
	Real const rama_score = potential_.eval_rpp_rama_score( pose.conformation(), res_lo.type_ptr(), res_hi.type_ptr(), mainchain_torsions );

	emap[ core::scoring::rama_prepro ] += rama_score;
}

utility::vector1< id::PartialAtomID >
RamaPreProEnergy::atoms_with_dof_derivatives( conformation::Residue const & res, pose::Pose const & pose ) const
{
	if (
			!is_allowed_type(res.type()) ||
			res.is_terminus() ||
			! res.has_lower_connect() ||
			! res.has_upper_connect() ) {
		return utility::vector1< id::PartialAtomID >();
	}
	return potential_.atoms_w_dof_derivatives( res, pose );
}

Real
RamaPreProEnergy::eval_intraresidue_dof_derivative(
	conformation::Residue const & res_lo,
	core::scoring::ResSingleMinimizationData const & /*min_data*/,
	id::DOF_ID const & /*dof_id*/,
	id::TorsionID const & tor_id,
	pose::Pose const & pose,
	core::scoring::ScoreFunction const & /*sfxn*/,
	core::scoring::EnergyMap const & weights
) const
{
	using namespace numeric;

	Real deriv(0.0);
	if ( tor_id.valid() && tor_id.type() == id::BB ) {
		if ( tor_id.rsd() != res_lo.seqpos() ) return 0.0;
		if ( res_lo.is_terminus() || !res_lo.has_lower_connect() || !res_lo.has_upper_connect() ) return 0.0;

		core::Size const conn_partner( res_lo.connected_residue_at_upper() );
		if ( !conn_partner ) return 0.0;

		conformation::Residue const &res_hi( pose.residue( conn_partner ) );

		if ( tor_id.torsion() == res_lo.type().mainchain_atoms().size() ) return 0.0; //No derivative for omega, the inter-residue torsion.

		utility::vector1 < core::Real > mainchain_torsions( res_lo.type().mainchain_atoms().size() - 1 );
		for ( core::Size i=1, imax=mainchain_torsions.size(); i<=imax; ++i ) mainchain_torsions[i] = nonnegative_principal_angle_degrees( res_lo.mainchain_torsion(i) );

		utility::vector1 < core::Real > gradient; //Dummy argument for below.
		potential_.eval_rpp_rama_derivatives( pose.conformation(), res_lo.type_ptr(), res_hi.type_ptr(), mainchain_torsions, gradient );

		debug_assert( gradient.size() == res_lo.type().mainchain_atoms().size() - 1 );
		deriv = gradient[tor_id.torsion()];
	}

	// note that the atomtree PHI dofs are in radians
	// use degrees since dE/dangle has angle in denominator
	return weights[ core::scoring::rama_prepro ] * numeric::conversions::degrees( deriv );
}

core::Size
RamaPreProEnergy::version() const {
	return 1; // Initial versioning
}

bool RamaPreProEnergy::is_allowed_type( core::chemical::ResidueType const & rt ) const {
	return ( rt.is_protein() || rt.is_aramid() || rt.is_peptoid() );
}

} // namespace energy_methods
} // namespace core
