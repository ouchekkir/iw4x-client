#include "STDInclude.hpp"

namespace Steam
{
	::Utils::Library Proxy::Client;
	::Utils::Library Proxy::Overlay;

	bool Proxy::Inititalize()
	{
		std::string directoy = Proxy::GetSteamDirectory();
		if (directoy.empty()) return false;

		SetDllDirectoryA(Proxy::GetSteamDirectory().data());

		Proxy::Client = ::Utils::Library(STEAMCLIENT_LIB, false);
		Proxy::Overlay = ::Utils::Library(GAMEOVERLAY_LIB, false);

		return (Proxy::Client.valid() && Proxy::Overlay.valid());
	}

	void Proxy::Uninititalize()
	{
		Proxy::Client = ::Utils::Library();
		Proxy::Overlay = ::Utils::Library();
	}

	std::string Proxy::GetSteamDirectory()
	{
		HKEY hRegKey;
		char SteamPath[MAX_PATH];
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, STEAM_REGISTRY_PATH, 0, KEY_QUERY_VALUE, &hRegKey) == ERROR_SUCCESS)
		{
			DWORD dwLength = sizeof(SteamPath);
			RegQueryValueExA(hRegKey, "InstallPath", NULL, NULL, reinterpret_cast<BYTE*>(SteamPath), &dwLength);
			RegCloseKey(hRegKey);

			return SteamPath;
		}

		return "";
	}

	void Proxy::SetOverlayNotificationPosition(uint32_t eNotificationPosition)
	{
		if (Proxy::Overlay.valid())
		{
			Proxy::Overlay.get<void(uint32_t)>("SetNotificationPosition")(eNotificationPosition);
		}
	}

	bool Proxy::IsOverlayEnabled()
	{
		if (Proxy::Overlay.valid())
		{
			return Proxy::Overlay.get<bool()>("IsOverlayEnabled")();
		}

		return false;
	}

	bool Proxy::BOverlayNeedsPresent()
	{
		if (Proxy::Overlay.valid())
		{
			return Proxy::Overlay.get<bool()>("BOverlayNeedsPresent")();
		}

		return false;
	}
}
