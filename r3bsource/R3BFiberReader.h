#ifndef R3BFIBERREADER_H
#define R3BFIBERREADER_H

#include "R3BReader.h"

struct EXT_STR_h101_FIBSIX_t;
typedef struct EXT_STR_h101_FIBSIX_t EXT_STR_h101_FIBSIX;
typedef struct EXT_STR_h101_FIBSIX_onion_t EXT_STR_h101_FIBSIX_onion;
class FairLogger;

class R3BFiberReader : public R3BReader
{
	public:
		R3BFiberReader(EXT_STR_h101_FIBSIX *, UInt_t);
		~R3BFiberReader();

		Bool_t Init(ext_data_struct_info *);
		Bool_t Read();
		void Reset();

	private:
		/* An event counter */
		//unsigned int fNEvent;
		/* Reader specific data structure from ucesb */
		EXT_STR_h101_FIBSIX* fData;
		/* Data offset */
		UInt_t fOffset;
		/* FairLogger */
		FairLogger*	fLogger;
		/* the structs of type R3BFi4MappedItem */
		TClonesArray* fArray; /**< Output array. */


	public:
		ClassDef(R3BFiberReader, 0);
};

#endif

