#include "re_cluster.h"
#include "re_cluster_util.h"
#include "initial_placement.h"
#include "cluster_placement.h"

bool move_mol_to_new_cluster(t_pack_molecule* molecule,
                             t_clustering_data& clustering_data,
                             bool during_packing) {
    auto& cluster_ctx = g_vpr_ctx.clustering();
    auto& helper_ctx = g_vpr_ctx.mutable_cl_helper();
    auto& device_ctx = g_vpr_ctx.device();
    auto& atom_ctx = g_vpr_ctx.mutable_atom();

    bool is_removed, is_created;
    ClusterBlockId old_clb;
    int molecule_size = get_array_size_of_molecule(molecule);

    //Backup the original pb of the molecule before the move
    std::vector<t_pb*> mol_pb_backup;
    for (int i_atom = 0; i_atom < molecule_size; i_atom++) {
        if (molecule->atom_block_ids[i_atom]) {
            t_pb* atom_pb_backup = new t_pb;
            atom_pb_backup->pb_deep_copy(atom_ctx.lookup.atom_pb(molecule->atom_block_ids[i_atom]));
            mol_pb_backup.push_back(atom_pb_backup);
        }
    }

    PartitionRegion temp_cluster_pr;
    t_lb_router_data* old_router_data = nullptr;
    t_lb_router_data* router_data = nullptr;

    //Check that there is a place for a new cluster of the same type
    old_clb = atom_to_cluster(molecule->atom_block_ids[molecule->root]);
    t_logical_block_type_ptr block_type = cluster_ctx.clb_nlist.block_type(old_clb);
    int block_mode = cluster_ctx.clb_nlist.block_pb(old_clb)->mode;

    unsigned int num_instances = 0;
    for (auto equivalent_tile : block_type->equivalent_tiles) {
        num_instances += device_ctx.grid.num_instances(equivalent_tile);
    }

    if (helper_ctx.num_used_type_instances[block_type] == num_instances) {
        VTR_LOG("The utilization of block_type %s is 100%. No space for new clusters\n", block_type->name);
        VTR_LOG("Atom %d move aborted\n", molecule->atom_block_ids[molecule->root]);
        return false;
    }

    //remove the molecule from its current cluster and check the cluster legality after
    is_removed = remove_mol_from_cluster(molecule, molecule_size, old_clb, old_router_data);

    if (!is_removed) {
        VTR_LOG("Atom: %zu move failed. Can't remove it from the old cluster\n", molecule->atom_block_ids[molecule->root]);
        return (is_removed);
    }

    //Create new cluster of the same type and mode.
    ClusterBlockId new_clb(helper_ctx.total_clb_num);
    is_created = start_new_cluster_for_mol(molecule,
                                           block_type,
                                           block_mode,
                                           helper_ctx.feasible_block_array_size,
                                           helper_ctx.enable_pin_feasibility_filter,
                                           new_clb,
                                           during_packing,
                                           clustering_data,
                                           &router_data,
                                           temp_cluster_pr);

    //Commit or revert the move
    if (is_created) {
        commit_mol_move(old_clb, new_clb, mol_pb_backup, old_router_data, clustering_data, during_packing, true);
        VTR_LOG("Atom:%zu is moved to a new cluster\n", molecule->atom_block_ids[molecule->root]);
    } else {
        int atom_idx = 0;
        for (int i_atom = 0; i_atom < molecule_size; i_atom++) {
            if (molecule->atom_block_ids[i_atom]) {
                atom_ctx.lookup.set_atom_clb(molecule->atom_block_ids[i_atom], old_clb);
                atom_ctx.lookup.set_atom_pb(molecule->atom_block_ids[i_atom], mol_pb_backup[atom_idx]);
                atom_idx++;
            }
        }
        VTR_LOG("Atom:%zu move failed. Can't start a new cluster of the same type and mode\n", molecule->atom_block_ids[molecule->root]);
    }

    //If the move is done after packing not during it, some fixes need to be done on the
    //clustered netlist
    if (is_created && !during_packing) {
        fix_clustered_netlist(molecule, molecule_size, old_clb, new_clb);
    }

    for (auto atom_pb_backup : mol_pb_backup) {
        cleanup_pb(atom_pb_backup);
        delete atom_pb_backup;
    }

    return (is_created);
}

