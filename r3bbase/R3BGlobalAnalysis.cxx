// ------------------------------------------------------------
// -----                  R3BGlobalAnalysis               -----
// -----          Created March 1st 2016 by M.Heil       -----
// ------------------------------------------------------------

/*
 * Analysis of ToFD data of experiment s438b  
 */


#include "R3BGlobalAnalysis.h"

#include "R3BTofdCalData.h"
#include "R3BTofdMappedData.h"

#include "R3BEventHeader.h"
#include "R3BTCalEngine.h"
#include "FairRunAna.h"
#include "FairRunOnline.h"
#include "FairRuntimeDb.h"
#include "FairRootManager.h"
#include "FairLogger.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"

#include "TClonesArray.h"
#include <iostream>
using namespace std;


R3BGlobalAnalysis::R3BGlobalAnalysis()
    : FairTask("GlobalAnalysis", 1)
    , fCalItemsTofd(NULL)
    , fMappedItemsTofd(NULL)
    , fTrigger(-1)
    , fClockFreq(1. / VFTX_CLOCK_MHZ * 1000.)
    , fNEvents(0)
{
}

R3BGlobalAnalysis::R3BGlobalAnalysis(const char* name, Int_t iVerbose)
    : FairTask(name, iVerbose)
    , fCalItemsTofd(NULL)
    , fMappedItemsTofd(NULL)
    , fTrigger(-1)
    , fClockFreq(1. / VFTX_CLOCK_MHZ * 1000.)
    , fNEvents(0)
{
}

R3BGlobalAnalysis::~R3BGlobalAnalysis()
{
}

