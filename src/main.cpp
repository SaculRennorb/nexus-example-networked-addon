#include "nexus.h"
#include "networking.h"
#include <stdint.h>

#define MY_ADDON_ID  -1234
#define _LOG(lvl, fmt, ...) Log(ELogLevel::ELogLevel_ ## lvl, "PingPongAddon", fmt, __VA_ARGS__)

enum PacketType : uint16_t {
	_UNKNOWN = 0,
	Ping = 1,
	Pong = 2,
};

// Create a custom Packet struct you want to send:
#pragma pack(push, 1)
struct MyPacket {
	PacketHeader Header;
	PacketType PacketType; // if we want to have multiple packets, we need to differentiate them somehow

	uint32_t Payload; // data we want to send

	PacketChecksum CRC; // just for sizing, we dont touch that
};
#pragma pack(pop)

// Send function: `bool PrepareAndBroadcastPacket(Packet* packet)`
PacketPrepareAndBroadcast* PrepareAndBroadcastPacket = 0;
LOGGER_LOG2 Log = 0;

void HandleIncomingPacket(Packet const* _packet)
{
	auto packet = (MyPacket const*)_packet;
	switch(packet->PacketType) {
	case PacketType::Ping: {
		_LOG(INFO, "ping received");

		MyPacket response = {0};
		response.Header.TargetAddon = MY_ADDON_ID;
		response.Header.LengthInU32s = PACKET_LEN(response);
		response.PacketType = PacketType::Pong;

		response.Payload = packet->Payload;

		PrepareAndBroadcastPacket((Packet*)&response);
		_LOG(TRACE, "pong sent");
	} break;

	case PacketType::Pong: {
		_LOG(INFO, "pong received");
		auto payload = packet->Payload;
		//NOTE(Rennorb): For the sake of keeping it short I dont format the number into the string here, but you get the idea.
	} break;


	default:
		_LOG(TRACE, "Unknown packet received");
	}
}


void ProcessMyKeybinds(const char* aIdentifier, bool aIsRelease)
{
	if(aIsRelease) return; // only on key press

	MyPacket ping = {0};
	ping.Header.TargetAddon = MY_ADDON_ID;
	ping.Header.LengthInU32s = PACKET_LEN(ping);
	ping.PacketType = PacketType::Ping;

	ping.Payload = 1;

	PrepareAndBroadcastPacket((Packet*)&ping);
	_LOG(TRACE, "Ping sent");
}


// Use the load function to get a function for sending packets, and to install a callback for receiving packets
void MyAddonLoad(AddonAPI* api)
{
	Log = api->Log;
	PrepareAndBroadcastPacket = api->PrepareAndBroadcastPacket;
	api->HandleIncomingPacket = &HandleIncomingPacket;
	api->RegisterKeybindWithString("send_pings", &ProcessMyKeybinds, "S");
}


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
};
extern "C" __declspec(dllexport) void* __cdecl GetAddonDef() { return &MyAddonDefinition; }

// Required by windows, we don't care about it 
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) { return TRUE; }