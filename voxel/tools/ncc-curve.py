def parse_args():
    from argparse import ArgumentParser
    parser = ArgumentParser(description="Analysis NCC curve")
    parser.add_argument('data', help='Input JSON data file')
    parser.add_argument('--voxel-x', type=int)
    parser.add_argument('--voxel-y', type=int)
    parser.add_argument('--voxel-z', type=int)
    parser.add_argument('--cam-i', type=int)
    parser.add_argument('--cam-j', type=int)
    return parser.parse_args()

def correlation(wi, wj):
    import cv2, numpy as np
    wi = np.divide(wi, 255.0)
    wj = np.divide(wj, 255.0)
    vi = np.subtract(wi, np.mean(wi)).flatten()
    vj = np.subtract(wj, np.mean(wj)).flatten()
    return np.dot(normalize(vi), normalize(vj))
    #return cv2.matchTemplate(wj, wi, cv2.TM_CCORR_NORMED)[0,0]

def gen_ncc_func(image_i, image_j, tfm_i, tfm_j, voxel_pos, voxel_dir):
    import cv2, numpy as np
    vpos_i = cv2.perspectiveTransform(np.array([[voxel_pos]]), tfm_i)[0,0]
    wi = cv2.getRectSubPix(image_i, (11,11), tuple(vpos_i[0:2]))
    wi = cv2.cvtColor(wi, cv2.COLOR_BGR2GRAY)
    #cv2.imshow("wi", cv2.resize(wi, (220,220), interpolation=cv2.INTER_NEAREST))
    def ncc(d):
        vp = np.add(voxel_pos, np.multiply(voxel_dir, d))
        vpos_j = cv2.perspectiveTransform(np.array([[vp]]), tfm_j)[0,0]
        wj = cv2.getRectSubPix(image_j, (11,11), tuple(vpos_j[0:2]))
        wj = cv2.cvtColor(wj, cv2.COLOR_BGR2GRAY)
        #cv2.imshow("wj", cv2.resize(wj, (220,220), interpolation=cv2.INTER_NEAREST))
        value = correlation(wj, wi)
        #print("NCC = " + str(value))
        #cv2.waitKey(0)
        return value
    return ncc

def normalize(x):
    import numpy as np
    n = np.sqrt(np.dot(x, x))
    return np.divide(x, n)

def gauss(x, sigma=1.0):
    import numpy as np
    xb = np.divide(np.square(x), 2.0 * sigma * sigma)
    a = 0.3989422804014327 / sigma
    return np.multiply(np.exp(np.negative(xb)), a)

