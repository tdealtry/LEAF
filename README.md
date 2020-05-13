# LEAF
Low Energy Algoritm Framework

This algorithm is an alternative and simple LE fitter than can be used for HyperK and SuperK.

*************************
2020/02/09: LEAF was convert as a C++ class and can be included in your code.

New compilation method:

	source RunAtStart.sh
	cd leaf/
	make clean; make
	
In order to use the class in your code look at example/

*************************

1. Source RunAtStart.sh after you updated your ROOT directory.

2. Enter app/ repository, and make clean;make.

3. inputs PDF, input from WCSim can be downloaded on sukap cluster. Please untar them in the LEAF repository.

4. You can use shell scripts in shell/ in order to run the fitter or launch on batch.

*************************

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
| `double Good;` | Goodness of fit of time/vertex fit. A smaller?/larger? number corresponds to a better fit??? |
| `double Wall;` | Distance to wall???|
| `int n50[PMTSUSED];`  | Maximum number of hits of an event within a 50 ns window |
| `double dir[PMTSUSED][3];` | x,y,z unit vector?/Euler angles?/? of reconstructed direction??? |
| `double dir_goodness[PMTSUSED];` | Goodness of fit of direction fit. A smaller?/larger? number corresponds to a better fit??? |
| `double dirKS[PMTSUSED];` | A KS test to check if the PMT hit of an event are clustered or not. A smaller?/larger? number corresponds to higher clustering |
| `double True_NLLDiff;`    | Difference between NLL of fitted time/vtx and the NLL of the true time/vtx |
| `double True_TimeDiff;`   | Difference between the fitted time and the true time |
| `double True_DistDiff;`   | Difference between the fitted vertex and the true vertex |
| `int InTime;`  | Number of PMT hit whose hit time corrected by the time of flight is within `fSTimePDFLimitsQueueNegative` and `fSTimePDFLimitsQueuePositive`. This is a debug variable, probably not useful for analyses |
