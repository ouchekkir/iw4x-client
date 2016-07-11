#include "STDInclude.hpp"

namespace Components
{
	wink::signal<wink::slot<Dvar::Callback>> Dvar::RegistrationSignal;

	Dvar::Var::Var(std::string dvarName) : Var()
	{
		this->dvar = Game::Dvar_FindVar(dvarName.data());

		if (!this->dvar)
		{
			// Quick-register the dvar
			Game::Dvar_SetStringByName(dvarName.data(), "");
			this->dvar = Game::Dvar_FindVar(dvarName.data());
		}
	}

	template <> Game::dvar_t* Dvar::Var::Get()
	{
		return this->dvar;
	}
	template <> char* Dvar::Var::Get()
	{
		if (this->dvar && this->dvar->type == Game::dvar_type::DVAR_TYPE_STRING && this->dvar->current.string)
		{
			return this->dvar->current.string;
		}

		return "";
	}
	template <> const char* Dvar::Var::Get()
	{
		return this->Get<char*>();
	}
	template <> int Dvar::Var::Get()
	{
		if (this->dvar && this->dvar->type == Game::dvar_type::DVAR_TYPE_INT)
		{
			return this->dvar->current.integer;
		}

		return 0;
	}
	template <> unsigned int Dvar::Var::Get()
	{
		return static_cast<unsigned int>(this->Get<int>());
	}
	template <> float Dvar::Var::Get()
	{
		if (this->dvar && this->dvar->type == Game::dvar_type::DVAR_TYPE_FLOAT)
		{
			return this->dvar->current.value;
		}

		return 0;
	}
	template <> float* Dvar::Var::Get()
	{
		static float val[4] = { 0 };

		if (this->dvar && (this->dvar->type == Game::dvar_type::DVAR_TYPE_FLOAT_2 || this->dvar->type == Game::dvar_type::DVAR_TYPE_FLOAT_3 || this->dvar->type == Game::dvar_type::DVAR_TYPE_FLOAT_4))
		{
			return this->dvar->current.vec4;
		}

		return val;
	}
	template <> bool Dvar::Var::Get()
	{
		if (this->dvar && this->dvar->type == Game::dvar_type::DVAR_TYPE_BOOL)
		{
			return this->dvar->current.boolean;
		}

		return false;
	}
	template <> std::string Dvar::Var::Get()
	{
		return this->Get<const char*>();
	}

	void Dvar::Var::Set(char* string)
	{
		this->Set(const_cast<const char*>(string));
	}
	void Dvar::Var::Set(const char* string)
	{
		if (this->dvar && this->dvar->name)
		{
			Game::Dvar_SetCommand(this->dvar->name, string);
		}
	}
	void Dvar::Var::Set(std::string string)
	{
		this->Set(string.data());
	}
	void Dvar::Var::Set(int integer)
	{
		if (this->dvar && this->dvar->name)
		{
			Game::Dvar_SetCommand(this->dvar->name, Utils::String::VA("%i", integer));
		}
	}
	void Dvar::Var::Set(float value)
	{
		if (this->dvar && this->dvar->name)
		{
			Game::Dvar_SetCommand(this->dvar->name, Utils::String::VA("%f", value));
		}
	}

	void Dvar::Var::SetRaw(int integer)
	{
		if (this->dvar)
		{
			this->dvar->current.integer = integer;
		}
	}

	template<> static Dvar::Var Dvar::Register(const char* name, bool value, Dvar::Flag flag, const char* description)
	{
		return Game::Dvar_RegisterBool(name, value, flag.val, description);
	}
	template<> static Dvar::Var Dvar::Register(const char* name, const char* value, Dvar::Flag flag, const char* description)
	{
		return Game::Dvar_RegisterString(name, value, flag.val, description);
	}
	template<> static Dvar::Var Dvar::Register(const char* name, int value, int min, int max, Dvar::Flag flag, const char* description)
	{
		return Game::Dvar_RegisterInt(name, value, min, max, flag.val, description);
	}

	void Dvar::OnInit(Dvar::Callback* callback)
	{
		Dvar::RegistrationSignal.connect(callback);
	}

	Game::dvar_t* Dvar::RegisterName(const char* name, const char* default, Game::dvar_flag flag, const char* description)
	{
		// Run callbacks
		Dvar::RegistrationSignal();

		// Name watcher
		Renderer::OnFrame([] ()
		{
			static std::string lastValidName = "Unknown Soldier";
			std::string name = Dvar::Var("name").Get<char*>();

			// Don't perform any checks if name didn't change
			if (name == lastValidName) return;

			std::string saneName = Colors::Strip(Utils::String::Trim(name));
			if (saneName.size() < 3 || (saneName[0] == '[' && saneName[1] == '{'))
			{
				Logger::Print("Username '%s' is invalid. It must at least be 3 characters long and not appear empty!\n", name.data());
				Dvar::Var("name").Set(lastValidName);
			}
			else
			{
				lastValidName = name;
			}
		});

		return Dvar::Register<const char*>(name, "Unknown Soldier", Dvar::Flag(flag | Game::dvar_flag::DVAR_FLAG_SAVED).val, description).Get<Game::dvar_t*>();
	}

	Dvar::Dvar()
	{
		// set flags of cg_drawFPS to archive
		Utils::Hook::Or<BYTE>(0x4F8F69, Game::dvar_flag::DVAR_FLAG_SAVED);

		// un-cheat cg_fov and add archive flags
		Utils::Hook::Xor<BYTE>(0x4F8E35, Game::dvar_flag::DVAR_FLAG_CHEAT | Game::dvar_flag::DVAR_FLAG_SAVED);

		// un-cheat cg_debugInfoCornerOffset and add archive flags
		Utils::Hook::Xor<BYTE>(0x4F8FC2, Game::dvar_flag::DVAR_FLAG_CHEAT | Game::dvar_flag::DVAR_FLAG_SAVED);

		// remove archive flags for cg_hudchatposition
		Utils::Hook::Xor<BYTE>(0x4F9992, Game::dvar_flag::DVAR_FLAG_SAVED);

		// remove write protection from fs_game
		Utils::Hook::Xor<DWORD>(0x6431EA, Game::dvar_flag::DVAR_FLAG_WRITEPROTECTED);

		// set cg_fov max to 90.0
		static float cgFov90 = 90.0f;
		Utils::Hook::Set<float*>(0x4F8E28, &cgFov90);

		// set max volume to 1
		static float volume = 1.0f;
		Utils::Hook::Set<float*>(0x408078, &volume);

		// Uncheat ui_showList
		Utils::Hook::Xor<BYTE>(0x6310DC, Game::dvar_flag::DVAR_FLAG_CHEAT);

		// Uncheat ui_debugMode
		Utils::Hook::Xor<BYTE>(0x6312DE, Game::dvar_flag::DVAR_FLAG_CHEAT);

		// Hook dvar 'name' registration
		Utils::Hook(0x40531C, Dvar::RegisterName, HOOK_CALL).Install()->Quick();
	}

	Dvar::~Dvar()
	{
		Dvar::RegistrationSignal.clear();
	}
}
