/*********************************************************************************/
/**	BQFitter.hh								**/
/**	Author: Guillaume Pronost (pronost@km.icrr.u-tokyo.ac.jp)		**/
/**	Original author: Benjamin Quilain					**/
/**	Date: December 18th 2019						**/
/**	Desc: Low-E Fitter for Hyper-K						**/
/**     + Add Low-E functions from Super-K					**/
/*********************************************************************************/


#ifndef BQFitter_hh
#define BQFitter_hh

//#include <algorithm>
//#include <ctime>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread> 
#include <mutex>

//WCSim Headers
#include "WCSimRootGeom.hh"

//ROOT Headers
#include "TFile.h"
#include "TFitter.h"
#include "TH1D.h"
#include "TMath.h"
#include "TMinuit.h"
#include "TObject.h"
#include "TRandom3.h"
#include "TSpline.h"

#define VERBOSE 		0

// Verbose level in functions:
#undef VERBOSE_VTX // In SearchVertex

#define NPMT_CONFIGURATION 	2
//Different PMT config in an mPMT. I assumed here a rotational symetry of the mPMT, so PMT 1 to 12 are the same, 13 to 18 are the same and 19 is separated
#define NGROUP_PMT 		3

#define NormalPMT		0
#define MiniPMT			1
#define AllPMT			2

#define mPMT_ID_SHIFT 		 1000000
#define MAX_PMT 		10000000

#define MAX_POS 		100000

#define VTX_X			0
#define VTX_Y			1
#define VTX_Z			2
#define VTX_T			3

#define GetPMTType(x)		x>=mPMT_ID_SHIFT?1:0
#define GetDistance(a,b)	sqrt( (a[0]-b[0])*(a[0]-b[0]) + (a[1]-b[1])*(a[1]-b[1]) + (a[2]-b[2])*(a[2]-b[2]) )

#define N_THREAD		30

#define CNS2CM 			21.58333

std::mutex mtx;

// Likelihood:
void MinuitLikelihood(int& nDim, double * gout, double & NLL, double par[], int flg);
void MinimizeVertex_CallThread(	int iStart, int iIte,	
						std::vector< std::vector<double> > initialVertex, double * limits, double stepSize, int nhits,
						int nCandidates, int tolerance, int verbose, bool likelihood, bool average,
						double lowerLimit, double upperLimit, int directionality);

void SearchVertex_CallThread(
			int iStart, int iIte,
			int nhits,int tolerance,bool likelihood,double lowerLimit, double upperLimit,int directionality);

bool SortOutputVector ( const std::vector<double>& v1, const std::vector<double>& v2 ) { 
	return v1[4] < v2[4]; 
} 
		
class BQFitter/* : public TObject */{

	public:
		BQFitter();
		~BQFitter();
		static BQFitter*		GetME();
		
		void SetGeometry( WCSimRootGeom * wGeo, double dDarkRate_Normal=8.4, double dDarkRate_mPMT=100. );
		void SetTrueVertexInfo(std::vector<double> vtx, double time);
		
		struct FitterOutput {
		
			double Vtx[3];
			double Time;
			int InTime;
			double Good;
			double Wall;
			
			int n50[3];
			double dir[3][3];
			double dir_goodness[3];
			
			double dirKS[3];
			
			
			double True_NLLDiff;
			double True_TimeDiff;
			double True_TistDiff;
		};
		
		
		struct FitterOutput MakeFit(bool bHybrid=true);
		
		// NLL
		double FindNLL_Likelihood(std::vector<double> vertexPosition, int nhits, double lowerLimit, double upperLimit, bool killEdges, bool scaleDR, int directionality);
		double FindNLL_NoLikelihood(std::vector<double> vertexPosition, int nhits, double lowerLimit, double upperLimit, bool killEdges, bool scaleDR, int directionality);
		double FindNLL(			std::vector<double> vertexPosition,int nhits, bool likelihood, int verbose, double lowerLimit, double upperLimit, 
						bool killEdges=false, bool scaleDR=false, int directionality=false);
						
		// Manage hit info
		void ResetHitInfo() { fHitInfo.clear(); }
		void AddHit(double time, double charge, int pmtType, int tubeNumber) {
		
			// tubeNumber is from 1 to xxx in Hit array
			// but from 0 to xxx -1 in PMT info
			tubeNumber -= 1;
			
			if ( tubeNumber < 0 ) {
				std::cout << "ERROR: tubeNumber is below 0 (" << tubeNumber << ")" << std::endl;
			}
			struct PMTHit hHit;
	
			if ( pmtType == 0 ) { // Normal PMT
				hHit.PMT = tubeNumber;
			}
			else { 		      // mPMT
				hHit.PMT = mPMT_ID_SHIFT + tubeNumber;
			}
			
			hHit.T = time;
			hHit.Q = charge;
			
			fHitInfo.push_back(hHit);
		}
		
