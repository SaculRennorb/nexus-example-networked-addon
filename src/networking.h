#pragma once

/*

	This header contains the public struct definitions for packages transferred by the routing system,
	as well as the function definitions for related callbacks.

	If you want to use the system, an example usage is available at 
	https://github.com/SaculRennorb/nexus-example-networked-addon/blob/master/src/main.cpp

*/

#include <stdint.h>

typedef int32_t AddonSignature;

#pragma pack(push, 1)
typedef struct {
	/// If this is zero, the Routing will assume it to target the addon the packet originated from.
	//TODO(Rennorb): endianess?
	int32_t TargetAddon;
	/// The total length of the packet in int32's this includes teh header and crc32 at the end, so it should be
	/// 1 + round_up(0.5 + byte_len(data) / 4) + 1
	int16_t LengthInU32s;
} PacketHeader;

static_assert(sizeof(PacketHeader) == 6);

#define PACKET_LEN(packet) sizeof(packet) / 4

#pragma warning(push)
#pragma warning(disable : 4200) // nonstandard data[]
/// This is the packet that an addon produces.
/// There is a crc32 missing at the end of the structure, which also should not be filled out by the addon - 
/// this is just how end of struct arrays work unfortunately, we cant have fields after it.
typedef struct {
	PacketHeader Header;
	/// Your data. This is just an array of u16 for alignment purposes, its actual content will not be touched by the routing.
	/// This must always come up to a full int32 alignment together with the header, or in other words: It must always contain 2n + 1 int16's.
	uint16_t Data[];
	//TODO(Rennorb): crc endianess?
} Packet;
#pragma warning(pop)
#pragma pack(pop)

typedef int32_t PacketChecksum;

//TODO(Rennorb) @cleanup: change case to match other exposed fns

/// The shape of function that a plugin can register to handle packets destined for it.
/// If you receive a packet from this callback it is already destined for you and the crc has been validated.
typedef void PacketHandler(Packet const* packet);

/// The shape of function that a plugin can register to handle packets destined for it.
/// This function will add addon id and crc into the packet, so there is no need for the addon itself to do it.
typedef void PacketPrepareAndBroadcast(Packet* packet);