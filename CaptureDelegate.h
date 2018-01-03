#ifndef __CaptureDelegate__
#define __CaptureDelegate__

#include "DeckLinkAPI.h"
#include "ProberState.h"
#include "util.h"
#include "assert.h"

class CaptureDelegate : public IDeckLinkInputCallback
{
public:
	CaptureDelegate(IDeckLink* deckLink);

	virtual HRESULT QueryInterface(UNUSED REFIID iid, UNUSED LPVOID *ppv) { return E_NOINTERFACE; }
	virtual ULONG AddRef(void);
	virtual ULONG Release(void);
	virtual HRESULT VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
	virtual HRESULT VideoInputFrameArrived(IDeckLinkVideoInputFrame*, IDeckLinkAudioInputPacket*);

	virtual void Start(void);
	virtual void Stop(void);

	virtual ProberState GetState(void);
	virtual std::string GetDetectedMode(void);
	virtual std::string GetActivePort(void);

private:
	IDeckLinkDisplayMode* selectFirstDisplayMode(void);
	IDeckLinkInput* queryInputInterface(void);
	bool hasRightEyeFrame(IDeckLinkVideoInputFrame* videoFrame);

	static const BMDPixelFormat     PIXEL_FORMAT = bmdFormat8BitYUV;

	static const BMDAudioSampleRate AUDIO_SAMPLE_RATE = bmdAudioSampleRate48kHz;
	static const int                AUDIO_SAMPLE_DEPTH = 16;
	static const int                AUDIO_CHANNELS = 16;

private:
	int32_t         m_refCount;
	IDeckLink*      m_deckLink;
	IDeckLinkInput* m_deckLinkInput;
};

#endif
