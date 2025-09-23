#ifndef GAME_EDITOR_H
#define GAME_EDITOR_H

#include <string>
#include <filesystem>
#include <vector>
#include <memory>

struct SDL_Window;
struct SDL_Texture;
struct SDL_Renderer;

enum class EntityType {
    CHARACTER,
    BACKGROUND,
    OBSTACLE,
    OTHER
};

struct GameEntity {
    std::string name;
    std::string imagePath;
    EntityType type;
    float x, y;
    float width, height;
    int zIndex;
    SDL_Texture* texture;
    bool isSelected;
    
    GameEntity() : x(0), y(0), width(100), height(100), zIndex(0), texture(nullptr), isSelected(false) {}
};

class GameEditor {
public:
    GameEditor();
    ~GameEditor();

    void SetRenderer(SDL_Renderer* renderer);
    bool RenderLauncher(bool& requestOpenFileDialog, std::string& outProjectPath);
    void RenderEditor();
    void OpenProject(const std::filesystem::path& projectPath);
    const std::string& CurrentProjectPath() const;
    int CurrentBuildNumber() const;

private:
    SDL_Renderer* renderer;
    
    bool showNewProjectPopup;
    char newProjectBuffer[256];

    std::string currentProjectPath;
    int buildNumber;

    bool showGrid;
    float canvasZoom;
    float cameraX, cameraY;
    
    // Entity management
    std::vector<std::unique_ptr<GameEntity>> entities;
    GameEntity* selectedEntity;
    bool showImportDialog;
    char importNameBuffer[256];
    char importPathBuffer[512];
    EntityType importType;
    
    // UI state
    bool showEntityInspector;
    char fileDialogPath[512];
    
    // File browser state
    bool showFileBrowser;
    std::string currentBrowserPath;
    std::vector<std::filesystem::directory_entry> browserEntries;
    std::string selectedFilePath;

    bool createNewProjectOnDisk(const std::filesystem::path& projectPath);
    SDL_Texture* LoadTexture(const std::string& imagePath);
    void AddEntity(const std::string& name, const std::string& imagePath, EntityType type);
    void RemoveSelectedEntity();
    void MoveEntityZIndex(GameEntity* entity, int direction);
    void SortEntitiesByZIndex();
    GameEntity* GetEntityAtPosition(float x, float y);
    void RenderCanvas();
    void RenderInspector();
    void RenderImportDialog();
    void RenderFileBrowser();
    void HandleCanvasInput();
    void CleanupTextures();
    void RefreshBrowserEntries();
    bool IsImageFile(const std::string& filename);
};

#endif // GAME_EDITOR_H
