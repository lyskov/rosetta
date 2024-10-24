// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

#include <protocols/mpi_refinement/util.hh>

#include <basic/options/keys/lh.OptionKeys.gen.hh>
#include <basic/options/option.hh>

#include <core/types.hh>
#include <core/id/AtomID.hh>
#include <core/pose/selection.hh>
#include <core/pose/Pose.hh>
#include <core/scoring/ScoreFunction.hh>
#include <core/scoring/ScoreType.hh>
#include <core/conformation/Residue.hh>

#include <core/io/silent/SilentStruct.hh>
#include <protocols/wum/SilentStructStore.hh>
#include <core/pose/PDBInfo.hh>
#include <core/kinematics/MoveMap.hh>
#include <core/optimization/CartesianMinimizer.hh>
#include <core/optimization/MinimizerOptions.hh>

#include <core/pack/task/PackerTask.fwd.hh>
#include <core/pack/task/TaskFactory.hh>
#include <core/pack/task/operation/TaskOperations.hh>
#include <protocols/minimization_packing/PackRotamersMover.hh>

#include <core/scoring/constraints/CoordinateConstraint.hh>
#include <core/scoring/constraints/AtomPairConstraint.hh>
#include <core/scoring/constraints/BoundConstraint.hh>
//#include <core/scoring/func/Func.hh>
#include <core/scoring/func/HarmonicFunc.hh>


#include <core/scoring/rms_util.hh>

#include <cmath>
#include <cstdlib> // atoi
#include <algorithm> // for sort
#include <fstream> // for ifstream
#include <ObjexxFCL/format.hh>
#include <ObjexxFCL/FArray2D.hh>

#include <basic/Tracer.hh>
#include <numeric/random/random.hh>

#include <utility/vector1.hh>
#include <utility/string_util.hh>

static basic::Tracer TR("MPI.LHR.util");

