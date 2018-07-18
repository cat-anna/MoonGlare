#pragma once

#include <Foundation/Flags.h>
#include <Libs/libSpace/src/Memory/DynamicMessageBuffer.h>

namespace MoonGlare {
namespace Debug {
namespace InsiderApi {

#define __LOG_ACTION_Tool(T, A)					__BeginLog(T, A)
#define __LOG_ACTION_F_Tool(T, ...)				__BeginLogf(T, __VA_ARGS__)

#pragma warning ( push )
#pragma warning ( disable: 4200 )
#pragma pack( push, 1 )

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;

struct Configuration {
	enum {
		Insider_Port			= 0xC000,
		recon_Port				= 0xC001,
		MaxMessageSize			= 32 * 1024,
	};
};

enum class SignatureValue : u32 {
	Empty	= 'CRGM',
	TCP		= 'CSGM',
};

enum class MessageTypes : u32 {
//meta elements
	DirectionDelta		 = 0x10000,
	GroupDelta			 = 0x01000,

//bidirectional
	BidirectionalBase	= 0,
	NOP					= 0,
	Ok,

//Errors
	UnknownError							= BidirectionalBase + GroupDelta,
	NotSupported,							//message is not supported
	NotPossibleInCurrentState,				//try later or on different scene or ...

//to engine
	EngineReciver							= BidirectionalBase + DirectionDelta,	
	Bind,									// hello message, tells recon where to send additional debug suff
	Release,
	ExecuteCode,							// recived payload is raw string to be passed to lua
	SetScriptCode,							//PayLoad_SetScriptCode
	GetScriptCode,							//PayLoad_GetScriptCode
	InfoRequest,							//no payload
	Ping,									//no payload
	OrbitLoggerStateRequest,			    //no payload

	EnumerationBase							= EngineReciver + GroupDelta,
	EnumerateScripts,						//Request to send description of all scripts. No payload
	EnumerateFolder,						//payload is PayLoad_EnumerateRequest
	EnumerateLua,							//payload is PayLoad_EnumerateRequest
	EnumerateAudio,							//no payload
	EnumerateMemory,						//no payload
	EnumerateEntities,						//no payload

//from engine
	EngineSender							= EngineReciver + DirectionDelta,
	ExecutionResult,	
	LogLine,	
	ScriptCode,								//PayLoad_ScriptCode
	InfoResponse,							//PayLoad_InfoResponse
	Pong,									//no payload
	OrbitLoggerStateResponse,			    //no payload

	//payload is PayLoad_ListBase and then dedicated struct
	EngineListOutputBase					= EngineSender + GroupDelta,
	ScriptList,								//PayLoad_ScriptList_Item
	FolderContentList,						// Folder content
	LuaElementList,
	AudioList,								//payload is PayLoad_AudioListItem
	MemoryStatusList,						//PayLoad_MemoryStatus
	EntitiesList,							//PayLoad_EntityInfo
	
	//Notification about something
	EngineNotificationBase					= EngineListOutputBase + GroupDelta,	
	NotificationSettingsChanged,			//PayLoad_NotificationSettingsChanged
	NotificationGlobalEvent,				//PayLoad_NotificationGlobalEvent
};

//----------------------------------------------------------------------------------

/** In tcp connection this header precedess MessageHeader to help gathering whole message */
struct TCPHeader {
	SignatureValue Signature;
	u32 MessageSize;
	u8 MessageHeader_Payload[0];
};

struct MessageHeader {
	SignatureValue Signature;
	MessageTypes MessageType;
	u32 PayloadSize;
	u32 RequestID;
	u8 PayLoad[0];
};

using InsiderMessageBuffer = Space::Memory::DynamicMessageBuffer < Configuration::MaxMessageSize, MessageHeader> ;

//----------------------------------------------------------------------------------

struct PayLoad_ExecuteCode {
	char Code[0];
};

struct PayLoad_ExecutionResult {
	int ReturnCode;
};

struct PayLoad_LogLine {
	char Line[0];
};

struct PayLoad_ListBase {
	u32 Count;
	u32 Unused_32_1;
	char List[0];
};

struct PayLoad_EnumerateRequest {
	u32 unused_32;
	u16 unused_16;
	u16 PathLen;
	char Path[0];
};

struct PayLoad_ScriptList_Item {
	u16 Index;
	u16 NameLen; //len must include trailling null character
	u16 DataLen;
	enum class ItemType : u8 {
		ScriptFile,
		BroadcastedCode,
	} Type;
	u8 unused_8;

