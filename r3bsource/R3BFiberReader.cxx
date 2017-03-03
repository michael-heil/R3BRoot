#include "FairLogger.h"

#include "TClonesArray.h"
#include "FairRootManager.h"
#include "R3BFiberReader.h"
#include "R3BFibMappedData.h"

extern "C" {
#include "ext_data_client.h"
#include "ext_h101_fibsix.h"
}

R3BFiberReader::R3BFiberReader(EXT_STR_h101_FIBSIX* data, UInt_t offset)
	: R3BReader("R3BFiberReader")
	, fData(data)
	, fOffset(offset)
	, fLogger(FairLogger::GetLogger())
    , fArray(new TClonesArray("R3BFibMappedData"))
{
}

R3BFiberReader::~R3BFiberReader()
{}

Bool_t R3BFiberReader::Init(ext_data_struct_info *a_struct_info)
{
	int ok;

	EXT_STR_h101_FIBSIX_ITEMS_INFO(ok, *a_struct_info, fOffset,
	    EXT_STR_h101_FIBSIX, 0);

	if (!ok) {
		perror("ext_data_struct_info_item");
		fLogger->Error(MESSAGE_ORIGIN,
		    "Failed to setup structure information.");
		return kFALSE;
	}

    // Register output array in tree
    FairRootManager::Instance()->Register("Fi6Mapped", "Land", fArray, kTRUE);

	return kTRUE;
}

Bool_t R3BFiberReader::Read()
{
	// Convert plain raw data to multi-dimensional array
    EXT_STR_h101_FIBSIX_onion* data = (EXT_STR_h101_FIBSIX_onion*)fData;

	// Display data
	// fLogger->Info(MESSAGE_ORIGIN, "  Event data:");


/*
  // this is the data structure we have to read:
  struct {
    uint32_t M;                 // number of channels with data
    uint32_t MI[65 / * M * /];  // channel number
    uint32_t ME[65 / * M * /];  // offset in v array for that channel
    uint32_t _;                 // num items in v
    uint32_t v[650 / * _ * /];  // the energy data
  } FIBERX[5];

*/

	
	// loop over all detectors
		
    uint32_t numChannels = data->FIBSIX_TCLM; // not necessarly number of hits! (b/c multi hit)
		
    // loop over channels
    uint32_t curChannelStart=0;     // index in v for first item of current channel
    for (unsigned int i=0;i<numChannels;i++){
	  uint32_t channel=data->FIBSIX_TCLMI[i]; // or 1..65
	  uint32_t nextChannelStart=data->FIBSIX_TCLME[i];  // index in v for first item of next channel
 	  for (unsigned int j = curChannelStart; j < nextChannelStart; j++){
/*		  
		  LOG(INFO) << "Data channel: "<< channel <<FairLogger::endl;
		  LOG(INFO) << "coarse leading: "<< data->FIBSIX_TCLv[j]<<FairLogger::endl;	
		  LOG(INFO) << "fine leading: "<< data->FIBSIX_TFLv[j]<<FairLogger::endl;	
		  LOG(INFO) << "coarse trailing: "<< data->FIBSIX_TCTv[j]<<FairLogger::endl;	
		  LOG(INFO) << "fine trailing: "<< data->FIBSIX_TFTv[j]<<FairLogger::endl;	
*/
          R3BFibMappedData* mapped = 
          new ((*fArray)[fArray->GetEntriesFast()])R3BFibMappedData(1,channel);
          
          mapped->fCoarseTime1LE = data->FIBSIX_TCLv[j];
          mapped->fFineTime1LE   = data->FIBSIX_TFLv[j];		
  	 	  mapped->fCoarseTime1TE = data->FIBSIX_TCTv[j];		
	  	  mapped->fFineTime1TE   = data->FIBSIX_TFTv[j];

      }
	  curChannelStart=nextChannelStart;
	}
}

void R3BFiberReader::Reset()
{
  // Reset the output array
  fArray->Clear();
}

ClassImp(R3BFiberReader)

