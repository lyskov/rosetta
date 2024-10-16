// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   core/scoring/nmer/NMerPSSMEnergy.hh
/// @brief  PSSMerence energy method implementation
/// @author Indigo King (indigo.c.king@gmail.com)

// Unit headers
#include <core/scoring/nmer/NMerPSSMEnergy.hh>
#include <core/scoring/nmer/NMerPSSMEnergyCreator.hh>

// Package headers
#include <core/scoring/EnergyMap.hh>

// Project headers
#include <core/pose/Pose.hh>
#include <core/chemical/AA.hh>
#include <core/conformation/Residue.hh>
#include <core/conformation/Conformation.hh>
#include <core/scoring/ScoringManager.hh>
#include <basic/database/open.hh>
#include <utility/file/file_sys_util.hh>

// C++ Headers
#include <string>

// Utility Headers
#include <utility/io/izstream.hh>
#include <utility/string_util.hh>

#include <basic/options/option.hh>
#include <basic/options/keys/score.OptionKeys.gen.hh>

#include <utility/vector1.hh>

#include <basic/Tracer.hh>

static basic::Tracer TR( "core.scoring.methods.NMerPSSMEnergy" );

namespace core {
namespace scoring {
namespace methods {


/// @details This must return a fresh instance of the NMerPSSMEnergy class,
/// never an instance already in use
methods::EnergyMethodOP
NMerPSSMEnergyCreator::create_energy_method(
	methods::EnergyMethodOptions const &
) const {
	return utility::pointer::make_shared< NMerPSSMEnergy >();
}

ScoreTypes
NMerPSSMEnergyCreator::score_types_for_method() const {
	ScoreTypes sts;
	sts.push_back( nmer_pssm );
	return sts;
}

void
NMerPSSMEnergy::nmer_length( Size const nmer_length ){
	nmer_length_ = nmer_length;
	//nmer residue energy is attributed to position 1
	nmer_cterm_ = nmer_length_ - 1 ;
}

void
NMerPSSMEnergy::gate_pssm_scores( bool const gate_pssm_scores ){
	gate_pssm_scores_ = gate_pssm_scores;
}

void
NMerPSSMEnergy::nmer_pssm_scorecut( Real const nmer_pssm_scorecut ){
	nmer_pssm_scorecut_ = nmer_pssm_scorecut;
}

void
NMerPSSMEnergy::initialize_from_options()
{
	using namespace basic::options;
	NMerPSSMEnergy::nmer_length( option[ OptionKeys::score::nmer_ref_seq_length ]() );
	NMerPSSMEnergy::gate_pssm_scores( option[ OptionKeys::score::nmer_pssm_scorecut ].user() );
	NMerPSSMEnergy::nmer_pssm_scorecut( option[ OptionKeys::score::nmer_pssm_scorecut ]() );
}

NMerPSSMEnergy::NMerPSSMEnergy() :
	parent( utility::pointer::make_shared< NMerPSSMEnergyCreator >() )
{
	NMerPSSMEnergy::initialize_from_options();
	read_nmer_pssms_from_options();
}

NMerPSSMEnergy::NMerPSSMEnergy( utility::vector1< std::map< chemical::AA, utility::vector1< core::Real > > > const & all_nmer_pssms_in ):
	parent( utility::pointer::make_shared< NMerPSSMEnergyCreator >() )
{
	//TODO: make this an argument of the function call
	NMerPSSMEnergy::initialize_from_options();

	all_nmer_pssms_.clear();
	for ( Size ipssm = 1; ipssm <= all_nmer_pssms_in.size(); ++ipssm ) {
		std::map< chemical::AA, utility::vector1< core::Real > > nmer_pssm;
		std::map< chemical::AA, utility::vector1< core::Real > > const nmer_pssm_in( all_nmer_pssms_in[ ipssm ] );
		//copy contents of input into new copy
		for ( auto const & it : nmer_pssm_in ) {
			nmer_pssm.insert( it );
		}
		// append new copy to our cleared instance
		all_nmer_pssms_.push_back( nmer_pssm );
	}
}

// full ctor, init pssm fnames with vector of fnames
NMerPSSMEnergy::NMerPSSMEnergy(
	core::Size const nmer_length,
	bool const gate_pssm_scores,
	core::Real const nmer_pssm_scorecut,
	utility::vector1< std::string > const & pssm_fname_vec
) :
	parent( utility::pointer::make_shared< NMerPSSMEnergyCreator >() )
{
	NMerPSSMEnergy::nmer_length( nmer_length  );
	NMerPSSMEnergy::gate_pssm_scores( gate_pssm_scores );
	NMerPSSMEnergy::nmer_pssm_scorecut( nmer_pssm_scorecut );
	NMerPSSMEnergy::read_nmer_pssm_fname_vector( pssm_fname_vec );
}


NMerPSSMEnergy::~NMerPSSMEnergy() = default;

void NMerPSSMEnergy::read_nmer_pssms_from_options() {

	using namespace basic::options;

	TR << "checking for NMerPSSMEnergy PSSM list" << std::endl;

	//check for pssm list file
	if ( option[ OptionKeys::score::nmer_pssm_list ].user() ) {
		std::string const pssm_list_fname( option[ OptionKeys::score::nmer_pssm_list ] );
		NMerPSSMEnergy::read_nmer_pssm_list( pssm_list_fname );
	}
	//use single pssm file
	if ( option[ OptionKeys::score::nmer_pssm ].user() ) {
		std::string const pssm_fname( option[ OptionKeys::score::nmer_pssm ] );
		NMerPSSMEnergy::read_nmer_pssm( pssm_fname );
	}
}

//read energy table list
void NMerPSSMEnergy::read_nmer_pssm_list( std::string pssm_list_fname ) {
	if ( !utility::file::file_exists( pssm_list_fname ) ) {
		pssm_list_fname = basic::database::full_name( pssm_list_fname, false );
	}
	TR << "reading NMerPSSMEnergy list from " << pssm_list_fname << std::endl;
	TR << "PSSMs are cleared each time a list is loaded!" << std::endl;
	all_nmer_pssms_.clear();
	utility::io::izstream in_stream( pssm_list_fname );
	if ( !in_stream.good() ) {
		utility_exit_with_message( "[ERROR] Error opening NMerPSSMEnergy list file" );
	}
	//now loop over all names in list
	std::string pssm_fname;
	while ( getline( in_stream, pssm_fname ) ) {
		utility::vector1< std::string > const tokens( utility::split( pssm_fname ) );
		//skip comments
		if ( tokens[ 1 ][ 0 ] == '#' ) continue;
		NMerPSSMEnergy::read_nmer_pssm( pssm_fname );
	}
}

//read pssm files from vector of filenames
void NMerPSSMEnergy::read_nmer_pssm_fname_vector( utility::vector1< std::string > const & pssm_fname_vec ) {
	//now loop over all names in vector
	for ( Size i = 1; i<= pssm_fname_vec.size(); ++i ) {
		std::string const & pssm_fname( pssm_fname_vec[ i ] );
		NMerPSSMEnergy::read_nmer_pssm( pssm_fname );
	}
}

//load PSSM with AA x seqpos scores
// PSSM format is 1 AA per line w/ nmer_length_ score vals
void NMerPSSMEnergy::read_nmer_pssm( std::string pssm_fname ) {

	std::map< core::chemical::AA, utility::vector1< core::Real > > const & nmer_pssm( core::scoring::ScoringManager::get_instance()->get_nmer_pssm( pssm_fname, nmer_length_ ) );
	all_nmer_pssms_.push_back( nmer_pssm );
}


EnergyMethodOP
NMerPSSMEnergy::clone() const
{
	return utility::pointer::make_shared< NMerPSSMEnergy >( *this );
}

bool NMerPSSMEnergy::operator== ( NMerPSSMEnergy const & other ) const
{
	// Check member variables
	if ( all_nmer_pssms_ != other.all_nmer_pssms_ ) return false;
	if ( nmer_length_ != other.nmer_length_ ) return false;
	if ( nmer_cterm_ != other.nmer_cterm_ ) return false;
	if ( gate_pssm_scores_ != other.gate_pssm_scores_ ) return false;
	if ( nmer_pssm_scorecut_ != other.nmer_pssm_scorecut_ ) return false;
	return true;
}

core::Size
NMerPSSMEnergy::n_pssms() const
{
	return all_nmer_pssms_.size();
}

core::Real
NMerPSSMEnergy::pssm_energy_at_frame_seqpos( Size const frame_seqpos, core::chemical::AA const aa, Size const idx_pssm ) const
{
	if ( !all_nmer_pssms_[ idx_pssm ].count( aa ) )  return 0.;
	return all_nmer_pssms_[ idx_pssm ].find( aa )->second[ frame_seqpos ];
}

//retrieves ref energy of NMer centered on seqpos
//we're changing this so energy is computed as sum of all frames that overlap w this seqpos
//that way, res energy is actually reflective
//…unless we recalc the whole pssm for each overlapping frame and reeval gate criterion
void
NMerPSSMEnergy::residue_energy(
	conformation::Residue const & rsd,
	pose::Pose const & pose,
	EnergyMap & emap
) const
{
	using namespace chemical;

	if ( all_nmer_pssms_.empty() ) return;
	Size const seqpos( rsd.seqpos() );
	//over each pssm
	for ( Size ipssm = 1; ipssm <= n_pssms(); ++ipssm ) {
		if ( all_nmer_pssms_[ ipssm ].empty() ) continue; //this really shouldn't happen, but just in case
		//calc nmer's score for this pssm
		Real rsd_energy( 0.0 );
		chemical::AA const rsd_aa( pose.residue( seqpos ).aa() );

		//loop effective p1 seqpos over all overlapping positions
		//need chain begin, end so dont run off end of sequence in multi-chain poses
		Size chain_begin( pose.conformation().chain_begin( pose.chain( seqpos ) ) );
		Size chain_end( pose.conformation().chain_end( pose.chain( seqpos ) ) );
		Size p1_seqpos_begin( seqpos - nmer_length_ + 1 < chain_begin ? chain_begin : seqpos - nmer_length_ + 1 );
		//will we run off end if we start p1 at seqpos?
		Size p1_seqpos_end( seqpos + nmer_length_ - 1 > chain_end ? chain_end - nmer_length_ + 1 : seqpos );
		//loop over each frame beginning
		for ( Size p1_seqpos = p1_seqpos_begin; p1_seqpos <= p1_seqpos_end; ++p1_seqpos ) {
			//get pssm index of seqpos in this p1 frame
			Size rsd_iseq_nmer( seqpos - p1_seqpos + 1 );
			//now go ahead and get pssm energy from all_nmer_pssms_[ ipssm ]
			Real rsd_energy_this_nmer( pssm_energy_at_frame_seqpos( rsd_iseq_nmer, rsd_aa, ipssm ) );

			//skip this part if not doing gating
			if ( gate_pssm_scores_ ) {
				Real energy( 0.0 );
				for ( Size iseq_nmer = 1; iseq_nmer <= nmer_length_; ++iseq_nmer ) {
					Size iseq_pose( iseq_nmer + p1_seqpos - 1 );
					//bail if we fall off end of chain
					if ( iseq_pose > chain_end ) break;
					chemical::AA const aa( pose.residue( iseq_pose ).aa() );
					Real this_rsd_energy( pssm_energy_at_frame_seqpos( iseq_nmer, aa, ipssm ) );
					energy += this_rsd_energy;
				}
				//gate energy at pssm_scorecut, thus ignoring low-scoring nmers
				//skip rsd_energy accumulation if total pssm score is low enough
				if ( energy < nmer_pssm_scorecut_ ) continue;
			}
			rsd_energy += rsd_energy_this_nmer;
		}
		//add sum of all frames' rsd energies into emap
		emap[ nmer_pssm ] += rsd_energy;
	}
	//normalize energy by number of pssms used
	//otherwise avg scores would become huge if we use lots of pssms instead of just 1
	emap[ nmer_pssm ] /= n_pssms();
	return;
}


Real
NMerPSSMEnergy::eval_dof_derivative(
	id::DOF_ID const &,
	id::TorsionID const &,
	pose::Pose const &,
	ScoreFunction const &,
	EnergyMap const &
) const
{
	return 0.0;
}

/// @brief NMerPSSMEnergy is context independent; indicates that no
/// context graphs are required
void
NMerPSSMEnergy::indicate_required_context_graphs( utility::vector1< bool > & ) const
{}
core::Size
NMerPSSMEnergy::version() const
{
	return 1; // Initial versioning
}
} // methods
} // scoring
} // core
