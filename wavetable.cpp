// compile with clang++ -O2 wavetable.cpp -o wavetable
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>


typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned char u8;

typedef struct Sample {
	// fmt
	u16 formattag, nchannels;
	u32 samplerate, bytespersec;
	u16 blockalign, bitspersample;
	int datastart;
	int dataend;
	int numsamples;
	int firstsample;
	short* samples;
} Sample;
Sample s;


static inline short read_sample(Sample* s, int pos, int chan) { // resample to 32khz crudely
	if (chan >= s->nchannels) chan = 0;
	int64_t p64 = (int64_t(pos) * s->samplerate * 65536) / 32000;
	int si = int(p64 >> 16);
	if (si < 0 || si >= s->numsamples - 1)
		return 0;
	int f = (int)(p64 & 65535);
	return (s->samples[si * s->nchannels] * (65536 - f) + s->samples[si * s->nchannels + s->nchannels] * f) >> 16;
}

const char* ParseWAV(Sample* s, const char* fname) {
	memset(s, 0, sizeof(*s));
	FILE* f = fopen(fname, "rb");
	if (!f)
		return "cant open";
	u32 wavhdr[3] = { 0 };
	fread(wavhdr, 4, 3, f);
	if (wavhdr[0] != 0x46464952) return "bad header 1";
	if (wavhdr[2] != 0x45564157) return "bad header 2";
	while (!feof(f)) {
		if (fread(wavhdr, 1, 8, f) < 8) break;
		int nextchunk = ftell(f) + wavhdr[1] + (wavhdr[1] & 1);
		if (wavhdr[0] == 0x61746164) { // 'data'
			s->datastart = ftell(f);
			s->dataend = s->datastart + wavhdr[1];
			int bytespersample = s->bitspersample / 8 * s->nchannels;
			s->numsamples = wavhdr[1] / (bytespersample);
			s->firstsample = s->datastart / bytespersample;
			printf("%d channels, %d samplerate, %d bits, %d samples\r\n", s->nchannels, s->samplerate, s->bitspersample, s->numsamples);
			s->samples = new short[(s->numsamples * s->nchannels * s->bitspersample / 8 + 3) / 2];
			s->numsamples = (int)fread(s->samples, s->nchannels * s->bitspersample / 8, s->numsamples, f);
			if (s->bitspersample == 24) {
				for (int i = 0; i < s->numsamples * s->nchannels; ++i) {
					short sh = *(short*)(((char*)(s->samples)) + (i * 3 + 1));
					s->samples[i] = sh;
				}
				s->bitspersample = 16;
			}
			fclose(f);
			return 0;
		}
		else if (wavhdr[0] == 0x20746d66) { // 'fmt '
			fread(s, 1, 16, f);
			if (s->formattag != 1)
				return fclose(f), "bad formattag";
			if (s->nchannels < 1 || s->nchannels>2)
				return fclose(f), "bad channel count";
			if (s->bitspersample < 16)
				return fclose(f), "bad bits per sample";
			//	if (s->samplerate < 8000 || s->samplerate>48000)
			//		return fclose(f),"bad samplerate";
		}
		fseek(f, nextchunk, SEEK_SET);
	}
	fclose(f);
	return "error";
}

#define WAVETABLE_SIZE (1024+9) // 9 octaves, top octave is 512 samples

enum EWavetables {
	WT_SAW,
	WT_SIN,
	WT_SIN_2,
	WT_SQUARE,
	WT_FM,
	WT_SIN_FOLD1,
	WT_SIN_FOLD2,
	WT_SIN_FOLD3,
	WT_NOISE_FOLD1,
	WT_NOISE_FOLD2,
	WT_NOISE_FOLD3,
	WT_WHITE_NOISE,
	WT_UNUSED1,
	WT_UNUSED2,
	WT_UNUSED3,
	WT_UNUSED4,
	WT_UNUSED5,
	WT_LAST,
};
const char* const wavetablenames[WT_LAST] = {
	"Saw",
	"Sin",
	"miunau0",
	"miunau1",
	"miunau2",
	"miunau3",
	"miunau4",
	"miunau5",
	"miunau6",
	"miunau7",
	"miunau8",
	"miunau9",
	"miunau10",
	"miunau11",
	"miunau12",
	"miunau13",
	"miunau14",
};
#define PI 3.1415926535897932384626433832795f