		//void FillHitInfo(std::vector< struct PMTHit > tHitInfo) { fHitInfo = tHitInfo; }
		
		void SearchVertex_thread(	int iStart, int iIte,	
						int nhits, 
						int tolerance=1, bool likelihood=false, 
						double lowerLimit=fSTimePDFLimitsQueueNegative, double upperLimit=fSTimePDFLimitsQueuePositive, int directionality=false);

		
		void MinimizeVertex_thread(	int iStart, int iIte,	
						std::vector< std::vector<double> > initialVertex, double * limits, double stepSize, int nhits,
						int nCandidates = 1, int tolerance = 1, int verbose=0, bool likelihood=false, bool average=false,
						double lowerLimit=fSTimePDFLimitsQueueNegative, double upperLimit=fSTimePDFLimitsQueuePositive, int directionality = true);
		
	private:
		
		void Init();
		void LoadSplines();
		void LoadPMTInfo();
		void MakePositionList();
		
		double GetDistanceOld(std::vector<double> A, std::vector<double> B);
		
		// Spline functions:
		double SplineIntegral			(TSpline3 * s, 			double start, double end, 		double stepSize=5e-1);
		double SplineIntegralAndSubstract	(TSpline3 * s0, TSpline3 * s1, 	double start, double end, 		double stepSize=5e-1);
		double SplineIntegralExpo		(TSpline3 * s, 			double start, double end, double sigma, double stepSize=5e-1);
			
		// NLL
		void MakeEventInfo(double lowerLimit, double upperLimit);

		double FindNLLDirectionality( std::vector<double> vertexPosition, int nhits, int verbose, double lowerLimit, double upperLimit);
		
		
		double FindDirectionTheta(std::vector<double> vertex,int tubeNumber, int verbose);

		// SearchVertex functions:
		std::vector< std::vector<double> > SearchVertex(		
						int nhits, 
						int tolerance=1, bool likelihood=false, 
						double lowerLimit=fSTimePDFLimitsQueueNegative, double upperLimit=fSTimePDFLimitsQueuePositive, int directionality=false);
						
		std::vector< std::vector<double> > SearchVertex_Main(		
						int nhits, 
						int tolerance=1, bool likelihood=false, 
						double lowerLimit=fSTimePDFLimitsQueueNegative, double upperLimit=fSTimePDFLimitsQueuePositive, int directionality=false);
						
						
		std::vector< std::vector<double> > SearchVertexFine(	
						std::vector< std::vector<double> > initialVertex, double * limits, double stepSize, int nhits,  
						int nCandidates = 1, int tolerance = 1, int verbose=0, bool likelihood=false, bool average=false, 
						double lowerLimit=fSTimePDFLimitsQueueNegative, double upperLimit=fSTimePDFLimitsQueuePositive, int directionality=false);
		
		// Minimize function:
		std::vector< std::vector<double> > MinimizeVertex(	
						std::vector< std::vector<double> > initialVertex, double * limits, double stepSize, int nhits,
						int nCandidates = 1, int tolerance = 1, int verbose=0, bool likelihood=false, bool average=false,
						double lowerLimit=fSTimePDFLimitsQueueNegative, double upperLimit=fSTimePDFLimitsQueuePositive, int directionality = true);
						
		std::vector< std::vector<double> > MinimizeVertex_Main(	
						std::vector< std::vector<double> > initialVertex, double * limits, double stepSize, int nhits,
						int nCandidates = 1, int tolerance = 1, int verbose=0, bool likelihood=false, bool average=false,
						double lowerLimit=fSTimePDFLimitsQueueNegative, double upperLimit=fSTimePDFLimitsQueuePositive, int directionality = true);

	// Variables:
	private:
	
		struct FitPosition {
			std::vector<double> Vtx;
			double NLL;
		};
		struct SortingNLL { 
			bool operator()(FitPosition const &a, FitPosition const &b) const { 
				return a.NLL < b.NLL;
			}
		};
		
		
		struct PMTHit {
			double T;
			double Q;
			int PMT;
		};
		
		struct PMTHitExt {
			double T;
			double Q;
			int PMT;
			
			double Dist;
			double ToF;
			
			double NormX;
			double NormY;
			double NormZ;
			
			double theta;
			double phi;
		};
		
		struct SortingToF { 
			bool operator()(PMTHitExt const &a, PMTHitExt const &b) const { 
				return a.ToF < b.ToF;
			}
		};
		
		struct EventInfo {
			int hits;
			double SignaloverNoise;
			double NoiseIntegral;
			double SignalIntegral;
		};
		
		static BQFitter* myFitter;

		// Spline
		TSpline3 * fSplineTimePDFQueue[NPMT_CONFIGURATION];
		TSpline3 * fSplineTimePDFDarkRate[NPMT_CONFIGURATION];
		
