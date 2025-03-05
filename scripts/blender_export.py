import bpy
import os

def export_vertices(output_path):
    bpy.ops.object.mode_set(mode="OBJECT")
    mesh = bpy.context.object  # Assumes the active object is a mesh
    
    if mesh is None or mesh.type != 'MESH':
        print("Please select a mesh object with an armature modifier.")
        return
    
    uv_layer = mesh.data.uv_layers.active.data
    
    lines = []

    nVertices = len(mesh.data.vertices)
    
    for vertex in mesh.data.vertices:
        position = vertex.co
        normal = vertex.normal
        uv = uv_layer[vertex.index].uv
        lines.append(f"{position.x:.6f} {position.z:.6f} {position.y:.6f} {normal.x:.6f} {normal.z:.6f} {normal.y:.6f} {uv.x:.6f} {uv.y:.6f}")

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
    
    with open(output_path, "w") as out_file:
        out_file.write(f"nV {nVertices} nF {nFaces}\n" + "\n".join(lines))
        print(f"File {output_path} has been exported succesfully!")

output_path = os.path.join(bpy.path.abspath('//'), ".mdl")
export_vertices(output_path)