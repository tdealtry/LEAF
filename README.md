# LEAF
Low Energy Algoritm Framework

This algorithm is an alternative and simple LE fitter than can be used for HyperK and SuperK.

~~~~~~~~~~~~~~~~~~~~~~~~~
2020/02/09: LEAF was convert as a C++ class and can be included in your code.

New compilation method:

	source RunAtStart.sh
	cd leaf/
	make clean; make
	
In order to use the class in your code look at example/

~~~~~~~~~~~~~~~~~~~~~~~~~

1. Source RunAtStart.sh after you updated your ROOT directory.

2. Enter app/ repository, and make clean;make.

3. inputs PDF, input from WCSim can be downloaded on sukap cluster. Please untar them in the LEAF repository.

4. You can use shell scripts in shell/ in order to run the fitter or launch on batch.

~~~~~~~~~~~~~~~~~~~~~~~~~

## Results

The class implementation returns a `BQFitter::FitterOutput` structure. The contents are shown below

Note that PMTSUSED can take the following 3 values
1. Use only 20" PMTs
2. Use only mPMTs
3. Use both 20" PMTs and mPMTs

| Variable | Description |
| -------- | ----------- |
| `double Vtx[3];` | x,y,z of reconstructed vertex. Units: same as input (WCSim nominally uses cm) |
| `double Time;` | Time of reconstructed vertex. Units: same as input (WCSim nominally uses ns) |
| `int InTime;`  | ??? |
| `double Good;` | Goodness of fit of time/vertex fit. A smaller?/larger? number corresponds to a better fit??? |
| `double Wall;` | Distance to wall???|
| `int n50[3];`  | ??? |
| `double dir[PMTSUSED][3];` | x,y,z unit vector?/Euler angles?/? of reconstructed direction??? |
| `double dir_goodness[PMTSUSED];` | Goodness of fit of direction fit. A smaller?/larger? number corresponds to a better fit??? |
| `double dirKS[PMTSUSED];` | ??? |
| `double True_NLLDiff;`    | ??? |
| `double True_TimeDiff;`   | ??? |
| `double True_TistDiff;`   | ??? |
