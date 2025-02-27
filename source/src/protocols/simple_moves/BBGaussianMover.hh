// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   protocols/simple_moves/BBGaussianMover.fwd.hh
/// @brief  Gaussian Perturbation to backbone
/// @author Yuan Liu (wendao@u.washington.edu)

#ifndef INCLUDED_protocols_simple_moves_BBGaussianMover_hh
#define INCLUDED_protocols_simple_moves_BBGaussianMover_hh

// Unit headers
#include <protocols/simple_moves/BBGaussianMover.fwd.hh>

// Package headers
#include <protocols/canonical_sampling/ThermodynamicMover.hh>

//core
#include <core/types.hh>
#include <core/pose/Pose.fwd.hh>
#include <core/conformation/Residue.fwd.hh>
//protocols
#include <protocols/moves/Mover.hh>
//movemap
#include <core/kinematics/MoveMap.fwd.hh>
#include <core/select/movemap/MoveMapFactory.fwd.hh>
//
#include <utility/vector1.hh>
#include <numeric/xyzVector.hh>

//std
#include <string>

#include <core/id/TorsionID_Range.hh> // MANUAL IWYU

namespace protocols {
namespace simple_moves {

/// @brief the basic class for implementing Gaussian Perturbation of bb
/// @note "Monte Carlo update for chain molecules: Biased Gaussian steps in torsional space"
/// "The Journal of Chemical Physics, Vol. 114, No. 18. (2001), pp. 8154-8158."
/// Two steps for perturbing the backbone and keeping the geometry constrain
/// Step 1: Gaussian Biased steps in torsional space:
/// the end atoms of the moving segment should be choosen by user
/// and to keep the geometry constrain(6), the DOF of the moving segment > 6
/// Step 2: "pivot" update the bb conformation, or followed by a chainclosure method (BBConRot)
class BBGaussianMover : public protocols::canonical_sampling::ThermodynamicMover
{
public:
	typedef core::Real Real;
	typedef core::Size Size;
	typedef core::pose::Pose Pose;
	typedef core::pose::PoseOP PoseOP;
	typedef core::pose::PoseCOP PoseCOP;
	typedef core::conformation::Residue Residue;

	typedef utility::vector1< Real > Vector;
	typedef numeric::xyzVector<Real> xyzVector;
	typedef utility::vector1< xyzVector > VdRdPhi;
	typedef utility::vector1< VdRdPhi > VMatrix;
	//typedef numeric::MathMatrix<Real> Matrix;
	typedef utility::vector1< Vector > Matrix;


public:
	BBGaussianMover(); //default: no seglist, no endlist
	BBGaussianMover( core::Size, core::Size, core::Size );
	~BBGaussianMover() override;

	protocols::moves::MoverOP clone() const override;

	//go through all the residue, count how many movable residue passed
	//count > n_pert_res (depends on the n_dof_angle)
	//stop by frozen res and cut point
	//DONE: apply smallmover to the end segment if their length is not
	//satisfy the n_pert_res_, dof: L-(n-1) => L+(n+1)

	void apply(Pose &pose) override;

	void init();
	void resize(core::Size, core::Size, core::Size);

	void factorA( Real const fA );
	void factorB( Real const fB );

	void get_G();
	void get_A();
	/// @note calculate L', update last_proposal_density
	Real get_L_prime();
	/// @note calculate L matrix and update pose
	Real get_L_move(Pose &);
	void get_VdRdPhi(Pose const &);

	/// @brief Generates the movemap used by this mover for the given pose
	core::kinematics::MoveMapCOP movemap(core::pose::Pose const &) const;

	void movemap(core::kinematics::MoveMapCOP new_movemap);
	/// @brief The movemap_factory will be used to generate a movemap, if an explcit one hasn't already been set.
	void movemap_factory(core::select::movemap::MoveMapFactoryCOP new_movemap_factory);

	//stat Nall N<0.6, if N/Nall>0.4, factorA/=2
	void update_counting_last_PDR(Real);

	using moves::Mover::last_proposal_density_ratio;


