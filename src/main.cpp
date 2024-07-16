#include "nexus.h"
#include "networking.h"
#include <stdint.h>
#include <stdio.h>

#define MY_ADDON_ID  -1234
//NOTE(Rennorb): We will set these in the Load event:
// Send function: `bool PrepareAndBroadcastPacket(Packet* packet)`
PacketPrepareAndBroadcast* PrepareAndBroadcastPacket = 0;
LOGGER_LOG2 Log = 0;
#define _LOG(lvl, str) Log(ELogLevel::ELogLevel_ ## lvl, "PingPongAddon", str)
#define _LOGF(lvl, fmt, ...) { \
	char buffer[4096]; \
	snprintf(buffer, 4095, fmt, __VA_ARGS__); \
	Log(ELogLevel::ELogLevel_ ## lvl, "PingPongAddon", buffer); \
}



enum PacketType : uint16_t {
	_UNKNOWN = 0,
	Ping = 1,
	Pong = 2,
};

// Extend the basic header.
struct MyPacketHeader : PacketHeader {
	// If we want to have multiple packets, we need to differentiate them somehow.
	PacketType PacketType;
};
static_assert(sizeof(MyPacketHeader) == 8);

// Create a custom Packet struct you want to send:
struct MyPacket {
	MyPacketHeader Header;

	uint32_t Payload; // data we want to send

	PacketChecksum CRC; // Just for sizing, we don't touch that
};



void ProcessMyKeybinds(const char* aIdentifier, bool aIsRelease)
{
	if(aIsRelease) return; // only on key press

	MyPacket ping {};
	ping.Header.TargetAddon = MY_ADDON_ID;
	ping.Header.LengthInU32s = PACKET_LEN(ping);
	ping.Header.PacketType = PacketType::Ping;

	static uint32_t next = 1;
	ping.Payload = next++; // set some random payload

	PrepareAndBroadcastPacket((Packet*)&ping);
	_LOGF(TRACE, "Ping sent with payload %d", ping.Payload);
}

void HandleIncomingPacket(Packet const* _packet)
{
	auto packet = (MyPacket const*)_packet;
	switch(packet->Header.PacketType) {
	case PacketType::Ping: {
		_LOGF(INFO, "ping received with payload %d", packet->Payload);

		MyPacket response {};
		response.Header.TargetAddon = MY_ADDON_ID;
		response.Header.LengthInU32s = PACKET_LEN(response);
		response.Header.PacketType = PacketType::Pong;

		response.Payload = packet->Payload; //copy over the payload

		PrepareAndBroadcastPacket((Packet*)&response);
		_LOGF(TRACE, "pong sent with copied payload %d", response.Payload);
	} break;

	case PacketType::Pong: {
		_LOGF(INFO, "pong received with payload %d", packet->Payload);
	} break;


	default:
		_LOGF(TRACE, FORMAT_DISCARD_MESSAGE(packet));
	}
}



// Use the load function to get a function for sending packets, and to install a callback for receiving packets
void MyAddonLoad(AddonAPI* api)
{
	Log = api->Log;
	/*
	PrepareAndBroadcastPacket = api->PrepareAndBroadcastPacket;
	api->HandleIncomingPacket = &HandleIncomingPacket;
	*/
	api->RegisterKeybindWithString("send_pings", &ProcessMyKeybinds, "S");
}

CapabilityRequirement MyRequiredCaps[] = {
	{"networking", UNBOUNDED_VERSION, UNBOUNDED_VERSION},
	{"plen_go_renderer", UNBOUNDED_VERSION, UNBOUNDED_VERSION},
	{},
};


// Usual addon stuff, expose the info struct...
void NOOP(){}

AddonDefinition MyAddonDefinition {
	.Signature   = MY_ADDON_ID,
	.APIVersion  = NEXUS_API_VERSION,
	.Name        = "Test",
	.Author      = "Rennorb",
	.Description = "Example addon with networking functionality.",
	.Load        = MyAddonLoad,
	.Unload      = NOOP, // required so we are allowed to hotreload
	.Flags       = EAddonFlags_UsesCaps,
	.RequiredCapabilities = MyRequiredCaps,
};
extern "C" __declspec(dllexport) void* __cdecl GetAddonDef() { return &MyAddonDefinition; }

// Required by windows, we don't care about it 
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) { return TRUE; }