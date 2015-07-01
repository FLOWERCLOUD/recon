def parse_args():
    from argparse import ArgumentParser
    parser = ArgumentParser(description="Perform Graph Cut")
    parser.add_argument('data', help='Input Graph data file')
    parser.add_argument('output', help='Output PLY file')
    parser.add_argument('--lb', type=float, default=0.5)
    parser.add_argument('--mju', type=float, default=2.0)
    return parser.parse_args()

# Current Graph-Cut algorithm
# https://github.com/pmneila/PyMaxflow

# Other Graph-Cut algorithms
# http://graph-tool.skewed.de/

def run(finput):
    import string, re, maxflow, numpy as np
    global ARGS

    # Prepare info
    level = int(finput.readline())
    width = long(finput.readline())
    voxel_h = float(finput.readline())
    length = width * width * width
    Wb = ARGS.lb * voxel_h * voxel_h * voxel_h
    Wij_prefix = 4.0 / 3.0 * np.pi * voxel_h * voxel_h

    # Setup graph
    graph = maxflow.Graph[float](length, length*3)
    nodes = graph.add_grid_nodes((width,width,width))
    for i in range(0,length):
        buf = string.split(finput.readline())
        x, y, z = map(int, string.split(buf[0][1:-1], ','))
        if (int(buf[1])):
            graph.add_tedge(nodes[x,y,z], Wb, 0.0)
        else:
            graph.add_tedge(nodes[x,y,z], 0.0, float('inf'))
    while True:
        buf = finput.readline()
        if not buf:
            break
        buf = string.split(buf)
        x, y, z = map(int, string.split(buf[0][1:-1], ','))
        n1 = nodes[x,y,z]
        w = Wij_prefix * np.exp(-ARGS.mju * float(buf[1]))
        if buf[2] == '+x' and x < width-1:
            n2 = nodes[x+1,y,z]
        elif buf[2] == '+y' and y < width-1:
            n2 = nodes[x,y+1,z]
        elif buf[2] == '+z' and z < width-1:
            n2 = nodes[x,y,z+1]
        else:
            continue
        graph.add_edge(n1, n2, w, w)

    # Run optimization
    flow = graph.maxflow()

    # Extract voxels
    #result = np.zeros((width,width,width), dtype=np.bool)
    result = []
    for index in np.ndindex(width,width,width):
        if graph.get_segment(nodes[index]) == 0:
            result.append(index)
            #result[index] = True
    return (result, width)

def visualize(voxels, width):
    import vtk, numpy as np
    points = vtk.vtkPoints()
    vertices = vtk.vtkCellArray()
    for index in voxels:
        pos = np.divide(np.array(index, dtype=np.float32), float(width), dtype=np.float32)
        pid = points.InsertNextPoint(pos)
        vertices.InsertNextCell(1)
        vertices.InsertCellPoint(pid)
    psrc = vtk.vtkPolyData()
    psrc.SetPoints(points)
    psrc.SetVerts(vertices)

    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputData(psrc)
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)

    renderer = vtk.vtkRenderer()
    window = vtk.vtkRenderWindow()
    window.AddRenderer(renderer)
    winter = vtk.vtkRenderWindowInteractor()
    winter.SetRenderWindow(window)

    renderer.AddActor(actor)
    renderer.SetBackground(.3, .6, .3)

    window.Render()
    winter.Start()

def save_ply(path, voxels, width):
    from plyfile import PlyData, PlyElement
    import numpy as np
    n = len(voxels)
    cubev = np.empty(n * 8, dtype=[('x','f4'),('y','f4'),('z','f4')])
    cubef = np.empty(n * 12, dtype=[('vertex_indices','i4', (3,))])
    vi = 0
    fi = 0
    off1 = np.divide((1,0,0), float(width), dtype=np.float32)
    off2 = np.divide((0,1,0), float(width), dtype=np.float32)
    off3 = np.divide((1,1,0), float(width), dtype=np.float32)
    off4 = np.divide((0,0,1), float(width), dtype=np.float32)
    off5 = np.divide((1,0,1), float(width), dtype=np.float32)
    off6 = np.divide((0,1,1), float(width), dtype=np.float32)
    off7 = np.divide((1,1,1), float(width), dtype=np.float32)
    for v in voxels:
        pos = np.divide(v, float(width), dtype=np.float32)
        cubev[vi+0] = pos
        cubev[vi+1] = np.add(pos, off1)
        cubev[vi+2] = np.add(pos, off2)
        cubev[vi+3] = np.add(pos, off3)
        cubev[vi+4] = np.add(pos, off4)
        cubev[vi+5] = np.add(pos, off5)
        cubev[vi+6] = np.add(pos, off6)
        cubev[vi+7] = np.add(pos, off7)
        cubef[fi+0] = ((vi+0, vi+2, vi+1),)
        cubef[fi+1] = ((vi+1, vi+2, vi+3),)
        cubef[fi+2] = ((vi+0, vi+6, vi+2),)
        cubef[fi+3] = ((vi+0, vi+4, vi+6),)
        cubef[fi+4] = ((vi+0, vi+5, vi+4),)
        cubef[fi+5] = ((vi+0, vi+1, vi+5),)
        cubef[fi+6] = ((vi+1, vi+3, vi+5),)
        cubef[fi+7] = ((vi+3, vi+7, vi+5),)
        cubef[fi+8] = ((vi+3, vi+2, vi+6),)
        cubef[fi+9] = ((vi+3, vi+6, vi+7),)
        cubef[fi+10] = ((vi+4, vi+5, vi+7),)
        cubef[fi+11] = ((vi+4, vi+7, vi+6),)
        vi += 8
        fi += 12
    vert_elem = PlyElement.describe(cubev, 'vertex')
    face_elem = PlyElement.describe(cubef, 'face')
    PlyData([vert_elem, face_elem]).write(path)

def mainfunc():
    global ARGS
    ARGS = parse_args()

    with open(ARGS.data, "r") as finput:
        voxels, width = run(finput)
    #print(voxels)
    #print(len(voxels))
    #visualize(voxels, width)
    save_ply(ARGS.output, voxels, width)

if __name__ == '__main__':
    mainfunc()

# python2 ../voxel/tools/run-graphcut.py --lb 0.5 --mju 2.25 graph-lv5.txt
