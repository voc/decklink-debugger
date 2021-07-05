#include "SubDeviceUtil.h"

#include "RefReleaser.hpp"
#include "util.h"

#define LLOG(x) LOG(x) << "SubDeviceUtil: "

bool SubDeviceUtil::IsSubDevice(IDeckLink *deckLink)
{
	LLOG(INFO) << __PRETTY_FUNCTION__;

	IDeckLink *parentDevice = QueryParentDevice(deckLink);
	RefReleaser<IDeckLink> parentDeviceReleaser(&parentDevice);

	bool isSubDevice = parentDevice != nullptr;
	LOG(DEBUG1) << "isSubDevice = " << isSubDevice;

	return isSubDevice;
}

bool SubDeviceUtil::SupportsDuplexMode(IDeckLink *deckLink)
{
	LLOG(INFO) << __PRETTY_FUNCTION__;

	HRESULT result;
	IDeckLinkProfileAttributes* deckLinkAttributes = nullptr;
	RefReleaser<IDeckLinkProfileAttributes> deckLinkAttributesReleaser(&deckLinkAttributes);

	LLOG(DEBUG1) << "querying IID_IDeckLinkProfileAttributes Interface";
	result = deckLink->QueryInterface(IID_IDeckLinkProfileAttributes, (void **)&deckLinkAttributes);
	throwIfNotOk(result, "Could not obtain the IDeckLinkProfileAttributes interface");

	LLOG(DEBUG1) << "querying BMDDeckLinkSupportsDuplexModeConfiguration flag";
	bool supportsDuplexModeConfiguration;
	//result = deckLinkAttributes->GetFlag(IDeckLinkProfileIterator, &supportsDuplexModeConfiguration);
	if(result != S_OK) {
		LLOG(DEBUG1) << "failed to query BMDDeckLinkSupportsDuplexModeConfiguration flag";
		return false;
	}

	return supportsDuplexModeConfiguration;
}

IDeckLink *SubDeviceUtil::QueryParentDevice(IDeckLink *deckLink)
{
	LLOG(INFO) << __PRETTY_FUNCTION__;

	HRESULT result;
	IDeckLinkProfileAttributes* deckLinkAttributes = nullptr;
	RefReleaser<IDeckLinkProfileAttributes> deckLinkAttributesReleaser(&deckLinkAttributes);

	LLOG(DEBUG1) << "querying IID_IDeckLinkProfileAttributes Interface";
	result = deckLink->QueryInterface(IID_IDeckLinkProfileAttributes, (void **)&deckLinkAttributes);
	throwIfNotOk(result, "Could not obtain the IDeckLinkProfileAttributes interface");

	LLOG(DEBUG1) << "querying BMDDeckLinkPairedDevicePersistentID attribute";
	int64_t pairedDeviceId;
	result = deckLinkAttributes->GetInt(BMDDeckLinkPersistentID, &pairedDeviceId);
	if(result != S_OK)
	{
		LLOG(DEBUG1) << "failed to query BMDDeckLinkPairedDevicePersistentID attribute, this is no SubDevice";
		return nullptr;
	}

	LLOG(DEBUG1) << "found paired device-id 0x" << std::hex << pairedDeviceId << ", looking device up";
	IDeckLink *pairedDevice = findDeckLinkInterfaceByPersistentId(pairedDeviceId);
	RefReleaser<IDeckLink> pairedDeviceReleaser(&pairedDevice);
	throwIfNull(pairedDevice, "did not find device for pairedDeviceId reported by Decklink-Device");

	if(!SupportsDuplexMode(pairedDevice))
	{
		LLOG(DEBUG) << "paired Device does not support Duplex-Mode and is this no Parent-Device";
		return nullptr;
	}

	LLOG(DEBUG) << "found matching Parent-Device";
	pairedDevice->AddRef();
	return pairedDevice;
}

IDeckLink *SubDeviceUtil::findDeckLinkInterfaceByPersistentId(int64_t pairedDeviceId)
{
	LLOG(INFO) << __PRETTY_FUNCTION__;

	HRESULT result;
	IDeckLinkIterator *deckLinkIterator = CreateDeckLinkIteratorInstance();
	RefReleaser<IDeckLinkIterator> iterReleaser(&deckLinkIterator);
	throwIfNull(deckLinkIterator,
		"A DeckLink iterator could not be created.");

	unsigned int i = 0;
	IDeckLink *deckLink = nullptr;
	while (deckLinkIterator->Next(&deckLink) == S_OK) {
		i++;
		LLOG(DEBUG) << "probing Device " << i;
		RefReleaser<IDeckLink> deckLinkReleaser(&deckLink);

		IDeckLinkProfileAttributes* deckLinkAttributes = nullptr;
		RefReleaser<IDeckLinkProfileAttributes> deckLinkAttributesReleaser(&deckLinkAttributes);

		LLOG(DEBUG1) << "querying IID_IDeckLinkProfileAttributes Interface";
		result = deckLink->QueryInterface(IID_IDeckLinkProfileAttributes, (void **)&deckLinkAttributes);
		throwIfNotOk(result, "Could not obtain the IDeckLinkProfileAttributes interface");

		LLOG(DEBUG1) << "querying BMDDeckLinkPersistentID attribute";
		int64_t persistent_id;
		result = deckLinkAttributes->GetInt(BMDDeckLinkPersistentID, &persistent_id);
		if(result != S_OK) {
			LLOG(DEBUG1) << "could not query the BMDDeckLinkPersistentID attribute, continuing with next device";
			continue;
		}

		if(pairedDeviceId == persistent_id)
		{
			LLOG(DEBUG) << "found matching device";
			deckLink->AddRef();
			return deckLink;
		}

		LLOG(DEBUG) << "not the device we are looking for";
	}

	LLOG(DEBUG) << "found no matching device";
	return NULL;
}
