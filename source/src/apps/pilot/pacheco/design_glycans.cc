// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file    design_glycans.cc
/// @brief   This application performs a simple "design" of any glycans found with a .pdb.
/// @note    This is a WiP, primarily being used for the development of carbohydrate design in Rosetta.
/// @example TODO: fill in later
/// @author  Labonte <JWLabonte@jhu.edu>
/// @author  Pacheco-Garcia <d.pacheco-garcia@wustl.edu>
/// @remarks Special thanks to Vikram, whose PackerPalette makes this even possible.


// Project headers
#include <devel/init.hh>

#include <protocols/moves/Mover.hh>
#include <protocols/minimization_packing/PackRotamersMover.hh>
#include <protocols/jd2/Job.hh>
#include <protocols/jd2/JobDistributor.hh>

#include <core/types.hh>
#include <core/conformation/Conformation.hh>
#include <core/scoring/ScoreFunction.hh>
#include <core/scoring/ScoreFunctionFactory.hh>
#include <core/pose/Pose.hh>
#include <core/pack/palette/CustomBaseTypePackerPalette.hh>
#include <core/pack/task/TaskFactory.hh>
#include <core/pack/task/operation/TaskOperations.hh>

#include <core/select/residue_selector/GlycanResidueSelector.hh>

#include <core/pack/task/operation/OperateOnResidueSubset.hh>
#include <core/pack/task/operation/ResLvlTaskOperations.hh>
#include <core/select/residue_selector/GlycanResidueSelector.hh>
#include <core/select/residue_selector/NeighborhoodResidueSelector.hh>


// Utility headers
#include <utility/pointer/owning_ptr.hh>
#include <utility/excn/Exceptions.hh>

// C++ headers
#include <string>
#include <iostream>


using namespace std;
using namespace utility;
using namespace core;
using namespace protocols;



// Class Definitions //////////////////////////////////////////////////////////

/// @brief  The design_glycans protocol.
class DesignGlycansProtocol : public moves::Mover {
public:  // Standard methods
	/// @brief  Default constructor.
	DesignGlycansProtocol() : moves::Mover()
	{
		init();
	}

	/// @brief  Copy constructor.
	DesignGlycansProtocol( DesignGlycansProtocol const & /*object_to_copy*/ ) = default;

	// Assignment operator
	DesignGlycansProtocol &
	operator=( DesignGlycansProtocol object_to_copy )
	{
		swap( *this, object_to_copy );
		return *this;
	}

	// Destructor
	virtual ~DesignGlycansProtocol() = default;


	// For using the copy-and-swap idiom...
	friend void
	swap( DesignGlycansProtocol & first, DesignGlycansProtocol & second )
	{
		using std::swap;
		swap( first.sf_, second.sf_ );
		swap( first.designer_, second.designer_ );
	}


public:  // Standard Rosetta methods
	// General methods
	/// @brief  Register options with the option system.
	static void
	register_options()
	{}

	/// @brief  Generate string representation of DesignGlycansProtocol for debugging purposes.
	virtual
	void
	show( std::ostream & output=std::cout ) const
	{
		using namespace std;
		using namespace moves;

		Mover::show( output );  // name, type, tag
		for ( string info : designer_->info() ) {
			output << info << endl;
		}
	}


	// Mover methods
	/// @brief  Return the name of the Mover.
	virtual
	std::string
	get_name() const
	{
		return type();
	}

	virtual
	moves::MoverOP
	clone() const
	{
		return utility::pointer::make_shared< DesignGlycansProtocol >( *this );
	}

	virtual
	moves::MoverOP
	fresh_instance() const
	{
		return utility::pointer::make_shared< DesignGlycansProtocol >();
	}

