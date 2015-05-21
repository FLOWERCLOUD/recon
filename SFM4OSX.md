## [VisualSFM](http://ccwu.me/vsfm/) by [Changchang Wu](http://ccwu.me/)

Probably the most straight forward way to start generating Point Clouds from a set of pictures.

**VisualSFM** is a GUI application for 3D reconstruction using structure from motion (SFM). The reconstruction system integrates several of my previous projects: [SIFT on GPU(SiftGPU)](http://www.cs.unc.edu/~ccwu/siftgpu/), [Multicore Bundle Adjustment](http://grail.cs.washington.edu/projects/mcba/), and [Towards Linear-time Incremental Structure from Motion](http://ccwu.me/vsfm/vsfm.pdf). VisualSFM runs fast by exploiting multicore parallelism for feature detection, feature matching, and bundle adjustment.

For dense reconstruction, this program supports Yasutaka Furukawa's [PMVS/CMVS](http://www.di.ens.fr/cmvs/) tool chain, and can prepare data for Michal Jancosek's [CMP-MVS](http://ptak.felk.cvut.cz/sfmservice/websfm.pl?menu=cmpmvs). In addition, the output of VisualSFM is natively supported by Mathias Rothermel and Konrad Wenzel's [SURE](http://www.ifp.uni-stuttgart.de/publications/software/sure/index.en.html).

[Dan’s Monaghan](www.luckybulldozer.com) wrote a [script that will compile VisualSfM on MacOS](https://github.com/luckybulldozer/VisualSFM_OS_X_Mavericks_Installer) you. 

Before that you should installing some basic dependences:

- [Homebrew](http://brew.sh/)

```
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

- [CUDA](https://developer.nvidia.com/cuda-downloads#mac)

- [XQuartz](http://xquartz.macosforge.org/landing/)

### Compile

To compile VisualSfM on Mac Yosemite (OS X 10.10) I have fork and modify [Dan’s Monaghan](www.luckybulldozer.com) script. Clone it and run it.

```
git clone git@github.com:tangrams/VisualSFM_OS_X.git
cd VisualSFM_OS_X
./vsfm_os_x_installer.sh
```

### Install

You can run VisualSfM directly from these folder. I really like been able to execute it from any where on my system. For that:

```
cd vsfm/bin
mv *.so /usr/local/lib 
mv * /usr/local/bin
```

### Uninstall

To uninstall VisualSfM from your system you just need to:

```
rm /usr/local/bin/VisualSFM
rm /usr/local/bin/cmvs
rm /usr/local/bin/genOption
rm /usr/local/bin/pmvs2
rm /usr/local/lib/libsiftgpu.so
rm /usr/local/lib/libpba.so
```

## [Bundler SFM](http://www.cs.cornell.edu/~snavely/bundler/) by [Noah Snavely](http://www.cs.cornell.edu/~snavely/)

Bundler is a structure-from-motion (SfM) system for unordered image collections (for instance, images from the Internet) written in C and C++. An earlier version of this SfM system was used in the [Photo Tourism](http://phototour.cs.washington.edu/) project. **For structure-from-motion datasets, please see the [BigSFM](http://www.cs.cornell.edu/projects/bigsfm/) page.**

Bundler takes a set of images, image features, and image matches as input, and produces a 3D reconstruction of camera and (sparse) scene geometry as output. The system reconstructs the scene incrementally, a few images at a time, using a modified version of the [Sparse Bundle Adjustment](http://users.ics.forth.gr/~lourakis/sba/) package of Lourakis and Argyros as the underlying optimization engine. Bundler has been successfully run on many Internet photo collections, as well as more structured collections.

The Bundler source distribution also contains potentially userful implementations of several computer vision algorithms, including:

- F-matrix estimation

- Calibrated 5-point relative pose

- Triangulation of multiple rays

Bundler produces sparse point clouds. For denser points, Dr. Yasutaka Furukawa has written a beautiful software package called [PMVS2](http://www.di.ens.fr/pmvs/) for running dense multi-view stereo. A typical pipeline is to run Bundler to get camera parameters, use the provided Bundle2PMVS program to convert the results into PMVS2 input, then run PMVS2. You might also be interested in Dr. Furukawa's [CMVS](http://www.di.ens.fr/cmvs/) view clustering software, which is a helpful preprocess to running PMVS2.

### Install dependences

- install Python correctly on OS X following [this tutorial](https://gist.github.com/patriciogonzalezvivo/77da993b14a48753efda)

- install one of the following implementations of [Lowe's Sift algorithm](http://www.cs.ubc.ca/~lowe/keypoints/)

    - [siftGPU](http://cs.unc.edu/~ccwu/siftgpu/)

    - [OpenSift](https://github.com/robwhess/opensift)

    - [vlfeat](http://www.vlfeat.org/)

```
git clone git://github.com/vlfeat/vlfeat.git
cd vlfeat
make
cd bin/maci64
cp sift /usr/local/bin
cp aib /usr/local/bin
cp mser /usr/local/bin
cp libvl.dylib /usr/local/lib
```

- install [JHead](http://www.sentex.net/~mwandel/jhead/)

```
brew install jhead
```

- install [Ceres-Solver](https://code.google.com/p/ceres-solver/)

```
brew install ceres-solver 
```

- install [ImageMagic](http://www.imagemagick.org/)

```
brew install imagemagick
```

- install python Pillow image library (ex-PIL) 

```
pip install Pillow
```

### Compile the binaries

Download Bundler code and modify the makefile to compile on OS X

```
git clone git@github.com:snavely/bundler_sfm.git
cd bundler_sfm
cp src/Makefile src/Makefile.bkp
sed -e 's/\-lgfortran//g' src/Makefile.bkp > src/Makefile 
open src/Makefile
```

Add ```-I/usr/local/include/``` in the ```INCLUDE_PATH``` and ```-L/usr/local/lib``` to the ```LIB_PATH``` in order to look like:

```
INCLUDE_PATH=-I../lib/imagelib -I../lib/sfm-driver -I../lib/matrix  \
    -I../lib/5point -I../lib/sba-1.5 -I../lib/ann_1.1_char/include  \
    -I../include -I/usr/local/include/

LIB_PATH=-L../lib -L../lib/ann_1.1_char/lib -L/usr/local/lib
```

Then modify the ```make``` file on minpack to point to the right fortran compiler

```
open lib/minpack/Makefile
```

Change ```FC=gcc``` for ```FC=gfortran```

```
make
```

### Create a VLFEAT to Lowe's SIFT convertion script

```
touch ToLoweSift.sh
chmod +x ToLoweSift.sh
mv ToLoweSift.sh /usr/local/bin
open /usr/local/bin/ToLoweSift.sh
```

Copy and paste:

```
#!/bin/sh

for file in "$@"; do
    echo "Treating $file" 
    if [ "${file##*.}" == "gz" ]; then
        echo "Uncompressing to ${file%.key.gz}.key"
        gzip -d "$file"
        echo "Converting ${file%.key.gz}.sift"
        cat "${file%.key.gz}.key" | wc -l | awk '{ print $1 " 128" }' > "${file%.key.gz}.sift"
        cat "${file%.key.gz}.key" | awk 'BEGIN { split("4 24 44 64 84 104 124 132", offsets); } { i1 = 0; tmp = $1; $1 = $2; $2 = tmp; for (i=1; i<9; i++) { i2 = offsets[i]; out = ""; for (j=i1+1; j<=i2; j++) { if (j != i1+1) { out = out " " }; out = out $j }; i1 = i2; print out } }' >> "${file%.key.gz}.sift"
        echo "Replacing ${file%.key.gz}.sift for ${file%.key.gz}.key"
        rm "${file%.key.gz}.key"
        mv ${file%.key.gz}.sift ${file%.key.gz}.key
        echo "Compressing ${file%.key.gz}.key"
        gzip -f ${file%.key.gz}.key
    else
        echo "Converting to ${file%.key}.sift"
        cat "$file" | wc -l | awk '{ print $1 " 128" }' > "${file%.key}.sift"
        cat "$file" | awk 'BEGIN { split("4 24 44 64 84 104 124 132", offsets); } { i1 = 0; tmp = $1; $1 = $2; $2 = tmp; for (i=1; i<9; i++) { i2 = offsets[i]; out = ""; for (j=i1+1; j<=i2; j++) { if (j != i1+1) { out = out " " }; out = out $j }; i1 = i2; print out } }' >> "${file%.key}.sift"
        echo "Replacing ${file%.key}.sift for ${file%.key}.key"
        rm "${file%.key}.sift"
        mv ${file%.key}.sift ${file%.key}.key
        echo "Compressing ${file%.key}.key"
        gzip -f ${file%.key}.key
    fi
done
```


### Tweak the scripts

If you are using VLfeat you need to change things here and there to make everything work for you.

```
open bin/toSift.sh
```

According to [this article](http://blog.aradine.com/2010/10/converting-vlfeat-sift-output-to-lowe.html) we have to replace:

```
echo "mogrify -format pgm $IMAGE_DIR/$d; $SIFT < $pgm_file > $key_file; rm $pgm_file; gzip -f $key_file"
```

for 

```
echo "wc -l $key_file.vlfeat | awk '{ print \$1 \" 128\" }' > $key_file"
echo "awk 'BEGIN { split("4 24 44 64 84 104 124 132", offsets); } { i1 = 0; tmp = $1; $1 = $2; $2 = tmp; for (i=1; i<9; i++) { i2 = offsets[i]; out = ""; for (j=i1+1; j<=i2; j++) { if (j != i1+1) { out = out " " }; out = out $j }; i1 = i2; print out } }' >> $key_file.key”
echo "rm $key_file.vlfeat"
```

Then we should do something similar on ```toSiftList.sh``` to match the format vlfeat-sift, also we need to correct the location of ```mogrify``` and erase the `gzip -f` call.

```
open bin/toSiftList.sh
```

And replace:

```
awk "{pgm = \$1; key = \$1; sub(\"jpg\$\", \"pgm\", pgm); sub(\"jpg\$\", \"key\", key); print \"/usr/bin/mogrify -format pgm \" \$1 \"; $SIFT < \" pgm \" > \" key \"; gzip -f \" key \"; rm \" pgm}" $IMAGE_LIST
```

for

```
awk "{pgm = \$1; key = \$1; sub(\"jpg\$\", \"pgm\", pgm); sub(\"jpg\$\", \"key\", key); print \"/usr/local/bin/mogrify -format pgm \" \$1 \"; $SIFT \" pgm \" -o \" key \"; rm \" pgm}" $IMAGE_LIST 
```

Then open and edit paths on ```RunBundler.sh``` to match your system. 

```
open RunBundler.sh
```

Replace:

```
BASE_PATH=$(dirname $(which $0));
```

for

```
BASE_PATH="/usr/local";
```

Also ```RunBundler.sh``` use ```toSiftList.sh``` which still export to VLfeat style so we need to add the following block of code after ```sh sift.txt``` (approx. line 92): 

```
### Convert VLFeat's keys to Lowe's
for file in *.key; do
  cat "$file" | wc -l | awk '{ print $1 " 128" }' > "${file%.key}.sift"
  cat "$file" | awk 'BEGIN { split("4 24 44 64 84 104 124 132", offsets); } { i1 = 0; tmp = $1; $1 = $2; $2 = tmp; for (i=1; i<9; i++) { i2 = offsets[i]; out = ""; for (j=i1+1; j<=i2; j++) { if (j != i1+1) { out = out " " }; out = out $j }; i1 = i2; print out } }' >> "${file%.key}.sift"
done
```

Then comment the ```gzip -d *.gz``` line that follows the ```echo "[- Matching keypoints (this can take a while) -]"``` and modyfy the next line to search for ```.sift``` files instead of ```.key``` onese. It should look like this:

```
# Match images (can take a while)
echo "[- Matching keypoints (this can take a while) -]"
awk '{print $1}' $IMAGE_LIST | sed 's/\.jpg$/\.sift/' > list_keys.txt
```


Finally if you follow [this tutorial](https://gist.github.com/patriciogonzalezvivo/77da993b14a48753efda) or you are using a Homebrew Python flavor edit the header of ```bundler.py```.

```
open utils/bundler.py
```

And replace

```
#!/usr/bin/python
```

to 

```
#!/usr/local/bin/python
```


### Install
 
Once we are sure everything is working and pointing to the right paths move them to you ```/usr/local``` path to install them on the system

```
mv RunBundler.sh /usr/local/bin
cd bin/
rm zlib1.dll
mv *.so /usr/local/lib
mv * /usr/local/bin
cd ../utils
mv * /usr/local/bin
```

### Uninstall

Just remove the files we copy

```
rm /usr/local/bin/RunBundler.sh
rm /usr/local/bin/ToSift.sh
rm /usr/local/bin/ToSiftList.sh
rm /usr/local/bin/bundler.py
rm /usr/local/bin/bundler
rm /usr/local/bin/Bundle2Ply
rm /usr/local/bin/Bundle2PMVS
rm /usr/local/bin/Bundle2Vis
rm /usr/local/bin/extract_focal.pl
rm /usr/local/bin/KeyMatchFull
rm /usr/local/bin/RadialUndistort
rm /usr/local/lib/libANN_char.so
rm /usr/local/bin/sift
rm /usr/local/bin/aib
rm /usr/local/bin/mser
rm /usr/local/lib/libvl.dylib
```

## [OSM-bundler](http://haiyangxu.github.io/osm-bundler/)

## [BigSfM](http://www.cs.cornell.edu/projects/bigsfm/)

## [OpenMVG](http://imagine.enpc.fr/~moulonp/openMVG/)

"Open Multiple View Geometry" is a library for computer-vision scientists and especially targeted to the Multiple View Geometry community. It is designed to provide an easy access to the classical problem solvers in Multiple View Geometry and solve them accurately.

```
git clone --recursive https://github.com/openMVG/openMVG.git
mkdir openMVG_Build
cd openMVG_Build
cmake -DCMAKE_BUILD_TYPE=RELEASE -G "Xcode" . ../openMVG/src/
```

If you want enable unit tests and examples to the build:

```
cmake -DCMAKE_BUILD_TYPE=RELEASE -DOpenMVG_BUILD_TESTS=ON -DOpenMVG_BUILD_EXAMPLES=ON -G "Xcode" . ../openMVG/src/
xcodebuild -configuration Release
```