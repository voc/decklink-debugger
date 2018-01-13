#ifndef __CaptureDelegate__
#define __CaptureDelegate__

#include <array>

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
	virtual bool               IsSubDevice(void)          { return m_isSubDevice; }
	virtual std::string        GetDetectedMode(void)      { return m_detectedMode; }
	virtual BMDPixelFormat     GetPixelFormat(void)       { return m_pixelFormat; }
	virtual BMDVideoConnection GetActiveConnection(void)  { return m_activeConnection; }
	virtual void               SelectNextConnection(void);

	virtual IDeckLinkVideoInputFrame* GetLastFrame(void)  { return m_lastFrame; }

private:
	IDeckLinkDisplayMode*   queryFirstDisplayMode(void);
	IDeckLinkInput*         queryInputInterface(void);
	IDeckLinkConfiguration* queryConfigurationInterface(IDeckLink* deckLink);
	IDeckLinkConfiguration* queryConfigurationInterface(void);
	IDeckLinkAttributes*    queryAttributesInterface(IDeckLink* deckLink);
	IDeckLinkAttributes*    queryAttributesInterface(void);
	int64_t                 queryInputConnections(void);
	BMDVideoConnection      querySelectedConnection(void);

	IDeckLink*              queryDeckLinkInterfaceByPersistentId(int64_t pairedDeviceId);
	int64_t                 getPairedDeviceId(void);

	void setDuplexToHalfDuplexModeIfSupported(void);
	void setDuplexToHalfDuplexModeIfSupported(IDeckLinkAttributes* m_deckLinkAttributes, IDeckLinkConfiguration* m_deckLinkConfiguration);

	void handleVideoFrame(IDeckLinkVideoInputFrame* videoFrame);
	void handleAudioFrame(IDeckLinkAudioInputPacket* audioFrame);

private:
	static const BMDPixelFormat     PIXEL_FORMAT = bmdFormat10BitYUV;
	static const BMDVideoInputFlags VIDEO_INPUT_FLAGS = bmdVideoInputEnableFormatDetection;

	static const BMDAudioSampleRate AUDIO_SAMPLE_RATE = bmdAudioSampleRate48kHz;
	static const int                AUDIO_SAMPLE_DEPTH_BYTES = 2;
	static const int                AUDIO_SAMPLE_DEPTH = AUDIO_SAMPLE_DEPTH_BYTES*8;
	static const int                AUDIO_CHANNELS = 16;

	static const int                ATTACK_IN_MS = 250;
	static const int                RELEASE_IN_MS = 500;

private:
	std::array<double, AUDIO_CHANNELS> GetChannelAudioLevelDbFS(void) { return m_channelAudioLevelDbFS; };

private:
	double attack_coef;
	double release_coef;

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

	std::array<uint16_t, AUDIO_CHANNELS> m_channelAudioLevel;
	std::array<double, AUDIO_CHANNELS> m_channelAudioLevelDbFS;
};

#endif
