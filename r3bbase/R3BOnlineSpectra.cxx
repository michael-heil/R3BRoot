// ------------------------------------------------------------
// -----                  R3BOnlineSpectra                -----
// -----          Created April 13th 2016 by M.Heil       -----
// ------------------------------------------------------------

/*
 * This task should fill histograms with detector variables which allow  
 * to test the detectors online
 *  
 */


#include "R3BOnlineSpectra.h"
#include "R3BLosCalData.h"
#include "R3BLosMappedData.h"
#include "R3BPaddleTamexMappedData.h"
#include "R3BPaddleCalData.h"
#include "R3BPspxMappedData.h"
#include "R3BPspxCalData.h"
#include "R3BEventHeader.h"
#include "R3BTCalEngine.h"
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


R3BOnlineSpectra::R3BOnlineSpectra()
    : FairTask("OnlineSpectra", 1)
    , fCalItemsLos(NULL)
    , fCalItemsTofd(NULL)
    , fCalItemsPspx(NULL)
    , fCalItemsPtof(NULL)
    , fMappedItemsLos(NULL)
    , fMappedItemsTofd(NULL)
    , fMappedItemsPspx(NULL)
    , fTrigger(-1)
    , fNofPlanes(4)  
    , fPaddlesPerPlane(6)    
    , flosVeffX(14.)
    , flosVeffY(14.)    
    , flosOffsetX(0.)
    , flosOffsetY(0.)
    , fClockFreq(1. / VFTX_CLOCK_MHZ * 1000.)
{
}

R3BOnlineSpectra::R3BOnlineSpectra(const char* name, Int_t iVerbose)
    : FairTask(name, iVerbose)
    , fCalItemsLos(NULL)
    , fCalItemsTofd(NULL)
    , fCalItemsPspx(NULL)
    , fCalItemsPtof(NULL)
    , fMappedItemsLos(NULL)
    , fMappedItemsTofd(NULL)
    , fMappedItemsPspx(NULL)
    , fTrigger(-1)
    , fNofPlanes(4)  
    , fPaddlesPerPlane(6)    
    , flosVeffX(14.)
    , flosVeffY(14.)    
    , flosOffsetX(0.)
    , flosOffsetY(0.)
    , fClockFreq(1. / VFTX_CLOCK_MHZ * 1000.)
{
}

R3BOnlineSpectra::~R3BOnlineSpectra()
{
}

