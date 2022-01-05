#include "globals.h"
#include "fluxmap.h"
#include "decoders/fluxmapreader.h"
#include "protocol.h"
#include "decoders/decoders.h"
#include "sector.h"
#include "victor9k.h"
#include "crc.h"
#include "bytes.h"
#include "fmt/format.h"
#include <string.h>
#include <algorithm>


//todo, delte me
#include <stdio.h>
#include <unistd.h>
const FluxPattern SECTOR_RECORD_PATTERN(32, VICTOR9K_SECTOR_RECORD);
const FluxPattern DATA_RECORD_PATTERN(32, VICTOR9K_DATA_RECORD);
const FluxMatchers ANY_RECORD_PATTERN({ &SECTOR_RECORD_PATTERN, &DATA_RECORD_PATTERN });

static int decode_data_gcr(uint8_t gcr)
{
    switch (gcr)
    {
		#define GCR_ENTRY(gcr, data) \
			case gcr: return data;
		#include "data_gcr.h"
		#undef GCR_ENTRY
    }
    return -1;
}

static Bytes decode(const std::vector<bool>& bits)
{
    Bytes output;
    ByteWriter bw(output);
    BitWriter bitw(bw);

    auto ii = bits.begin();
    while (ii != bits.end())
    {
        uint8_t inputfifo = 0;
        for (size_t i=0; i<5; i++)
        {
            if (ii == bits.end())
                break;
            inputfifo = (inputfifo<<1) | *ii++;
        }

        uint8_t decoded = decode_data_gcr(inputfifo);
        bitw.push(decoded, 4);
    }
    bitw.flush();

    return output;
}

class Victor9kDecoder : public AbstractDecoder
{
public:
	Victor9kDecoder(const DecoderProto& config):
		AbstractDecoder(config)
	{}

    RecordType advanceToNextRecord()
	{
		const FluxMatcher* matcher = nullptr;
		_sector->clock = _fmr->seekToPattern(ANY_RECORD_PATTERN, matcher);
		if (matcher == &SECTOR_RECORD_PATTERN)
			return SECTOR_RECORD;
		if (matcher == &DATA_RECORD_PATTERN)
			return DATA_RECORD;
		return UNKNOWN_RECORD;
	}

    void decodeSectorRecord()
	{
		/* Skip the sync marker bits, a series of 1s. */
		while (readRawBits(1)[0]) { };

		//get next bits and recover above false bit from while exit 
		auto firstBytes = readRawBits(9 + (3*10));
		firstBytes.insert(firstBytes.begin(), false);
	
		/* Read header. */
		auto bytes = decode(firstBytes).slice(0, 4);
    	
		uint8_t headerId = bytes[0];

if (headerId != VICTOR9K_HEADER_ID) {
			fprintf(stdout, "didn\'t match VICTOR9K_HEADER_ID\n");
			return;
		} else {
			fprintf(stdout, "did match VICTOR9K_HEADER_ID\n");
}

		uint8_t rawTrack = bytes[1];
		_sector->logicalSector = bytes[2];
		uint8_t gotChecksum = bytes[3];
fprintf(stdout, "headerId: %i, rawTrack: %i, gotChecksum: %i\n",
	headerId, rawTrack, gotChecksum);
		_sector->logicalTrack = rawTrack & 0x7f;
		_sector->logicalSide = rawTrack >> 7;
		uint8_t wantChecksum = bytes[1] + bytes[2];
fprintf(stdout, "logicalSide: %i logicalTrack: %i logicalSector: %i \n",
			 _sector->logicalSide, _sector->logicalTrack , _sector->logicalSector);
		if ((_sector->logicalSector > 20) || (_sector->logicalTrack > 85) || (_sector->logicalSide > 1))
			return;

		//  unintuitive but correct 
		// _sector->status = (gotChecksum == wantChecksum) ? _sector->status = Sector::DATA_MISSING : Sector::BAD_CHECKSUM; 
		if (wantChecksum == gotChecksum) {
			_sector->status = Sector::DATA_MISSING; /* unintuitive but correct */
		} else {
			_sector->status = Sector::BAD_CHECKSUM; 	
		}
	}

    void decodeDataRecord()
	{
		/* Skip the sync marker bits, a series of 1s. */
		while (readRawBits(1)[0]) { };

		/* Read data header. */
		auto dataIdBits = readRawBits(9);
		dataIdBits.insert(dataIdBits.begin(), false);   //recover above false bit from while exit 

		auto dataIdByte = decode(dataIdBits).slice(0, 1);
std::cout<<"dataIdByte: "; 
		/* Check that this is actually a data record. */
		if (dataIdByte[0] != VICTOR9K_DATA_ID) {
			fprintf(stdout, "didn\'t match VICTOR9K_DATA_ID\n");
			return;
		} else {
			fprintf(stdout, "did match VICTOR9K_DATA_ID\n");
		}

		/* Read data. */
		const uint8_t checksum_length = 2; 
		auto bytes = decode(readRawBits((VICTOR9K_SECTOR_LENGTH+checksum_length)*10))
			.slice(0, VICTOR9K_SECTOR_LENGTH+checksum_length);
		ByteReader br(bytes);

		_sector->data = br.read(VICTOR9K_SECTOR_LENGTH);
		uint16_t gotChecksum = sumBytes(_sector->data);
		uint16_t wantChecksum = br.read_le16();
		_sector->status = (gotChecksum == wantChecksum) ? Sector::OK : Sector::BAD_CHECKSUM;
	}
};

std::unique_ptr<AbstractDecoder> createVictor9kDecoder(const DecoderProto& config)
{
	return std::unique_ptr<AbstractDecoder>(new Victor9kDecoder(config));
}


