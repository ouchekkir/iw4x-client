#include "..\..\STDInclude.hpp"

namespace Components
{
	__int64* QuickPatch::GetStatsID()
	{
		static __int64 id = 0x110000100001337;
		return &id;
	}

	QuickPatch::QuickPatch()
	{
		// protocol version (workaround for hacks)
		Utils::Hook::Set<int>(0x4FB501, PROTOCOL);

		// protocol command
		Utils::Hook::Set<int>(0x4D36A9, PROTOCOL);
		Utils::Hook::Set<int>(0x4D36AE, PROTOCOL);
		Utils::Hook::Set<int>(0x4D36B3, PROTOCOL);

		// internal version is 99, most servers should accept it
		Utils::Hook::Set<int>(0x463C61, 208);

		// remove system pre-init stuff (improper quit, disk full)
		Utils::Hook::Set<BYTE>(0x411350, 0xC3);

		// remove STEAMSTART checking for DRM IPC
		Utils::Hook::Nop(0x451145, 5);
		Utils::Hook::Set<BYTE>(0x45114C, 0xEB);

		// LSP disabled
		Utils::Hook::Set<BYTE>(0x435950, 0xC3); // LSP HELLO
		Utils::Hook::Set<BYTE>(0x49C220, 0xC3); // We wanted to send a logging packet, but we haven't connected to LSP!
		Utils::Hook::Set<BYTE>(0x4BD900, 0xC3); // main LSP response func
		Utils::Hook::Set<BYTE>(0x682170, 0xC3); // Telling LSP that we're playing a private match

		// Don't delete config files if corrupted
		Utils::Hook::Set<BYTE>(0x47DCB3, 0xEB);

		// hopefully allow alt-tab during game, used at least in alt-enter handling
		Utils::Hook::Set<DWORD>(0x45ACE0, 0xC301B0);

		// fs_basegame
		Utils::Hook::Set<char*>(0x6431D1, "data");

		// UI version string
		Utils::Hook::Set<char*>(0x43F73B, "iw4x IW4x: r" REVISION_STR);

		// console version string
		Utils::Hook::Set<char*>(0x4B12BB, "iw4x IW4x r" REVISION_STR " (built " __DATE__ " " __TIME__ ")");

		// version string
		Utils::Hook::Set<char*>(0x60BD56, "iw4x IW4x (r" REVISION_STR ")");

		// console title
		Utils::Hook::Set<char*>(0x4289E8, "iw4x IW4x (r" REVISION_STR "): Console");

		// window title
		Utils::Hook::Set<char*>(0x5076A0, "iw4x IW4x: Multiplayer");

		// sv_hostname
		Utils::Hook::Set<char*>(0x4D378B, "IW4Host");

		// shortversion
		Utils::Hook::Set<char*>(0x60BD91, VERSION_STR);

		// console logo
		Utils::Hook::Set<char*>(0x428A66, "data/images/logo.bmp");

		// splash logo
		Utils::Hook::Set<char*>(0x475F9E, "data/images/splash.bmp");

		// Numerical ping (cg_scoreboardPingText 1)
		Utils::Hook::Set<BYTE>(0x45888E, 1);
		Utils::Hook::Set<BYTE>(0x45888C, Game::dvar_flag::DVAR_FLAG_CHEAT);

		// increase font sizes for chat on higher resolutions
		static float float13 = 13.0f;
		static float float10 = 10.0f;
		Utils::Hook::Set<float*>(0x5814AE, &float13);
		Utils::Hook::Set<float*>(0x5814C8, &float10);

		// Enable commandline arguments
		Utils::Hook::Set<BYTE>(0x464AE4, 0xEB);

		// remove limit on IWD file loading
		Utils::Hook::Set<BYTE>(0x642BF3, 0xEB);

		// Disable UPNP
		Utils::Hook::Nop(0x60BE24, 5);

		// disable the IWNet IP detection (default 'got ipdetect' flag to 1)
		Utils::Hook::Set<BYTE>(0x649D6F0, 1);

		// Fix stats sleeping
		Utils::Hook::Set<BYTE>(0x6832BA, 0xEB);
		Utils::Hook::Set<BYTE>(0x4BD190, 0xC3);

		// default sv_pure to 0
		Utils::Hook::Set<BYTE>(0x4D3A74, 0);

		// Force debug logging
		Utils::Hook::Nop(0x4AA89F, 2);
		Utils::Hook::Nop(0x4AA8A1, 6);

		// Rename stat file - TODO: beautify
		Utils::Hook::SetString(0x71C048, "iw4x.stat");

		// Patch stats steamid
		Utils::Hook::Nop(0x682EBF, 20);
		Utils::Hook::Nop(0x6830B1, 20);
		Utils::Hook(0x682EBF, QuickPatch::GetStatsID, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x6830B1, QuickPatch::GetStatsID, HOOK_CALL).Install()->Quick();
	}
}