static float fwavetable[WT_LAST][WAVETABLE_SIZE];
float softtri(float x) {
	x *= PI * 0.5f;
	float y = sinf(x)
		- sinf(x * 3.f) / 9.f
		+ sinf(x * 5.f) / 25.f
		- sinf(x * 7.f) / 49.f
		+ sinf(x * 9.f) / 81.f
		- sinf(x * 11.f) / 121.f
		+ sinf(x * 13.f) / 169.f
		- sinf(x * 15.f) / 225.f;
	return y * (8.f / PI / PI);
}
uint32_t wang_hash(uint32_t seed)
{
	seed = (seed ^ 61) ^ (seed >> 16);
	seed *= 9;
	seed = seed ^ (seed >> 4);
	seed *= 0x27d4eb2d;
	seed = seed ^ (seed >> 15);
	return seed;
}
float noisey(int i) {
	return (wang_hash(i) & 65535) / 32768.f - 1.f;
}
Sample cycles[16];
const char* cyclenames[17] = {
	//	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"c:/temp/ss.wav"
		0,0,
	"c0.wav",
	"c1.wav",
	"c2.wav",
	"c3.wav",
	"c4.wav",
	"c5.wav",
	"c6.wav",
	"c7.wav",
	"c8.wav",
	"c9.wav",
	"c10.wav",
	"c11.wav",
	"c12.wav",
	"c13.wav",
	"c14.wav",
};
float cyclegain[16];
bool cycledone[16];
float eval_wave(int shape, int i) {
	i &= 65535;
	if (cyclenames[shape]) {
		if (!cycledone[shape]) {
			printf("%s...\n", cyclenames[shape]);
			if (const char* err = ParseWAV(&cycles[shape], cyclenames[shape]))
				printf("ERROR READING WAV %d %s %s \n", shape, cyclenames[shape], err);
			cycledone[shape] = true;
			short max = 0;
			for (int i = 0; i < cycles[shape].numsamples; ++i) {
				short s = cycles[shape].samples[i * cycles[shape].nchannels];
				if (s < 0) s = -s;
				if (s > max) max = s;
			}
			cyclegain[shape] = 1.f / max;
		}
		if (cycles[shape].numsamples > 100) {
			int i0 = (i * cycles[shape].numsamples) / 65536;
			int i1 = (i0 + 1);
			if (i1 >= cycles[shape].numsamples) i1 = 0;
			float s0 = cycles[shape].samples[i0 * cycles[shape].nchannels] * cyclegain[shape];
			float s1 = cycles[shape].samples[i1 * cycles[shape].nchannels] * cyclegain[shape];
			float t = ((i * cycles[shape].numsamples) & 65535) * (1.f / 65536.f);
			return s0 + (s1 - s0) * t;
		}
	}
	float ph = i * (PI * 2.f / 65536.f);
	float ns = 0.f;
	int seed = 0;
	float f = 1.f;
	if (shape > 4) for (int oct = 15; oct >= 2; --oct) {
		ns += noisey((i >> oct) + seed) * f;
		f *= 0.5f;
		seed += 232532;
	}
	switch (shape) {
	case WT_SAW:
		return (i - 32768) * (1.f / 32768.f); // saw
	case WT_SQUARE:
		return (i < 32768) ? -1.f : 1.f; // square
	case WT_SIN:
		return cosf(ph); // sin
	case WT_SIN_2:
		return cosf(ph) * 0.75f + cosf(ph * 3.f) * 0.5f; // sin
	case WT_FM:
		return cosf(ph + sinf(ph * 7.f + sinf(ph * 11.f) * 0.1f) * 0.7f); // fm thing
	case WT_SIN_FOLD1:
		return softtri(sinf(ph) * 2.f); // folded sin
	case WT_SIN_FOLD2:
		return softtri(sinf(ph) * 6.f); // folded sin
	case WT_SIN_FOLD3:
		return softtri(sinf(ph) * 12.f); // folded sin
	case WT_NOISE_FOLD1:
		return softtri(sinf(ph) + 2.f * ns); // folded noise
	case WT_NOISE_FOLD2:
		return softtri(sinf(ph) + 8.f * ns); // folded noise
	case WT_NOISE_FOLD3:
		return softtri(sinf(ph) + 32.f * ns); // folded noise
	case WT_WHITE_NOISE:
		return ((wang_hash(i) & 65535) - 32768) * (1.f / 32768.f);
	default:
		return 0.f;
	}

	//	return ns; // noise

}

int main(int argc, char **argv)
{
	float kernel[256];
	float totk = 0.f;
	for (int i = 0; i < 256; ++i) {
		float x = i * PI / 28.f; // 32 is good. 24 may be a bit more open
		kernel[i] = i ? (0.5f + 0.5f * cosf(i * PI / 256.f)) * sinf(x) / x : 1.f;
		totk += kernel[i] * (i ? 2.f : 1.f);
	}
	for (int i = 0; i < 256; ++i) {
		kernel[i] /= totk;
	}
	// memoize the shapes :)
	static float wshape[WT_LAST][65536];
	for (int shape = 0; shape < WT_LAST; ++shape)
		for (int i = 0; i < 65536; ++i)
			wshape[shape][i] = eval_wave(shape, i);
	FILE* fh = fopen("wavetable.h", "w");
	if (fh) {
		fprintf(fh, "// generated for WT_LAST %d WAVETABLE_SIZE %d \nconst short wavetable[%d][%d]={\n", WT_LAST, WAVETABLE_SIZE, WT_LAST, WAVETABLE_SIZE);
	}
	for (int shape = 0; shape < WT_LAST; ++shape) {
		//s16* dst = wavetable + shape * WAVETABLE_SIZE;
		float* fdst = fwavetable[shape];
		if (fh) fprintf(fh, "\t// %s\n\t{", wavetablenames[shape]);
		for (int octave = 0; octave < 9; ++octave) {
			int n = (512 >> octave);
			for (int i = 0; i <= n; ++i) {
				float x = 0.f;
				for (int j = -255; j < 256; ++j) {
					x += kernel[abs(j)] * wshape[shape][65535 & ((i << (octave + 7)) + (j << (octave + 2)))];
				}
				*fdst++ = x;
				short s = (short)(/**dst++ = */ x * (16384.f));
				if (fh) fprintf(fh, "%d,", s);
			}
			if (fh) fprintf(fh, "\n\t");
		}
		if (fh) fprintf(fh, "},");

	}
	if (fh) {
		fprintf(fh, "\n};\n");
		fclose(fh);
	}

    return 0;
}


