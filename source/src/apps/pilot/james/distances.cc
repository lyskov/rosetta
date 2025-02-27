// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file
/// @brief
/// @author James Thompson



#include <core/scoring/ScoreFunction.hh>
#include <core/scoring/ScoreFunction.fwd.hh>
#include <core/scoring/ScoreFunctionFactory.hh>

#include <core/types.hh>


#include <core/pose/Pose.hh>

#include <basic/options/option.hh>

#include <devel/init.hh>


#include <ObjexxFCL/format.hh>

#include <utility/vector1.hh>

#include <apps/pilot/james/james_util.hh>

// C++ headers
#include <iostream>
#include <string>


// option key includes

#include <basic/options/keys/constraints.OptionKeys.gen.hh>
#include <basic/options/keys/in.OptionKeys.gen.hh>


#include <utility/excn/Exceptions.hh>


using namespace ObjexxFCL::format;

///////////////////////////////////////////////////////////////////////////////

int
main( int argc, char* argv [] )
{
	try {

		// options, random initialization
		devel::init( argc, argv );

		using namespace basic::options;
		using namespace basic::options::OptionKeys;

		core::scoring::ScoreFunctionOP scorefxn( core::scoring::get_score_function() );
		utility::vector1< std::string > pdbfiles = option[ in::file::s ]();
		core::Real distance_cutoff = 200;
		if ( option[ constraints::cst_weight ].user() ) {
			distance_cutoff = option[ constraints::cst_weight ]();
		}

		utility::vector1< std::string >::const_iterator iter;
		for ( iter = pdbfiles.begin(); iter != pdbfiles.end(); ++iter ) {
			std::string pdbfile = *iter;
			if ( pdbfile == "" ) {
				utility_exit_with_message( "Unable to open file: " + pdbfile + '\n' );
			}
			core::pose::Pose pose;
			//std::cerr << "READING " << pdbfile << std::endl;
			core::import_pose::pose_from_file( pose, pdbfile , core::import_pose::PDB_file); // default is standard fullatom residue_set

			//std::string outfile  = pdbfile + ".distances";
			//std::ofstream output( outfile.c_str() );
			//if ( ! output.is_open() ) {
			// utility_exit_with_message( "Unable to open file: " + outfile + '\n' );
			//}
			std::ostream & output( std::cout );

			// iterate over all atom pairs
			output  << A( 10, "resi_idx" )
				<< A( 10, "resj_idx" )
				<< A(  6, "resi" )
				<< A(  6, "resj" )
				<< A(  8, "atomi"    )
				<< A(  8, "atomj"    )
				<< A( 10, "burial_i" )
				<< A( 10, "burial_j" )
				// << A( 10, "temp_i"   )
				// << A( 10, "temp_j"   )
				<< A( 10, "dist"     )
				<< std::endl;

			(*scorefxn)(pose);
			// calculate burial
			utility::vector1< int > burial = calculate_burial( pose );
			for ( unsigned int i = 1; i <= pose.size(); ++i ) {
				for ( unsigned int j = 1; j <= pose.size(); ++j ) {
					core::conformation::Residue resi = pose.residue(i);
					core::conformation::Residue resj = pose.residue(j);

					for ( unsigned int m = 1; m <= resi.natoms(); ++m ) {
						for ( unsigned int n = 1; n <= resj.natoms(); ++n ) {

							if ( i == j ) continue;

							// skip hydrogen atoms
							if ( resi.atom_type(m).is_hydrogen() ||
									resj.atom_type(n).is_hydrogen()
									) {
								continue;
							}

							//if ( ! pose.residue_type(i).atom_is_backbone(m) ||
							//    ! pose.residue_type(j).atom_is_backbone(n) ) {
							// //std::cerr << "atom_name == |" << resi.atom_name(m) << "|"
							// //  << std::endl;
							// continue; // temporary!
							//}

							// skip atoms that don't share a type
							if ( resi.atom_type(m).name() != resj.atom_type(n).name() ) {
								continue;
							}

							//if ( resi.atom_type(m).name() != "CA" ) {
							// continue;
							//}

							core::Real const distance(
								pose.residue(i).xyz(m).distance( resj.xyz(n) )
							);
							if ( distance > distance_cutoff ) continue;

							output  << I( 10, i )
								<< I( 10, j )
								<< A(  6, resi.name1() )
								<< A(  6, resj.name1() )
								<< A(  8, resi.atom_name(m) )
								<< A(  8, resj.atom_name(n) )
								<< I( 10, burial[i] )
								<< I( 10, burial[j] )
								<< F( 10, 4, distance ) // distance
								<< std::endl;
						}  // for ( unsigned int n = 1; n <= resj.natoms(); ++n )
					} //  for ( unsigned int m = 1; m <= resi.natoms(); ++m )
				} //  for ( unsigned int j = i + 1; j <= pose.size(); ++j )
			}  // for ( unsigned int i = 1; i <= pose.size(); ++i )
			//output.close();
		} //  for ( iter = pdbfiles.begin(); iter != pdbfiles.end(); ++iter )

	} catch (utility::excn::Exception const & e ) {
		e.display();
		return -1;
	}

	return 0;
} // int main( int argc, char * argv [] )
