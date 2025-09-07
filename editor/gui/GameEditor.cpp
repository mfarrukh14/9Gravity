#include "GameEditor.h"

#include <imgui.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cmath>

GameEditor::GameEditor()
    : showNewProjectPopup(false),
      buildNumber(0.1),
      showGrid(true),
      canvasZoom(1.0f),
      cameraX(0.0f),
      cameraY(0.0f)
{
    newProjectBuffer[0] = '\0';
}

GameEditor::~GameEditor() = default;

const std::string &GameEditor::CurrentProjectPath() const
{
    return currentProjectPath;
}

int GameEditor::CurrentBuildNumber() const
{
    return buildNumber;
}

bool GameEditor::createNewProjectOnDisk(const std::filesystem::path &projectPath)
{
    try
    {
        if (std::filesystem::exists(projectPath))
        {
            std::cerr << "Project path already exists: " << projectPath << "\n";
            return false;
        }

        std::filesystem::create_directories(projectPath / "src");
        std::filesystem::create_directories(projectPath / "assets");

        std::ofstream maincpp(projectPath / "src" / "main.cpp");
        maincpp << R"(#include <iostream>

int main() {
    std::cout << "Hello from your new game project!\n";
    return 0;
}
)";
        maincpp.close();

        std::ofstream scene(projectPath / "scene.json");
        scene << R"({
  "scene_name": "New Scene",
  "entities": [],
  "meta": {
    "created_by": "GameEditor",
    "version": "0.1"
  }
}
)";
        scene.close();

        std::ofstream readme(projectPath / "README.md");
        readme << "# " << projectPath.filename().string() << "\n\n"
               << "This is a minimal automatically-created game project.\n";
        readme.close();

        return true;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Failed to create project on disk: " << ex.what() << std::endl;
        return false;
    }
}

void GameEditor::OpenProject(const std::filesystem::path &projectPath)
{
    currentProjectPath = projectPath.string();
    buildNumber = 0;
    cameraX = cameraY = 0.0f;
    canvasZoom = 1.0f;
}

bool GameEditor::RenderLauncher(bool &requestOpenFileDialog, std::string &outProjectPath)
{
    bool switchToEditor = false;

    ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_FirstUseEver);
    ImGui::Begin("Game Editor Launcher", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::TextWrapped("Welcome to the simple Game Editor launcher. Create a new project or load an existing one.");

    ImGui::Spacing();
    if (ImGui::Button("New Project", ImVec2(150, 0)))
    {
        showNewProjectPopup = true;
        newProjectBuffer[0] = '\0';
        ImGui::OpenPopup("New Project");
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Project", ImVec2(150, 0)))
    {
        requestOpenFileDialog = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Quit", ImVec2(150, 0)))
    {
        currentProjectPath.clear();
        switchToEditor = false;
        ImGui::End();
        return false; 
    }

    ImGui::Separator();
    ImGui::Text("Recent Projects:");
    if (!currentProjectPath.empty())
    {
        ImGui::TextWrapped("Current project: %s", currentProjectPath.c_str());
    }
    else
    {
        ImGui::TextWrapped("No project open.");
    }

    if (ImGui::BeginPopupModal("New Project", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Enter new project name (folder will be created in current directory):");
        ImGui::InputText("Project name", newProjectBuffer, sizeof(newProjectBuffer));
        ImGui::Spacing();

        ImGui::BeginGroup();
        if (ImGui::Button("Create", ImVec2(120, 0)))
        {
            std::string name(newProjectBuffer);
            if (name.empty())
            {
                // no name; keep popup open (we could show an error)
            }
            else
            {
                std::filesystem::path p = std::filesystem::current_path() / name;
                if (createNewProjectOnDisk(p))
                {
                    OpenProject(p);
                    outProjectPath = p.string();
                    switchToEditor = true;
                    ImGui::CloseCurrentPopup();
                }
                else
                {
                    std::cerr << "Failed to create project: " << p << std::endl;
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndGroup();

        ImGui::EndPopup();
    }

    // bottom status bar
    ImGui::Dummy(ImVec2(0, 10));
    ImGui::Separator();
    ImGui::Text("Build #: %d", buildNumber);
    ImGui::SameLine();


    ImGui::End();

    return switchToEditor;
}

void GameEditor::RenderEditor()
{

    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::Begin("Editor (Dockspace)", nullptr, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save Scene"))
            {
                // TODO: implement save
            }
            if (ImGui::MenuItem("Close Project"))
            {
                // Closing is handled in main loop
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Show Grid", nullptr, &showGrid);
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::BeginChild("toolbar", ImVec2(0, 40), false);
    ImGui::Text("Toolbar:");
    ImGui::SameLine();
    if (ImGui::Button("Select"))
    {
    }
    ImGui::SameLine();
    if (ImGui::Button("Move"))
    {
    }
    ImGui::SameLine();
    if (ImGui::Button("Sprite"))
    {
    }
    ImGui::SameLine();
    if (ImGui::Button("Build"))
    {
        buildNumber++;
    }
    ImGui::SameLine(0, 20);
    ImGui::Text("Build #: %d", buildNumber);
    ImGui::EndChild();

    ImGui::Separator();

    ImGui::BeginChild("editor_main", ImVec2(ImGui::GetContentRegionAvail().x * 0.7f, 0), true);
    ImGui::Text("Canvas (2D): use mouse to pan/zoom (placeholder)");
    ImGui::InvisibleButton("canvas_placeholder", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 20));
    ImVec2 canvasPos = ImGui::GetItemRectMin();
    ImVec2 canvasSize = ImGui::GetItemRectSize();

    ImDrawList *draw = ImGui::GetWindowDrawList();
    if (showGrid)
    {
        const float gridStep = 32.0f * canvasZoom;
        for (float x = std::fmod(cameraX, gridStep); x < canvasSize.x; x += gridStep)
        {
            draw->AddLine(ImVec2(canvasPos.x + x, canvasPos.y), ImVec2(canvasPos.x + x, canvasPos.y + canvasSize.y), IM_COL32(200, 200, 200, 40));
        }
        for (float y = std::fmod(cameraY, gridStep); y < canvasSize.y; y += gridStep)
        {
            draw->AddLine(ImVec2(canvasPos.x, canvasPos.y + y), ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + y), IM_COL32(200, 200, 200, 40));
        }
    }

    draw->AddRectFilled(ImVec2(canvasPos.x + 100 - cameraX, canvasPos.y + 100 - cameraY),
                        ImVec2(canvasPos.x + 180 - cameraX, canvasPos.y + 160 - cameraY),
                        IM_COL32(100, 180, 240, 200));
    draw->AddText(ImVec2(canvasPos.x + 105 - cameraX, canvasPos.y + 105 - cameraY), IM_COL32(255, 255, 255, 255), "Entity");

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("inspector", ImVec2(0, 0), true);
    ImGui::Text("Inspector");
    ImGui::Separator();
    ImGui::TextWrapped("Project: %s", currentProjectPath.c_str());
    ImGui::Spacing();
    ImGui::Text("Camera: (%.1f, %.1f) Zoom: %.2f", cameraX, cameraY, canvasZoom);
    ImGui::SliderFloat("Zoom", &canvasZoom, 0.25f, 4.0f);

    ImGui::EndChild();

    ImGui::End(); 
}
