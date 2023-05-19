#ifndef __FASTLLM_H__
#define __FASTLLM_H__

#include "axmol.h"
#include "chatglm.h"

USING_NS_AX;

class FastLLM :public Node
{
public:
	static FastLLM* getInstance();
	virtual bool init();

	std::string getResponse(std::string input);
private:
	static FastLLM* _fastLLM;
	fastllm::ChatGLMModel chatGLM;
};

#endif