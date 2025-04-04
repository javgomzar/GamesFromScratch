from matplotlib import pyplot as plt
from math import tau
import numpy as np


rng = np.random.default_rng()

def generate_cloud() -> np.ndarray:
    N = 2000
    R = 10
    phi = tau * rng.random(N)
    costheta = 2.0 * rng.random(N) - 1.0
    sintheta = np.sqrt(1.0 - np.pow(costheta, 2))
    r = R * np.cbrt(rng.random(N))
    return np.column_stack([r * sintheta * np.cos(phi), r * sintheta * np.sin(phi), r * costheta + 15.0])

D = 0.1
D_KILL = 30 * D
def iterate(in_nodes, cloud, parents) -> tuple:
    cloud_size = len(cloud)
    nodes_size = len(in_nodes)
    nearest_nodes = np.zeros(cloud_size, dtype=np.int64)
    directions = np.zeros((cloud_size, 3), dtype=np.float64)
    positions = in_nodes[:,:3]
    cloud_min_distances = np.zeros(cloud_size, dtype=np.float64)
    for i in range(0, cloud_size):
        attractor = cloud[i,:]
        d = attractor - positions
        distances = np.sqrt(d[:,0] * d[:,0] + d[:,1] * d[:,1] + d[:,2] * d[:,2])
        argmin = np.argmin(distances)
        nearest_nodes[i] = argmin
        distance = distances[argmin]
        directions[i] = d[argmin] / distance
        cloud_min_distances[i] = distance

    for i in range(1, nodes_size):
        attractors = directions[nearest_nodes == i]
        if len(attractors) > 0:
            direction = np.mean(attractors, axis=0)
            direction = direction / np.sqrt(direction[0] * direction[0] + direction[1] * direction[1] + direction[2] * direction[2])
            new_position = positions[i,:] + direction * D
            in_nodes = np.vstack([in_nodes, new_position])
            parents = np.append(parents, i)

    cloud = cloud[cloud_min_distances > D_KILL]

    return in_nodes, cloud, parents

def draw_tree(nodes, cloud, parents):
    fig = plt.figure()
    ax = fig.add_subplot(projection='3d')
    ax.scatter(cloud[:,0],cloud[:,1],cloud[:,2])
    for i in range(1, len(nodes)):
        node = nodes[i]
        parent = nodes[parents[i]]
        ax.plot([parent[0], node[0]], [parent[1], node[1]], [parent[2], node[2]], color='black')
    ax.axis('equal')
    plt.show()

if __name__ == "__main__":
    cloud = generate_cloud()
    nodes = np.stack([[0.0,0.0,0.0],[0.0,0.0,0.5]])
    parents = np.zeros(2, dtype=np.int64)
    parents[0] = -1

    nodes, cloud, parents = iterate(nodes, cloud, parents)
    for i in range(0,1000):
        nodes, cloud, parents = iterate(nodes, cloud, parents)
        # draw_tree(nodes, cloud, parents)
        cloud_size = len(cloud)
        if cloud_size == 0:
            break
        else:
            print(cloud_size)

    draw_tree(nodes, cloud, parents)
    
    # Save to file
    np.save("scripts/tree_nodes.npy", nodes)
    np.save("scripts/tree_parents.npy", parents)    
