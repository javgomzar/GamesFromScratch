import bpy # type: ignore
import os

def export_vertices(output_path, export_armature_weights = False):
    bpy.ops.object.mode_set(mode="OBJECT")
    mesh = bpy.context.object  # Assumes the active object is a mesh
    
    if mesh is None or mesh.type != 'MESH':
        print("Please select a mesh object with an armature modifier.")
        return
    
    uv_layer = mesh.data.uv_layers.active.data
    
    lines = []

    nVertices = len(mesh.data.vertices)

    # Collecting bone information
    bones = {}
    exclude = ['Knee', 'Wrist', 'Elbow', 'Heel']
    if export_armature_weights:
        if mesh.parent.type == 'ARMATURE':
            i = 0
            for bone_name, bone in mesh.parent.data.bones.items():
                if bone_name.split(" ")[0] not in exclude:
                    bones[bone_name] = {
                        "id": i,
                        "name": bone_name,
                        "parent": bone.parent.name if bone.parent else None,
                        "head": bone.head,
                        "tail": bone.tail
                    }
                    i += 1

    nBones = len(bones)
    
    # Vertex loop
    for vertex in mesh.data.vertices:
        position = vertex.co
        normal = vertex.normal
        uv = uv_layer[vertex.index].uv

        line = f"{position.x:.6f} {position.z:.6f} {position.y:.6f} {normal.x:.6f} {normal.z:.6f} {normal.y:.6f} {uv.x:.6f} {uv.y:.6f}"

        if export_armature_weights:
            vertex_weights = []
            for group in vertex.groups:
                group_name = mesh.vertex_groups[group.group].name
                if group.weight > 0.01 and group_name.split(" ")[0] not in exclude:
                    vertex_weights.append((group_name, group.weight))
            (id0, w0), (id1, w1) = (-1, 0.0), (-1, 0.0)
            if len(vertex_weights) > 0:
                if len(vertex_weights) == 1:
                    bone0, w0 = vertex_weights[0]
                    id0 = bones[bone0]['id']
                else:
                    vertex_weights.sort(key=lambda x: x[1], reverse=True)
                    (bone0, w0), (bone1, w1) = vertex_weights[:2]
                    id0, id1 = bones[bone0]['id'], bones[bone1]['id']
            line += f" {id0} {id1} {w0} {w1}"

        lines.append(line)

    # Faces loop
    nFaces = 0

    for face in mesh.data.polygons:
        face_vertices = list(face.vertices)
        n_vertices = len(face_vertices)
        if n_vertices == 3:
            lines.append(f"{face_vertices[0]} {face_vertices[1]} {face_vertices[2]}")
            nFaces += 1
        elif n_vertices > 3:
            faces = [f"{face_vertices[0]} {face_vertices[i]} {face_vertices[i+1]}" for i in range(1, n_vertices-1)]
            nFaces += len(faces)
            lines.append("\n".join(faces))
    
    # Exporting file
    with open(output_path, "w") as out_file:
        header = f"nV {nVertices} nF {nFaces}"
        if export_armature_weights:
            header += f" nB {nBones}"
        header += "\n"
        out_file.write(header + "\n".join(lines))
        print(f"File {output_path} has been exported succesfully!")

output_path = os.path.join(bpy.path.abspath('//'), ".mdl")
export_vertices(output_path, True)