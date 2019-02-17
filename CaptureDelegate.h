#ifndef __CaptureDelegate__
#define __CaptureDelegate__

#include "DeckLinkAPI.h"

#include "RefReleaser.hpp"
#include "util.h"

class CaptureDelegate : public IDeckLinkInputCallback
{
public:
	CaptureDelegate(IDeckLink* deckLink, IDeckLinkInput* deckLinkInput);

	virtual HRESULT QueryInterface(UNUSED REFIID iid, UNUSED LPVOID *ppv) { return E_NOINTERFACE; }
	virtual ULONG AddRef();
	virtual ULONG Release();

	virtual HRESULT VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
	virtual HRESULT VideoInputFrameArrived(IDeckLinkVideoInputFrame*, IDeckLinkAudioInputPacket*);

	virtual void Start();
	virtual void Stop();

	virtual bool               GetSignalDetected()    { return m_hasSignal; }
	virtual std::string        GetDetectedMode()      { return m_detectedMode; }
	virtual BMDPixelFormat     GetPixelFormat()       { return m_pixelFormat; }
	virtual BMDVideoConnection GetActiveConnection()  { return m_activeConnection; }
	virtual void               SelectNextConnection();

	virtual IDeckLinkVideoInputFrame* GetLastFrame()  { return m_lastFrame; }

private:
	ULONG m_refCount;

	IDeckLinkDisplayMode*   queryFirstDisplayMode();
	IDeckLinkConfiguration* queryConfigurationInterface();
	IDeckLinkAttributes*    queryAttributesInterface();

	int64_t                 queryInputConnections();
	BMDVideoConnection      querySelectedConnection();

	void setDuplexToHalfDuplexModeIfSupported();
	static void setDuplexToHalfDuplexMode(IDeckLink *deckLink);

private:
	static const BMDPixelFormat     PIXEL_FORMAT = bmdFormat10BitYUV;
	static const BMDVideoInputFlags VIDEO_INPUT_FLAGS = bmdVideoInputEnableFormatDetection;

	static const BMDAudioSampleRate AUDIO_SAMPLE_RATE = bmdAudioSampleRate48kHz;
	static const int                AUDIO_SAMPLE_DEPTH = 16;
	static const int                AUDIO_CHANNELS = 16;

private:
	IDeckLink*                m_deckLink;
	IDeckLinkInput*           m_deckLinkInput;

	IDeckLinkVideoInputFrame*             m_lastFrame;
	RefReleaser<IDeckLinkVideoInputFrame> m_lastFrameReleaser;

	IDeckLinkConfiguration*             m_deckLinkConfiguration;
	RefReleaser<IDeckLinkConfiguration> m_deckLinkConfigurationReleaser;

	IDeckLinkAttributes*             m_deckLinkAttributes;
	RefReleaser<IDeckLinkAttributes> m_deckLinkAttributesReleaser;

	int64_t                   m_decklinkConnections;

	bool               m_hasSignal;
	std::string        m_detectedMode;
	BMDPixelFormat     m_pixelFormat;
	BMDVideoConnection m_activeConnection;
};

#endif
