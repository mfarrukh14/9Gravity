#ifndef GAME_EDITOR_H
#define GAME_EDITOR_H

#include <string>
#include <filesystem>

struct SDL_Window;

class GameEditor {
public:
    GameEditor();
    ~GameEditor();


    bool RenderLauncher(bool& requestOpenFileDialog, std::string& outProjectPath);


    void RenderEditor();
    void OpenProject(const std::filesystem::path& projectPath);
    const std::string& CurrentProjectPath() const;
    int CurrentBuildNumber() const;

private:

    bool showNewProjectPopup;
    char newProjectBuffer[256];

    std::string currentProjectPath;
    int buildNumber;

    bool showGrid;
    float canvasZoom;
    float cameraX, cameraY;

    bool createNewProjectOnDisk(const std::filesystem::path& projectPath);
};

#endif // GAME_EDITOR_H