		// Histo
		TH1D * hPMTDirectionality_1D[NPMT_CONFIGURATION][NGROUP_PMT];
		
		
		double fDarkRate_dir_proba[NPMT_CONFIGURATION][NGROUP_PMT];
		
		static double fSTimePDFLimitsQueueNegative; 
		static double fSTimePDFLimitsQueuePositive; 
		double fSTimePDFLimitsQueueNegative_fullTimeWindow;
		double fSTimePDFLimitsQueuePositive_fullTimeWindow;
		double fTimeWindowSizeFull;
		
		double fMinimizeLimitsNegative;
		double fMinimizeLimitsPositive;
		
		double fHitTimeLimitsNegative;
		double fHitTimeLimitsPositive;
		
		double fDarkRate_ns[NPMT_CONFIGURATION];
		
		// Detector geometry:
		double fTankRadius;
		double fTankHeight;
		double fTankHalfHeight;
		double fLightSpeed;
		double fSearchVtxStep;
		
		// Fitter parameters:
		double fIntegrationTimeWindow;
		bool   fStepByStep;
		bool   fUseDirectionality;
		bool   fLimit_mPMT;
		int    fAveraging;
		bool   fHighEnergy;
		
		// WCSim objects:
		WCSimRootGeom * fWCGeo;
		double fDarkRate_Normal;
		double fDarkRate_mPMT;

		// Input/Output
		std::vector< PMTHit > fHitInfo;
		//double fTrueVtxPos[5];
		std::vector<double> fTrueVtxPos;
		std::vector< std::vector<double> > fTrueVtxPosDouble;
		double fPDFNorm_fullTimeWindow;
		std::vector< std::vector<double> > fRecoVtxPosFinal;
		
		double fRecoVtx_X; 
		double fRecoVtx_Y; 
		double fRecoVtx_Z; 
		double fRecoVtx_T; 
		double fRecoVtx_R2; 
		double fRecoVtx_R;
		double fRecoVtx_Good;
		
		// Analysis:
		// Wall
		double fRecoVtx_Wall;
		
		// Directions
		std::vector<double> fVtxReco_dirSimple;
		std::vector<double> fVtxReco_dir;
		
		// Energy
		double fRecoVtx_E;
		double fRecoVtx_E_Simple;
		
		// DirKS
		double fVtxReco_dirKS;
			
		// InTime hits
		std::vector< PMTHitExt > fInTime20;
		std::vector< PMTHitExt > fInTime30;
		std::vector< PMTHitExt > fInTime50;
		
		// Random generator
		TRandom3 * fRand;
		//TRandom3 * fRand2;
				
		// PMT informations
		// Load every thing at the beginning to speed up fitting
		// mPMT tube ID start with 9000000
		std::vector< std::vector<double> > 	fPMT_Info; 
		// 0 -> PMT X
		// 1 -> PMT Y
		// 2 -> PMT Z
		// 3 -> PMT DIR X
		// 4 -> PMT DIR Y
		// 5 -> PMT DIR Z
		std::vector<int> 			fPMT_Group; 
		std::vector<int> 			fPMT_TubeInMPMT; 
		
		
		double fLastLowerLimit;
		double fLastUpperLimit;
		struct EventInfo fEventInfo[NPMT_CONFIGURATION];
		
		std::vector< std::vector<double> > fPositionList;
		
		
		std::vector< std::vector<double> > fThreadOutput;
		
		// Constant for direction fit
		double fEP10[21];
		std::vector< std::vector<double> > fHitPat;
		
		// Constant for EffHit computation
		double fEffHitLambda[12];
		int fNearPMT;
		double fOccupancyTable[10][10];
		
	private:
		// Analysis function
	
		// Load some constants needed to compute the direction
		void LoadDirConstant();						

		// Geometric function
		double GroupPMTs(int pmtType, int pmt_number);
		
		// Get number of in time hits in twindow from tof subtracted t-distribution 
		void ComputeInTimeHit(int iType=NormalPMT);
		
		// Get distance between final vertex and wall:
		void ComputeDistanceFromWall();
		
		// Return the detection efficiency for a given incident angle
		double GetPMTAngleEfficiency(double dCosPM);
		
		// Return a vector from the direction and the angles
		std::vector<double> TransformInVector(std::vector<double> lDir, double dPhi, double dCosTheta );
		
		// Return dirKS
		double GetDirKS(std::vector<double> lDir);
		
		// Return ?
		double GetDir_lkflf(double dCos, int iE, bool bSimple);
		
		// Return the direction
		// if Simple = false, energy is ignored
		// if Simple = true, energy is needed
		std::vector<double> GetDirection(double dEnergy, std::vector<PMTHitExt> tInTime, bool bSimple, double dCutOff);
		
		
		void MakeAnalysis(int iType);

	//ClassDef(BQFitter,1)  
};

#endif