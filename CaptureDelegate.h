#ifndef __CaptureDelegate__
#define __CaptureDelegate__

#include "DeckLinkAPI.h"
#include "util.h"

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

	virtual bool               GetSignalDetected(void)    { return m_hasSignal; }
	virtual std::string        GetDetectedMode(void)      { return m_detectedMode; }
	virtual BMDVideoConnection GetActiveConnection(void)  { return m_activeConnection; }
	virtual void               SelectNextConnection(void);

	virtual IDeckLinkVideoInputFrame* GetLastFrame(void)  { return m_lastFrame; }

private:
	IDeckLinkDisplayMode* queryFirstDisplayMode(void);
	IDeckLinkInput*       queryInputInterface(void);
	int64_t               queryInputConnections(void);
	BMDVideoConnection    querySelectedConnection(void);

private:
	static const BMDPixelFormat     PIXEL_FORMAT = bmdFormat10BitYUV;
	static const BMDVideoInputFlags VIDEO_INPUT_FLAGS = bmdVideoInputEnableFormatDetection;

	static const BMDAudioSampleRate AUDIO_SAMPLE_RATE = bmdAudioSampleRate48kHz;
	static const int                AUDIO_SAMPLE_DEPTH = 16;
	static const int                AUDIO_CHANNELS = 16;

private:
	int32_t                   m_refCount;
	int64_t                   m_decklinkConnections;
	IDeckLink*                m_deckLink;
	IDeckLinkInput*           m_deckLinkInput;
	IDeckLinkVideoInputFrame* m_lastFrame;

	bool               m_hasSignal;
	std::string        m_detectedMode;
	BMDVideoConnection m_activeConnection;
};

#endif
