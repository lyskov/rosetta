// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the Rosetta Commons.
// (c) For more information, see http://www.rosettacommons.org. Questions about this can be
// (c) addressed to University of Washington CoMotion, email: license@uw.edu.
///
/// @file   protocols/cyclic_peptide/CreateTorsionConstraint.cc
/// @brief  Add torsion constraints to the current pose conformation.
/// @author Vikram K. Mulligan (vmullig@uw.edu)
/// @author modified by Parisa Hosseinzadeh (parisah@uw.edu)

#include <protocols/cyclic_peptide/CreateTorsionConstraint.hh>
#include <protocols/cyclic_peptide/CreateTorsionConstraintCreator.hh>

#include <core/pose/Pose.hh>



#include <core/scoring/func/FuncFactory.hh>
#include <core/scoring/func/Func.hh>
#include <core/scoring/constraints/Constraint.fwd.hh>
#include <core/scoring/constraints/DihedralConstraint.hh>
#include <core/scoring/constraints/AmbiguousConstraint.hh>

#include <core/chemical/ResidueType.hh>


#include <utility/tag/Tag.hh>
#include <basic/Tracer.hh>
// XSD XRW Includes
#include <utility/tag/XMLSchemaGeneration.hh>
#include <protocols/moves/mover_schemas.hh>

static basic::Tracer TR( "protocols.cyclic_peptide.CreateTorsionConstraint" );

