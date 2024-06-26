// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file core/io/silent/BinarySilentStruct.hh
///
/// @brief
/// @author Frank DiMaio
/// @author Mike Tyka

#ifndef INCLUDED_core_io_silent_BinarySilentStruct_hh
#define INCLUDED_core_io_silent_BinarySilentStruct_hh

// mini headers
#include <core/types.hh>
#include <core/pose/Pose.fwd.hh>
#include <core/io/silent/SilentStruct.hh>
#include <core/kinematics/FoldTree.hh>
#include <core/kinematics/Jump.hh>

// C++ Headers
#include <iosfwd>
#include <string>

#include <core/conformation/symmetry/SymmetryInfo.hh>
#include <utility/vector1.hh>

#include <core/id/NamedAtomID.hh> // DO NOT AUTO-REMOVE


namespace core {
namespace io {
namespace silent {

class BinarySilentStruct;

typedef utility::pointer::shared_ptr< BinarySilentStruct > BinarySilentStructOP;


/// why this inheritance pathway? this makes no sense!
//class BinarySilentStruct : public ProteinSilentStruct {
class BinarySilentStruct : public SilentStruct {

	typedef SilentStruct Parent;

public:

	/// @brief Constructors.
	BinarySilentStruct( SilentFileOptions const & opts, Size const nres_in );

	BinarySilentStruct( SilentFileOptions const & opts );

	BinarySilentStruct(
		SilentFileOptions const & opts,
		core::pose::Pose const & pose,
		std::string tag = "empty_tag"
	);

	BinarySilentStruct( BinarySilentStruct const & src );

	BinarySilentStructOP shared_from_this() { return utility::pointer::static_pointer_cast<BinarySilentStruct>( SilentStruct::shared_from_this() ); }

	SilentStructOP clone() const override {
		return utility::pointer::make_shared< BinarySilentStruct >( *this );
	}

	/// @brief Assignment operator.
	BinarySilentStruct & operator= ( BinarySilentStruct const & src );

	/// @brief Re-dimension the storage capacity of this BinarySilentStruct to the given number of residues.
	void resize(
		Size const nres_in
	);

	// destructor
	~BinarySilentStruct() override;

	/// @brief Initialize object from a set of lines.
	/// @details Does not skip read from container.
	bool init_from_lines(
		utility::vector1< std::string > const & lines,
		SilentFileData & container
	) override;

	/// @brief Initialize object from a set of lines.
	/// @details If skip_read_from_container is true, then the container object is not used.
	bool init_from_lines(
		utility::vector1< std::string > const & lines,
		SilentFileData & container,
		bool const skip_read_from_container
	);

	/// @brief Configure the conformation of the given Pose with the
	/// conformational data within this BinarySilentStruct. Calls
	/// pose.clear() and rebuilds Pose from scratch using FA_STANDARD residue
	/// types.
	void fill_pose(
		core::pose::Pose & pose,
		bool const metapatches = true
	) const override;

	/// @brief Configure the conformation of the given Pose with the
	/// conformational data within this BinarySilentStruct. Calls
	/// pose.clear() and rebuilds Pose from scratch using the user-specified
	/// residue types.
	void fill_pose(
		core::pose::Pose & pose,
		core::chemical::ResidueTypeSet const & residue_set,
		bool const metapatches = true
	) const override;

	/// @brief opposite of fill_pose
	void fill_struct( core::pose::Pose const & pose, std::string const & tag ) override;

	/// @brief for stepwise modeling, setup other_poses inside full_model_info
	void
	setup_other_poses( core::pose::Pose & pose, core::chemical::ResidueTypeSet const & residue_set ) const;

	/// @brief print header information
	void print_header( std::ostream & out ) const override;

	/// @brief Prints the conformation information within this
	/// BinarySilentStruct to the given std::ostream.
	void print_conformation( std::ostream & output ) const override;

	/// @brief returns the positions of the CA atoms
	ObjexxFCL::FArray2D< Real > get_CA_xyz() const override;

	// model quality-related methods.
	virtual Real CA_rmsd( BinarySilentStruct other_pss );

	/// @brief calculates the RMSD between the C-alpha atoms of a Pose built from
	/// the torsions in this ProteinSilentStruct and the C-alpha atoms from this
	/// ProteinSilentStruct.
	core::Real get_debug_rmsd() override;


	void add_jump( kinematics::Jump jump ) {
		jumps_.push_back( jump.rt() );
	}

	kinematics::RT const & jump( Size jump_num ) const {
		return jumps_[ jump_num ];
	}

	void fold_tree( kinematics::FoldTree f ) {
		fold_tree_ = f;
	}

	kinematics::FoldTree const& fold_tree( ) const {
		return fold_tree_;
	}

	bool fullatom() const {
		return fullatom_;
	}

	void fullatom( bool fullatom ) {
		fullatom_ = fullatom;
	}

	char secstruct( unsigned int seqpos ) const {
		return secstruct_[seqpos];
	}

	void secstruct( unsigned int seqpos, char ss ) {
		secstruct_[seqpos] = ss;
	}

	void symmetry_info( core::conformation::symmetry::SymmetryInfo & s ) {
		symminfo_ = utility::pointer::make_shared< core::conformation::symmetry::SymmetryInfo >( s );
	}

	core::conformation::symmetry::SymmetryInfoCOP symmetry_info( ) const {
		return symminfo_;
	}

	bool is_symmetric() const {
		return symminfo_->get_use_symmetry();
	}

	void parse_chain_endings( std::istream & stream );
	void chain_endings( utility::vector1< Size > const & endings );
	std::string chain_endings_str() const;
	utility::vector1< Size > const & chain_endings() const {
		return chain_endings_;
	}
	void add_chain_ending( Size const seqpos );

private:
	bool fullatom_;

	utility::vector1< char > secstruct_;

	utility::vector1< std::pair <core::id::NamedAtomID, core::id::NamedAtomID> > noncanonical_residue_connections_;
	utility::vector1< kinematics::RT > jumps_;
	bool bJumps_use_IntraResStub_;
	kinematics::FoldTree fold_tree_;

	core::conformation::symmetry::SymmetryInfoOP symminfo_;
	utility::vector1< Size > chain_endings_;
	utility::vector1< utility::vector1< numeric::xyzVector<float> > > atm_coords_;
}; // class BinarySilentStruct

} // namespace silent
} // namespace io
} // namespace core

#endif
