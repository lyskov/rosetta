// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

// @file   core/scoring/facts/FACTSEnergy.cc
// @brief
// @author: Hahnbeom Park

// Unit headers
#include <core/energy_methods/FACTSEnergy.hh>
#include <core/energy_methods/FACTSEnergyCreator.hh>
#include <core/scoring/facts/FACTSPotential.hh>

// Project headers
#include <core/scoring/ScoringManager.hh>
#include <core/scoring/ScoreFunction.hh>
#include <core/scoring/methods/EnergyMethodOptions.hh>
#include <core/conformation/Residue.hh>

#include <core/pose/Pose.hh>
#include <core/id/AtomID.fwd.hh>
#include <core/pose/datacache/CacheableDataType.hh>
#include <core/conformation/RotamerSetBase.hh>
#include <core/conformation/RotamerSetCacheableDataType.hh>

// options
#include <basic/options/option.hh>
#include <basic/options/keys/score.OptionKeys.gen.hh>

#include <basic/prof.hh>

// Utility headers
#include <utility/vector1.hh>

#include <core/scoring/facts/FACTSPose.hh> // AUTO IWYU For FACTSPoseInfo
#include <ObjexxFCL/FArray2D.hh> // AUTO IWYU For FArray2D


namespace core {
namespace energy_methods {



/// energy method creator
core::scoring::methods::EnergyMethodOP FACTSEnergyCreator::create_energy_method(
	core::scoring::methods::EnergyMethodOptions const & options
) const {
	return utility::pointer::make_shared< FACTSEnergy >( options );
}

core::scoring::ScoreTypes
FACTSEnergyCreator::score_types_for_method() const {
	core::scoring::ScoreTypes sts;
	sts.push_back( core::scoring::facts_elec );
	sts.push_back( core::scoring::facts_solv );
	sts.push_back( core::scoring::facts_sasa );
	return sts;
}


/// energy method definition
FACTSEnergy::FACTSEnergy( FACTSEnergy const & /*src*/ ) = default;

FACTSEnergy::FACTSEnergy( core::scoring::methods::EnergyMethodOptions const & options ):
	parent( utility::pointer::make_shared< FACTSEnergyCreator >() ),
	potential_( core::scoring::ScoringManager::get_instance()->get_FACTSPotential() ),
	exclude_DNA_DNA_( options.exclude_DNA_DNA() ) {
	//fpd  this should be in energymethodoptions
	max_dis_ = basic::options::option[ basic::options::OptionKeys::score::facts_GBpair_cut ]();
}


void
FACTSEnergy::setup_for_packing(
	pose::Pose & pose,
	utility::vector1< bool > const & residues_repacking,
	utility::vector1< bool > const &
) const {
	//std::cerr << "FACTSEnergy::setup_for_packing" << std::endl;
	pose.update_residue_neighbors();
	potential_.setup_for_packing( pose, residues_repacking );
}

void FACTSEnergy::prepare_rotamers_for_packing(
	pose::Pose const & pose,
	conformation::RotamerSetBase & rotamer_set
) const {
	//std::cerr << "FACTSEnergy::prepare_rotamers_for_packing" << std::endl;
	potential_.get_rotamers_born_radii( pose, rotamer_set );
}

void FACTSEnergy::update_residue_for_packing(
	pose::Pose & pose,
	Size resid
) const {
	//std::cerr << "FACTSEnergy::update_residue_for_packing" << std::endl;
	potential_.update_residue_for_packing( pose, resid );
}


void FACTSEnergy::setup_for_scoring( pose::Pose & pose, core::scoring::ScoreFunction const & ) const {
	//std::cerr << "FACTSEnergy::setup_for_scoring" << std::endl;
	potential_.setup_for_scoring( pose, false );
}

void FACTSEnergy::setup_for_derivatives( pose::Pose & pose, core::scoring::ScoreFunction const & ) const {
	//std::cerr << "FACTSEnergy::setup_for_derivatives" << std::endl;
	potential_.setup_for_derivatives( pose );
}

/////////////////////////////////////////////////////////////////////////////
// scoring
/////////////////////////////////////////////////////////////////////////////

void FACTSEnergy::residue_pair_energy(
	conformation::Residue const & rsd1,
	conformation::Residue const & rsd2,
	pose::Pose const & pose,
	core::scoring::ScoreFunction const &,
	core::scoring::EnergyMap & emap
) const
{
	if ( exclude_DNA_DNA_ && rsd1.is_DNA() && rsd2.is_DNA() ) return;

	auto const & facts_info
		( static_cast< core::scoring::facts::FACTSPoseInfo const & >( pose.data().get( pose::datacache::CacheableDataType::FACTS_POSE_INFO ) ) ); // SHOULD BE FAST!

	Real E_elec, E_solv_self, E_solv_pair;
	potential_.evaluate_polar_energy( rsd1, facts_info.residue_info( rsd1.seqpos() ), rsd2, E_elec, E_solv_self, E_solv_pair );
	emap[ core::scoring::facts_elec ] += E_elec;
	emap[ core::scoring::facts_solv ] += E_solv_self + E_solv_pair;
}


void FACTSEnergy::eval_intrares_energy(
	conformation::Residue const & rsd,
	pose::Pose const & pose,
	core::scoring::ScoreFunction const &,
	core::scoring::EnergyMap & emap
) const {
	if ( exclude_DNA_DNA_ && rsd.is_DNA() ) return;

	core::scoring::facts::FACTSResidueInfo const & facts
		( pose.data().get< core::scoring::facts::FACTSPoseInfo >( core::pose::datacache::CacheableDataType::FACTS_POSE_INFO ).residue_info( rsd.seqpos()));

	Real E_elec, E_solv_self, E_solv_pair;
	potential_.evaluate_polar_energy( rsd, facts, rsd, E_elec, E_solv_self, E_solv_pair );
	emap[ core::scoring::facts_elec ] += E_elec;
	emap[ core::scoring::facts_solv ] += E_solv_self + E_solv_pair;
	emap[ core::scoring::facts_sasa ] += potential_.evaluate_nonpolar_energy( rsd, facts, rsd );
}


void FACTSEnergy::evaluate_rotamer_intrares_energies(
	conformation::RotamerSetBase const & set,
	pose::Pose const & pose,
	core::scoring::ScoreFunction const & sfxn,
	utility::vector1< core::PackerEnergy > & energies
) const {
	using namespace conformation;
	using namespace numeric;
	using core::conformation::RotamerSetCacheableDataType::FACTS_ROTAMER_SET_INFO;

	if ( exclude_DNA_DNA_ && pose.residue( set.resid() ).is_DNA() ) return;

	auto const & facts_info
		( set.data().get< core::scoring::facts::FACTSRotamerSetInfo >( FACTS_ROTAMER_SET_INFO ) );

	for ( Size ii = 1, ii_end = set.num_rotamers(); ii <= ii_end; ++ii ) {

		Real E_elec, E_solv_pair, E_solv_self;
		potential_.evaluate_polar_otf_energy( *set.rotamer( ii ), facts_info.residue_info( ii ),
			*set.rotamer( ii ), facts_info.residue_info( ii ),
			E_elec, E_solv_self, E_solv_pair
		);

		Real const E_sasa
			( potential_.evaluate_nonpolar_energy( *set.rotamer( ii ), facts_info.residue_info( ii ),
			*set.rotamer( ii ) ));

		energies[ ii ] += static_cast< core::PackerEnergy > ( sfxn[ core::scoring::facts_elec ] * E_elec
			+ sfxn[ core::scoring::facts_solv ] * (E_solv_pair + E_solv_self)
			+ sfxn[ core::scoring::facts_sasa ] * E_sasa );

	}
}

void FACTSEnergy::evaluate_rotamer_intrares_energy_maps(
	conformation::RotamerSetBase const & set,
	pose::Pose const & pose,
	core::scoring::ScoreFunction const & ,
	utility::vector1< core::scoring::EnergyMap > & emaps
) const
{
	using namespace conformation;
	using namespace numeric;
	using core::conformation::RotamerSetCacheableDataType::FACTS_ROTAMER_SET_INFO;

	if ( exclude_DNA_DNA_ && pose.residue( set.resid() ).is_DNA() ) return;

	auto const & facts_info
		( set.data().get< core::scoring::facts::FACTSRotamerSetInfo >( FACTS_ROTAMER_SET_INFO ) );

	utility::vector1< Real > dBRi1( pose.residue(set.resid()).natoms(), 0.0 );
	utility::vector1< Real > dBRi2( pose.residue(set.resid()).natoms(), 0.0 );
	utility::vector1< Real > dSAi1( pose.residue(set.resid()).natoms(), 0.0 );
	utility::vector1< Real > dSAi2( pose.residue(set.resid()).natoms(), 0.0 );

	for ( Size ii = 1, ii_end = set.num_rotamers(); ii <= ii_end; ++ii ) {

		Real E_elec, E_solv_pair, E_solv_self;
		potential_.evaluate_polar_otf_energy( *set.rotamer( ii ), facts_info.residue_info( ii ),
			*set.rotamer( ii ), facts_info.residue_info( ii ),
			E_elec, E_solv_self, E_solv_pair
		);

		Real const E_sasa
			( potential_.evaluate_nonpolar_energy( *set.rotamer( ii ), facts_info.residue_info( ii ),
			*set.rotamer( ii ) ));

		(emaps[ ii ])[ core::scoring::facts_elec ] += E_elec ;
		(emaps[ ii ])[ core::scoring::facts_solv ] += E_solv_pair + E_solv_self ;
		(emaps[ ii ])[ core::scoring::facts_sasa ] += E_sasa ;
	}
}

void FACTSEnergy::evaluate_rotamer_pair_energies(
	conformation::RotamerSetBase const & set1,
	conformation::RotamerSetBase const & set2,
	pose::Pose const & pose,
	core::scoring::ScoreFunction const &, // sfxn,
	core::scoring::EnergyMap const & weights,
	ObjexxFCL::FArray2D< core::PackerEnergy > & energy_table
) const
{
	using namespace conformation;
	using namespace numeric;
	using core::conformation::RotamerSetCacheableDataType::FACTS_ROTAMER_SET_INFO;

	//cout << "eval_pairres" << endl;
	if ( exclude_DNA_DNA_ && pose.residue( set1.resid() ).is_DNA() && pose.residue( set2.resid() ).is_DNA() ) return;

	PROF_START( basic::FACTS_ROTAMER_PAIR_ENERGIES );

	auto const & facts_info1
		( set1.data().get< core::scoring::facts::FACTSRotamerSetInfo >( FACTS_ROTAMER_SET_INFO ) );

	auto const & facts_info2
		( set2.data().get< core::scoring::facts::FACTSRotamerSetInfo >( FACTS_ROTAMER_SET_INFO ) );

	// Dummy
	utility::vector1< Real > dBRi1( pose.residue(set1.resid()).natoms(), 0.0 );
	utility::vector1< Real > dBRi2( pose.residue(set2.resid()).natoms(), 0.0 );
	utility::vector1< Real > dSAi1( pose.residue(set1.resid()).natoms(), 0.0 );
	utility::vector1< Real > dSAi2( pose.residue(set2.resid()).natoms(), 0.0 );

	for ( Size ii = 1; ii <= set1.get_n_residue_types(); ++ii ) {
		Size const ii_offset = set1.get_residue_type_begin( ii );
		Residue const & ii_example_rotamer( *set1.rotamer( ii_offset ));

		Vector const & ii_coord( ii_example_rotamer.nbr_atom_xyz() );
		Real const ii_radius( ii_example_rotamer.nbr_radius() );

		for ( Size jj = 1; jj <= set2.get_n_residue_types(); ++jj ) {
			Size const jj_offset = set2.get_residue_type_begin( jj );
			Residue const & jj_example_rotamer( *set2.rotamer( jj_offset ));

			if ( exclude_DNA_DNA_ && ii_example_rotamer.is_DNA() && jj_example_rotamer.is_DNA() ) continue;

			Vector const & jj_coord( jj_example_rotamer.nbr_atom_xyz() );
			Real const jj_radius( jj_example_rotamer.nbr_radius() );

			if ( ii_coord.distance_squared( jj_coord ) >= std::pow(ii_radius+jj_radius+packing_interaction_cutoff(), 2 ) )  continue;

			for ( Size kk = 1, kke = set1.get_n_rotamers_for_residue_type( ii ); kk <= kke; ++kk ) {
				Size const kk_rot_id = ii_offset + kk - 1;
				for ( Size ll = 1, lle = set2.get_n_rotamers_for_residue_type( jj ); ll <= lle; ++ll ) {
					Size const ll_rot_id = jj_offset + ll - 1;

					Real E_elec, E_solv_pair, E_solv_self;
					potential_.evaluate_polar_otf_energy( *set1.rotamer( kk_rot_id ), facts_info1.residue_info( kk_rot_id ),
						*set2.rotamer( ll_rot_id ), facts_info2.residue_info( ll_rot_id ),
						E_elec, E_solv_self, E_solv_pair );
					Real const E_sasa
						( potential_.evaluate_nonpolar_energy( *set1.rotamer( kk_rot_id ),
						facts_info1.residue_info( kk_rot_id ), *set2.rotamer( ll_rot_id ) ) );

					energy_table( ll_rot_id, kk_rot_id ) += static_cast< core::PackerEnergy >(
						weights[ core::scoring::facts_elec ] * E_elec
						+ weights[ core::scoring::facts_solv ] * (E_solv_self + E_solv_pair)
						+ weights[ core::scoring::facts_sasa ] * E_sasa );
				}
			}
		}
	}
	PROF_START( basic::FACTS_ROTAMER_PAIR_ENERGIES );
}


void FACTSEnergy::evaluate_rotamer_background_energies(
	conformation::RotamerSetBase const & set,
	conformation::Residue const & rsd,
	pose::Pose const & pose,
	core::scoring::ScoreFunction const &, // sfxn,
	core::scoring::EnergyMap const & weights,
	utility::vector1< core::PackerEnergy > & energy_vector
) const
{
	//cout << "eval_background" << endl;

	PROF_START( basic::FACTS_ROTAMER_BACKGROUND_ENERGIES );

	using conformation::Residue;
	using core::conformation::RotamerSetCacheableDataType::FACTS_ROTAMER_SET_INFO;

	core::scoring::facts::FACTSResidueInfo const & facts_rsd_info
		( pose.data().get< core::scoring::facts::FACTSPoseInfo >( core::pose::datacache::CacheableDataType::FACTS_POSE_INFO ).residue_info(rsd.seqpos()));

	auto const & facts_set_info
		( set.data().get< core::scoring::facts::FACTSRotamerSetInfo >( FACTS_ROTAMER_SET_INFO ) );

	for ( Size ii = 1; ii <= set.get_n_residue_types(); ++ii ) {
		Size const ii_offset = set.get_residue_type_begin( ii );
		Residue const & ii_example_rotamer( *set.rotamer( ii_offset ));

		Vector const & ii_coord( ii_example_rotamer.nbr_atom_xyz() );
		Real const ii_radius( ii_example_rotamer.nbr_radius() );

		if ( exclude_DNA_DNA_ && ii_example_rotamer.is_DNA() && rsd.is_DNA() ) continue;

		Vector const & jj_coord( rsd.nbr_atom_xyz() );
		Real const jj_radius( rsd.nbr_radius() );

		if ( ii_coord.distance_squared( jj_coord ) >= std::pow(ii_radius+jj_radius+packing_interaction_cutoff(), 2 ) )  continue;

		for ( Size kk = 1, kke = set.get_n_rotamers_for_residue_type( ii ); kk <= kke; ++kk ) {
			Size const kk_rot_id = ii_offset + kk - 1;

			Real E_elec, E_solv_self, E_solv_pair;
			potential_.evaluate_polar_otf_energy( *set.rotamer( kk_rot_id ), facts_set_info.residue_info( kk_rot_id ),
				rsd, facts_rsd_info,
				E_elec, E_solv_self, E_solv_pair );
			Real const E_sasa
				( potential_.evaluate_nonpolar_energy( *set.rotamer( kk_rot_id ), facts_set_info.residue_info( kk_rot_id ),
				rsd) );
			energy_vector[ kk_rot_id ] += static_cast< core::PackerEnergy > (
				weights[ core::scoring::facts_elec ] * E_elec
				+ weights[ core::scoring::facts_solv ] * (E_solv_self+E_solv_pair)
				+ weights[ core::scoring::facts_sasa ] * E_sasa );
			//std::cout << "Background: " << ii << " " << kk << " " << kk_rot_id << " " << polarE << " " << nonpolarE << std::endl;
		} // kk - rotamers for residue types
	} // ii - residue types for rotamer set
	PROF_STOP( basic::FACTS_ROTAMER_BACKGROUND_ENERGIES );
}


void FACTSEnergy::evaluate_rotamer_background_energy_maps(
	conformation::RotamerSetBase const & set,
	conformation::Residue const & rsd,
	pose::Pose const & pose,
	core::scoring::ScoreFunction const &, // sfxn,
	core::scoring::EnergyMap const & ,
	utility::vector1< core::scoring::EnergyMap > & emaps
) const
{
	using conformation::Residue;
	using core::conformation::RotamerSetCacheableDataType::FACTS_ROTAMER_SET_INFO;

	core::scoring::facts::FACTSResidueInfo const & facts_rsd_info
		( pose.data().get< core::scoring::facts::FACTSPoseInfo >( core::pose::datacache::CacheableDataType::FACTS_POSE_INFO ).residue_info(rsd.seqpos()));

	auto const & facts_set_info
		( set.data().get< core::scoring::facts::FACTSRotamerSetInfo >( FACTS_ROTAMER_SET_INFO ) );

	for ( Size ii = 1; ii <= set.get_n_residue_types(); ++ii ) {
		Size const ii_offset = set.get_residue_type_begin( ii );
		Residue const & ii_example_rotamer( *set.rotamer( ii_offset ));

		Vector const & ii_coord( ii_example_rotamer.nbr_atom_xyz() );
		Real const ii_radius( ii_example_rotamer.nbr_radius() );

		if ( exclude_DNA_DNA_ && ii_example_rotamer.is_DNA() && rsd.is_DNA() ) continue;

		Vector const & jj_coord( rsd.nbr_atom_xyz() );
		Real const jj_radius( rsd.nbr_radius() );

		if ( ii_coord.distance_squared( jj_coord ) >= std::pow(ii_radius+jj_radius+packing_interaction_cutoff(), 2 ) )  continue;

		for ( Size kk = 1, kke = set.get_n_rotamers_for_residue_type( ii ); kk <= kke; ++kk ) {
			Size const kk_rot_id = ii_offset + kk - 1;

			Real E_elec, E_solv_self, E_solv_pair;
			potential_.evaluate_polar_otf_energy( *set.rotamer( kk_rot_id ), facts_set_info.residue_info( kk_rot_id ),
				rsd, facts_rsd_info,
				E_elec, E_solv_self, E_solv_pair );
			Real const E_sasa
				( potential_.evaluate_nonpolar_energy( *set.rotamer( kk_rot_id ), facts_set_info.residue_info( kk_rot_id ),
				rsd ) );
			(emaps[ kk_rot_id ])[ core::scoring::facts_elec ] += E_elec;
			(emaps[ kk_rot_id ])[ core::scoring::facts_solv ] += E_solv_self + E_solv_pair;
			(emaps[ kk_rot_id ])[ core::scoring::facts_sasa ] += E_sasa;
		} // kk - rotamers for residue types
	} // ii - residue types for rotamer set
}


void FACTSEnergy::eval_atom_derivative(
	id::AtomID const & atom_id,
	pose::Pose const & pose,
	kinematics::DomainMap const & domain_map,
	core::scoring::ScoreFunction const &,
	core::scoring::EnergyMap const & weights,
	Vector & F1,
	Vector & F2
) const {
	potential_.eval_atom_polar_derivative( atom_id, weights[ core::scoring::facts_elec ], weights[ core::scoring::facts_solv ],
		pose, domain_map, exclude_DNA_DNA_, F1, F2 );
	potential_.eval_atom_nonpolar_derivative( atom_id, weights[ core::scoring::facts_sasa ], pose, domain_map, exclude_DNA_DNA_, F1, F2 );
}

Distance
FACTSEnergy::atomic_interaction_cutoff() const {
	return ( max_dis_  );
}

Size FACTSEnergy::version() const
{
	return 1;
}

}
}
