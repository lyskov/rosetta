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

// libRosetta headers

#ifndef INCLUDED_core_import_pose_pose_stream_PDBPoseInputStream_HH
#define INCLUDED_core_import_pose_pose_stream_PDBPoseInputStream_HH

#include <core/chemical/ResidueTypeSet.fwd.hh>
#include <core/pose/Pose.fwd.hh>

#include <core/import_pose/pose_stream/PoseInputStream.hh>

#include <utility/file/FileName.hh>


namespace core {
namespace import_pose {
namespace pose_stream {

class PDBPoseInputStream : public PoseInputStream {

public:
	PDBPoseInputStream() {}

	PDBPoseInputStream(
		utility::vector1< utility::file::FileName > fns
	) {
		set_filenames( fns );
	}

	PDBPoseInputStream( utility::file::FileName const & fn ) {
		utility::vector1< utility::file::FileName > fns;
		fns.push_back( fn );
		set_filenames( fns );
	}

	PDBPoseInputStream(
		utility::vector1< utility::file::FileName > fns,
		utility::vector1< utility::file::FileName > list_fns
	) {
		set_filenames( fns );
		add_list_filenames( list_fns );
	}

	~PDBPoseInputStream() override {}

	void set_filenames( utility::vector1< utility::file::FileName > filenames );

	utility::vector1< utility::file::FileName > get_filenames();

	bool has_another_pose() override;

	void reset() override;

	void fill_pose(
		core::pose::Pose & pose,
		core::chemical::ResidueTypeSet const & residue_set,
		bool const metapatches = true
	) override;
	void fill_pose( core::pose::Pose&,
		bool const metapatches = true ) override;

	utility::vector1< core::pose::PoseOP > get_all_poses(
		core::chemical::ResidueTypeSet const & residue_set
	) override;

	/// @brief adds a list of files each containing lists of PDBs
	void add_list_filenames(
		utility::vector1< utility::file::FileName > list_fns
	);

	/// @brief Get a string describing the last pose and where it came from.
	/// @details PDB filename.
	/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
	std::string get_last_pose_descriptor_string() const override;

private:
	utility::vector1< utility::file::FileName > filenames_;
	utility::vector1< utility::file::FileName >::const_iterator current_position_;
}; // PDBPoseInputStream

} // pose_stream
} // import_pose
} // core

#endif
