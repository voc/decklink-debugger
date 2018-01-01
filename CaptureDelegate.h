#ifndef __CaptureDelegate__
#define __CaptureDelegate__

#include "DeckLinkAPI.h"
#include "util.h"

class CaptureDelegate : public IDeckLinkInputCallback
{
public:
	CaptureDelegate();

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(UNUSED REFIID iid, UNUSED LPVOID *ppv) { return E_NOINTERFACE; }
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE  Release(void);
	virtual HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
	virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame*, IDeckLinkAudioInputPacket*);

private:
	int32_t				m_refCount;
};

#endif
