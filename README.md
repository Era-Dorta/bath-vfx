# vfx
* http://www.vlfeat.org/
Install vlfeat and add to path
* http://people.csail.mit.edu/celiu/OpticalFlow/
Install optical flow code and add to path
* http://cs.brown.edu/~black/code.html/
* http://www.mit.edu/~kimo/software/matlabexr
Install exr reader and writer for matlab
1. sudo apt-get install libopenexr-dev
2. mex "file.cpp" -lIlmImf -lIex -lImath -lHalf -I/usr/include/OpenEXR -L/usr/lib/x86_64-linux-gnu
* http://www.cs.ubc.ca/research/flann/
 Install flann for matlab, on ubuntu, download the source code from above, then run 
1. sudo apt-get install libflann-dev
2. Unzip the code and go to the src/matlab folder
3. Copy the folder into your matlab folder and rename it to flann
4. Open matlab and cd into the flann folder
5. run mex -L/usr/lib -lflann -I/usr/include nearest_neighbors.cpp
