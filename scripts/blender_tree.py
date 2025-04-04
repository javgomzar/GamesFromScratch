import bpy # type: ignore
import bmesh # type: ignore
from mathutils import Vector # type: ignore
import numpy as np
import os


nodes_path = os.path.join(bpy.path.abspath('//'), "tree_nodes.npy")
parents_path = os.path.join(bpy.path.abspath('//'), "tree_parents.npy")
print("NumPy arrays loaded")

# Load edges from numpy array
nodes = np.load(nodes_path)
parents = np.load(parents_path)
n_nodes = len(nodes)

# Compute radius of branch at each vertex
ns_children = np.zeros(n_nodes, dtype=np.int64)
for i in range(0, n_nodes):
    n_children = len(parents[parents == i])
    ns_children[i] = n_children

radii = np.zeros(n_nodes, dtype=np.float64)
tips = (ns_children == 0).nonzero()[0]
is_merge_point = ns_children > 1
merge_points = (is_merge_point).nonzero()[0]
initial_r = 0.1
n = 2.5
for tip in tips:
    radii[tip] = initial_r
    parent = parents[tip]
    while not is_merge_point[parent]:
        radii[parent] = initial_r
        parent = parents[parent]

remaining = np.copy(merge_points)
while len(remaining) > 0:
    for merge_point in remaining:
        branches = (parents == merge_point).nonzero()[0]
        for branch in branches:
            if radii[branch] < initial_r:
                break
        else:
            radius = np.power(np.sum(np.power(radii[branches], n)), 1/n)
            radii[merge_point] = radius
            parent = parents[merge_point]
            while not is_merge_point[parent]:
                radii[parent] = radius
                parent = parents[parent]
                if parent == -1:
                    break
    remaining = (is_merge_point & (radii < initial_r)).nonzero()[0]

# Import edges into Blender
mesh = bpy.context.object.data  # Assumes the active object is a mesh

bm = bmesh.new()
bm.from_mesh(mesh)

root = bm.verts.new((nodes[0][0], nodes[0][1], nodes[0][2]))
verts = [root]
skin_layer = bm.verts.layers.skin.active
skin_data = root[skin_layer]
skin_data.radius = (radii[0], radii[0])
skin_data.use_root = True
for i in range(1, n_nodes):
    # Add vertex
    vert = nodes[i]
    vert = bm.verts.new((vert[0], vert[1], vert[2]))
    verts.append(vert)

    # Add edge
    parent = verts[parents[i]]
    bm.edges.new((vert, parent))

    # Adjust radius
    skin_data = vert[skin_layer]
    radius = radii[i]
    skin_data.radius = (radius, radius)

# Update mesh
bm.to_mesh(mesh)
bm.free()
