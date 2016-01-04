#define MAX_SOURCEFILES	64

namespace Components
{
	class Menus : public Component
	{
	public:
		Menus();
		~Menus();
		const char* GetName() { return "Menus"; };

		static void FreeEverything();

		static void Add(std::string menu);

	private:
		static std::vector<Game::menuDef_t*> MenuList;
		static std::vector<Game::MenuList*> MenuListList;
		static std::vector<std::string> CustomMenus;

		static Game::XAssetHeader MenuFileLoad(Game::XAssetType type, const char* filename);
		static Game::MenuList* LoadMenuList(Game::MenuList* menuList);
		static Game::MenuList* LoadScriptMenu(const char* menu);
		static std::vector<Game::menuDef_t*> LoadMenu(Game::menuDef_t* menudef);
		static std::vector<Game::menuDef_t*> LoadMenu(std::string file);

		static Game::script_t* LoadMenuScript(std::string name, std::string buffer);
		static int LoadMenuSource(std::string name, std::string buffer);

		static int ReserveSourceHandle();
		static bool IsValidSourceHandle(int handle);

		static Game::menuDef_t* ParseMenu(int handle);

		static void FreeMenuSource(int handle);

		static void FreeMenuList(Game::MenuList* menuList);
		static void FreeMenu(Game::menuDef_t* menudef);

		static void RemoveMenu(Game::menuDef_t* menudef);
		static void RemoveMenuList(Game::MenuList* menuList);

		static void AddMenuListHook(Game::UiContext *dc, Game::MenuList *menuList, int close);

		static Game::menuDef_t* FindMenuByName(Game::UiContext* dc, const char* name);

		// Ugly!
		static int KeywordHash(char* key);
	};
}