class VoxelVisualizer:
    def __init__(self, cameras, model, x, y, z):
        import numpy as np
        voxel_f = np.array([(float(x)+0.5) / float(model['width']),
                            (float(y)+0.5) / float(model['height']),
                            (float(z)+0.5) / float(model['depth'])]);
        pos = np.array([np.interp(voxel_f[0], [0,1], model['bbox'][:,0]),
                        np.interp(voxel_f[1], [0,1], model['bbox'][:,1]),
                        np.interp(voxel_f[2], [0,1], model['bbox'][:,2])])
        h = (model['bbox'][1,0]-model['bbox'][0,0]) / float(model['width'])
        self.voxel_pos = pos
        self.voxel_size = h
        self.cameras = cameras
        self.model = model

    def visualize_ncc_curve(self, cam_i, cam_j):
        import cv2, numpy as np
        voxel_pos = self.voxel_pos
        voxel_size = self.voxel_size
        cameras = self.cameras
        # load images
        image_i = cv2.imread(cameras[cam_i]['image_path'])
        image_j = cv2.imread(cameras[cam_j]['image_path'])
        # compute direction from voxel
        voxel_dir = np.subtract(cameras[cam_i]['center'], voxel_pos)
        voxel_dir = normalize(voxel_dir)
        voxel_dir = np.multiply(voxel_dir, voxel_size * 0.5)
        # project the voxel into the two images
        tfm_i = np.dot(cameras[cam_i]['intrinsic'], cameras[cam_i]['extrinsic'])
        tfm_j = np.dot(cameras[cam_j]['intrinsic'], cameras[cam_j]['extrinsic'])
        vpos_i = cv2.perspectiveTransform(np.array([[voxel_pos]]), tfm_i)[0,0]
        vpos_j0 = cv2.perspectiveTransform(np.array([[np.subtract(voxel_pos, voxel_dir)]]), tfm_j)[0,0]
        vpos_j1 = cv2.perspectiveTransform(np.array([[voxel_pos]]), tfm_j)[0,0]
        vpos_j2 = cv2.perspectiveTransform(np.array([[np.add(voxel_pos, voxel_dir)]]), tfm_j)[0,0]
        # draw epipolar lines
        canvas = np.copy(image_i)
        cv2.circle(canvas, tuple(vpos_i[0:2].astype(int)), 5, (0,0,255), 3)
        cv2.imshow("Camera i = %d" % cam_i, canvas)
        canvas = np.copy(image_j)
        cv2.circle(canvas, tuple(vpos_j1[0:2].astype(int)), 5, (0,0,255), 1)
        cv2.line(canvas, tuple(vpos_j0[0:2].astype(int)), tuple(vpos_j2[0:2].astype(int)), (0,255,255), 2)
        cv2.imshow("Camera j = %d" % cam_j, canvas)
        # compute NCC
        xdata = np.arange(-5.0, 5.0, 0.01)
        ydata = np.array(map(gen_ncc_func(image_i, image_j, tfm_i, tfm_j, voxel_pos, voxel_dir), xdata))
        #for i in range(0,len(xdata)):
        #    print("sj(d=%f) = %f" % (xdata[i], ydata[i]))
        #from pysmoothing import sgolayfilt
        #ydata = sgolayfilt(ydata, 3, 7)
        # find maxima
        from scipy.signal import argrelmax
        maxima = argrelmax(ydata)
        # plot NCC
        import matplotlib.pyplot as plt
        plt.figure()
        plt.suptitle("NCC curve (i=%d, j=%d)" % (cam_i, cam_j))
        plt.axis([np.min(xdata), np.max(xdata), -1.1, 1.1])
        plt.xlabel("d")
        plt.ylabel("Sj(d) = NCC(i,j,o(d))")
        plt.axvline(1.0, -1.0, 1.0, linestyle='--', color='r')
        plt.axvline(-1.0, -1.0, 1.0, linestyle='--', color='r')
        plt.axvline(3.0, -1.0, 1.0, linestyle=':', color='r')
        plt.axvline(-3.0, -1.0, 1.0, linestyle=':', color='r')
        plt.plot(xdata, ydata)
        plt.plot(xdata[maxima], ydata[maxima], '.')

    def find_closest_cameras(self, cam_i, x):
        import numpy as np
        cameras = self.cameras
        ci = cameras[cam_i]
        ni = normalize(np.subtract(ci['center'], x))
        def f(j):
            cj = cameras[j]
            nj = normalize(np.subtract(cj['center'], x))
            dp = np.dot(ni, nj)
            #return (dp <= 0.999) and (dp >= 0.9396926207859084)
            return (dp <= 0.984807753012208) and (dp >= 0.9396926207859084)
        jcams = filter(f, range(0, len(cameras)))
        return jcams

    def gen_score(self, cam_i):
        import cv2, numpy as np
        voxel_pos = self.voxel_pos
        voxel_size = self.voxel_size
        cameras = self.cameras
        # prepare cam_i
        image_i = cv2.imread(cameras[cam_i]['image_path'])
        tfm_i = np.dot(cameras[cam_i]['intrinsic'], cameras[cam_i]['extrinsic'])
        # voxel direction
        voxel_dir = np.subtract(cameras[cam_i]['center'], voxel_pos)
        voxel_dir = normalize(voxel_dir)
        voxel_dir = np.multiply(voxel_dir, voxel_size * 0.5)
        # prepare closest cameras
        jcams = self.find_closest_cameras(cam_i, voxel_pos)
        #print("nearest cameras for %d = " % cam_i, jcams)
        #for cam_j in jcams:
        #    self.visualize_ncc_curve(cam_i, cam_j)
        def prepare_camj(j):
            image_j = cv2.imread(cameras[j]['image_path'])
            tfm_j = np.dot(cameras[j]['intrinsic'], cameras[j]['extrinsic'])
            ncc_j = gen_ncc_func(image_i, image_j, tfm_i, tfm_j, voxel_pos, voxel_dir)
            return (j, image_j, tfm_j, ncc_j)
        jcams = map(prepare_camj, jcams)
        # gen func
        def score(d):
            s = np.array(map(lambda t: t[-1](d), jcams))
            return np.mean(s)
        return score

    def visualize_score(self, cam_i):
        import cv2, numpy as np
        score = self.gen_score(cam_i)
        xdata = np.arange(-5.0, 5.0, 0.05)
        ydata = np.array(map(score, xdata))
        for i in range(0,len(xdata)):
            print("C(%f) = %f" % (xdata[i], ydata[i]))
        # plot score
        import matplotlib.pyplot as plt
        plt.figure()
        plt.suptitle("Score")
        plt.axis([np.min(xdata), np.max(xdata), -1.1, 1.1])
        plt.xlabel("d")
        plt.ylabel("C(d)")
        plt.axvline(1.0, -1.0, 1.0, linestyle='--', color='r')
        plt.axvline(-1.0, -1.0, 1.0, linestyle='--', color='r')
        plt.plot(xdata, ydata)

    def vote1(self, cam_i):
        import numpy as np
        score = self.gen_score(cam_i)
        xdata = np.arange(-5.0, 5.0, 1.0) # d range
        ydata = np.array(map(score, xdata))
        c0 = score(0)
        if np.all(c0 >= ydata):
            return c0
        return 0.0

    def vote2(self, cam_i):
        import numpy as np
        score = self.gen_score(cam_i)
        xdata = np.arange(-5.0, 5.0, 0.1) # d range
        ydata = np.array(map(score, xdata))
        c0 = np.max(ydata[np.abs(xdata) < 1.0])
        if np.all(c0 >= ydata):
            return c0
        return 0.0

    def visualize_votes1(self):
        import numpy as np
        xdata = np.array(range(0,len(self.cameras)))
        ydata = np.array(map(lambda i: self.vote1(i), xdata))
        import matplotlib.pyplot as plt
        plt.figure()
        plt.suptitle("Vote (version 1)")
        plt.axis([np.min(xdata), np.max(xdata), -1.1, 1.1])
        plt.xlabel("i-th camera")
        plt.ylabel("VOTE(i)")
        plt.plot(xdata, ydata, '.')

    def visualize_votes2(self):
        import numpy as np
        xdata = np.array(range(0,len(self.cameras)))
        ydata = np.array(map(lambda i: self.vote2(i), xdata))
        #for i in range(0,len(xdata)):
        #    print("vote(i=%d) = %f" % (xdata[i], ydata[i]))
        import matplotlib.pyplot as plt
        plt.figure()
        plt.suptitle("Vote (version 2)")
        plt.axis([np.min(xdata), np.max(xdata), -1.1, 1.1])
        plt.xlabel("i-th camera")
        plt.ylabel("VOTE(i)")
        plt.plot(xdata, ydata, '.')
        #
        total = np.nansum(np.array(filter(lambda x: x is not None, ydata)), dtype=np.float)
        rho = np.exp(np.multiply(-0.05, total))
        print("total = %f" % total, "rho = %f" % rho)

