// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file    protocols/grafting/GraftMoverBase.cc
/// @brief   Base class for GraftMovers
/// @author  Jared Adolf-Bryfogle (jadolfbr@gmail.com)

//Unit headers
#include <protocols/grafting/GraftMoverBase.hh>
#include <protocols/moves/Mover.hh>

//Core headers
#include <core/pose/Pose.hh>
#include <core/pose/variant_util.hh>
#include <core/kinematics/MoveMap.fwd.hh>

#include <core/chemical/VariantType.hh>

//Protocol Headers


//Basic Headers
#include <basic/Tracer.hh>

//Utility Headers
#include <protocols/grafting/util.hh>


static basic::Tracer TR( "protocols.grafting.GraftMoverBase" );

namespace protocols {
namespace grafting {

using namespace core;
using namespace core::pose;
using namespace protocols::moves;
using namespace core::scoring;
using core::Size;
using core::kinematics::MoveMapOP;
using core::pose::Pose;
using core::pose::PoseOP;



GraftMoverBase::GraftMoverBase(std::string mover_name):
	moves::Mover(mover_name),
	start_(0),
	end_(0),
	original_end_(0),
	Nter_overhang_length_(0),
	Cter_overhang_length_(0),
	copy_pdbinfo_(false)
{

}

GraftMoverBase::GraftMoverBase(core::Size start, core::Size end, std::string mover_name):
	moves::Mover(mover_name),
	start_(start),
	end_(end),
	original_end_(end),
	Nter_overhang_length_(0),
	Cter_overhang_length_(0),
	copy_pdbinfo_(false)

{

}

GraftMoverBase::GraftMoverBase(const core::Size start, const core::Size end, std::string mover_name, core::pose::Pose const & piece, core::Size Nter_overhang_length, core::Size Cter_overhang_length) :
	moves::Mover(mover_name),
	start_(start),
	end_(end),
	original_end_(end),
	Nter_overhang_length_(0),
	Cter_overhang_length_(0),
	copy_pdbinfo_(false)

{
	set_piece(piece, Nter_overhang_length, Cter_overhang_length);
}


void
GraftMoverBase::set_piece(Pose const & piece, core::Size Nter_overhang_length, core::Size Cter_overhang_length){
	piece_ = utility::pointer::make_shared< core::pose::Pose >(piece);
	Nter_overhang_length_ = Nter_overhang_length;
	Cter_overhang_length_=Cter_overhang_length;
	insertion_length_ = piece.size()-Cter_overhang_length_-Nter_overhang_length_;

}

Pose
GraftMoverBase::insert_piece(Pose const & pose, Pose & piece){

	//Delete overhang if necessary.
	insertion_length_ = piece.size()-Cter_overhang_length_-Nter_overhang_length_;
	delete_overhang_residues(piece, Nter_overhang_length_, Cter_overhang_length_);

	//piece_->dump_pdb("cdr_pose.pdb");
	//strip termini variants from insert if necessary
	core::pose::remove_variant_type_from_pose_residue(piece, core::chemical::LOWER_TERMINUS_VARIANT, 1);
	core::pose::remove_variant_type_from_pose_residue(piece, core::chemical::UPPER_TERMINUS_VARIANT, insertion_length_);

	//Delete residues for pose.
	Pose final_pose(pose);
	delete_region(final_pose, start_+1, end_-1);

	//final_pose.dump_pdb("deleted_scaffold_pose.pdb");
	final_pose = insert_pose_into_pose(final_pose, piece, start_, start_+1, copy_pdbinfo_);

	//Update end residue number.
	end_ = start_+insertion_length_+1;
	TR <<"Insertion complete."<<std::endl;
	return final_pose;

}


// Mover methods
std::string
GraftMoverBase::get_name() const
{
	return "GraftMoverBase";
}

void
GraftMoverBase::set_insert_region(const core::Size start, const core::Size end){
	start_ = start;
	end_ = end;
	original_end_=end;
}

void
GraftMoverBase::copy_pdbinfo(bool copy_pdbinfo){
	copy_pdbinfo_ = copy_pdbinfo;
}

///////Accessors and Mutators of private data for derived classes

void
GraftMoverBase::original_end(core::Size original_end){
	original_end_ = original_end;
}

Size GraftMoverBase::original_end(){return original_end_;}

void GraftMoverBase::insertion_length(core::Size insertion_length){
	insertion_length_ = insertion_length;
}

Size GraftMoverBase::insertion_length(){return insertion_length_;}

Size GraftMoverBase::start(){return start_;}
void GraftMoverBase::start(core::Size start){
	start_ = start;
}
Size GraftMoverBase::end(){return end_;}
void GraftMoverBase::end(core::Size end){
	end_ = end;
	original_end_=end;
}
Size GraftMoverBase::Nter_overhang_length(){return Nter_overhang_length_;}
void GraftMoverBase::Nter_overhang_length(core::Size overhang){
	Nter_overhang_length_ = overhang;
}

Size GraftMoverBase::Cter_overhang_length(){return Cter_overhang_length_;}
void GraftMoverBase::Cter_overhang_length(core::Size overhang){
	Cter_overhang_length_ = overhang;
}
PoseCOP GraftMoverBase::piece(){return piece_;}
void GraftMoverBase::piece(PoseCOP piece){piece_ = piece;}

}  // namespace grafting
}  // namespace protocols
