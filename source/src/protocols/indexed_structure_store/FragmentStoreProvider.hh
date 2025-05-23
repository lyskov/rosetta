// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.

/// @file ./src/protocols/indexed_structure_store/FragmentStoreProvider.hh
/// @brief Abstract fragment store backing interface.
/// @author Brian D. Weitzner (bweitzner@lyell.com)
//

#ifndef INCLUDED_protocols_indexed_structure_store_FragmentStoreProvider_HH
#define INCLUDED_protocols_indexed_structure_store_FragmentStoreProvider_HH

// Utility Headers

#include <utility/VirtualBase.hh>

#include <protocols/indexed_structure_store/FragmentStoreProvider.fwd.hh>
#include <protocols/indexed_structure_store/FragmentStore.fwd.hh>

#include <string>

namespace protocols
{
namespace indexed_structure_store
{

class FragmentStoreProvider : public utility::VirtualBase
{
public:

	// @brief Retrieves information from the backing store.
	virtual FragmentStoreOP get_fragment_store(std::string store_name) = 0;

	// @brief Append information to the backing store.
	virtual void append_to_fragment_store(FragmentStoreOP fragment_store, std::string store_name, std::string group_field, std::string group_type) = 0;

	virtual void set_target_filename(std::string target_filename) = 0;

};

}
}

#endif