InitStatus R3BOnlineSpectra::Init()
{

    LOG(INFO) << "R3BOnlineSpectra::Init " << FairLogger::endl;
    


	// try to get a handle on the EventHeader. EventHeader may not be 
	// present though and hence may be null. Take care when using.
	
    FairRootManager* mgr = FairRootManager::Instance();
    if (NULL == mgr)
        FairLogger::GetLogger()->Fatal(MESSAGE_ORIGIN, "FairRootManager not found");
    header = (R3BEventHeader*)mgr->GetObject("R3BEventHeader");

	// get access to Mapped data
    fMappedItemsLos = (TClonesArray*)mgr->GetObject("LosMapped");

	// get access to cal data
    fCalItemsLos = (TClonesArray*)mgr->GetObject("LosCal");
 
    TCanvas *cLos = new TCanvas("Los", "LOS", 10, 10, 500, 500);
    cLos->Divide(2, 2);
 
    fh_los_channels = new TH1F("los_channels", "LOS channels", 6, 0., 6.); 
    fh_los_tres = new TH1F("los_time_res", "LOS Time resolution", 4000, -2., 2.);
    fh_los_pos = new TH2F("los_position", "LOS xy position", 1000, -5., 5.,1000, -5., 5.);
    fh_los_pos->GetXaxis()->SetTitle("X position / cm");
    fh_los_pos->GetYaxis()->SetTitle("Y position / cm");  
    
    FairRunOnline *run = FairRunOnline::Instance();
    
    cLos->cd(1);
    fh_los_channels->Draw();
    cLos->cd(2);
    fh_los_tres->Draw();
    cLos->cd(3);
    fh_los_pos->Draw();
    cLos->cd(0);
    run->AddObject(cLos);

    fMappedItemsTofd = (TClonesArray*)mgr->GetObject("TofdMapped");


    TCanvas *cTofd_planes = new TCanvas("TOFD_planes", "TOFD planes", 10, 10, 500, 500);
    cTofd_planes->Divide(3, 2);
	
    for (Int_t j = 0; j < fNofPlanes; j++)
    {
         char strName1[255];
         sprintf(strName1, "tofd_channels_plane_%d", j);
         char strName2[255];
         sprintf(strName2, "Tofd channels plane %d", j);        
         fh_tofd_channels[j] = new TH1F(strName1, strName2, 16, -8., 8.);
         cTofd_planes->cd(j+1);
         fh_tofd_channels[j]->Draw();
    }
    
    
   
    TCanvas* cTofd[fNofPlanes];
    for (Int_t i = 0; i < fNofPlanes; i++)
    {		
        char strName[255];
        sprintf(strName, "TOFD_Plane_%d", i);
        cTofd[i] = new TCanvas(strName, "", 10, 10, 500, 500);
		cTofd[i]->Divide(3,4);
        for (Int_t j = 0; j < fPaddlesPerPlane; j++)
        {
            sprintf(strName, "ToT_Plane_%d_Bar_%d_PM_1", i, j);
            fh_tofd_TotPm1[i][j] = new TH1F(strName, "", 5000, 0., 500.);
            cTofd[i]->cd(2*j+1);
            fh_tofd_TotPm1[i][j]->Draw();
            sprintf(strName, "ToT_Plane_%d_Bar_%d_PM_2", i, j);
            fh_tofd_TotPm2[i][j] = new TH1F(strName, "", 5000, 0., 500.);
            cTofd[i]->cd(2*j+2);
            fh_tofd_TotPm1[i][j]->Draw();
           
        }
	}

    fCalItemsTofd = (TClonesArray*)mgr->GetObject("TofdCal");


//Ptof detector

    TCanvas *cPtof_plane = new TCanvas("Ptof_plane", "Ptof plane", 10, 10, 500, 500);
    cPtof_plane->Divide(1, 2);
	
    fh_ptof_channels = new TH1F("Ptof_channels", "Ptof channels", 65, 0., 65.);
    cPtof_plane->cd(1);
    fh_ptof_channels->Draw();

    fh_ptof_channels_cut = new TH1F("Ptof_channels_cut", "Ptof channels with cut", 65, 0., 65.);
    cPtof_plane->cd(2);
    fh_ptof_channels_cut->Draw();
    run->AddObject(cPtof_plane);

    TCanvas *cPtof_test = new TCanvas("Ptof_test", "Ptof test", 10, 10, 500, 500);
    cPtof_test->Divide(1, 1);
	
    fh_ptof_test1 = new TH1F("Ptof_test1", "Ptof test1", 1000, 0., 100.);
    fh_ptof_test2 = new TH1F("Ptof_test2", "Ptof test2", 1000, 0., 100.);
    cPtof_test->cd(1);
    fh_ptof_test1->Draw();
 //   cPtof_test->cd(2);
//    fh_ptof_test2->Draw();
    
    run->AddObject(cPtof_test);
   
   
    TCanvas* cPtof1;
    cPtof1 = new TCanvas("Ptof1", "Ptof1", 10, 10, 500, 500);
    cPtof1->Divide(8,8);
    for (Int_t j = 1; j < 100; j++){
        char strName[255];
        sprintf(strName, "ToT_Bar_%d_PM_1", j);
        fh_ptof_TotPm1[j] = new TH1F(strName, "", 1000, 0., 100.);
        cPtof1->cd(j);
        fh_ptof_TotPm1[j]->Draw();           
    }
    run->AddObject(cPtof1);
    
    TCanvas* cPtof2;
    cPtof2 = new TCanvas("Ptof2", "Ptof2", 10, 10, 500, 500);
    cPtof2->Divide(8,8);
    for (Int_t j = 1; j < 100; j++){
        char strName[255];
        sprintf(strName, "ToT_Bar_%d_PM_2", j);
        fh_ptof_TotPm2[j] = new TH1F(strName, "", 1000, 0., 100.);
        cPtof2->cd(j);
        fh_ptof_TotPm2[j]->Draw();           
    }
    run->AddObject(cPtof2);
	

    fCalItemsPtof = (TClonesArray*)mgr->GetObject("PtofCal");



    // Pspx Data
    fMappedItemsPspx = (TClonesArray*)mgr->GetObject("PspxMappedData");
    fCalItemsPspx = (TClonesArray*)mgr->GetObject("PspxCalData");
    
    for(UInt_t i=0;i<4;i++){
          fh_pspx_strips_psp[i] = new TH1F(Form("pspx_strips_psp%d",i), Form("Pspx strips PSP %d",i+1), 16, 1, 17); 
    
	  fh_pspx_energy_psp[i] = new TH1F(Form("pspx_energy_psp%d",i), Form("Pspx cathode energy PSP %d",i+1), 200, 0, 35000); 
	  
	  fh_pspx_multiplicity_psp[i] = new TH1F(Form("pspx_multiplicity_psp%d",i), Form("Pspx multiplicity PSP %d",i+1), 10, 0, 10); 
    }
    
    fh_pspx_strips_psp[0]->GetXaxis()->SetTitle("y position / strips with 3mm width");
    fh_pspx_strips_psp[1]->GetXaxis()->SetTitle("x position / strips with 3mm width");
    fh_pspx_strips_psp[2]->GetXaxis()->SetTitle("y position / strips with 3mm width");
    fh_pspx_strips_psp[3]->GetXaxis()->SetTitle("x position / strips with 3mm width"); 
    
    fh_pspx_pos1_strips = new TH2F("pspx_pos1_strips", "Pspx Position1", 16, 1, 17,16,1,17); 
    fh_pspx_pos2_strips = new TH2F("pspx_pos2_strips", "Pspx Position2", 16, 1, 17,16,1,17);  
    
    fh_pspx_pos1_strips->GetYaxis()->SetTitle("y position / strips with 3mm width");
    fh_pspx_pos1_strips->GetXaxis()->SetTitle("x position / strips with 3mm width"); 
    fh_pspx_pos2_strips->GetYaxis()->SetTitle("y position / strips with 3mm width");
    fh_pspx_pos2_strips->GetXaxis()->SetTitle("x position / strips with 3mm width");
    
    fh_pspx_pos1_energy = new TH2F("pspx_pos1_energy", "Pspx Position1", 32, -1, 1,32,-1,1); 
    fh_pspx_pos2_energy = new TH2F("pspx_pos2_energy", "Pspx Position2", 32, -1, 1,32,-1,1);  
    
    fh_pspx_pos1_energy->GetYaxis()->SetTitle("y position / a.u.");
    fh_pspx_pos1_energy->GetXaxis()->SetTitle("x position / a.u."); 
    fh_pspx_pos2_energy->GetYaxis()->SetTitle("y position / a.u.");
    fh_pspx_pos2_energy->GetXaxis()->SetTitle("x position / a.u.");
    
    
    fh_pspx_cor_x_strips = new TH2F("pspx_cor_x_strips", "Pspx Position1", 16, 1, 17,16,1,17); 
    fh_pspx_cor_y_strips = new TH2F("pspx_cor_y_strips", "Pspx Position2", 16, 1, 17,16,1,17);  
    
    fh_pspx_cor_x_strips->GetYaxis()->SetTitle("x position / strips with 3mm width");
    fh_pspx_cor_x_strips->GetXaxis()->SetTitle("x position / strips with 3mm width"); 
    fh_pspx_cor_y_strips->GetYaxis()->SetTitle("y position / strips with 3mm width");
    fh_pspx_cor_y_strips->GetXaxis()->SetTitle("y position / strips with 3mm width");
    
    fh_pspx_cor_x_energy = new TH2F("pspx_cor_x_energy", "Pspx Position1", 32, -1, 1,32,-1,1); 
    fh_pspx_cor_y_energy = new TH2F("pspx_cor_y_energy", "Pspx Position2", 32, -1, 1,32,-1,1);  
    
    fh_pspx_cor_x_energy->GetYaxis()->SetTitle("x position / a.u.");
    fh_pspx_cor_x_energy->GetXaxis()->SetTitle("x position / a.u."); 
    fh_pspx_cor_y_energy->GetYaxis()->SetTitle("y position / a.u.");
    fh_pspx_cor_y_energy->GetXaxis()->SetTitle("y position / a.u.");
    
    TCanvas *cpspx_position = new TCanvas("pspx_position", "pspx_position", 10, 10, 500, 500);
    cpspx_position->Divide(2, 2);
    
    cpspx_position->cd(1);
    fh_pspx_pos1_strips->Draw();
    cpspx_position->cd(2);
    fh_pspx_pos2_strips->Draw();
    cpspx_position->cd(3);
    fh_pspx_pos1_energy->Draw();
    cpspx_position->cd(4);
    fh_pspx_pos2_energy->Draw();
    cpspx_position->cd(0);
    run->AddObject(cpspx_position);
    
    
    TCanvas *cpspx_strips = new TCanvas("pspx_strips", "pspx_strips", 10, 10, 500, 500);
    cpspx_strips->Divide(2, 2);
    
    cpspx_strips->cd(1);
    fh_pspx_strips_psp[0]->Draw();
    cpspx_strips->cd(2);
    fh_pspx_strips_psp[1]->Draw();
    cpspx_strips->cd(3);
    fh_pspx_strips_psp[2]->Draw();
    cpspx_strips->cd(4);
    fh_pspx_strips_psp[3]->Draw();
    cpspx_strips->cd(0);
    run->AddObject(cpspx_strips);
    
    
    TCanvas *cpspx_energy = new TCanvas("pspx_energy", "pspx_energy", 10, 10, 500, 500);
    cpspx_energy->Divide(2, 2);
    
    cpspx_energy->cd(1);
    fh_pspx_energy_psp[0]->Draw();
    cpspx_energy->cd(2);
    fh_pspx_energy_psp[1]->Draw();
    cpspx_energy->cd(3);
    fh_pspx_energy_psp[2]->Draw();
    cpspx_energy->cd(4);
    fh_pspx_energy_psp[3]->Draw();
    cpspx_energy->cd(0);
    run->AddObject(cpspx_energy);
    
    
    TCanvas *cpspx_multiplicity = new TCanvas("pspx_multiplicity", "pspx_multiplicity", 10, 10, 500, 500);
    cpspx_multiplicity->Divide(2, 2);
    
    cpspx_multiplicity->cd(1);
    fh_pspx_multiplicity_psp[0]->Draw();
    cpspx_multiplicity->cd(2);
    fh_pspx_multiplicity_psp[1]->Draw();
    cpspx_multiplicity->cd(3);
    fh_pspx_multiplicity_psp[2]->Draw();
    cpspx_multiplicity->cd(4);
    fh_pspx_multiplicity_psp[3]->Draw();
    cpspx_multiplicity->cd(0);
    run->AddObject(cpspx_energy);
    
    TCanvas *cpspx_cor = new TCanvas("pspx_cor", "pspx_cor", 10, 10, 500, 500);
    cpspx_cor->Divide(2, 2);
    
    cpspx_cor->cd(1);
    fh_pspx_cor_x_strips->Draw();
    cpspx_cor->cd(2);
    fh_pspx_cor_y_strips->Draw();
    cpspx_cor->cd(3);
    fh_pspx_cor_x_energy->Draw();
    cpspx_cor->cd(4);
    fh_pspx_cor_y_energy->Draw();
    cpspx_strips->cd(0);
    run->AddObject(cpspx_cor);
    
    return kSUCCESS;
}



