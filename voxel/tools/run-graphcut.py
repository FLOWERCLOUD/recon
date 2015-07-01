def parse_args():
    from argparse import ArgumentParser
    parser = ArgumentParser(description="Perform Graph Cut")
    parser.add_argument('data', help='Input Graph data file')
    parser.add_argument('--lb', type=float, default=0.5)
    parser.add_argument('--mju', type=float, default=12.75)
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

def mainfunc():
    global ARGS
    ARGS = parse_args()

    with open(ARGS.data, "r") as finput:
        voxels, width = run(finput)
    #print(voxels)
    #print(len(voxels))

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

if __name__ == '__main__':
    mainfunc()

# python2 ../voxel/tools/run-graphcut.py --lb 0.5 --mju 2.25 graph-lv5.txt
