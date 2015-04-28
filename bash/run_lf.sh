#!/bin/bash
date
echo "****************************************"
echo "****************************************"
echo "Computation starting"
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
EXE_PATH=~/workspaces/eclipse/lf/Release/lf
echo $EXE_PATH
cd ~/workspaces/matlab/vfx/Data/skinRender/microgeometry
# Runs cores for each image
EXT="c"
#EXT=""
I=1
while [  $I -lt 11 ]; do
	echo "****************************************"
	echo "Image num:" $I
	echo "****************************************"
	#echo $EXE_PATH "useYIQ: true" srcExample: "original/A0_"$I$EXT".png" filteredExample: "original/A1_"$I$EXT".png" srcImage: "synthesized/B0_"$I$EXT".png" outputImageName: "synthesized/B1_"$I$EXT".png"
	#$EXE_PATH srcExample: "original/A0_"$I$EXT".png" filteredExample: "original/A1_"$I$EXT".png" srcImage: "synthesized/B0_"$I$EXT".png" outputImageName: "synthesized/B1_"$I$EXT".png" useInterface: "false" &
	if [ $EXT == "c" ]; then
		$EXE_PATH coherenceEps: "10" matchMeanVariance: "true" useYIQ: "true" usePCA: "true" srcExample: "original/A0_"$I$EXT".png" filteredExample: "original/A1_"$I$EXT".png" srcImage: "synthesized/B0_"$I$EXT".png" outputImageName: "synthesized/B1_"$I$EXT".png" useInterface: "false" &
	else
		$EXE_PATH srcExample: "original/A0_"$I$EXT".png" filteredExample: "original/A1_"$I$EXT".png" srcImage: "synthesized/B0_"$I$EXT".png" outputImageName: "synthesized/B1_"$I$EXT".png" useInterface: "false" &
	fi	
	let I=I+1 
done
echo "Computation ended"
echo "****************************************"
echo "****************************************"