def mainfunc():
    global ARGS
    ARGS = parse_args()

    import cv2, numpy as np, matplotlib.pyplot as plt
    cameras = []
    model = {}

    # Load from data
    import json
    with open(ARGS.data, 'r') as f:
        rootobj = json.loads(f.read())
        for jc in rootobj['cameras']:
            c = dict(
              center = jc['center'],
              image_path = jc['image'],
              extrinsic = np.array(jc['extrinsic']),
              intrinsic = np.array(jc['intrinsic'])
            )
            cameras.append(c)
        model['width'] = rootobj['model']['width']
        model['height'] = rootobj['model']['height']
        model['depth'] = rootobj['model']['depth']
        model['bbox'] = np.array(rootobj['model']['virtual_box'])

    visualizer = VoxelVisualizer(cameras, model, ARGS.voxel_x, ARGS.voxel_y, ARGS.voxel_z)
    print("voxel_size = %f" % visualizer.voxel_size)
    #visualizer.visualize_ncc_curve(ARGS.cam_i, ARGS.cam_j)
    #visualizer.visualize_score(ARGS.cam_i)
    #visualizer.visualize_votes1()
    visualizer.visualize_votes2()

    for cam_i in range(0, len(cameras)):
        if cam_i != 29:
            continue
        for cam_j in visualizer.find_closest_cameras(cam_i, visualizer.voxel_pos):
            visualizer.visualize_ncc_curve(cam_i, cam_j)

    # Wait
    plt.show()
    #cv2.waitKey(0)

if __name__ == '__main__':
    mainfunc()

# python2 ../voxel/tools/ncc-curve.py --voxel-x=55 --voxel-y=30 --voxel-z=58 --cam-i=34 --cam-j=36 data.json