InitStatus R3BGlobalAnalysis::Init()
{

    LOG(INFO) << "R3BGlobalAnalysis::Init " << FairLogger::endl;
    
    FairRootManager* mgr = FairRootManager::Instance();    
    if (NULL == mgr) FairLogger::GetLogger()->Fatal(MESSAGE_ORIGIN, "FairRootManager not found");
    header = (R3BEventHeader*)mgr->GetObject("R3BEventHeader");

    FairRunOnline *run = FairRunOnline::Instance();


    // Tofd data
    // get access to Mapped data
    fMappedItemsTofd = (TClonesArray*)mgr->GetObject("TofdMapped");

    // get access to cal data
    fCalItemsTofd = (TClonesArray*)mgr->GetObject("TofdCal");

    // define histograms
    fh_tofd_channels1 = new TH1F("Tofd_channels1", "ToFD channels PM1", 20, 0, 20.);
    fh_tofd_channels1->GetXaxis()->SetTitle("Bar number");
    fh_tofd_channels1->GetYaxis()->SetTitle("counts");  

    fh_tofd_channels2 = new TH1F("Tofd_channels2", "ToFD channels PM2", 20, 0, 20.);
    fh_tofd_channels2->GetXaxis()->SetTitle("Bar number");
    fh_tofd_channels2->GetYaxis()->SetTitle("counts");  

    for (Int_t j = 0; j < 16; j++){
	char strName1[255];
	sprintf(strName1, "tofd_tot_bar_%d", j+1);
	char strName2[255];
	sprintf(strName2, "Tofd ToT bar %d", j+1);        
        fh_tofd_tot[j] = new TH1F(strName1, strName2, 500, 0, 50.);
        fh_tofd_tot[j]->GetXaxis()->SetTitle("ToT in ns");
        fh_tofd_tot[j]->GetYaxis()->SetTitle("counts");  
    }

    for (Int_t j = 0; j < 16; j++){
	char strName1[255];
	sprintf(strName1, "tofd_tot1_bar_%d", j+1);
	char strName2[255];
	sprintf(strName2, "Tofd ToT1 bar %d", j+1);        
        fh_tofd_tot1[j] = new TH1F(strName1, strName2, 500, 0, 50.);
        fh_tofd_tot1[j]->GetXaxis()->SetTitle("ToT in ns");
        fh_tofd_tot1[j]->GetYaxis()->SetTitle("counts");  
    }

    for (Int_t j = 0; j < 16; j++){
	char strName1[255];
	sprintf(strName1, "tofd_tot2_bar_%d", j+1);
	char strName2[255];
	sprintf(strName2, "Tofd ToT2 bar %d", j+1);        
        fh_tofd_tot2[j] = new TH1F(strName1, strName2, 500, 0, 50.);
        fh_tofd_tot2[j]->GetXaxis()->SetTitle("ToT in ns");
        fh_tofd_tot2[j]->GetYaxis()->SetTitle("counts");  
    }
    
    for (Int_t j = 0; j < 16; j++){
	char strName1[255];
	sprintf(strName1, "tofd_tot_vs_pos_bar_%d", j+1);
	char strName2[255];
	sprintf(strName2, "Tofd ToT vs pos bar %d", j+1);        
        fh_tofd_tot_vs_pos[j]= new TH2F(strName1, strName2, 200, -50, 50., 500, 0, 50.);
        fh_tofd_tot_vs_pos[j]->GetXaxis()->SetTitle("Pos in cm");
        fh_tofd_tot_vs_pos[j]->GetYaxis()->SetTitle("ToT in ns");  
    }
    
    // define canvas in which all histograms are grouped
    TCanvas *ctofd1 = new TCanvas("tofd", "tofd", 10, 10, 500, 500);
    ctofd1->Divide(1, 2);
    ctofd1->cd(1);
    fh_tofd_channels1->Draw();    
    ctofd1->cd(2);
    fh_tofd_channels2->Draw();    
    ctofd1->cd(0);
    run->AddObject(ctofd1);
 

    TCanvas *ctofd2 = new TCanvas("tofd_tot1", "tofd_tot1", 10, 10, 500, 500);
    ctofd2->Divide(4, 4);
    for (Int_t j = 1; j < 17; j++){
        ctofd2->cd(j);
        fh_tofd_tot1[j-1]->Draw();    
    }
    ctofd2->cd(0);
    run->AddObject(ctofd2);
 
    TCanvas *ctofd4 = new TCanvas("tofd_tot2", "tofd_tot2", 10, 10, 500, 500);
    ctofd4->Divide(4, 4);
    for (Int_t j = 1; j < 17; j++){
        ctofd4->cd(j);
        fh_tofd_tot2[j-1]->Draw();    
    }
    ctofd4->cd(0);
    run->AddObject(ctofd4);
 
    TCanvas *ctofd5 = new TCanvas("tofd_tot", "tofd_tot", 10, 10, 500, 500);
    ctofd5->Divide(4, 4);
    for (Int_t j = 1; j < 17; j++){
        ctofd5->cd(j);
        fh_tofd_tot[j-1]->Draw();    
    }
    ctofd5->cd(0);
    run->AddObject(ctofd5);

    TCanvas *ctofd3 = new TCanvas("tofd_tot_vs_pos", "tofd_tot_vs_pos", 10, 10, 500, 500);
    ctofd3->Divide(4, 4);
    for (Int_t j = 1; j < 17; j++){
        ctofd3->cd(j);
        fh_tofd_tot_vs_pos[j-1]->Draw("colz");        
    }
    ctofd3->cd(0);
    run->AddObject(ctofd3);


       
    return kSUCCESS;
}



