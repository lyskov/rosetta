// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file core/scoring/sasa/SasaMethod.hh
/// @brief Abstract class defining a 'SasaMethod'
/// @author Jared Adolf-Bryfogle (jadolfbr@gmail.com)

#ifndef INCLUDED_core_scoring_sasa_SASAMETHOD_HH
#define INCLUDED_core_scoring_sasa_SASAMETHOD_HH

#include <core/pose/Pose.fwd.hh>
#include <core/scoring/sasa/SasaMethod.fwd.hh>
#include <utility/VirtualBase.hh>

#include <core/id/AtomID_Map.fwd.hh>
#include <core/conformation/Residue.fwd.hh>

#include <string>

#include <core/types.hh> // AUTO IWYU For Real, Size

#ifdef    SERIALIZATION
// Cereal headers
#include <cereal/access.fwd.hpp>
#include <cereal/types/polymorphic.fwd.hpp>
#endif // SERIALIZATION

namespace core {
namespace scoring {
namespace sasa {

/// @brief The selection mode.  Allows selecting polar SASA only,
/// hydrophobic SASA only, polar SASA only, etc.
/// @details If you add to this list, update the SasaMethod::sasa_metric_name_from_mode() function!
/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
enum class
	SasaMethodHPMode {
	ALL_SASA = 1, //Keep first
	POLAR_SASA,
	HYDROPHOBIC_SASA,
	INVALID_MODE, //Keep second-to-last
	END_OF_LIST = INVALID_MODE //Keep last
};

/// @brief Type of Radii to use.
/// @details
///       LJ:  Refers to Leonard Jones radii - Rosetta uses radii at the minimum of the potential (sigma2).
///       Legacy:  Refers to radii optimized for a no longer in use term, but some protocols have been optimized to use it.
///       naccess:  Refers to radii used in the program naccess.  Originally derived from Chothia.  Do not use for all-atom SASA as hydrogens are implicitly included.
///                           'The Nature of the Accessible and Buried Surfaces in Proteins' J. Mol. Biol. (1976) 105, 1-14
///       reduce:   Radii used by the program reduce.  Hydrogens are explicitly included in the radii.
///
enum SasaRadii {
	LJ = 1,
	legacy,
	naccess,
	reduce,

	chothia=naccess,
	SasaRadii_total = reduce
};


/// @brief Abstract base class for SasaMethods.  Feel free to edit as needed.
///  Virt atms are skipped as radii=0
///
class SasaMethod : public utility::VirtualBase {


public:

	SasaMethod(Real probe_radius, SasaRadii radii_set);
	~SasaMethod() override;

	/// @brief Calculate Sasa.  Atoms not calculated have -1 sasa in AtomID_Map.  This is carried over for compatability purposes.
	virtual Real
	calculate(
		const pose::Pose & pose,
		const id::AtomID_Map<bool> & atom_subset,
		id::AtomID_Map< Real > & atom_sasa,
		utility::vector1< Real > & rsd_sasa) = 0;

	virtual std::string
	get_name() const = 0;


public:

	/// @brief Include the probe radius in calc.  Typical for SASA.
	void
	set_include_probe_radius_in_calc(bool include_probe_radius);

	/// @brief Set the probe radius.  Typical value is that of water at 1.4 A
	void
	set_probe_radius(Real probe_radius);

	/// @brief Set the radii type.
	void
	set_radii_set(SasaRadii radii_set);

public:

	/// @brief Given the name of the SasaMethodHPMode, get the mode.
	/// @returns SasaMethodHPMode::INVALID_MODE if the string can't be parsed; the correct
	/// SasaMethodHPMode if it can.
	/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
	static SasaMethodHPMode sasa_metric_mode_from_name( std::string const &mode_name );

	/// @brief Given the SasaMethodHPMode, get the name.
	/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
	static std::string sasa_metric_name_from_mode( SasaMethodHPMode const mode );

	/// @brief Construct a comma-separeted string listing all of the sasa metric modes.
	/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
	static std::string list_sasa_method_hp_modes();

	/// @brief Set whether we're counting all SASA (default), polar SASA, or hydrophobic SASA.
	/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
	void set_sasa_method_hp_mode( SasaMethodHPMode const mode_in );

	/// @brief Get whether we're counting all SASA (default), polar SASA, or hydrophobic SASA.
	/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
	inline SasaMethodHPMode sasa_method_hp_mode() const { return sasa_method_hp_mode_; }


	///////////Legacy Options ///////////
public:
	//void
	//set_expand_polar_radii(bool expand_polars, core::Size expansion_radius = 1.0);

	/// @brief Legacy option to increase polar hydrogen radii to 1.08A.  Supported for now.
	void
	set_use_big_polar_hydrogen(bool big_polar_h);

public:

	/// @brief Given a residue, an atom index, and a SasaMethodHPMode, determine whether the atom is one to skip (returns true)
	/// or count (returns false).
	/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
	static bool
	skip_atom( core::conformation::Residue const & rsd, core::Size const atom_index, SasaMethodHPMode const hp_mode );

	/// @brief Classify an atom on a residue as "polar" for the purposes of SASA
	static bool
	is_polar_atom( core::conformation::Residue const & rsd, core::Size const atom_index );

protected:
	Real probe_radius_;
	SasaRadii radii_set_;

	bool include_probe_radius_;
	bool use_big_polar_H_;

private:

	/// @brief Are we counting all SASA (default), polar SASA, or hydrophobic SASA?
	/// @author Vikram K. Mulligan (vmulligan@flatironinstitute.org).
	SasaMethodHPMode sasa_method_hp_mode_ = SasaMethodHPMode::ALL_SASA;

#ifdef    SERIALIZATION
protected:
	friend class cereal::access;
	SasaMethod();

public:
	template< class Archive > void save( Archive & arc ) const;
	template< class Archive > void load( Archive & arc );
#endif // SERIALIZATION

};


}
}
}

#ifdef    SERIALIZATION
CEREAL_FORCE_DYNAMIC_INIT( core_scoring_sasa_SasaMethod )
#endif // SERIALIZATION


#endif //#ifndef INCLUDED_protocols/antibody_design_SASAMETHOD_HH
