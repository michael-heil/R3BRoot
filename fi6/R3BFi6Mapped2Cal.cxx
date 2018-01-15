// ------------------------------------------------------------
// -----                  R3BFi6Mapped2TCal                -----
// -----          Created Feb 13th 2018 by M.Heil          -----
// ------------------------------------------------------------



#include "R3BFi6Mapped2Cal.h"

#include "R3BTCalEngine.h"
#include "R3BFibMappedData.h"
#include "R3BTCalPar.h"
#include "R3BEventHeader.h"

#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairRootManager.h"
#include "FairLogger.h"

#include "TClonesArray.h"
#include "TMath.h"

#define IS_NAN(x) TMath::IsNaN(x)

R3BFi6Mapped2Cal::R3BFi6Mapped2Cal()
	: FairTask("Fi6Tcal", 1)
	, fMappedItems(NULL)
	, fCalItems(new TClonesArray("R3BFibCalData"))
	, fNofCalItems(0)
	, fNofTcalPars(0)
	, fTcalPar(NULL)
	, fClockFreq(1. / VFTX_CLOCK_MHZ * 1000.)
{
}

R3BFi6Mapped2Cal::R3BFi6Mapped2Cal(const char* name, Int_t iVerbose)
	: FairTask(name, iVerbose)
	, fMappedItems(NULL)
	, fCalItems(new TClonesArray("R3BFibCalData"))
	, fNofCalItems(0)
	, fNofTcalPars(0)
	, fTcalPar(NULL)
	, fClockFreq(1. / VFTX_CLOCK_MHZ * 1000.)
{
}

R3BFi6Mapped2Cal::~R3BFi6Mapped2Cal()
{
    if (fCalItems)
    {
	delete fCalItems;
	fCalItems = NULL;
	fNofCalItems = 0;
    }
}

InitStatus R3BFi6Mapped2Cal::Init()
{
	if (fTcalPar == NULL)
	{
		LOG(ERROR) << "Have no TCal parameter container" << FairLogger::endl;
		return kFATAL; 
	}

	fNofTcalPars = fTcalPar->GetNumModulePar();
	if (fNofTcalPars == 0)
	{
		LOG(ERROR) << "No TCal parameters in container Fi6TCalPar" << FairLogger::endl;
		return kFATAL;
	}

	LOG(INFO) << "R3BFi6Mapped2Cal::Init : read " << fNofTcalPars << " modules" << FairLogger::endl;

	FairRootManager* mgr = FairRootManager::Instance();
	if (NULL == mgr)
		FairLogger::GetLogger()->Fatal(MESSAGE_ORIGIN, "FairRootManager not found");

	// get access to Mapped data
	fMappedItems = (TClonesArray*)mgr->GetObject("Fi6Mapped");
	if (NULL == fMappedItems)
		FairLogger::GetLogger()->Fatal(MESSAGE_ORIGIN, "Branch Fi6Mapped not found");


	// request storage of Cal data in output tree
	mgr->Register("Fi6Cal", "Land", fCalItems, kTRUE);

	return kSUCCESS;
}



// Note that the container may still be empty at this point.
void R3BFi6Mapped2Cal::SetParContainers()
{
	fTcalPar = (R3BTCalPar*)FairRuntimeDb::instance()->getContainer("Fi6TCalPar");
	if (!fTcalPar)
		LOG(ERROR) << "Could not get access to Fi6TCalPar-Container." << FairLogger::endl;
}



InitStatus R3BFi6Mapped2Cal::ReInit()
{
    SetParContainers();
    return kSUCCESS;
}


 
void R3BFi6Mapped2Cal::Exec(Option_t* option)
{
	Int_t nHits = fMappedItems->GetEntriesFast();

	for (Int_t ihit = 0; ihit < nHits; ihit++)
	{
		R3BFibMappedData* mapped = (R3BFibMappedData*)fMappedItems->At(ihit);

        Int_t iPlane = mapped->GetPlaneId();
		Int_t iFiber = mapped->GetFiberId();   
		
		LOG(DEBUG) << "Fiber: " << iFiber << " plane: "<< iPlane << FairLogger::endl;

		R3BFibCalData*  cal  = new ((*fCalItems)[fNofCalItems++]) R3BFibCalData(iPlane, iFiber);
//		R3BPaddleCalData*  cal  = new ((*fCalItems)[fNofCalItems++]) R3BPaddleCalData(iPlane,iBar);
		LOG(DEBUG) << "Fiber: " << iFiber << " plane: "<< iPlane << FairLogger::endl;

		// convert times to ns

		
		for (int tube=0;tube<2;tube++) // PM1 and PM2
			for (int edge=0;edge<2;edge++) // loop over leading and trailing edge
			{
				// check if there is indeed data for this tube and edge:
				
				// fetch calib params:
		        LOG(DEBUG) << "tube: " << tube << " edge: "<< edge << FairLogger::endl;
				R3BTCalModulePar* par = fTcalPar->GetModuleParAt(iPlane, iFiber, tube*2 + edge + 1); // 1..4
				if (!par)
				{
					LOG(INFO) << "R3BFi6Mapped2Cal::Exec : Tcal par not found, Plane: " << 
					iPlane << ", Fiber: " << iFiber << ", Tube: " << (tube+1) << FairLogger::endl;
					continue;
				}


				// Convert TDC to [ns] ...
				if (mapped->GetFineTime(tube , edge) == -1) continue;
				
	 	        LOG(DEBUG) << "Parameter: " << par->GetTimeVFTX( mapped->GetFineTime(tube , edge) ) 
	 	        << " tube: " << tube << " edge: "<< edge << FairLogger::endl;
				
				
				Double_t time_ns = par->GetTimeVFTX( mapped->GetFineTime(tube , edge) );

				if (time_ns < 0. || time_ns > fClockFreq )
				{
					LOG(ERROR) << 
					"R3BFi6Mapped2Cal::Exec : bad time in ns: Plane= " << iPlane << 
					", Fiber= " << iFiber << ",tube= " << (tube+1) << ",edge= " << edge <<
					", time in channels = " << mapped->GetFineTime(tube,edge) <<
					", time in ns = " << time_ns  << FairLogger::endl;
					continue;
				}
		
				// ... and add clock time
				time_ns = fClockFreq - time_ns + mapped->GetCoarseTime(tube , edge) * fClockFreq;
                LOG(DEBUG) << "Test: Fiber: "<<iFiber<<"  tube= "<<tube<<"  edge= "<<edge<<"  time in ns = " << time_ns  << FairLogger::endl;
				cal->SetTime(tube , edge , time_ns);

			}
	}
}

void R3BFi6Mapped2Cal::FinishEvent()
{
	if (fCalItems)
	{
		fCalItems->Clear();
		fNofCalItems = 0;
	}
}

void R3BFi6Mapped2Cal::FinishTask()
{
}

ClassImp(R3BFi6Mapped2Cal)