	/// @brief  Apply the corresponding protocol to <pose>.
	virtual
	void
	apply( core::pose::Pose & pose )
	{

		show( cout );

		// Print some information about the starting pose.
		cout << endl << "Starting Pose Sequences:" << endl;
		print_sequences( pose );

		cout << endl << "Starting Score:" << endl;
		sf_->show( cout, pose );

		// Begin design protocol.

		cout << endl << "Designing..." << endl;
		designer_->apply( pose );

		cout << endl << "Ending Pose Sequences:" << endl;
		print_sequences( pose );

		cout << endl << "Ending Score:" << endl;
		sf_->show( cout, pose );
	}


private:  // Private methods
	// Set command-line options.  (Called by init())
	void
	set_commandline_options()
	{}

	// Initialize data members from arguments.
	void
	init()
	{
		using namespace pack::palette;
		using namespace pack::task;
		using namespace minimization_packing;
		using namespace operation;
		using namespace select::residue_selector;

		type( "DesignGlycansProtocol" );


		sf_ = scoring::get_score_function();

		// Set up the TaskFactory.
		CustomBaseTypePackerPaletteOP pp( utility::pointer::make_shared< CustomBaseTypePackerPalette >() );

		GlycanResidueSelectorOP glycan_res_selector(utility::pointer::make_shared< GlycanResidueSelector >() );

		PreventRepackingRLTOP prevent_repacking_rlt( utility::pointer::make_shared<  PreventRepackingRLT>() );
		NeighborhoodResidueSelectorOP nbr_Glycans_selector( utility::pointer::make_shared < NeighborhoodResidueSelector>() );
		nbr_Glycans_selector->set_focus_selector( glycan_res_selector );
		nbr_Glycans_selector->set_include_focus_in_subset( true );
		OperateOnResidueSubsetOP prevent_subset_repacking( utility::pointer::make_shared< OperateOnResidueSubset>( prevent_repacking_rlt, nbr_Glycans_selector, true ) );

		RestrictToRepackingRLTOP restrict_to_repack( utility::pointer::make_shared< RestrictToRepackingRLT >() );
		NeighborhoodResidueSelectorOP nbr_selector( utility::pointer::make_shared < NeighborhoodResidueSelector>() );
		nbr_selector->set_focus_selector( glycan_res_selector );
		nbr_selector->set_include_focus_in_subset( false ) ;
		OperateOnResidueSubsetOP prevent_nbr_design( utility::pointer::make_shared< OperateOnResidueSubset>( restrict_to_repack, nbr_selector, false ) );


		TaskFactoryOP tf( utility::pointer::make_shared< TaskFactory >() );
		tf->set_packer_palette( pp );
		tf->push_back( utility::pointer::make_shared< operation::IncludeCurrent >() );
		tf->push_back( prevent_subset_repacking );
		tf->push_back( prevent_nbr_design );

		// Instantiate the Movers.
		designer_ = utility::pointer::make_shared< PackRotamersMover >( sf_ );
		designer_->task_factory( tf );

		set_commandline_options();
	}


	// Print the sequence for each chain in a Pose.
	void
	print_sequences( core::pose::Pose const & pose )
	{
		Size const n_chains( pose.conformation().num_chains() );
		for ( core::uint i( 1 ); i <= n_chains; ++i ) {
			cout << " Chain " << i << ": ";
			cout << pose.chain_sequence( i ) << endl;
		}
	}


private:  // Private data
	core::scoring::ScoreFunctionOP sf_;

	// Movers
	protocols::minimization_packing::PackRotamersMoverOP designer_;
};


// Constants & Type Definitions ///////////////////////////////////////////////
int const SUCCESS( 0 );
int const FAILURE( -1 );

typedef utility::pointer::shared_ptr< DesignGlycansProtocol > DesignGlycansProtocolOP;


// Main ///////////////////////////////////////////////////////////////////////
int
main( int argc, char *argv[] )
{
	try {
		// Initialize Rosetta.
		cout << "Initializing Rosetta..." << endl;

		devel::init( argc, argv );

		// Construct the protocol.
		DesignGlycansProtocolOP protocol( utility::pointer::make_shared< DesignGlycansProtocol >() );


		// Distribute the mover.

		protocols::jd2::JobDistributor::get_instance()->go( protocol );
	} catch ( utility::excn::Exception const & e ) {
		e.display();
		return FAILURE;
	}
	return SUCCESS;
}