void R3BGlobalAnalysis::Exec(Option_t* option)
{

    FairRootManager* mgr = FairRootManager::Instance();
    if (NULL == mgr) FairLogger::GetLogger()->Fatal(MESSAGE_ORIGIN, "FairRootManager not found");
    
    // check for requested trigger (Todo: should be done globablly / somewhere else)
    if ((fTrigger >= 0) && (header) && (header->GetTrigger() != fTrigger))
		return;



    // if mapped data of tofd are available, fill the histograms
    if(fMappedItemsTofd)
    {

      Int_t nHits = fMappedItemsTofd->GetEntriesFast();    
      // loop over hits
      for (Int_t ihit = 0; ihit < nHits; ihit++)     
      {
    	R3BTofdMappedData *mapped = (R3BTofdMappedData*)fMappedItemsTofd->At(ihit);
        if (!mapped) continue; // should not happen
     
        Int_t const iPlane = mapped->GetDetectorId(); // 1..n
        Int_t const iBar   = mapped->GetBarId();   // 1..n
        Int_t const iSide  = mapped->GetSideId();   // 1..n
        Int_t const iEdge  = mapped->GetEdgeId(); 

           
        if(iSide==1) fh_tofd_channels1->Fill(iBar);  
        if(iSide==2) fh_tofd_channels2->Fill(iBar);  
             
      }
    }

    // if calibrated data of LOS are available, fill histograms
    if(fCalItemsTofd)
    {
      // define leading and trailing edge times of PMT1 and PMT2
      Double_t t1l=0.;
      Double_t t2l=0.;
      Double_t t1t=0.;
      Double_t t2t=0.;
      
      // define time-over-threshold of PMT1 and PMT2
      Double_t tot1=0.;
      Double_t tot2=0.;
      
      Double_t veff=5.7;
      
      Int_t nHits = fCalItemsTofd->GetEntriesFast();    
      
      // loop over hits
      for (Int_t ihit = 0; ihit < nHits; ihit++)     
      {

    	  R3BTofdCalData *cal = (R3BTofdCalData*)fCalItemsTofd->At(ihit);
          if (!cal) continue; // should not happen

      Int_t const iPlane  = cal->GetDetectorId();    // 1..n
      Int_t const iBar  = cal->GetBarId();    // 1..n

          // get all times of one bar
	t1l = cal->GetTimeBL_ns();
	t1t = cal->GetTimeBT_ns();
	t2l = cal->GetTimeTL_ns();
	t2t = cal->GetTimeTT_ns();  

	  // calculate time over threshold and check if clock counter went out of range

          while(t1t - t1l < 0.) {
	    t1t=t1t+2048.*fClockFreq; 
	  }

          while(t2t-t2l < 0.) {
	    t2t=t2t+2048.*fClockFreq; 
          }
	 
       
          tot1=t1t - t1l;		      
          // negative time-over-thresholds should not happen
	  if(tot1<0) {
	          LOG(WARNING) << "Negative ToT "<< tot1<<FairLogger::endl;	
	          LOG(WARNING) << "times1: " << t1t << " " << t1l << FairLogger::endl;		  
	      }

          tot2=t2t - t2l;	
          // negative time-over-thresholds should not happen
          if(tot2<0) {
              LOG(WARNING) << "Negative ToT "<< tot2<<FairLogger::endl;              
              LOG(WARNING) << "times2: " << t2t << " " << t2l << FairLogger::endl;		 
          }
 
          fh_tofd_tot[iBar-1]->Fill(sqrt(tot1*tot2));
          fh_tofd_tot1[iBar-1]->Fill(tot1);
          fh_tofd_tot2[iBar-1]->Fill(tot2);
          if(fNEvents<600000 || fNEvents>1000000)
          fh_tofd_tot_vs_pos[iBar-1]->Fill((t1l-t2l)*veff,sqrt(tot1*tot2));
      }	 
   }  
   fNEvents += 1;
 
}

void R3BGlobalAnalysis::FinishEvent()
{
    if (fCalItemsTofd)
    {
        fCalItemsTofd->Clear();
    }
    if (fMappedItemsTofd)
    {
        fMappedItemsTofd->Clear();
    }

}

void R3BGlobalAnalysis::FinishTask()
{
    for (Int_t j = 0; j < 16; j++){
        fh_tofd_tot[j]->Write();
        fh_tofd_tot1[j]->Write();
        fh_tofd_tot2[j]->Write();
        fh_tofd_tot_vs_pos[j]->Write();
	}

}

ClassImp(R3BGlobalAnalysis)
