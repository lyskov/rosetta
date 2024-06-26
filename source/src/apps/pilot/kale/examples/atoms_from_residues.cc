#include <devel/init.hh>
#include <basic/Tracer.hh>

#include <core/pose/Pose.hh>
#include <core/conformation/Residue.hh>
#include <core/import_pose/import_pose.hh>

static basic::Tracer TR( "apps.pilot.kale.examples" );

using namespace core;

int main(int argc, char **argv) {

	devel::init(argc, argv);

	pose::Pose pose;
	import_pose::pose_from_file(pose, "structures/marked_loop.8.pdb", core::import_pose::PDB_file);

	for ( Size i = 1; i <= pose.size(); i++ ) {
		conformation::Residue residue = pose.residue(i);

		for ( Size j = 1; j <= residue.natoms(); j++ ) {
			TR << "Atom ID: " << j << std::endl;
			residue.atom_type(j).print(TR);
			TR << std::endl;
		}
	}

}