namespace protocols {
namespace mpi_refinement {

// make sure that ss is not being re-used to accumulate wrong info

utility::vector1< std::pair< core::Size, core::Size > >
get_loop_info_full( core::io::silent::SilentStructOP ss,
	utility::vector1< bool > & is_terminus,
	std::string const & mode
)
{
	using namespace basic::options;
	using namespace basic::options::OptionKeys;

	utility::vector1< core::Size > resnum;
	is_terminus.resize( 0 );
	std::string str;

	core::pose::Pose pose;
	ss->fill_pose( pose );

	utility::vector1< std::pair< core::Size, core::Size > > loopregion;

	// Get info from ss first, than lookup input option
	if ( mode == "loop"  ) {
		str = ss->get_string_value("loopresidues");
		if ( str != "" ) {
			TR.Debug << "Loop string from input silent: " << str << std::endl;
		} else if ( option[ lh::loop_string ].user() ) {
			str = option[ lh::loop_string ]();
			TR.Debug << "Loop string from command: " << str << std::endl;
		}
	} else if ( mode == "segment"  ) { // read seg
		str = ss->get_string_value("segresidues");
		if ( str != "" ) {
			TR.Debug << "Seg string from input silent: " << str << std::endl;
		} else if ( option[ lh::seg_string ].user() ) {
			str = option[ lh::seg_string ]();
			TR.Debug << "Seg string from command: " << str << std::endl;
		}
	} else {
		TR << "Unknown mode! return nothing." << std::endl;
		return loopregion;
	}

	utility::vector1< std::string > const str_residues( utility::string_split( str , ',' ) );

	for ( std::string const & res : str_residues ) { // this is from boost, copied from core/pose/selection.cc
		if ( res == "" ) continue;
		if ( res.find('-') != std::string::npos ) {
			// Handle residue range
			utility::vector1< std::string > const str_limits( utility::string_split( res , '-' ) );
			if ( str_limits.size() != 2 ) continue;

			core::Size const start ( core::pose::parse_resnum( str_limits[1], pose ) );
			core::Size const end ( core::pose::parse_resnum( str_limits[2], pose ) );
			if ( start && end && start > end ) {
				utility_exit_with_message("Invalid residue range: " + res);
			}

			// 1. prob by looplen
			loopregion.push_back( std::make_pair( start, end ) );

			bool terminus =  pose.residue( start ).is_terminus() || pose.residue( end ).is_terminus();
			is_terminus.push_back( terminus );
		}
	}

	return loopregion;
}

// make sure that ss is not being re-used to accumulate wrong info
void
get_loop_info( core::io::silent::SilentStructOP ss,
	core::Size &res1,
	core::Size &res2,
	bool &is_terminus )
{
	using namespace basic::options;
	using namespace basic::options::OptionKeys;

	// Initialize
	res1 = 0; res2 = 0;
	core::pose::Pose pose;
	ss->fill_pose( pose );

	utility::vector1< bool > is_region_terminus;
	utility::vector1< std::pair< core::Size, core::Size > > loopregion =
		get_loop_info_full( ss, is_region_terminus );

	if ( loopregion.size() == 0 ) {
		utility_exit_with_message( "No loop info found! check -lh:loop_string." );
	}

	utility::vector1< core::Real > aprob;
	core::Real probsum( 0.0 );
	core::Real const pbase = 5; // to enhance prob for shorter loops

	for ( core::Size iloop = 1; iloop <= loopregion.size(); ++iloop ) {
		core::Size start = loopregion[iloop].first;
		core::Size end = loopregion[iloop].second;

		probsum += pbase + (core::Real)(end - start + 1);
		aprob.push_back( probsum );
	}

	// Normalize
	TR << "Loop prob. by len:";
	for ( core::Size i = 1; i <= aprob.size(); ++i ) {
		aprob[i] /= probsum;
		TR << " " << std::setw(6) << aprob[i];
	}

	// 2. take inverse prob. on number loops already visited
	probsum = 0.0;
	utility::vector1< core::Real > aprob_nvisit( aprob.size(), 0.0 );
	for ( core::Size i = 1; i <= aprob.size(); ++i ) {
		std::stringstream sstream( "" );
		core::Real n_sampled = ss->get_energy( sstream.str() );
		core::Real prob = std::exp( -n_sampled );
		probsum += prob;
		aprob_nvisit[i] = probsum;
	}

	// Normalize
	TR << ", prob. by visited:";
	for ( core::Size i = 1; i <= aprob_nvisit.size(); ++i ) {
		aprob_nvisit[i] /= probsum;
		TR << " " << std::setw(6) << aprob_nvisit[i];
	}

	TR << ", overall: ";
	for ( core::Size i = 1; i <= aprob_nvisit.size(); ++i ) {
		aprob[i] = 0.5*(aprob[i] + aprob_nvisit[i]);
		TR << " " << std::setw(6) << aprob[i];
	}
	TR << std::endl;

	// Pick by random
	core::Real rannum = numeric::random::rg().uniform();
	//TR << "Rannum: " << rannum;
	core::Size isel( 1 );
	for ( core::Size i = 1; i <= aprob.size(); ++i ) {
		if ( rannum <= aprob[i] ) {
			res1 = loopregion[i].first;
			res2 = loopregion[i].second;
			std::stringstream sstream( "" );
			sstream << "nsampled_loop" << i;
			core::Real nsampled_new = ss->get_energy( sstream.str() ) + 1.0;
			ss->add_energy( sstream.str(), nsampled_new );
			isel = i;
			break;
		}
	}

	is_terminus = is_region_terminus[isel];
}

void
constrain_residue( core::pose::Pose &pose,
	core::Size const resno,
	utility::vector1< core::Size > exclres,
	std::string const & cst_type,
	core::Real const stdev)
{
	using namespace core::scoring::constraints;

	if ( !pose.residue( resno ).has( " CA ") ) return;

	core::Size iatm( pose.residue( resno ).atom_index(" CA ") );
	core::id::AtomID atomID( iatm, resno );
	core::Vector const &xyz = pose.xyz( atomID );

	if ( cst_type == "coordinate" ) {
		core::scoring::func::FuncOP fx( new BoundFunc( 0.0, 1.0, stdev, "loopanchor" ) );
		pose.add_constraint( ConstraintCOP( ConstraintOP(
			new CoordinateConstraint( atomID, atomID, xyz, fx )))
		);
	} else { // ca-ca atompair
		for ( core::Size jres = 1; jres <= pose.size(); ++jres ) {
			if ( !pose.residue( jres ).has( " CA " ) ) continue;
			if ( jres == resno || exclres.contains( jres ) ) continue;

			core::Size jatm( pose.residue( jres ).atom_index(" CA ") );
			core::id::AtomID atomID2( jatm, jres );
			core::Vector const &xyz2 = pose.xyz( atomID2 );
			core::Real const dist = xyz.distance( xyz2 );

			if ( dist <= 10.0 ) {
				core::scoring::func::FuncOP fx( new core::scoring::func::HarmonicFunc( dist, stdev ) );
				pose.add_constraint( ConstraintCOP( ConstraintOP(
					new AtomPairConstraint( atomID, atomID2, fx )
					)));
			}
		}
	}
}

utility::vector1< core::Size >
get_touched_res( core::pose::Pose const pose,
	utility::vector1< core::Size > const loopres,
	core::Real dist_cut
)
{

	utility::vector1< core::Size > touched_res = loopres;

	for ( core::Size i = 1; i <= loopres.size(); ++i ) {
		core::Size const ires( loopres[i] );
		if ( ires < 1 || ires > pose.size() ) continue;

		// Get crds for search: mainchain + nbr
		utility::vector1< core::Vector > loopcrds;
		for ( core::Size iatm = 1; iatm <= pose.residue(ires).n_mainchain_atoms(); ++iatm ) {
			loopcrds.push_back( pose.residue(ires).xyz(iatm) );
		}

		core::Vector Cb_crd_i = pose.residue( ires ).nbr_atom_xyz();
		loopcrds.push_back( Cb_crd_i );

		// iter through
		for ( core::Size i_crd = 1; i_crd <= loopcrds.size(); ++i_crd ) {
			core::Vector &crd_i = loopcrds[i_crd];

			for ( core::Size jres = 1; jres <= pose.size(); ++jres ) {
				if ( touched_res.contains( jres ) ) continue;

				core::Vector const Cb_crd_j = pose.residue( jres ).nbr_atom_xyz();

				if ( crd_i.distance( Cb_crd_j ) < dist_cut ) {
					touched_res.push_back( jres );
					break;
				}
			}

		}
	}

	return touched_res;
}

protocols::minimization_packing::PackRotamersMoverOP
setup_packer( core::pose::Pose const &pose,
	core::kinematics::MoveMap const mm,
	core::scoring::ScoreFunctionCOP sfxn )
{
	using namespace core::pack;
	using namespace core::pack::task;
	using namespace core::pack::task::operation;

	// Setup TaskOperation from movemap
	TaskFactoryOP local_tf( new TaskFactory() );
	local_tf->push_back( utility::pointer::make_shared< RestrictToRepacking >() );
	PreventRepackingOP turn_off_packing( new PreventRepacking() );

	for ( core::Size ires = 1; ires <= pose.size(); ++ires ) {
		if ( !mm.get_chi(ires) ) {
			turn_off_packing->include_residue(ires);
		}
	}
	local_tf->push_back( turn_off_packing );

	//Include current rotamer by default
	local_tf->push_back( utility::pointer::make_shared< IncludeCurrent >() );

	protocols::minimization_packing::PackRotamersMoverOP packer
		( new protocols::minimization_packing::PackRotamersMover( sfxn ) );

	packer->task_factory(local_tf);

	return packer;
}

void
add_movemap_from_loopres( core::kinematics::MoveMap &mm,
	core::pose::Pose const &pose,
	utility::vector1< core::Size > const loopres,
	bool const nonideal )
{
	for ( core::Size ires = 1; ires <= loopres.size(); ++ires ) {
		core::Size const resno( loopres[ ires ] );
		mm.set_bb( resno, true );
		mm.set_chi( resno, true );

		if ( nonideal ) {
			for ( core::Size j=1; j<=pose.residue_type(resno).natoms(); ++j ) {
				mm.set( core::id::DOF_ID(core::id::AtomID(j,resno), core::id::THETA ), true );
				mm.set( core::id::DOF_ID(core::id::AtomID(j,resno), core::id::D ), true );
			}
		}
	}
}

// This will only work in Cartesian space because
// local loophash doesn't care about ideal geometry
void ramp_minpack_loop( core::pose::Pose &pose,
	utility::vector1< core::Size > const loopres,
	core::scoring::ScoreFunctionCOP sfxn,
	bool const nonideal,
	bool const ramp,
	bool const efficient,
	bool const envmin
)
{
	// Expand loopres into its neighbors
	utility::vector1< core::Size > touched_residue = get_touched_res( pose, loopres );

	core::kinematics::MoveMap mm;
	mm.set_bb( false );  mm.set_chi( false );  mm.set_jump( true );

	add_movemap_from_loopres( mm, pose, loopres, nonideal );

	for ( core::Size ires = 1; ires <= touched_residue.size(); ++ires ) {
		core::Size const resno( touched_residue[ ires ] );
		mm.set_chi( resno, true );
		if ( envmin ) mm.set_bb( resno, true );

		if ( nonideal ) {
			for ( core::Size j=1; j<=pose.residue_type(resno).natoms(); ++j ) {
				mm.set( core::id::DOF_ID(core::id::AtomID(j,resno), core::id::THETA ), true );
				mm.set( core::id::DOF_ID(core::id::AtomID(j,resno), core::id::D ), true );
			}
		}
	}

	protocols::minimization_packing::PackRotamersMoverOP packer = setup_packer( pose, mm, sfxn );

	core::optimization::CartesianMinimizer minimizer;
	core::optimization::MinimizerOptions minoption( "lbfgs_armijo_nonmonotone",
		0.00001, true, false, false );

	if ( ramp ) {
		// Ramp schedule
		float w_ramp[] = { 0.02, 0.25, 0.55, 1.0 };
		int max_iter[] = { 50, 50, 100, 200 };

		// Pack & Min
		core::scoring::ScoreFunctionOP sfxn_loc = sfxn->clone();
		if ( nonideal && (*sfxn_loc)[ core::scoring::cart_bonded ] < 1.e-3 ) {
			sfxn_loc->set_weight( core::scoring::cart_bonded, 0.8 );
		}

		for ( int i = 0; i< 4; ++i ) {
			packer->apply( pose );
			minoption.max_iter( (core::Size)(max_iter[i]) );
			sfxn_loc->set_weight( core::scoring::fa_rep,
				(*sfxn)[ core::scoring::fa_rep ] * (core::Real)(w_ramp[i]) );

			minimizer.run( pose, mm, *sfxn_loc, minoption );
		}
	} else {
		packer->apply( pose );
		if ( efficient ) {
			minoption.max_iter( 30 );
		} else {
			minoption.max_iter( 200 );
		}
		minimizer.run( pose, mm, *sfxn, minoption );
	}
}

void ramp_minpack_pose( core::pose::Pose &pose,
	core::scoring::ScoreFunctionCOP sfxn,
	bool const nonideal,
	bool const ramp
)
{
	utility::vector1< core::Size > reslist;
	for ( core::Size ires = 1; ires <= pose.size(); ++ires ) {
		reslist.push_back( ires );
	}
	ramp_minpack_loop( pose, reslist, sfxn, nonideal, ramp );

}

void add_poseinfo_to_ss( core::io::silent::SilentStruct &ss,
	core::pose::Pose const &ref_pose,
	std::string const & suffix )
{
	core::pose::Pose pose;
	ss.fill_pose( pose );

	// 1. Get resmap: this might be duplication but lets just use
	std::map< core::Size, core::Size > resmap;

	for ( core::Size ii = 1; ii <= pose.size(); ++ii ) {
		core::Size ii_pdb( pose.pdb_info()->number( ii ) );
		if ( !pose.residue( ii ).is_protein() ) continue;

		for ( core::Size jj = 1; jj <= ref_pose.size(); ++jj ) {
			if ( !ref_pose.residue(jj).is_protein() ) continue;

			core::Size jj_pdb( ref_pose.pdb_info()->number( jj ) );

			if ( ii_pdb == jj_pdb ) {
				resmap[ii] = jj;
				break;
			}
		}
	}

	// 2. run gdtmm, gdtha
	// use new gdttm based on TMscore
	core::Real gdtha, gdttm;

	core::scoring::CA_gdttm( pose, ref_pose, gdttm, gdtha, resmap );

	ss.add_energy( "gdttm"+suffix, gdttm );
	ss.add_energy( "gdtha"+suffix, gdtha );
}

core::Real Zscore_to_library( core::Real const score,
	core::Real const mean,
	core::Real const stdev,
	core::Real const maxval,
	core::Real const minval
) {
	core::Real Zscore = ( score - mean ) / stdev;
	debug_assert( minval < maxval );

	if ( Zscore > maxval ) Zscore = maxval;
	if ( Zscore < minval ) Zscore = minval;
	return Zscore;
}

utility::vector1< core::Size >
loopstring_to_loopvector( std::string const & loopstr,
	core::Size const ext )
{

	utility::vector1< std::string > const str_residues( utility::string_split( loopstr , ',' ) );
	utility::vector1< core::Size > loopregion;

	for ( std::string const & res : str_residues ) { // this is from boost, copied from core/pose/selection.cc
		if ( res == "" ) continue;
		if ( res.find('-') != std::string::npos ) {
			// Handle residue range
			utility::vector1< std::string > const str_limits( utility::string_split( res , '-' ) );
			if ( str_limits.size() != 2 ) continue;

			auto start = (core::Size) (std::atoi( str_limits[1].c_str() ));
			auto end = (core::Size) (std::atoi( str_limits[2].c_str() ));

			start = start - ext > 1? start - ext : 1;

			for ( core::Size ires = start; ires <= end; ++ires ) {
				loopregion.push_back( ires );
			}
		}
	}

	return loopregion;
}

utility::vector1< utility::vector1< core::Size > >
loopstring_to_loopregions( std::string const & loopstr )
{

	utility::vector1< std::string > const str_residues( utility::string_split( loopstr , ',' ) );
	utility::vector1< utility::vector1< core::Size > > loopregions;

	//bool contains_loop( false );
	for ( std::string const & res : str_residues ) { // this is from boost, copied from core/pose/selection.cc
		if ( res == "" ) continue;
		if ( res.find('-') != std::string::npos ) {
			// Handle residue range
			utility::vector1< std::string > const str_limits( utility::string_split( res , '-' ) );
			if ( str_limits.size() != 2 ) continue;

			auto const start = (core::Size) (std::atoi( str_limits[1].c_str() ));
			auto const end = (core::Size) (std::atoi( str_limits[2].c_str() ));

			utility::vector1< core::Size > loopregion;
			for ( core::Size ires = start; ires <= end; ++ires ) loopregion.push_back( ires );
			loopregions.push_back( loopregion );

		}
	}

	return loopregions;
}

void
copy_pose_crd( core::pose::Pose const pose_frame,
	core::pose::Pose &pose_work,
	utility::vector1< utility::vector1< core::Size > > const loopregions )
{
	core::pose::Pose pose_ref( pose_work );
	pose_work = pose_frame;

	for ( core::Size ireg = 1; ireg <= loopregions.size(); ++ireg ) {
		utility::vector1< core::Size > const &loopreg = loopregions[ireg];
		for ( core::Size resno = loopreg[1]; resno <= loopreg[loopreg.size()]; ++resno ) {
			if ( resno < 1 ) continue;

			core::conformation::Residue const &refres = pose_ref.residue( resno );
			core::conformation::Residue const &workres = pose_work.residue( resno );

			for ( core::Size iatm = 1; iatm <= refres.natoms(); ++iatm ) {
				std::string const &atomname = refres.atom_name( iatm );

				if ( !workres.has( atomname ) ) {
					continue;
				}

				core::Size iatm_work = workres.atom_index( atomname );
				core::id::AtomID atomid( iatm_work, resno );
				core::Vector const &crd = refres.xyz( iatm );
				pose_work.set_xyz( atomid, crd );
			}
		}
	}
}

void mean_and_stdev( utility::vector1< core::Real > values,
	core::Real const frac,
	core::Real &shave_cut,
	core::Real &mean,
	core::Real &stdev )
{
	// Sort
	std::vector< core::Real > values_cut( values.size() );
	for ( core::Size i = 1; i <= values.size(); ++i ) {
		values_cut[i-1] = values[i];
	}
	std::sort( values_cut.begin(), values_cut.end() );

	// Get mean/stdev on 90% (to remove outliers)
	auto start = (core::Size)(values.size()*0.1);
	auto end = (core::Size)(values.size()*0.9);

	mean = 0.0; shave_cut = 0.0; stdev = 0.0;

	for ( core::Size i = start; i < end; ++i ) mean += values_cut[i];
	mean /= (core::Real)( end - start );

	for ( core::Size i = start; i < end; ++i ) {
		stdev += ( values_cut[i] - mean )*( values_cut[i] - mean );
	}

	stdev /= (core::Real)( end - start );
	stdev = std::sqrt( stdev );

	//shaving criteria
	auto ncut2 = (core::Size)((end-start)*(1.0-frac));
	if ( ncut2 < 1 ) ncut2 = 1;
	shave_cut = values_cut[start+ncut2];
}

void
superimpose_all( core::io::silent::SilentStructCOP ss_ref,
	protocols::wum::SilentStructStore &structs,
	utility::vector1< std::string > const columns_copy
)
{
	core::pose::Pose pose1, pose2;
	TR.Debug << "Ref: " << ss_ref->decoy_tag() << std::endl;
	ss_ref->fill_pose( pose1 );
	protocols::wum::SilentStructStore structs_al;

	for ( core::Size iss = 0; iss < structs.size(); ++iss ) {
		core::io::silent::SilentStructOP ss2 = structs.get_struct( iss );
		std::string decoytag( ss2->decoy_tag() );
		TR.Debug << "Running " << decoytag << std::endl;

		utility::vector1< core::Real > info_pass;
		for ( core::Size i = 1; i <= columns_copy.size(); ++i ) info_pass.push_back( ss2->get_energy(columns_copy[i]) );

		ss2->fill_pose( pose2 );
		core::scoring::calpha_superimpose_pose( pose2, pose1 );

		ss2->fill_struct( pose2, decoytag );
		for ( core::Size i = 1; i <= columns_copy.size(); ++i ) ss2->add_energy( columns_copy[i], info_pass[i] );

		structs_al.add( ss2 );
	}
	structs.clear();
	structs.add( structs_al );
}

core::Real CA_Sscore( core::io::silent::SilentStructOP ss1,
	core::io::silent::SilentStructOP ss2,
	core::Real &rmsd,
	utility::vector1< core::Size > const & loopres,
	bool const superimpose,
	core::Real const dbase
)
{
	core::Real const dbase2( dbase*dbase );

	/*
	ObjexxFCL::FArray2D< numeric::Real > crd1 = ss1->get_CA_xyz();
	ObjexxFCL::FArray2D< numeric::Real > crd2 = ss2->get_CA_xyz();
	runtime_assert( crd1.size() == crd2.size() );

	int n = (int)( crd1.size()/3 );

	// Grab structural alignment part from numeric
	ObjexxFCL::FArray1D< numeric::Real > ww( n, 1.0 );
	ObjexxFCL::FArray2D< numeric::Real > uu( 3, 3, 0.0 );

	// structure alignment (ww, uu, ctx are dummy here)
	numeric::model_quality::findUU( crd1, crd2, ww, n, uu, rmsd );
	*/

	utility::vector1< core::Vector > crd1, crd2;
	if ( superimpose ) {
		core::pose::Pose pose1, pose2;
		ss1->fill_pose( pose1 );
		ss2->fill_pose( pose2 );
		core::scoring::calpha_superimpose_pose( pose1, pose2 );

		for ( core::Size i_ca = 1; i_ca <= pose1.total_residue(); ++i_ca ) {
			if ( !loopres.contains( i_ca ) ) continue;
			if ( !pose1.residue( i_ca ).has( "CA" ) ) continue;
			crd1.push_back( pose1.residue( i_ca ).xyz("CA") );
			crd2.push_back( pose2.residue( i_ca ).xyz("CA") );
		}

	} else {
		ObjexxFCL::FArray2D< numeric::Real > FAcrd1 = ss1->get_CA_xyz();
		ObjexxFCL::FArray2D< numeric::Real > FAcrd2 = ss2->get_CA_xyz();
		core::Size const n = FAcrd1.size()/3;

		for ( core::Size i_ca = 1; i_ca <= n; ++i_ca ) {
			core::Vector xyz1, xyz2;
			for ( core::Size j = 0; j < 3; ++j ) {
				xyz1[j] = FAcrd1(j,i_ca);
				xyz2[j] = FAcrd2(j,i_ca);
			}
			crd1.push_back( xyz1 );
			crd2.push_back( xyz2 );
		}
	}

	// Get Sscore
	core::Real Sscore( 0.0 );
	rmsd = 0.0;
	core::Size m( loopres.size() );
	for ( core::Size i_ca = 1; i_ca <= crd1.size(); ++i_ca ) {
		core::Real d2( 0.0 );
		core::Vector const &xyz1 = crd1[i_ca];
		core::Vector const &xyz2 = crd2[i_ca];
		for ( core::Size j = 1; j <= 3; ++j ) {
			d2 += ( xyz1[j] - xyz2[j] ) * ( xyz1[j] - xyz2[j] );
		}

		rmsd += d2;
		Sscore += 1.0/(1.0 + d2/dbase2);
	}

	rmsd /= (core::Real)(m);
	Sscore /= (core::Real)(m);
	rmsd = std::sqrt( rmsd );
	return (Sscore);
}

core::Real CA_Sscore( core::io::silent::SilentStructOP ss1,
	core::io::silent::SilentStructOP ss2,
	core::Real &rmsd,
	bool const superimpose,
	core::Real const dbase
)
{
	ObjexxFCL::FArray2D< numeric::Real > crd1 = ss1->get_CA_xyz();
	core::Size n = crd1.size()/3;
	utility::vector1< core::Size > loopres;
	for ( core::Size ires = 1; ires <= n; ++ires ) loopres.push_back( ires );

	return CA_Sscore( ss1, ss2, rmsd, loopres, superimpose, dbase );
}

core::Real
distance( core::io::silent::SilentStructOP ss1,
	core::io::silent::SilentStructOP ss2,
	std::string const & similarity_measure,
	bool const superimpose )
{
	using namespace basic::options;
	using namespace basic::options::OptionKeys;

	std::string ssname = ss2->decoy_tag();
	core::Real dist( 0.0 );

	if ( ssname == ss1->decoy_tag()  ) {
		dist = 0.0;
	} else {
		if ( similarity_measure == "Sscore"  ) {
			core::Real dumm; // Dummy variable for return-by-ref rmsd
			dist = CA_Sscore( ss1, ss2, dumm, superimpose, 2.0 );

		} else if ( similarity_measure == "rmsd"  ) {
			CA_Sscore( ss1, ss2, dist, superimpose, 2.0 );

		} else if ( similarity_measure == "looprmsd"  ) {
			std::string loopstr = option[ lh::loop_string ]();
			utility::vector1< core::Size > loopres = loopstring_to_loopvector( loopstr );
			CA_Sscore( ss1, ss2, dist, loopres, superimpose, 2.0 );
		} else {
			TR.Warning << "Unknown similarity measure '" << similarity_measure << "' - returning zero for distance metric." << std::endl;
		}
	}
	return dist;
}


void
add_init_dev_penalty( core::io::silent::SilentStructOP ss,
	ObjexxFCL::FArray2D< core::Real > const init_xyz,
	std::string const & mode,
	core::Real const iha_cut,
	core::Real const iha_penalty_slope
)

{
	core::Real igdttm, igdtha;

	// in case init_pose undefined
	core::Real const score( ss->get_energy("score") );
	core::Real penalty( 0.0 );

	// use crd (instead of pose) to reduce enumeration time
	ObjexxFCL::FArray2D< core::Real > xyz_tmp = ss->get_CA_xyz();

	// in case when nres do not match
	if ( xyz_tmp.size() != init_xyz.size() ) {
		igdtha = 10.0; // assign very small number

	} else {
		core::scoring::xyz_gdttm( init_xyz, xyz_tmp, igdttm, igdtha );

		if ( igdtha < iha_cut ) {
			core::Real dha(iha_cut-igdtha);
			if ( mode == "absolute"  ) {
				penalty = std::min(1.0, iha_penalty_slope*dha*dha );
			} else if ( mode == "relative"  ) {
				core::Real f = (dha/iha_cut - 0.03);
				if ( f < 0.0 ) {
					penalty = 0.0;
				} else {
					penalty = std::min(1.0, iha_penalty_slope*f*f );
				}
			}
		}
	}

	ss->add_energy( "penaltysum", score*(1.0-penalty) );
	ss->add_energy( "igdtha", igdtha );
}

// support for pose0 instead of FArray2D
void
add_init_dev_penalty( core::io::silent::SilentStructOP ss,
	core::pose::Pose pose0,
	std::string const &  mode,
	core::Real const iha_cut,
	core::Real const iha_penalty_slope
)
{
	core::Size nres( 0 );
	for ( core::Size ires = 1; ires <= pose0.total_residue(); ++ires ) {
		if ( !pose0.residue(ires).is_virtual_residue() ) nres++;
	}
	ObjexxFCL::FArray2D< core::Real > init_xyz( 3, nres );

	for ( core::Size ires = 1; ires <= pose0.total_residue(); ++ires ) {
		if ( pose0.residue(ires).is_virtual_residue() ) continue;
		core::Vector const &xyz = pose0.residue(ires).xyz("CA");
		for ( core::Size k = 1; k <= 3; ++k ) init_xyz(k,ires) = xyz[k-1];
	}

	add_init_dev_penalty( ss, init_xyz, mode, iha_cut, iha_penalty_slope );
}

// support for multiple poses
void
add_init_dev_penalty( protocols::wum::SilentStructStore &structs,
	core::pose::Pose const pose0,
	std::string const & mode,
	core::Real const iha_cut,
	core::Real const iha_penalty_slope
)
{
	using namespace ObjexxFCL::format;

	// fill FArray2D init_xyz
	core::Size nres( pose0.total_residue() );

	ObjexxFCL::FArray2D< core::Real > init_xyz( 3, nres );
	core::Vector xyz( 0.0 );
	for ( core::Size ires = 1; ires <= pose0.total_residue(); ++ires ) {
		// I know this is dangerous; hack way to match nres to ss->get_CA_xyz()
		// as it adds virtual residue "CA" (don't know what it is...)
		if ( pose0.residue(ires).is_virtual_residue() ) {
			for ( core::Size k = 1; k <= 3; ++k ) init_xyz(k,ires) = xyz[k-1];
		} else {
			xyz = pose0.residue(ires).xyz("CA");
			for ( core::Size k = 1; k <= 3; ++k ) init_xyz(k,ires) = xyz[k-1];
		}
	}

	//
	for ( core::Size iss = 0; iss < structs.size(); ++iss ) {
		core::io::silent::SilentStructOP ss = structs.get_struct( iss );

		add_init_dev_penalty( ss, init_xyz, mode, iha_cut, iha_penalty_slope );

		core::Real fpenalty = 1.0 - (ss->get_energy("penaltysum"))/(ss->get_energy("score"));
		TR << "Penalty for " << A(30,ss->decoy_tag()) << ": "
			<< F(6,2,ss->get_energy("igdtha")) << " "
			<< F(8,3,fpenalty) << std::endl;
	}
}

// ss2 format parser
std::map< core::Size, utility::vector1< core::Real > >
read_ss2( std::string ssfile )
{
	std::map< core::Size, utility::vector1< core::Real > > ss2;

	// order: C H E
	std::ifstream infile( ssfile.c_str() );
	std::string line;

	while ( getline(infile,line) ) {
		utility::vector1< std::string > tokens ( utility::split( line ) );
		if ( tokens.size() < 6 ) continue;

		utility::vector1< core::Real > ps( 3, 0.0 );
		core::Size resno = atoi( tokens[1].c_str() );
		for ( core::Size i = 1; i <= 3; ++i ) ps[i] = atof( tokens[i+3].c_str() );
		ss2[resno] = ps;
	}

	return ss2;
}

} // namespace mpi_refinement
} // namespace
