#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <hardware/custom.h>
#include <hardware/cia.h>

struct WaveData
{
	char *data;
	int len;
	int nChannels;
	int frequency;
};

extern struct Custom custom;
extern struct CIA ciaa;
struct audioSAGA
{
	char *data;
	unsigned int numLongs;
	unsigned short volume;
	unsigned short ctrl;
	unsigned short period;
};

struct ChunkHeader
{
	unsigned char ckID[4];
	long ckSize;
};

struct ChunkFileHeader
{
	struct ChunkHeader hdr;
	unsigned char formType[4];
};
struct CommChunk
{
	unsigned short numChannels;
	unsigned long numSampleFrames;
	unsigned short sampleSize;
	float sampleRate;
};

void playMusic(struct WaveData *waveData)
{
	volatile struct audioSAGA *soundHW = (struct audioSAGA *)0xdff400;

	soundHW->data = waveData->data;
	soundHW->numLongs = waveData->len / 4;
	soundHW->volume = 0x4040;
	soundHW->ctrl = 0x0001;
	soundHW->period = 3546890 / (waveData->frequency);

	custom.dmacon = 0x8001;
}
int channel = 1;

void playSound(struct WaveData *waveData)
{
	volatile struct audioSAGA *soundHW;
	if (channel == 1)
	{
		soundHW = (struct audioSAGA *)0xdff410;
	}
	else if (channel == 2)
	{
		soundHW = (struct audioSAGA *)0xdff420;
	}
	else
	{
		soundHW = (struct audioSAGA *)0xdff430;
	}

	soundHW->data = waveData->data;
	soundHW->numLongs = waveData->len / 4;
	soundHW->volume = 0x4040;
	soundHW->ctrl = 0x0003;
	soundHW->period = 3546890 / (waveData->frequency);

	if (channel == 1)
	{
		custom.dmacon = 0x8002;
		channel = 2;
	}
	else if (channel == 2)
	{
		custom.dmacon = 0x8004;
		channel = 3;
	}
	else
	{
		custom.dmacon = 0x8008;
		channel = 1;
	}

}


void floatFromExtended(unsigned char *extended __asm("a0"), float *result __asm("a1"));

struct ExtendedFloat
{
	uint16_t sign : 1;
	uint16_t exponent : 15;
	uint64_t mantissa;
};

struct WaveData *loadMusic(const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if(!fp)
		return NULL;
	struct ChunkFileHeader fileHdr;
	fread(&fileHdr, sizeof(fileHdr), 1, fp);
	if(memcmp(fileHdr.hdr.ckID, "FORM", 4) || memcmp(fileHdr.formType, "AIFF", 4))
	{
		fclose(fp);
		return NULL;
	}

	struct CommChunk commChunk;
	struct WaveData *waveData;
	waveData = (struct WaveData *)malloc(sizeof(struct WaveData));
	memset(waveData, 0, sizeof(struct WaveData));
	
	while(!ftell(fp) < (fileHdr.hdr.ckSize + 8) && !feof(fp))
	{
		struct ChunkHeader chunkHdr;
		if(fread(&chunkHdr, sizeof(chunkHdr), 1, fp) != 1)
		{
			break;
		}

		if(!memcmp(chunkHdr.ckID, "COMM", 4))
		{
			fread(&commChunk.numChannels, sizeof(short), 1, fp);
			fread(&commChunk.numSampleFrames, sizeof(long), 1, fp);
			fread(&commChunk.sampleSize, sizeof(short), 1, fp);

			struct ExtendedFloat extFloat;
			fread(&extFloat, sizeof(extFloat), 1, fp);

			commChunk.sampleRate = 44100.0f;
//			floatFromExtended((unsigned char *)&extFloat, &commChunk.sampleRate);
		}
		else if(!memcmp(chunkHdr.ckID, "SSND", 4))
		{
			int offset, blockSize;
			fread(&offset, 1, sizeof(int), fp);
			fread(&blockSize, 1, sizeof(int), fp);
			fseek(fp, offset, SEEK_CUR);

			waveData->len = chunkHdr.ckSize - 8;
			waveData->data = (char *)malloc(waveData->len);
			int rVal = fread(waveData->data, 1, waveData->len, fp);
		}
		else
		{
			fseek(fp, chunkHdr.ckSize, SEEK_CUR);
		}
	}
	waveData->nChannels = commChunk.numChannels;
	waveData->frequency = (int)commChunk.sampleRate;
	fclose(fp);

	return waveData;
}

void FreeMusic(struct WaveData *waveData)
{
	custom.dmacon = 0x0001; // Audio off..

	if(!waveData)
		return;
	free(waveData->data);
	free(waveData);

}
#if TEST
int main(int argc, char *argv[])
{
	struct WaveData *waveData = loadMusic("main_menu_new.wav");

	if(waveData)
		playMusic(waveData);

	while(ciaa.ciapra & 0x40)
		;
	//custom.dmacon = 0x0001; // Audio off..
	FreeMusic(waveData);

	return 0;
}
#endif
