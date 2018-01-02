#ifndef __CaptureDelegate__
#define __CaptureDelegate__

#include "DeckLinkAPI.h"
#include "util.h"

class CaptureDelegate : public IDeckLinkInputCallback
{
public:
	CaptureDelegate();

	virtual HRESULT QueryInterface(UNUSED REFIID iid, UNUSED LPVOID *ppv) { return E_NOINTERFACE; }
	virtual ULONG AddRef(void);
	virtual ULONG Release(void);
	virtual HRESULT VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
	virtual HRESULT VideoInputFrameArrived(IDeckLinkVideoInputFrame*, IDeckLinkAudioInputPacket*);

private:
	int32_t				m_refCount;
};

#endif
