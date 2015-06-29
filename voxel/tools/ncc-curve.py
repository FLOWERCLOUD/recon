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
        import cv2, numpy as np, matplotlib.pyplot as plt
        voxel_pos = self.voxel_pos
        voxel_size = self.voxel_size
        cameras = self.cameras
        plt.figure()
        # load images
        image_i = cv2.imread(cameras[cam_i]['image_path'])
        image_j = cv2.imread(cameras[cam_j]['image_path'])
        # compute direction from voxel
        voxel_dir = np.subtract(cameras[cam_i]['center'], voxel_pos)
        voxel_dir = normalize(voxel_dir)
        voxel_dir = np.multiply(voxel_dir, voxel_size * 1.0)
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
        nhk = 32
        xdata = np.array(map(lambda x: float(x-nhk) / float(nhk), range(0,2*nhk+1)))
        ydata = np.array(map(gen_ncc_func(image_i, image_j, tfm_i, tfm_j, voxel_pos, voxel_dir), xdata))
        from pysmoothing import sgolayfilt
        ydata = sgolayfilt(ydata, 3, 7)
        plt.plot(xdata, ydata)
        # find maxima
        from scipy.signal import argrelmax
        maxima = argrelmax(ydata)
        plt.axis([np.min(xdata), np.max(xdata), -1.1, 1.1])
        plt.plot(xdata[maxima], ydata[maxima], '.')

    def gen_combined_ncc(self, cam_i):
        import cv2, numpy as np, matplotlib.pyplot as plt
        from pysmoothing import sgolayfilt
        from scipy.signal import argrelmax, gaussian
        voxel_pos = self.voxel_pos
        voxel_size = self.voxel_size
        cameras = self.cameras
        # prepare cam_i
        image_i = cv2.imread(cameras[cam_i]['image_path'])
        tfm_i = np.dot(cameras[cam_i]['intrinsic'], cameras[cam_i]['extrinsic'])
        # voxel direction
        voxel_dir = np.subtract(cameras[cam_i]['center'], voxel_pos)
        voxel_dir = normalize(voxel_dir)
        voxel_dir = np.multiply(voxel_dir, voxel_size * 1.0)
        # find Sj and dk
        sjdk = None
        dk = None
        for cam_j in range(0, len(cameras)):
            if cam_j == cam_i:
                continue
            # prepare cam_j
            image_j = cv2.imread(cameras[cam_j]['image_path'])
            tfm_j = np.dot(cameras[cam_j]['intrinsic'], cameras[cam_j]['extrinsic'])
            # compute NCC
            ncc = gen_ncc_func(image_i, image_j, tfm_i, tfm_j, voxel_pos, voxel_dir)
            nhk = 32
            xdata = np.array(map(lambda x: float(x-nhk) / float(nhk), range(0,2*nhk+1)))
            ydata = np.array(map(ncc, xdata))
            ydata = sgolayfilt(ydata, 3, 7)
            # find maxima
            maxima = argrelmax(ydata)
            # save result
            if sjdk is not None:
                sjdk = np.concatenate((sjdk, ydata[maxima]))
                dk = np.concatenate((dk, xdata[maxima]))
            else:
                sjdk = np.array(ydata[maxima])
                dk = np.array(xdata[maxima])
        sjdk = np.divide(sjdk, float(len(cameras)))
        def c_func(d):
            w = gauss(np.subtract(d, dk))
            return np.inner(sjdk, w)
        return (c_func, len(dk))

    #def visualize_combined_ncc(self, cam_i):
    #    import numpy as np, matplotlib.pyplot as plt
    #    c, nmaxima = self.gen_combined_ncc(cam_i)
    #    x = np.arange(-5.0, 5.0, 0.01)
    #    y = np.array(map(c, x))
    #    plt.figure()
    #    plt.plot(x, y)

    #def visualize_nmaxima(self):
    #    import numpy as np, matplotlib.pyplot as plt
    #    data = np.array(map(lambda x: self.gen_combined_ncc(x)[1], range(0,len(self.cameras))))
    #    plt.figure()
    #    plt.plot(data)

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
    visualizer.visualize_ncc_curve(ARGS.cam_i, ARGS.cam_j)
    #visualizer.visualize_combined_ncc(ARGS.cam_i)
    #visualizer.visualize_nmaxima()

    #plt.figure()
    #x = np.arange(-5.0, 5.0, 0.01)
    #plt.plot(x, gauss(x))

    # Visualize NCC
    #
    #from scipy.interpolate import spline
    #nxdata = np.linspace(-1.0, 1.0, 512)
    #nydata = spline(xdata, ydata, nxdata)
    #plt.plot(nxdata, nydata)

    # Find maxima
    #
    #maxima = argrelmax(nydata)
    #print(maxima)
    #
    #plt.plot(nxdata[maxima], nydata[maxima], '.')

    # Parzen Window

    # Wait
    plt.show()
    #cv2.waitKey(0)

if __name__ == '__main__':
    mainfunc()

# python2 ../voxel/tools/ncc-curve.py --voxel-x=55 --voxel-y=30 --voxel-z=58 --cam-i=34 --cam-j=36 data.json