bool move_mol_to_existing_cluster(t_pack_molecule* molecule,
                                  const ClusterBlockId& new_clb,
                                  bool during_packing,
                                  t_clustering_data& clustering_data) {
    //define required contexts
    auto& cluster_ctx = g_vpr_ctx.clustering();
    auto& atom_ctx = g_vpr_ctx.mutable_atom();

    //define local variables
    bool is_removed, is_added;
    AtomBlockId root_atom_id = molecule->atom_block_ids[molecule->root];
    int molecule_size = get_array_size_of_molecule(molecule);
    t_lb_router_data* old_router_data = nullptr;
    std::vector<AtomBlockId> new_clb_atoms = cluster_to_atoms(new_clb);

    //Check that the old and new clusters are the same type
    ClusterBlockId old_clb = atom_to_cluster(root_atom_id);
    if (cluster_ctx.clb_nlist.block_type(old_clb) != cluster_ctx.clb_nlist.block_type(new_clb)) {
        VTR_LOG("Atom:%zu move aborted. New and old cluster blocks are not of the same type",
                root_atom_id);
        return false;
    }

    //Check that the old and new clusters are the mode
    if (cluster_ctx.clb_nlist.block_pb(old_clb)->mode != cluster_ctx.clb_nlist.block_pb(new_clb)->mode) {
        VTR_LOG("Atom:%zu move aborted. New and old cluster blocks are not of the same mode",
                root_atom_id);
        return false;
    }

    //Backup the original pb of the atom before the move
    std::vector<t_pb*> mol_pb_backup;
    for (int i_atom = 0; i_atom < molecule_size; i_atom++) {
        if (molecule->atom_block_ids[i_atom]) {
            t_pb* atom_pb_backup = new t_pb;
            atom_pb_backup->pb_deep_copy(atom_ctx.lookup.atom_pb(molecule->atom_block_ids[i_atom]));
            mol_pb_backup.push_back(atom_pb_backup);
        }
    }

    //remove the molecule from its current cluster and check the cluster legality
    is_removed = remove_mol_from_cluster(molecule, molecule_size, old_clb, old_router_data);
    if (!is_removed) {
        VTR_LOG("Atom: %zu move failed. Can't remove it from the old cluster\n", root_atom_id);
        return false;
    }

    //Add the atom to the new cluster
    is_added = pack_mol_in_existing_cluster(molecule, new_clb, new_clb_atoms, during_packing, clustering_data);

    //Commit or revert the move
    if (is_added) {
        commit_mol_move(old_clb, new_clb, mol_pb_backup, old_router_data, clustering_data, during_packing, false);
        VTR_LOG("Atom:%zu is moved to a new cluster\n", molecule->atom_block_ids[molecule->root]);
    } else {
        int atom_idx = 0;
        for (int i_atom = 0; i_atom < molecule_size; i_atom++) {
            if (molecule->atom_block_ids[i_atom]) {
                atom_ctx.lookup.set_atom_clb(molecule->atom_block_ids[i_atom], old_clb);
                atom_ctx.lookup.set_atom_pb(molecule->atom_block_ids[i_atom], mol_pb_backup[atom_idx]);
                atom_idx++;
            }
        }
        VTR_LOG("Atom:%zu move failed. Can't start a new cluster of the same type and mode\n", molecule->atom_block_ids[molecule->root]);
    }

    //If the move is done after packing not during it, some fixes need to be done on the
    //clustered netlist
    if (is_added && !during_packing) {
        fix_clustered_netlist(molecule, molecule_size, old_clb, new_clb);
    }

    for (auto atom_pb_backup : mol_pb_backup) {
        cleanup_pb(atom_pb_backup);
        delete atom_pb_backup;
    }

    return (is_added);
}