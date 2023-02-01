COS351 - Yellowstone Project Part 4: Fire Mapping

Steps:
	1. Fire generation
	2. Image generation
	3. Animation generation

---------------

1. This project simulates and visualizes the spread of a fire over Yellowstone. To execute this program one can create some data by first providing their desired path for file placement in fire.c (in the spots denoted with "ADD PATH HERE...") and then the following in order to fire (also in the "YellowstoneFireVisualization/" directory):

	- The desired number of processors to be used.

	- An initial simulation size: This is the size of the area to be simulated in the program. This value will be squared to create said area of units.

	- A forest density: This will be provided as a decimal and approximately be the percentage of trees in the area provided by the user.

	- A fire probability: This will also be provided as a decimal and be the approximate chance that a fire will crop up at any one given tree in the area at the *start* of the simulation.

	- A wind direction: This can be written as either 'N', 'S', 'E', or 'W' and will increase or decrease the likelihood of a fire spreading in a particular direction depending on the wind orientation provided. For example, a direction of 'N' means that the wind is blowing to the north, so any cells to the north of a cell on fire will be more likely to ignite as well, and any cells to the south of a cell on fire will be less likely to ignite.

	- A wind speed: This will be between 0 and 33 (in units of mph) and affect the magnitude with which a fire will spread in a particular direction based on the wind direction given. If one wishes to not employ wind within their particular simulation, simply pass in any direction for the wind direction, and "0" for the wind speed.

	- This frequency with which one desires to print out data text files for subsequent imaging. If you would like an image of the fire spread at every stage of the program, simply use the number "1".

---------------

An example command for this program is as follows, where "x" is the desired number of processors to be used, "a" is the initial simulation size, "b" is the forest density, "c" is the fire probability, d" is the wind direction (don't forget quotes!), "e" is the wind speed and "f" is the file write frequency:

mpirun -n x ./fire a b c d e f

---------------

2. After data files have been generated in fire.c and paths for file reading have been provided to colorIt.c, the following command can be provided to colorIt.c to have the program start creating pictures of the data created in step 1; in this case "a" is the number of files one wishes to create pictures for:

./colorIt a

---------------

3. Finally, one can create an animation of their image files from step 2, by using FFmpeg and the following example command in the products folder - where one's images are located (note: this command can be altered with other commands provided by FFmpeg):

ffmpeg -framerate 1 -i image%d.png -c:v libx264 -r 30 -pix_fmt yuv420p output.mp4 -i audio.mp3 -t .2
