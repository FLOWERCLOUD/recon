def parse_args():
    from argparse import ArgumentParser
    parser = ArgumentParser(description="Visualize XZ Plane of Graph")
    parser.add_argument('graph', help='Input Graph data file')
    parser.add_argument('--plane-y', type=int)
    return parser.parse_args()

def load_graph(path):
    import string, numpy as np
    with open(path, "r") as finput:
        level = int(finput.readline())
        width = long(finput.readline())
        voxel_h = float(finput.readline())
        finput.readline() # voxel minpos
        finput.readline() # voxel maxpos
        canvas = np.zeros((width, width, width, 3), dtype=np.float32)
        length = width * width * width
        for i in range(0,length):
            buf = string.split(finput.readline())
            x, y, z, frgnd = map(int, buf)
            frgnd = bool(frgnd)
        while True:
            buf = finput.readline()
            if not buf:
                break
            buf = string.split(buf)
            x, y, z, rho = map(float, buf[0:4])
            if buf[-1] == '+x':
                canvas[y,z,x,0] = rho
            elif buf[-1] == '+y':
                canvas[y,z,x,1] = rho
            elif buf[-1] == '+z':
                canvas[y,z,x,2] = rho
    return canvas

def mainfunc():
    global ARGS
    ARGS = parse_args()
    image = load_graph(ARGS.graph)
    import numpy as np
    #image = np.multiply(image, 255, dtype=np.uint8)
    def plot(y):
        import matplotlib.pyplot as plt
        from mpl_toolkits.mplot3d import Axes3D
        img = image[y]
        plt.figure()
        plt.axis([0,img.shape[1],0,img.shape[0]])
        plt.suptitle("Voxel Y = %d" % y)
        plt.xlabel("Voxel X")
        plt.ylabel("Voxel Z")
        plt.imshow(img, interpolation='nearest')

        X = np.arange(0, img.shape[1], 1)
        Y = np.arange(0, img.shape[0], 1)
        X, Y = np.meshgrid(X, Y)

        #fig = plt.figure()
        #plt.suptitle("+x edge")
        #ax = fig.add_subplot(111, projection='3d')
        #ax.plot_surface(X, Y, img[:,:,0], rstride=1, cstride=1)

        #fig = plt.figure()
        #plt.suptitle("+y edge")
        #ax = fig.add_subplot(111, projection='3d')
        #ax.plot_surface(X, Y, img[:,:,1], rstride=1, cstride=1)

        #fig = plt.figure()
        #plt.suptitle("+z edge")
        #ax = fig.add_subplot(111, projection='3d')
        #ax.plot_surface(X, Y, img[:,:,2], rstride=1, cstride=1)

        fig = plt.figure()
        plt.suptitle("magtitude")
        ax = fig.add_subplot(111, projection='3d')
        ax.plot_surface(X, Y, np.sqrt(np.square(img[:,:,0]) + np.square(img[:,:,1]) + np.square(img[:,:,2])), rstride=1, cstride=1)

        plt.show()

    if ARGS.plane_y:
        plot(ARGS.plane_y)
    else:
        while True:
            y = int(raw_input("y = "))
            plot(y)



if __name__ == "__main__":
    mainfunc()