void R3BOnlineSpectra::Exec(Option_t* option)
{
    Double_t timeLos=0.;
    Double_t cher1=0.;
    Double_t cher2=0.;
    Double_t cher3=0.;
    Double_t cher4=0.;

    
    Double_t timeTofd=0;
    FairRootManager* mgr = FairRootManager::Instance();
    if (NULL == mgr)
        FairLogger::GetLogger()->Fatal(MESSAGE_ORIGIN, "FairRootManager not found");
	// check for requested trigger (Todo: should be done globablly / somewhere else)
    if ((fTrigger >= 0) && (header) && (header->GetTrigger() != fTrigger))
		return;


    if(fMappedItemsLos) 
    {
      Int_t nHits = fMappedItemsLos->GetEntriesFast();
      for (Int_t ihit = 0; ihit < nHits; ihit++)
      {
         R3BLosMappedData* hit = (R3BLosMappedData*)fMappedItemsLos->At(ihit);
         if (!hit) continue;

         // channel numbers are stored 1-based (1..n)
         Int_t iDet = hit->GetDetector(); // 1..
         Int_t iCha = hit->GetChannel();  // 1..
         fh_los_channels->Fill(iCha);				         
      }
    }

    if(fCalItemsLos)
    {
      Int_t nHits = fCalItemsLos->GetEntriesFast();    
      for (Int_t ihit = 0; ihit < nHits; ihit++)     
      {
    	  R3BLosCalData *calData = (R3BLosCalData*)fCalItemsLos->At(ihit);

          Int_t iDet=calData->GetDetector();
          //LOG(DEBUG) << "test1 : "<<flosOffsetX<<"  "<<flosOffsetY<<"  "<<flosVeffX<<"  "<<flosVeffY<<FairLogger::endl;
	      //LOG(DEBUG) <<"test2 : "<<calData->fTime_r_ns<<FairLogger::endl;

          if(iDet==1){
	          fh_los_tres->Fill((calData->fTime_r_ns+calData->fTime_l_ns)/2. -     		
	                  (calData->fTime_t_ns+calData->fTime_b_ns)/2.);
	          fh_los_pos->Fill((calData->fTime_r_ns-calData->fTime_l_ns-flosOffsetX)*flosVeffX,
	                 (calData->fTime_b_ns-calData->fTime_t_ns-flosOffsetY)*flosVeffY);
              timeLos=(calData->fTime_r_ns+calData->fTime_l_ns+calData->fTime_t_ns+calData->fTime_b_ns)/4.;		 
	      }
      }
             			
			  
    }

    if(fMappedItemsTofd)
    {
      Int_t nHits = fMappedItemsTofd->GetEntriesFast();    
      for (Int_t ihit = 0; ihit < nHits; ihit++)     
      {
    	R3BPaddleTamexMappedData *hit = (R3BPaddleTamexMappedData*)fMappedItemsTofd->At(ihit);
        if (!hit) continue; // should not happen

        Int_t iPlane = hit->GetPlaneId(); // 1..n
        Int_t iBar   = hit->GetBarId();   // 1..n

            
        if(iPlane<=fNofPlanes){
          fh_tofd_channels[iPlane-1]->Fill(iBar);
          fh_tofd_channels[iPlane-1]->Fill(-iBar-1);
        }
      }
    }

    
    if(fCalItemsTofd)
    {
      Double_t tot1=0.;
      Double_t tot2=0.;
      Double_t t1l=0.;
      Double_t t2l=0.;
      Double_t t1t=0.;
      Double_t t2t=0.;
     
      Int_t nHits = fCalItemsTofd->GetEntriesFast();    
      
      for (Int_t ihit = 0; ihit < nHits; ihit++)     
      {
    	  R3BPaddleCalData *hit = (R3BPaddleCalData*)fCalItemsTofd->At(ihit);
          if (!hit) continue; // should not happen

          Int_t iPlane  = hit->GetPlane();    // 1..n
          Int_t iBar  = hit->GetBar();    // 1..n

                 // get all times of one bar
          t1l=hit->fTime1L_ns;
          t2l=hit->fTime2L_ns;
          t1t=hit->fTime1T_ns;
          t2t=hit->fTime2T_ns;

	      // calculate time over threshold and check if clock counter went out of range
          while(t1t - t1l < 0.) {
	          t1t=t1t+2048.*fClockFreq; 
	      }

          while(t2t-t2l < 0.) {
	          t2t=t2t+2048.*fClockFreq; 
          }
	      while(t1l-timeLos<0.){
	          t1t=t1t+2048.*fClockFreq; 
	          t1l=t1l+2048.*fClockFreq; 
	          t2t=t2t+2048.*fClockFreq; 
	          t2l=t2l+2048.*fClockFreq; 			  
		  }
       
          tot1=t1t - t1l;		      
          if(tot1<0) {
	          LOG(WARNING) << "Negative ToT "<< tot1<<FairLogger::endl;	
	          LOG(WARNING) << "times1: " << t1t << " " << t1l << FairLogger::endl;		  
	      }

          tot2=t2t - t2l;	
          if(tot2<0) {
              LOG(WARNING) << "Negative ToT "<< tot2<<FairLogger::endl;              
              LOG(WARNING) << "times2: " << t2t << " " << t2l << FairLogger::endl;		 
          }
 
          fh_tofd_TotPm1[iPlane-1][iBar-1]->Fill(tot1);
          fh_tofd_TotPm2[iPlane-1][iBar-1]->Fill(tot2);
        	
      }	

    }

    if(fCalItemsPtof)
    {
      Double_t tot1=0.;
      Double_t tot2=0.;
      Double_t t1l=0.;
      Double_t t2l=0.;
      Double_t t1t=0.;
      Double_t t2t=0.;
      Bool_t bar60=false;
      Bool_t bar61=false;
      Bool_t bar62=false;
      Bool_t bar63=false;
      
      Int_t nHits = fCalItemsPtof->GetEntriesFast();    
      LOG(DEBUG) << "nHits: " << nHits << FairLogger::endl;
      for (Int_t ihit = 0; ihit < nHits; ihit++)     
      {
    	  R3BPaddleCalData *hit = (R3BPaddleCalData*)fCalItemsPtof->At(ihit);
          if (!hit) continue; // should not happen

          Int_t iBar  = hit->GetBar();    // 1..n
                 // get all times of one bar
          t1l=hit->fTime1L_ns;
          t2l=hit->fTime2L_ns;
          t1t=hit->fTime1T_ns;
          t2t=hit->fTime2T_ns;
//          if(!(t1l>0 && t2l>0 && t1t>0 && t2t>0)) continue;
          fh_ptof_channels->Fill(iBar);
      LOG(DEBUG) << "Bar: " << iBar << FairLogger::endl;
      LOG(DEBUG) << "times PM1: " << t1l<<"  "<<t1t<<"  "<<t1t-t1l << FairLogger::endl;
      LOG(DEBUG) << "times PM2: " << t2l<<"  "<<t2t<<"  "<<t2t-t2l << FairLogger::endl;
          if(iBar==60) bar60=true;
          if(iBar==61) bar61=true;
          if(iBar==62) bar62=true;
          if(iBar==63) bar63=true;
          
	      // calculate time over threshold and check if clock counter went out of range
          while(t1t - t1l < 0.) {
	          t1t=t1t+2048.*fClockFreq; 
	      }

          while(t2t-t2l < 0.) {
	          t2t=t2t+2048.*fClockFreq; 
          }
	      while(t1l-timeLos<0.){
	          t1t=t1t+2048.*fClockFreq; 
	          t1l=t1l+2048.*fClockFreq; 
	          t2t=t2t+2048.*fClockFreq; 
	          t2l=t2l+2048.*fClockFreq; 			  
		  }
       
          tot1=t1t - t1l;		      
          if(tot1<0) {
	          LOG(WARNING) << "Negative ToT "<< tot1<<FairLogger::endl;	
	          LOG(WARNING) << "times1: " << t1t << " " << t1l << FairLogger::endl;		  
	      }

          tot2=t2t - t2l;	
          if(tot2<0) {
              LOG(WARNING) << "Negative ToT "<< tot2<<FairLogger::endl;              
              LOG(WARNING) << "times2: " << t2t << " " << t2l << FairLogger::endl;		 
          }
 
//          fh_ptof_TotPm1[iBar]->Fill(tot1);
//          fh_ptof_TotPm2[iBar]->Fill(tot2);
          if(iBar==33) fh_ptof_test1->Fill(sqrt(tot1*tot1));	
      }	
      
      
      //once again
      
      
      nHits = fCalItemsPtof->GetEntriesFast();    
//      LOG(DEBUG) << "nHits: " << nHits << FairLogger::endl;
      for (Int_t ihit = 0; ihit < nHits; ihit++)     
      {
    	  R3BPaddleCalData *hit = (R3BPaddleCalData*)fCalItemsPtof->At(ihit);
          if (!hit) continue; // should not happen

          Int_t iBar  = hit->GetBar();    // 1..n
                 // get all times of one bar
          t1l=hit->fTime1L_ns;
          t2l=hit->fTime2L_ns;
          t1t=hit->fTime1T_ns;
          t2t=hit->fTime2T_ns;

          
	      // calculate time over threshold and check if clock counter went out of range
          while(t1t - t1l < 0.) {
	          t1t=t1t+2048.*fClockFreq; 
	      }

          while(t2t-t2l < 0.) {
	          t2t=t2t+2048.*fClockFreq; 
          }
	      while(t1l-timeLos<0.){
	          t1t=t1t+2048.*fClockFreq; 
	          t1l=t1l+2048.*fClockFreq; 
	          t2t=t2t+2048.*fClockFreq; 
	          t2l=t2l+2048.*fClockFreq; 			  
		  }
       
          tot1=t1t - t1l;		      
          if(tot1<0) {
//	          LOG(WARNING) << "Negative ToT "<< tot1<<FairLogger::endl;	
//	          LOG(WARNING) << "times1: " << t1t << " " << t1l << FairLogger::endl;		  
	      }

          tot2=t2t - t2l;	
          if(tot2<0) {
//              LOG(WARNING) << "Negative ToT "<< tot2<<FairLogger::endl;              
//              LOG(WARNING) << "times2: " << t2t << " " << t2l << FairLogger::endl;		 
          }
 

          if(bar60 && bar61 && bar62 && bar63){
             fh_ptof_TotPm1[iBar]->Fill(tot1);
             fh_ptof_TotPm2[iBar]->Fill(tot2);
	         if(iBar==7) fh_ptof_test2->Fill(sqrt(tot1*tot2));	
             fh_ptof_channels_cut->Fill(iBar);

	      }

      }

   }

    if(fCalItemsPspx)
    {      
      Int_t nHits = fCalItemsPspx->GetEntriesFast();    
      Double_t max_energy1[4] = {0,0,0,0};
      Double_t max_energy2[4] = {0,0,0,0};
      Double_t max_strip[4] = {0,0,0,0};
      
      for (Int_t ihit = 0; ihit < nHits; ihit++)     
      {
	R3BPspxCalData *calData = (R3BPspxCalData*)fCalItemsPspx->At(ihit);
	if(calData->GetEnergy1()>max_energy1[calData->GetDetector()-1] && calData->GetStrip()!=17){
	  max_energy1[calData->GetDetector()-1]=calData->GetEnergy1();
	  max_energy2[calData->GetDetector()-1]=calData->GetEnergy2();
	  max_strip[calData->GetDetector()-1]=calData->GetStrip();
	} 
	
	
	if(calData->GetDetector()==1){
	  if(calData->GetStrip()==17){
	    fh_pspx_energy_psp[0]->Fill(calData->GetEnergy1());  
	  }
	} else if(calData->GetDetector()==2){
	  if(calData->GetStrip()==17){
	    fh_pspx_energy_psp[1]->Fill(calData->GetEnergy1());  
	  }
	} else if(calData->GetDetector()==3){ 
	  if(calData->GetStrip()==17){
	    fh_pspx_energy_psp[2]->Fill(calData->GetEnergy1());  
	  }
	} else if(calData->GetDetector()==4){
	  if(calData->GetStrip()==17){
	    fh_pspx_energy_psp[3]->Fill(calData->GetEnergy1());  
	  }
	} 
      }
      
      fh_pspx_strips_psp[0]->Fill(max_strip[0]);  
      fh_pspx_strips_psp[1]->Fill(max_strip[1]);  
      fh_pspx_strips_psp[2]->Fill(max_strip[2]);  
      fh_pspx_strips_psp[3]->Fill(max_strip[3]);  
      
      if(max_energy1[0]!=0 && max_energy1[1]!=0){
	  fh_pspx_pos1_strips->Fill(max_strip[1],max_strip[0]); 
	  if(max_energy2[0]!=0 && max_energy2[1]!=0){
	      fh_pspx_pos1_energy->Fill(-(max_energy1[0]-max_energy2[0])/(max_energy1[0]+max_energy2[0]),(max_energy1[1]-max_energy2[1])/(max_energy1[1]+max_energy2[1]));
	  }
      }
      if(max_energy1[2]!=0 && max_energy1[3]!=0){
	  fh_pspx_pos2_strips->Fill(max_strip[3],max_strip[2]);
	  if(max_energy2[2]!=0 && max_energy2[3]!=0){
	      fh_pspx_pos2_energy->Fill(-(max_energy1[2]-max_energy2[2])/(max_energy1[2]+max_energy2[2]),-(max_energy1[3]-max_energy2[3])/(max_energy1[3]+max_energy2[3]));
	  }
      }
      
      if(max_energy1[1]!=0 && max_energy1[3]!=0){
	  fh_pspx_cor_x_strips->Fill(max_strip[1],max_strip[3]);
	  if(max_energy2[1]!=0 && max_energy2[3]!=0){
	      fh_pspx_cor_x_energy->Fill(-(max_energy1[1]-max_energy2[1])/(max_energy1[1]+max_energy2[1]),-(max_energy1[3]-max_energy2[3])/(max_energy1[3]+max_energy2[3]));
	  }
      }
      if(max_energy1[0]!=0 && max_energy1[2]!=0){
	  fh_pspx_cor_y_strips->Fill(max_strip[0],max_strip[2]);
	  if(max_energy2[0]!=0 && max_energy2[2]!=0){
	      fh_pspx_cor_y_energy->Fill(-(max_energy1[0]-max_energy2[0])/(max_energy1[0]+max_energy2[0]),-(max_energy1[2]-max_energy2[2])/(max_energy1[2]+max_energy2[2]));
	  }
      }
      
    }

   
    if(fMappedItemsPspx)
    {
      Int_t mult1=0;
      Int_t mult2=0;
      Int_t mult3=0;
      Int_t mult4=0;
      
      Int_t nHits = fMappedItemsPspx->GetEntriesFast();    
      
      for (Int_t ihit = 0; ihit < nHits; ihit++)     
      {
	R3BPspxMappedData *mappedData = (R3BPspxMappedData*)fMappedItemsPspx->At(ihit);
	if(mappedData->GetDetector()==1){
	  mult1++;
	} else if(mappedData->GetDetector()==2){
	  mult2++;
	} else if(mappedData->GetDetector()==3){
	  mult3++;
	} else if(mappedData->GetDetector()==4){
	  mult4++;
	} 
	
      }
      fh_pspx_multiplicity_psp[0]->Fill(mult1);
      fh_pspx_multiplicity_psp[1]->Fill(mult2);
      fh_pspx_multiplicity_psp[2]->Fill(mult3);
      fh_pspx_multiplicity_psp[3]->Fill(mult4);
      
    }
 
}

void R3BOnlineSpectra::FinishEvent()
{
    if (fCalItemsLos)
    {
        fCalItemsLos->Clear();
    }
    if (fCalItemsTofd)
    {
        fCalItemsTofd->Clear();
    }
    if (fCalItemsPtof)
    {
        fCalItemsPtof->Clear();
    }
    if (fMappedItemsLos)
    {
        fMappedItemsLos->Clear();
    }
    if (fMappedItemsTofd)
    {
        fMappedItemsTofd->Clear();
    }
    if (fMappedItemsPspx)
    {
        fMappedItemsPspx->Clear();
    }
    if (fMappedItemsPspx)
    {
        fMappedItemsPspx->Clear();
    }

}

void R3BOnlineSpectra::FinishTask()
{
}

ClassImp(R3BOnlineSpectra)
