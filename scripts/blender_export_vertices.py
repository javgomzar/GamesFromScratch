from dataclasses import dataclass, field
import bpy # type: ignore
from mathutils import Vector # type: ignore
import os


@dataclass
class Bone:
    id: int
    name: str
    head: Vector
    tail: Vector

@dataclass
class Vertex:
    id: int
    position: Vector
    normal: Vector
    bone_ids: list[int] = field(init=False, default_factory=list)
    bone_weights: list[float] = field(init=False, default_factory=list)

@dataclass
class Loop:
    id: int
    ids: list[int] = field(init=False, default_factory=list)
    vertex: Vertex
    vertex_index: int
    uv: Vector
    order: int = -1

def collect_bones(mesh) -> dict[str, Bone]:
    result = {}
    exclude = ['Knee', 'Wrist', 'Elbow', 'Heel']
    id = 0
    for bone_name, bone in mesh.parent.data.bones.items():
        if bone_name.split("_")[0] not in exclude:
            result[bone_name] = Bone(id, bone_name, bone.head_local, bone.tail_local)
            id += 1
    return result

def consolidate_loops(loops: list[Loop]) -> list[Loop]:
    result = []
    for loop in loops:
        for existing_loop in result:
            if (loop.vertex_index == existing_loop.vertex_index and 
                round(loop.uv.x, 6) == round(existing_loop.uv.x, 6) and 
                round(loop.uv.y, 6) == round(existing_loop.uv.y, 6)):
                existing_loop.ids.append(loop.id)
                break
        else:
            result.append(loop)
    result.sort(key=lambda x: x.id)
    return result


def export_vertices(output_path):
    bpy.ops.object.mode_set(mode="OBJECT")
    mesh = bpy.context.object  # Assumes the active object is a mesh
    
    if mesh is None or mesh.type != 'MESH':
        raise Exception("Please select a mesh object with an armature modifier.")
    
    uv_layer = mesh.data.uv_layers.active.data
    
    lines = []

    # Collecting bone information
    bones = {}
    if mesh.parent and mesh.parent.type == 'ARMATURE':
        bones = collect_bones(mesh)

    nBones = len(bones)
    
    # Vertex loop
    vertices = []
    for vertex in mesh.data.vertices:
        position = vertex.co
        normal = vertex.normal

        vertex_obj = Vertex(vertex.index, position, normal)

        if nBones > 0:
            vertex_weights = []
            for group in vertex.groups:
                group_name = mesh.vertex_groups[group.group].name
                if group.weight > 0.01 and group_name in bones:
                    vertex_weights.append((group_name, group.weight))
            (id0, w0), (id1, w1) = (-1, 0.0), (-1, 0.0)
            if len(vertex_weights) > 0:
                if len(vertex_weights) == 1:
                    bone0, w0 = vertex_weights[0]
                    id0 = bones[bone0].id
                    w0 = 1.0
                else:
                    vertex_weights.sort(key=lambda x: x[1], reverse=True)
                    (bone0, w0), (bone1, w1) = vertex_weights[:2]
                    id0, id1 = bones[bone0].id, bones[bone1].id
                    w1 = 1.0 - w0
            vertex_obj.bone_ids.extend([id0, id1])
            vertex_obj.bone_weights.extend([w0, w1])

        vertices.append(vertex_obj)

    loops = []
    for loop in mesh.data.loops:
        loop_obj = Loop(loop.index, vertices[loop.vertex_index], loop.vertex_index, uv_layer[loop.index].uv)
        loops.append(loop_obj)
    loops = consolidate_loops(loops)
    i = 0
    for loop in loops:
        loop.order = i
        i += 1
        position = loop.vertex.position
        normal = loop.vertex.normal
        line = f"{position.x:.6f} {position.z:.6f} {position.y:.6f} {normal.x:.6f} {normal.z:.6f} {normal.y:.6f} {loop.uv.x:.6f} {loop.uv.y:.6f}"
        if nBones > 0:
            line += f" {loop.vertex.bone_ids[0]} {loop.vertex.bone_ids[1]} {loop.vertex.bone_weights[0]:.6f} {loop.vertex.bone_weights[1]:.6f}"
        lines.append(line)

    nVertices = len(loops)

    # Faces loop
    nFaces = 0
    faces = []
    for face in mesh.data.polygons:
        raw_face_loops = list(face.loop_indices)
        face_loops = []
        for raw_loop in raw_face_loops:
            for loop in loops:
                if loop.id == raw_loop or raw_loop in loop.ids:
                    face_loops.append(loop.order)
                    break
            else:
                raise Exception(f"Loop {raw_loop} not found")
        n_vertices = len(face_loops)
        if n_vertices == 3:
            lines.append(f"{face_loops[0]} {face_loops[1]} {face_loops[2]}")
            nFaces += 1
        elif n_vertices > 3:
            faces = [f"{face_loops[0]} {face_loops[i]} {face_loops[i+1]}" for i in range(1, n_vertices-1)]
            nFaces += len(faces)
            lines.append("\n".join(faces))

    # Bones loop
    if nBones > 0:
        for bone in bones.values():
            lines.append(f"{bone.id} {bone.name} {bone.head.x:.6f} {bone.head.z:.6f} {bone.head.y:.6f} {bone.tail.x:.6f} {bone.tail.z:.6f} {bone.tail.y:.6f}")
    
    # Exporting file
    with open(output_path, "w") as out_file:
        header = f"nV {nVertices} nF {nFaces}"
        if nBones > 0:
            header += f" nB {nBones}"
        header += "\n"
        out_file.write(header + "\n".join(lines))
        print(f"File {output_path} has been exported succesfully!")

output_path = os.path.join(bpy.path.abspath('//'), ".mdl")
export_vertices(output_path)