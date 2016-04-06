#include "FairLogger.h"

#include "TClonesArray.h"
#include "FairRootManager.h"
#include "R3BLosReader.h"
#include "R3BLosMappedData.h"

extern "C" {
#include "ext_data_client.h"
#include "ext_h101_los.h"
#include EXP_SPECIFIC_H101_FILE
}

#define NUM_LOS_DETECTORS 2
#define NUM_LOS_CHANNELS  5

R3BLosReader::R3BLosReader(EXT_STR_h101* data)
	: R3BReader("R3BLosReader")
	, fData(data)
	, fLogger(FairLogger::GetLogger())
    , fArray(new TClonesArray("R3BLosMappedData"))
{
}

R3BLosReader::~R3BLosReader()
{}

Bool_t R3BLosReader::Init(ext_data_struct_info *a_struct_info)
{
	int ok;

	EXT_STR_h101_LOS_ITEMS_INFO(ok, *a_struct_info, EXT_STR_h101, 0);

	if (!ok) {
		perror("ext_data_struct_info_item");
		fLogger->Error(MESSAGE_ORIGIN,
		    "Failed to setup structure information.");
		return kFALSE;
	}

    // Register output array in tree
    FairRootManager::Instance()->Register("LosMapped", "Land", fArray, kTRUE);

	// clear struct_writer's output struct. Seems ucesb doesn't do that
	// for channels that are unknown to the current ucesb config.
	EXT_STR_h101_onion* data = (EXT_STR_h101_onion*)fData;
	for (int d=0;d<NUM_LOS_DETECTORS;d++)
	    data->LOS[d].TFM=0;

	// clear struct_writer's output struct. Seems ucesb doesn't do that
	// for channels that are unknown to the current ucesb config.
	EXT_STR_h101_onion* data = (EXT_STR_h101_onion*)fData;
	for (int d=0;d<NUM_LOS_DETECTORS;d++)
		for (int c=0;c<NUM_LOS_CHANNELS;c++)
			data->LOS[d]._[c].TF=0;

	return kTRUE;
}

Bool_t R3BLosReader::Read()
{
	// Convert plain raw data to multi-dimensional array
    EXT_STR_h101_onion* data = (EXT_STR_h101_onion*)fData;

/*

  struct {
    struct {
      uint32_t TF;
      uint32_t TC;
    } _[5];
  } LOS[2];
 
*/

	for (int d=0;d<NUM_LOS_DETECTORS;d++)
		for (int c=0;c<NUM_LOS_CHANNELS;c++)
		{
			uint32_t channel=data->LOS[d].TFMI[i]; // or 1..65
			uint32_t nextChannelStart=data->LOS[d].TFME[i];  // index in v for first item of next channel
			for (int j=curChannelStart;j<nextChannelStart;j++)
				new ((*fArray)[fArray->GetEntriesFast()])
					R3BLosMappedData(
						d+1,
						channel,
						data->LOS[d].TCv[j],
						data->LOS[d].TFv[j]
						); // det,channel,energy
			
			new ((*fArray)[fArray->GetEntriesFast()])
				R3BLosMappedItem(d+1,					// detector 1..n
								 c+1,					// channel  1..n
								 data->LOS[d]._[c].TC,  // coarse time
								 data->LOS[d]._[c].TF); // fine time
		}	
}

void R3BLosReader::Reset()
{
    // Reset the output array
    fArray->Clear();
}

ClassImp(R3BLosReader)

