/********************************************************************
	Created:	2014/07/31  20:17
	Filename: 	RLib_TextSpeaker.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_TextSpeaker.h"

#ifdef _USE_TEXTSPEAKER

#pragma warning(disable:4996)

#include <sapi.h>           // SAPI
#include <sphelper.h>       // SAPI Helper
#include <ObjBase.h>        // COM initialization functions
#include <WinError.h>       // HRESULT, FAILED

#include "../RLib_GlobalizeString.h"
#include "../RLib_File.h"
#pragma warning(default:4996)

using namespace System::Microsoft;

#define CHECK_SPEAKER							\
		if (this->m_data == nullptr)			\
		{										\
			trace(!"TextSpeaker can't work!");  \
			return;								\
		}
#define SPEAKER reinterpret_cast<TextSpeakerData *>(this->m_data)

//-------------------------------------------------------------------------
// Class to *automatically* initialize and release COM.
// CoInitialize(Ex) is called in the constructor, and CoUninitialize is
// called in the destructor.
class ComAutoInit
{
public:
	// Initializes COM using CoInitialize.
	// On failure, signals error using AtlThrow.
	ComAutoInit()
	{
		HRESULT hr = ::CoInitialize(NULL);
		if (FAILED(hr)){
			trace(!"CoInitialize() failed in ComAutoInit constructor");
		}
	}

	// Initializes COM using CoInitializeEx.
	// On failure, signals error using AtlThrow.
	explicit ComAutoInit(__in DWORD dwCoInit)
	{
		HRESULT hr = ::CoInitializeEx(NULL, dwCoInit);
		if (FAILED(hr)){
			trace(!"CoInitializeEx() failed in ComAutoInit constructor");
		}
	}

	// Uninitializes COM using CoUninitialize.
	~ComAutoInit(){
		::CoUninitialize();
	}

	RLIB_DECLARE_DYNCREATE;
};

//-------------------------------------------------------------------------

struct TextSpeakerData
{
public:
	// COM initialization and cleanup (must precede other COM related data members)
	ComAutoInit m_comInit;

	// Text to speech engine
	CComPtr<ISpVoice> m_tts;

	// Default voice token
	CComPtr<ISpObjectToken> m_voiceToken;
public:
	RLIB_DECLARE_DYNCREATE;
};

//-------------------------------------------------------------------------

TextSpeaker::TextSpeaker()
{
	this->m_data = nullptr;

	TextSpeakerData *pdata = new TextSpeakerData();

	//
	// Create text to speech engine
	//
	HRESULT hr = pdata->m_tts.CoCreateInstance(CLSID_SpVoice);
	if (FAILED(hr)){
		trace(!"Text-to-speech creation failed.");
		return;
	}

	//
	// Get token corresponding to default voice 
	//
	hr = SpGetDefaultTokenFromCategoryId(SPCAT_VOICES, &pdata->m_voiceToken, FALSE);
	if (FAILED(hr)){
		trace(!"Can't get default voice token.");
		return;
	}

	//
	// Set default voice
	//
	hr = pdata->m_tts->SetVoice(pdata->m_voiceToken);
	if (FAILED(hr)){
		trace(!"Can't set default voice.");
		return;
	}

	this->m_data = pdata;
}

//-------------------------------------------------------------------------

TextSpeaker::~TextSpeaker()
{
	if (this->m_data != nullptr){
		delete reinterpret_cast<TextSpeakerData *>(this->m_data);
	} //if
}

//-------------------------------------------------------------------------

void TextSpeaker::Speak(String text, bool async /* = true */)
{
	CHECK_SPEAKER;
	//
	// Input text must not be empty
	//
	if (text.IsEmpty()){
		trace(!"Empty text!");
		return;
	}

	//
	// Speak input text
	//
	ULONG streamNumber;
	HRESULT hr = SPEAKER->m_tts->Speak(
		GlobalizeString(text).toUnicode(),
		(async ? SPF_IS_NOT_XML | SPF_ASYNC | SPF_PURGEBEFORESPEAK : SPF_IS_NOT_XML | SPF_PURGEBEFORESPEAK),
		&streamNumber);
	if (FAILED(hr)){
		trace(!"Speak failed.");
	}
}

//-------------------------------------------------------------------------

void TextSpeaker::Stop()
{
	CHECK_SPEAKER;
	HRESULT hr = SPEAKER->m_tts->Speak(NULL, SPF_PURGEBEFORESPEAK, 0);
	if (FAILED(hr)){
		trace(!"Stop failed.");
	}
}

//-------------------------------------------------------------------------

void TextSpeaker::Pause()
{
	CHECK_SPEAKER;
	HRESULT hr = SPEAKER->m_tts->Pause();
	if (FAILED(hr)){
		trace(!"Pause failed.");
	}
}

//-------------------------------------------------------------------------

void TextSpeaker::Resume()
{
	CHECK_SPEAKER;
	HRESULT hr = SPEAKER->m_tts->Resume();
	if (FAILED(hr)){
		trace(!"Resume failed.");
	}
}

//-------------------------------------------------------------------------

void TextSpeaker::SpeakToWav(String text, String wavfile, bool async /* = true */)
{
	CHECK_SPEAKER;

	CSpStreamFormat          OriginalFmt;
	CComPtr<ISpStreamFormat> cpOldStream;
	if (FAILED(SPEAKER->m_tts->GetOutputStream(&cpOldStream)) ||
		FAILED(OriginalFmt.AssignFormat(cpOldStream))){
		trace(!"redirect output stream failed.");
		return;
	}

	CComPtr<ISpStream> cpWavStream;
	HRESULT hr = SPBindToFile(IO::Path::ToDosPath(wavfile), SPFM_CREATE_ALWAYS, &cpWavStream,
							  &OriginalFmt.FormatId(), OriginalFmt.WaveFormatExPtr());
	if (FAILED(hr)){
		trace(!"save to file failed.");
		return;
	}

	SPEAKER->m_tts->SetOutput(cpWavStream, TRUE);
	this->Speak(text, async);
	if (async) SPEAKER->m_tts->WaitUntilDone(INFINITE);
	SPEAKER->m_tts->SetOutput(cpOldStream, FALSE);

	cpWavStream.Release();
}

#endif // _USE_TEXTSPEAKER