CC=g++
STRIP=strip

CFLAGS=-Wno-multichar -I $(SDK_PATH) -fno-rtti
LDFLAGS=-lm -ldl -lpthread
SDK_PATH=$(HOME)/VOC/Blackmagic-DeckLink-SDK-10.9.9/Linux/include/

default: decklink-debugger

%.o : %.cpp
	$(CC) $(CFLAGS) -I $(SDK_PATH) -c $< -o $@

DeckLinkAPIDispatch.o: $(SDK_PATH)/DeckLinkAPIDispatch.cpp
DeckLinkCaptureDelegate.o: DeckLinkCaptureDelegate.cpp
decklink-debugger.o: main.cpp DeckLinkCaptureDelegate.h DeckLinkCaptureDelegateList.h

decklink-debugger: decklink-debugger.o DeckLinkAPIDispatch.o DeckLinkCaptureDelegate.o
	$(CC) $^ $(LDFLAGS) -o $@
	$(STRIP) $@

clean:
	-rm -f decklink-debugger.o
	-rm -f decklink-debugger