	/// @brief get the ratio of proposal densities for the last move
	Real last_proposal_density_ratio() override;
	/// @brief get whether detailed balance is preserved (i.e. no Ramachandran biasing)
	bool preserve_detailed_balance() const override;
	/// @brief set whether detailed balance is preserved (i.e. no Ramachandran biasing)
	void set_preserve_detailed_balance( bool preserve_detailed_balance ) override;
	/// dummy
	utility::vector1<core::id::TorsionID_Range>
	torsion_id_ranges( core::pose::Pose & /*pose*/ ) override {
		return utility::vector1<core::id::TorsionID_Range>();
	}

	// for kic
	void init_kic_loop(core::Size looplength, core::kinematics::MoveMapCOP mm);

	std::string
	get_name() const override;

	static
	std::string
	mover_name();

	static
	void
	provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd );


protected:
	void setup_list(Pose const &);

	//r1, r2, r: r rotate around axis r1->r2
	xyzVector get_dRdPhi(xyzVector const &r1, xyzVector const &r2, xyzVector const &r)
	{
		//dr/dphi = r x axis
		xyzVector axis((r2-r1).normalize());
		xyzVector r_local(r-r1);
		return r_local.cross(axis);
	}

	//r1, r2, r3, r: r rotate around axis (r3-r2)x(r2xr1)
	xyzVector get_dRdTheta(xyzVector const &r1, xyzVector const &r2, xyzVector const &r3, xyzVector const &r)
	{
		xyzVector axis((r3-r2).cross(r2-r1).normalize());
		xyzVector r_local(r-r2);
		return r_local.cross(axis);
	}

	// for rosetta_scripts
	void parse_my_tag(
		TagCOP,
		basic::datacache::DataMap &
	) override;

	//foreward
	Real cholesky_fw(Matrix &, core::Size, Vector &, Vector &, core::Size from=1, core::Size to=0, Real scale=1.0);
	//backward
	Real cholesky_bw(Matrix &, core::Size, Vector &, Vector &, core::Size from=1, core::Size to=0, Real scale=1.0);

	void pivot_range_randomly(Pose &, core::Size, core::Size);

protected:
	core::Size n_end_atom_;
	core::Size n_dof_angle_;
	core::Size n_pert_res_;
	core::Size resnum_;
	/// @note mark the segment to be perturb
	utility::vector1< std::pair<core::Size,Size> > available_seg_list_;
	/// @note <resi, atomname> 0 means the last res of the mobile segment
	utility::vector1< std::pair<core::Size, std::string> > end_atom_list_;
	/// @note {dr/dphi_i * dr/dphi_j} -- n_dof_angle^2
	Matrix matrix_G;
	/// @note perturbation from G
	Matrix matrix_A;
	/// @note {dr_i/dphi_j} -- n_dof_angle*n_end_atom
	VMatrix matrix_dRdPhi;

	// proposal density ratio
	Real last_proposal_density_ratio_;
	bool preserve_detailed_balance_;

	//movemap
	core::kinematics::MoveMapCOP movemap_;
	core::select::movemap::MoveMapFactoryCOP movemap_factory_;

	//move up
	Vector dphi_;
	Real factorA_;
	Real factorB_;

	//auto factorA
	core::Size N_auto_all;
	core::Size N_auto_small;

	//new features
	// logic: two scenarios
	// 1. given a list of pivot residues (by movemap), random selet continuous 4 res, fix 3 atoms of the last res
	// 2. given a list of pivot residues (by movemap), use all of them, fix atoms in end_atom_list
	bool use_all_pivot_res_;
	bool auto_adjust_factorA_; //P(lastP<0.6)<0.4
	bool fix_short_segment_;
	bool shrink_frag_ends_; //when use_all_pivot_res_=true and no kic, this should be true
};

/// @brief a particular gaussian mover from the original paper
/// @note using 8 torsion angles as DOF, 3 atoms (Ca,C,O) as end
class BBG8T3AMover : public BBGaussianMover
{
public:
	protocols::moves::MoverOP clone() const override;

public:
	BBG8T3AMover();
	~BBG8T3AMover() override;

	std::string get_name() const override;
};

}//namespace simple_moves
}//namespace protocols

#endif
