#include "FastLLM.h"
#include "httplib.h"
#include "textconvertor.h"

USING_NS_AX;

FastLLM* FastLLM::_fastLLM = nullptr;

FastLLM* FastLLM::getInstance() 
{
	if (_fastLLM == nullptr)
	{
		_fastLLM = new FastLLM();
		if (_fastLLM && _fastLLM->init())
		{
			_fastLLM->autorelease();
			_fastLLM->retain();
		}
		else
		{
			AX_SAFE_DELETE(_fastLLM);
			_fastLLM = nullptr;
		}
	}

	return _fastLLM;
}

bool FastLLM::init()
{
	fastllm::SetThreads(8);
	chatGLM.LoadFromFile("chatglm-6b-int4.bin");

	return true;
}

std::string FastLLM::getResponse(std::string input)
{
	std::stringstream ss;
	std::string ret = chatGLM.Response(input, &ss);

	return ret;
}