	char Name[0];
};

struct PayLoad_FolderContent_Item {
	u16 Index;
	u16 NameLen; //len must include trailling null character
	u16 Flags;
	u8 unused_8_1[2];

	enum class FlagBits {
		File, Folder, Shadowed, SymbolicLink, Cached, Used,
	};

	DefineFlag(Flags, FlagBit(FlagBits::File), File);
	DefineFlag(Flags, FlagBit(FlagBits::Folder), Folder);
	DefineFlag(Flags, FlagBit(FlagBits::Shadowed), Shadowed);
	DefineFlag(Flags, FlagBit(FlagBits::SymbolicLink), SymbolicLink);
	DefineFlag(Flags, FlagBit(FlagBits::Cached), Cached);
	DefineFlag(Flags, FlagBit(FlagBits::Used), Used);

	char Name[0];
};

struct PayLoad_LuaElement_Item {
	u16 Index;
	u16 NameLen; //len must include trailing null character
	u16 ValueLen; //len must include trailing null character
	u8 LuaType;
	u8 unused_8;
	char Name_Value[0];
};

struct PayLoad_NotificationSettingsChanged {
	u32 Group;
};

struct PayLoad_NotificationGlobalEvent {
	u32 event;
};

struct PayLoad_GetScriptCode {
	u16 NameLength;
	char Name[0];
};

struct PayLoad_SetScriptCode {
	u16 NameLength;
	u16 DataLength;
	union {
		u8 Flags;
		struct {
			u8 OverwriteContainerFile: 1;
		};
	};
	char Name_Data[0];
};

struct PayLoad_ScriptCode {
	u16 DataLength;
	char Data[0];
};

struct PayLoad_InfoResponse {
	u16 VersionLength;
	u16 ExeNameLength;
	u16 BuildDateLength;
	u16 reserved;
	char Data[0];
};

struct PayLoad_AudioListItem {
	u16 Index;
	u8 unused;
	enum class AudioType : u8 {
		Unknown, Sound, Music,
	} Type;
	u16 NameLen;
	u16 ClassNameLen;
	char Name_Class[0];
};

struct PayLoad_MemoryStatus {
	u16 Index;
	u16 unused;
	u32 Capacity;
	u32 Allocated;
	u16 ElementSize;
	u64 ID;
	u64 ParentID;
	u16 unused_16_0;
	u32 unused_32_0;
	u16 NameLen;
	u16 OwnerNameLen;
	u8 Name_OwnerName[0];
};

struct PayLoad_OrbitLoggerStateResponse {
	struct ChannelInfo {
		u8 Enabled;
		OrbitLogger::LogChannel Channel;
		char Name[8];
		u32 LinesPushed;
	};
	ChannelInfo m_Table[OrbitLogger::LogChannels::MaxLogChannels];
};

struct PayLoad_EntityInfo {
	Entity::IntValue_t SelfEntity;
	Entity::IntValue_t ParentEntity;
	Entity::IntValue_t FirstChildEntity;
	Entity::IntValue_t NextSiblingEntity;
	Entity::IntValue_t PrevSiblingEntity;
	uint8_t Flags;

	u8 Name[0];
};

#pragma pack( pop )
#pragma warning ( pop )

} //namespace InsiderApi
} //namespace Debug
} //namespace MoonGlare
