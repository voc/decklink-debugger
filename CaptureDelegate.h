#ifndef __CaptureDelegate__
#define __CaptureDelegate__

#include "DeckLinkAPI.h"
#include "util.h"

class CaptureDelegate : public IDeckLinkInputCallback
{
public:
	CaptureDelegate(IDeckLink* deckLink);

	virtual HRESULT QueryInterface(UNUSED REFIID iid, UNUSED LPVOID *ppv) { return E_NOINTERFACE; }
	virtual ULONG AddRef();
	virtual ULONG Release();
	virtual HRESULT VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
	virtual HRESULT VideoInputFrameArrived(IDeckLinkVideoInputFrame*, IDeckLinkAudioInputPacket*);

	virtual void Start();
	virtual void Stop();

	virtual bool               GetSignalDetected()    { return m_hasSignal; }
	virtual bool               IsSubDevice()          { return m_isSubDevice; }
	virtual std::string        GetDetectedMode()      { return m_detectedMode; }
	virtual BMDPixelFormat     GetPixelFormat()       { return m_pixelFormat; }
	virtual BMDVideoConnection GetActiveConnection()  { return m_activeConnection; }
	virtual void               SelectNextConnection();

	virtual IDeckLinkVideoInputFrame* GetLastFrame()  { return m_lastFrame; }

private:
	IDeckLinkDisplayMode*   queryFirstDisplayMode();
	IDeckLinkInput*         queryInputInterface();
	IDeckLinkConfiguration* queryConfigurationInterface(IDeckLink* deckLink);
	IDeckLinkConfiguration* queryConfigurationInterface();
	IDeckLinkAttributes*    queryAttributesInterface(IDeckLink* deckLink);
	IDeckLinkAttributes*    queryAttributesInterface();
	int64_t                 queryInputConnections();
	BMDVideoConnection      querySelectedConnection();

	IDeckLink*              queryDeckLinkInterfaceByPersistentId(int64_t pairedDeviceId);
	int64_t                 getPairedDeviceId();

	void setDuplexToHalfDuplexModeIfSupported();
	void setDuplexToHalfDuplexModeIfSupported(IDeckLinkAttributes* m_deckLinkAttributes, IDeckLinkConfiguration* m_deckLinkConfiguration);

private:
	static const BMDPixelFormat     PIXEL_FORMAT = bmdFormat10BitYUV;
	static const BMDVideoInputFlags VIDEO_INPUT_FLAGS = bmdVideoInputEnableFormatDetection;

	static const BMDAudioSampleRate AUDIO_SAMPLE_RATE = bmdAudioSampleRate48kHz;
	static const int                AUDIO_SAMPLE_DEPTH = 16;
	static const int                AUDIO_CHANNELS = 16;

private:
	IDeckLinkAttributes*      m_deckLinkAttributes = NULL;
	IDeckLinkConfiguration*   m_deckLinkConfiguration = NULL;

	int32_t                   m_refCount;
	int64_t                   m_decklinkConnections;
	IDeckLink*                m_deckLink;
	IDeckLinkInput*           m_deckLinkInput;
	IDeckLinkVideoInputFrame* m_lastFrame;

	bool               m_hasSignal;
	std::string        m_detectedMode;
	BMDPixelFormat     m_pixelFormat;
	BMDVideoConnection m_activeConnection;

	bool               m_isSubDevice;
	int64_t            m_pairedDeviceId;
};

#endif
