// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file   core/select/jump_selector/JumpSelector.hh
/// @brief  The JumpSelector class identifies a subset of jumps from a Pose
/// @author Andrew Leaver-Fay (aleaverfay@gmail.com)

#ifndef INCLUDED_core_select_jump_selector_JumpSelector_HH
#define INCLUDED_core_select_jump_selector_JumpSelector_HH

// Unit headers
#include <core/select/jump_selector/JumpSelector.fwd.hh>

// Package headers
#include <core/pose/Pose.fwd.hh>
#include <core/types.hh>

// Basic headers
#include <basic/datacache/DataMap.fwd.hh>

// Utility Headers
#include <utility/VirtualBase.hh>
#include <utility/tag/Tag.fwd.hh>

// C++ headers
#include <string>

namespace core {
namespace select {
namespace jump_selector {

class JumpSelector : public utility::VirtualBase {
public:

	/// @brief Constructor.
	///
	JumpSelector();

	/// @brief Destructor.
	///
	~JumpSelector() override;

	/// @brief Clone operator.
	/// @details All JumpSelectors must implement a clone() operator.  This must create a copy of the object and
	/// return a JumpSelectorOP to the newly-created object.
	virtual JumpSelectorOP clone() const = 0;

	/// @brief Return a JumpSubset indicating a selection of Jumps from the input
	/// Pose; the JumpSubset is an array of booleans of length pose.num_jump()
	/// where a value of "true" for position i indicates that jump i is a part of
	/// the selected subset -- and a value of "false" would indicate that it is not.
	virtual
	JumpSubset
	apply(
		core::pose::Pose const & pose
	) const = 0;

	/// @brief Initialize any data members of this instance from an input tag
	/// and a DataMap object
	virtual void parse_my_tag(
		utility::tag::TagCOP tag,
		basic::datacache::DataMap & datacache
	);

	virtual
	std::string
	get_name() const = 0;

	/// @brief Calls apply and returns a vector of the selected jump ids
	utility::vector1< core::Size >
	selection_jumps( core::pose::Pose const & pose ) const;

};


} //namespace jump_selector
} //namespace select
} //namespace core


#endif