namespace protocols {
namespace cyclic_peptide {

CreateTorsionConstraint::CreateTorsionConstraint() = default;
CreateTorsionConstraint::~CreateTorsionConstraint()= default;

void CreateTorsionConstraint::set(
	utility::vector1<core::Size> const &res1,
	utility::vector1<std::string> const &atom1,
	utility::vector1<core::Size> const &res2,
	utility::vector1<std::string> const &atom2,
	utility::vector1<core::Size> const &res3,
	utility::vector1<std::string> const &atom3,
	utility::vector1<core::Size> const &res4,
	utility::vector1<std::string> const &atom4,
	utility::vector1<std::string> const &cst_func
)
{
	res1_=res1;
	atom1_=atom1;
	res2_=res2;
	atom2_=atom2;
	res3_=res3;
	atom3_=atom3;
	res4_=res4;
	atom4_=atom4;
	cst_func_=cst_func;
}

///////////////////////////////////////////////////////////////////
///////////////////        APPLY FUNCTION     /////////////////////
///////////////////////////////////////////////////////////////////
// the actual main apply.


void CreateTorsionConstraint::apply( core::pose::Pose & pose )
{
	core::scoring::constraints::ConstraintCOPs constraints;

	for ( core::Size i_cst=1; i_cst<=cst_func_.size(); ++i_cst ) {
		if ( cst_func_[i_cst] == "" ) {}
		else {
			if ( ambiguous_ ) {
				std::istringstream data(cst_func_[i_cst]);
				std::string func_type;
				data >> func_type;
				core::scoring::func::FuncFactory func_factory;
				core::scoring::func::FuncOP func = func_factory.new_func( func_type );
				func->read_data( data );
				core::Size atomno1 = pose.residue_type(res1_[i_cst]).atom_index(atom1_[i_cst]);
				core::Size atomno2 = pose.residue_type(res2_[i_cst]).atom_index(atom2_[i_cst]);
				core::Size atomno3 = pose.residue_type(res3_[i_cst]).atom_index(atom3_[i_cst]);
				core::Size atomno4 = pose.residue_type(res4_[i_cst]).atom_index(atom4_[i_cst]);
				constraints.push_back(core::scoring::constraints::ConstraintCOP( utility::pointer::make_shared< core::scoring::constraints::DihedralConstraint >(core::id::AtomID(atomno1,res1_[i_cst]),
					core::id::AtomID(atomno2,res2_[i_cst]),
					core::id::AtomID(atomno3,res3_[i_cst]),
					core::id::AtomID(atomno4,res4_[i_cst]),
					func ) )
				);
			} else {
				std::istringstream data(cst_func_[i_cst]);
				std::string func_type;
				data >> func_type;
				core::scoring::func::FuncFactory func_factory;
				core::scoring::func::FuncOP func = func_factory.new_func( func_type );
				func->read_data( data );
				core::Size atomno1 = pose.residue_type(res1_[i_cst]).atom_index(atom1_[i_cst]);
				core::Size atomno2 = pose.residue_type(res2_[i_cst]).atom_index(atom2_[i_cst]);
				core::Size atomno3 = pose.residue_type(res3_[i_cst]).atom_index(atom3_[i_cst]);
				core::Size atomno4 = pose.residue_type(res4_[i_cst]).atom_index(atom4_[i_cst]);
				pose.add_constraint( core::scoring::constraints::ConstraintCOP( utility::pointer::make_shared< core::scoring::constraints::DihedralConstraint >(core::id::AtomID(atomno1,res1_[i_cst]),
					core::id::AtomID(atomno2,res2_[i_cst]),
					core::id::AtomID(atomno3,res3_[i_cst]),
					core::id::AtomID(atomno4,res4_[i_cst]),
					func ) )
				);
			} //end if
		} //end if
	} //end for
	if ( ambiguous_ ) {
		pose.add_constraint( core::scoring::constraints::ConstraintCOP ( utility::pointer::make_shared< core::scoring::constraints::AmbiguousConstraint >(constraints) ) );
	}
}

/// @brief parse XML (specifically in the context of the parser/scripting scheme)
void
CreateTorsionConstraint::parse_my_tag(
	TagCOP tag,
	basic::datacache::DataMap &
)
{
	utility::vector1< utility::tag::TagCOP > const branch_tags( tag->getTags() );
	utility::vector1< utility::tag::TagCOP >::const_iterator tag_it;
	for ( tag_it = branch_tags.begin(); tag_it != branch_tags.end(); ++tag_it ) {
		if ( (*tag_it)->getName() == "Add" ) {
			res1_.push_back( (*tag_it)->getOption< core::Size >( "res1" ) );
			atom1_.push_back( (*tag_it)->getOption< std::string >( "atom1" ) );
			res2_.push_back( (*tag_it)->getOption< core::Size >( "res2" ) );
			atom2_.push_back( (*tag_it)->getOption< std::string >( "atom2" ) );
			res3_.push_back( (*tag_it)->getOption< core::Size >( "res3" ) );
			atom3_.push_back( (*tag_it)->getOption< std::string >( "atom3" ) );
			res4_.push_back( (*tag_it)->getOption< core::Size >( "res4" ) );
			atom4_.push_back( (*tag_it)->getOption< std::string >( "atom4" ) );

			cst_func_.push_back( (*tag_it)->getOption< std::string >( "cst_func", "" ) );
		}
	}
}

moves::MoverOP CreateTorsionConstraint::clone() const { return utility::pointer::make_shared< CreateTorsionConstraint >( *this ); }
moves::MoverOP CreateTorsionConstraint::fresh_instance() const { return utility::pointer::make_shared< CreateTorsionConstraint >(); }





std::string CreateTorsionConstraint::get_name() const {
	return mover_name();
}

std::string CreateTorsionConstraint::mover_name() {
	return "CreateTorsionConstraint";
}

void CreateTorsionConstraint::provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd )
{
	using namespace utility::tag;
	AttributeList attlist;
	AttributeList subelement_attlist;
	subelement_attlist
		+ XMLSchemaAttribute::required_attribute( "res1", xsct_non_negative_integer, "Residue containing first atom" )
		+ XMLSchemaAttribute::required_attribute( "atom1", xs_string, "Name of first atom in torsion" )
		+ XMLSchemaAttribute::required_attribute( "res2", xsct_non_negative_integer, "Residue containing second atom" )
		+ XMLSchemaAttribute::required_attribute( "atom2", xs_string, "Name of second atom in torsion" )
		+ XMLSchemaAttribute::required_attribute( "res3", xsct_non_negative_integer, "Residue containing third atom" )
		+ XMLSchemaAttribute::required_attribute( "atom3", xs_string, "Name of third atom in torsion" )
		+ XMLSchemaAttribute::required_attribute( "res4", xsct_non_negative_integer, "Residue containing fourth atom" )
		+ XMLSchemaAttribute::required_attribute( "atom4", xs_string, "Name of fourth atom in torsion" )
		+ XMLSchemaAttribute::required_attribute( "cst_func", xs_string, "Function to use for this torsion constraint" );
	XMLSchemaSimpleSubelementList subelements;
	subelements.add_simple_subelement( "Add", subelement_attlist, "Adds a torsion constraint for the four specified atoms" );

	protocols::moves::xsd_type_definition_w_attributes_and_repeatable_subelements( xsd, mover_name(), "Adds torsion constraints to a pose", attlist, subelements );

}

std::string CreateTorsionConstraintCreator::keyname() const {
	return CreateTorsionConstraint::mover_name();
}

protocols::moves::MoverOP
CreateTorsionConstraintCreator::create_mover() const {
	return utility::pointer::make_shared< CreateTorsionConstraint >();
}

void CreateTorsionConstraintCreator::provide_xml_schema( utility::tag::XMLSchemaDefinition & xsd ) const
{
	CreateTorsionConstraint::provide_xml_schema( xsd );
}


} // moves
} // protocols
