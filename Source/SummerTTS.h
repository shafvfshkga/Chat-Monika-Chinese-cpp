#ifndef __SUMMERTTS_H__
#define __SUMMERTTS_H__

#include "axmol.h"
#include "SynthesizerTrn.h"
#include "utils.h"
#include "Hanz2Piny.h"
#include "hanzi2phoneid.h"

USING_NS_AX;

class SummerTTS :public Node
{
public:
	static SummerTTS* getInstance();
	virtual bool init();

	void convertAudioToWavBuf(
		char* toBuf,
		char* fromBuf,
		int totalAudioLen);
	bool generateVoice(std::string input);
private:
	static SummerTTS* _summerTTS;
	Hanz2Piny hanz2piny;

	SynthesizerTrn* synthesizer;
};

#endif