def parse_args():
    from argparse import ArgumentParser
    parser = ArgumentParser(description="Visualize Plane XZ of Graph")
    parser.add_argument('graph', help='Input Graph data file')
    parser.add_argument('--output', help='Output Image file')
    parser.add_argument('--plane-y', type=int)
    return parser.parse_args()

def run(finput):
    import string, numpy as np
    global ARGS
    level = int(finput.readline())
    width = long(finput.readline())
    voxel_h = float(finput.readline())
    length = width * width * width
    canvas = np.zeros((width, width, width, 3), dtype=np.float32)
    for i in range(0,length):
        buf = string.split(finput.readline())
        x, y, z = map(int, string.split(buf[0][1:-1], ','))
        frgnd = bool(int(buf[1]))
    while True:
        buf = finput.readline()
        if not buf:
            break
        buf = string.split(buf)
        x, y, z = map(int, string.split(buf[0][1:-1], ','))
        rho = float(buf[1])
        if buf[2] == '+x':
            canvas[y,z,x,0] = rho
        elif buf[2] == '+y':
            canvas[y,z,x,1] = rho
        elif buf[2] == '+z':
            canvas[y,z,x,2] = rho
    return canvas

def mainfunc():
    global ARGS
    ARGS = parse_args()
    with open(ARGS.graph, "r") as finput:
        image = run(finput)
    import numpy as np, matplotlib.pyplot as plt
    #image = np.minimum(np.maximum(image, 0.0), 1.0)
    image = np.multiply(image, 255, dtype=np.uint8)
    if ARGS.plane_y:
        image = image[ARGS.plane_y]
        if ARGS.output:
            plt.imsave(ARGS.output, image)
        else:
            plt.axis([0,image.shape[1],0,image.shape[0]])
            plt.imshow(image, interpolation='nearest')
            plt.show()
    else:
        while True:
            y = int(raw_input("y = "))
            img = image[y]
            plt.axis([0,img.shape[1],0,img.shape[0]])
            plt.suptitle("Voxel Y = %d" % y)
            plt.xlabel("Voxel X")
            plt.ylabel("Voxel Z")
            plt.imshow(img, interpolation='nearest')
            plt.show()

if __name__ == "__main__":
    mainfunc()
