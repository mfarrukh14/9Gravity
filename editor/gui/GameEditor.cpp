#include "GameEditor.h"

#include <imgui.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <SDL3/SDL.h>
#ifdef HAVE_SDL3_IMAGE
#include <SDL3_image/SDL_image.h>
#endif

GameEditor::GameEditor()
    : renderer(nullptr),
      showNewProjectPopup(false),
      buildNumber(0.1),
      showGrid(true),
      canvasZoom(1.0f),
      cameraX(0.0f),
      cameraY(0.0f),
      selectedEntity(nullptr),
      showImportDialog(false),
      importType(EntityType::CHARACTER),
      showEntityInspector(true),
      showFileBrowser(false),
      currentBrowserPath(".")
{
    newProjectBuffer[0] = '\0';
    importNameBuffer[0] = '\0';
    importPathBuffer[0] = '\0';
    fileDialogPath[0] = '\0';
}

GameEditor::~GameEditor() {
    CleanupTextures();
}

void GameEditor::SetRenderer(SDL_Renderer* renderer) {
    this->renderer = renderer;
}

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
    
    // Clear existing entities
    CleanupTextures();
    entities.clear();
    selectedEntity = nullptr;
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
            ImGui::MenuItem("Entity Inspector", nullptr, &showEntityInspector);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Entity"))
        {
            if (ImGui::MenuItem("Import Character"))
            {
                importType = EntityType::CHARACTER;
                showImportDialog = true;
                strcpy(importNameBuffer, "New Character");
            }
            if (ImGui::MenuItem("Import Background"))
            {
                importType = EntityType::BACKGROUND;
                showImportDialog = true;
                strcpy(importNameBuffer, "New Background");
            }
            if (ImGui::MenuItem("Import Obstacle"))
            {
                importType = EntityType::OBSTACLE;
                showImportDialog = true;
                strcpy(importNameBuffer, "New Obstacle");
            }
            if (ImGui::MenuItem("Import Other"))
            {
                importType = EntityType::OTHER;
                showImportDialog = true;
                strcpy(importNameBuffer, "New Object");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete Selected", nullptr, false, selectedEntity != nullptr))
            {
                RemoveSelectedEntity();
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // Toolbar
    ImGui::BeginChild("toolbar", ImVec2(0, 40), false);
    ImGui::Text("Toolbar:");
    ImGui::SameLine();
    if (ImGui::Button("Select"))
    {
        // Select tool is already active by default
    }
    ImGui::SameLine();
    if (ImGui::Button("Import"))
    {
        importType = EntityType::CHARACTER;
        showImportDialog = true;
        strcpy(importNameBuffer, "New Entity");
    }
    ImGui::SameLine();
    if (ImGui::Button("Build"))
    {
        buildNumber++;
    }
    ImGui::SameLine(0, 20);
    ImGui::Text("Build #: %d", buildNumber);
    
    if (selectedEntity)
    {
        ImGui::SameLine(0, 30);
        ImGui::Text("Selected: %s", selectedEntity->name.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Z+"))
        {
            MoveEntityZIndex(selectedEntity, 1);
        }
        ImGui::SameLine();
        if (ImGui::Button("Z-"))
        {
            MoveEntityZIndex(selectedEntity, -1);
        }
    }
    ImGui::EndChild();

    ImGui::Separator();

    // Main content area
    float inspectorWidth = showEntityInspector ? ImGui::GetContentRegionAvail().x * 0.3f : 0.0f;
    float canvasWidth = ImGui::GetContentRegionAvail().x - inspectorWidth;

    RenderCanvas();
    
    if (showEntityInspector)
    {
        ImGui::SameLine();
        RenderInspector();
    }

    // Handle import dialog
    if (showImportDialog)
    {
        RenderImportDialog();
    }

    // Handle file browser
    if (showFileBrowser)
    {
        RenderFileBrowser();
    }

    ImGui::End();
}

SDL_Texture* GameEditor::LoadTexture(const std::string& imagePath) {
    if (!renderer) return nullptr;
    
#ifdef HAVE_SDL3_IMAGE
    SDL_Surface* surface = IMG_Load(imagePath.c_str());
    if (!surface) {
        std::cerr << "Failed to load image: " << imagePath << " - " << IMG_GetError() << std::endl;
        return nullptr;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    
    if (!texture) {
        std::cerr << "Failed to create texture from: " << imagePath << " - " << SDL_GetError() << std::endl;
    }
    
    return texture;
#else
    // Fallback: return nullptr when SDL_image is not available
    std::cerr << "Image loading disabled: SDL3_image not available. Cannot load: " << imagePath << std::endl;
    return nullptr;
#endif
}

void GameEditor::AddEntity(const std::string& name, const std::string& imagePath, EntityType type) {
    auto entity = std::make_unique<GameEntity>();
    entity->name = name;
    entity->imagePath = imagePath;
    entity->type = type;
    entity->texture = LoadTexture(imagePath);
    entity->x = 100.0f;
    entity->y = 100.0f;
    entity->zIndex = static_cast<int>(entities.size());
    
    if (entity->texture) {
        float w, h;
        SDL_GetTextureSize(entity->texture, &w, &h);
        entity->width = w;
        entity->height = h;
    }
    
    entities.push_back(std::move(entity));
    SortEntitiesByZIndex();
}

void GameEditor::RemoveSelectedEntity() {
    if (!selectedEntity) return;
    
    auto it = std::find_if(entities.begin(), entities.end(),
        [this](const std::unique_ptr<GameEntity>& entity) {
            return entity.get() == selectedEntity;
        });
    
    if (it != entities.end()) {
        if ((*it)->texture) {
            SDL_DestroyTexture((*it)->texture);
        }
        entities.erase(it);
        selectedEntity = nullptr;
    }
}

void GameEditor::MoveEntityZIndex(GameEntity* entity, int direction) {
    if (!entity) return;
    
    entity->zIndex += direction;
    SortEntitiesByZIndex();
}

void GameEditor::SortEntitiesByZIndex() {
    std::sort(entities.begin(), entities.end(),
        [](const std::unique_ptr<GameEntity>& a, const std::unique_ptr<GameEntity>& b) {
            return a->zIndex < b->zIndex;
        });
}

GameEntity* GameEditor::GetEntityAtPosition(float x, float y) {
    // Check entities in reverse z-order (highest z-index first)
    for (auto it = entities.rbegin(); it != entities.rend(); ++it) {
        GameEntity* entity = it->get();
        if (x >= entity->x && x <= entity->x + entity->width &&
            y >= entity->y && y <= entity->y + entity->height) {
            return entity;
        }
    }
    return nullptr;
}

void GameEditor::RenderCanvas() {
    float canvasWidth = showEntityInspector ? 
        ImGui::GetContentRegionAvail().x * 0.7f : 
        ImGui::GetContentRegionAvail().x;
    
    ImGui::BeginChild("editor_main", ImVec2(canvasWidth, 0), true);
    
    ImGui::Text("Canvas (2D): Click to select entities, drag to move, scroll to zoom");
    
    HandleCanvasInput();
    
    ImGui::InvisibleButton("canvas_placeholder", 
        ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 20));
    
    ImVec2 canvasPos = ImGui::GetItemRectMin();
    ImVec2 canvasSize = ImGui::GetItemRectSize();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    
    // Draw grid
    if (showGrid) {
        const float gridStep = 32.0f * canvasZoom;
        for (float x = std::fmod(cameraX, gridStep); x < canvasSize.x; x += gridStep) {
            draw->AddLine(ImVec2(canvasPos.x + x, canvasPos.y), 
                         ImVec2(canvasPos.x + x, canvasPos.y + canvasSize.y), 
                         IM_COL32(200, 200, 200, 40));
        }
        for (float y = std::fmod(cameraY, gridStep); y < canvasSize.y; y += gridStep) {
            draw->AddLine(ImVec2(canvasPos.x, canvasPos.y + y), 
                         ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + y), 
                         IM_COL32(200, 200, 200, 40));
        }
    }
    
    // Draw entities
    for (const auto& entity : entities) {
        float screenX = canvasPos.x + (entity->x - cameraX) * canvasZoom;
        float screenY = canvasPos.y + (entity->y - cameraY) * canvasZoom;
        float screenW = entity->width * canvasZoom;
        float screenH = entity->height * canvasZoom;
        
        // Draw entity rectangle (fallback if no texture)
        ImU32 color = IM_COL32(100, 180, 240, 200);
        switch (entity->type) {
            case EntityType::CHARACTER: color = IM_COL32(100, 240, 100, 200); break;
            case EntityType::BACKGROUND: color = IM_COL32(240, 240, 100, 200); break;
            case EntityType::OBSTACLE: color = IM_COL32(240, 100, 100, 200); break;
            case EntityType::OTHER: color = IM_COL32(180, 100, 240, 200); break;
        }
        
        if (entity->texture) {
            // TODO: Draw texture using ImGui image integration
            // For now, draw colored rectangle
            draw->AddRectFilled(ImVec2(screenX, screenY), 
                              ImVec2(screenX + screenW, screenY + screenH), 
                              color);
        } else {
            draw->AddRectFilled(ImVec2(screenX, screenY), 
                              ImVec2(screenX + screenW, screenY + screenH), 
                              color);
        }
        
        // Draw selection outline
        if (entity->isSelected || entity.get() == selectedEntity) {
            draw->AddRect(ImVec2(screenX - 2, screenY - 2), 
                         ImVec2(screenX + screenW + 2, screenY + screenH + 2), 
                         IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
        }
        
        // Draw entity name and z-index
        char labelText[256];
        snprintf(labelText, sizeof(labelText), "%s (Z:%d)", entity->name.c_str(), entity->zIndex);
        draw->AddText(ImVec2(screenX + 2, screenY + 2), 
                     IM_COL32(255, 255, 255, 255), labelText);
    }
    
    ImGui::EndChild();
}

void GameEditor::RenderInspector() {
    ImGui::BeginChild("inspector", ImVec2(0, 0), true);
    ImGui::Text("Inspector");
    ImGui::Separator();
    
    ImGui::TextWrapped("Project: %s", currentProjectPath.c_str());
    ImGui::Spacing();
    
    ImGui::Text("Camera: (%.1f, %.1f) Zoom: %.2f", cameraX, cameraY, canvasZoom);
    ImGui::SliderFloat("Zoom", &canvasZoom, 0.1f, 5.0f);
    
    ImGui::Separator();
    ImGui::Text("Entities (%zu)", entities.size());
    
    // Entity list
    for (size_t i = 0; i < entities.size(); ++i) {
        GameEntity* entity = entities[i].get();
        bool isSelected = (entity == selectedEntity);
        
        ImGui::PushID(static_cast<int>(i));
        if (ImGui::Selectable(entity->name.c_str(), isSelected)) {
            selectedEntity = isSelected ? nullptr : entity;
        }
        
        // Right-click context menu
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Move Up")) {
                MoveEntityZIndex(entity, 1);
            }
            if (ImGui::MenuItem("Move Down")) {
                MoveEntityZIndex(entity, -1);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete")) {
                selectedEntity = entity;
                RemoveSelectedEntity();
            }
            ImGui::EndPopup();
        }
        
        ImGui::PopID();
    }
    
    ImGui::Separator();
    
    // Selected entity properties
    if (selectedEntity) {
        ImGui::Text("Selected Entity: %s", selectedEntity->name.c_str());
        
        char nameBuffer[256];
        strcpy(nameBuffer, selectedEntity->name.c_str());
        if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
            selectedEntity->name = nameBuffer;
        }
        
        const char* typeNames[] = {"Character", "Background", "Obstacle", "Other"};
        int currentType = static_cast<int>(selectedEntity->type);
        if (ImGui::Combo("Type", &currentType, typeNames, 4)) {
            selectedEntity->type = static_cast<EntityType>(currentType);
        }
        
        ImGui::DragFloat("X", &selectedEntity->x, 1.0f);
        ImGui::DragFloat("Y", &selectedEntity->y, 1.0f);
        ImGui::DragFloat("Width", &selectedEntity->width, 1.0f, 1.0f, 1000.0f);
        ImGui::DragFloat("Height", &selectedEntity->height, 1.0f, 1.0f, 1000.0f);
        ImGui::DragInt("Z-Index", &selectedEntity->zIndex);
        
        if (ImGui::Button("Sort by Z-Index")) {
            SortEntitiesByZIndex();
        }
        
        ImGui::TextWrapped("Image: %s", selectedEntity->imagePath.c_str());
    }
    
    ImGui::EndChild();
}

void GameEditor::RenderImportDialog() {
    ImGui::OpenPopup("Import Entity");
    
    if (ImGui::BeginPopupModal("Import Entity", &showImportDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        const char* typeNames[] = {"Character", "Background", "Obstacle", "Other"};
        int currentType = static_cast<int>(importType);
        ImGui::Combo("Entity Type", &currentType, typeNames, 4);
        importType = static_cast<EntityType>(currentType);
        
        ImGui::InputText("Entity Name", importNameBuffer, sizeof(importNameBuffer));
        ImGui::InputText("Image Path", importPathBuffer, sizeof(importPathBuffer));
        
        ImGui::SameLine();
        if (ImGui::Button("Browse...")) {
            showFileBrowser = true;
            currentBrowserPath = ".";
            RefreshBrowserEntries();
        }
        
        ImGui::Spacing();
        
        if (ImGui::Button("Import", ImVec2(120, 0))) {
            if (strlen(importNameBuffer) > 0 && strlen(importPathBuffer) > 0) {
                AddEntity(importNameBuffer, importPathBuffer, importType);
                showImportDialog = false;
                importNameBuffer[0] = '\0';
                importPathBuffer[0] = '\0';
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            showImportDialog = false;
            importNameBuffer[0] = '\0';
            importPathBuffer[0] = '\0';
        }
        
        ImGui::EndPopup();
    }
}

void GameEditor::HandleCanvasInput() {
    ImGuiIO& io = ImGui::GetIO();
    
    if (ImGui::IsItemHovered()) {
        // Zoom with mouse wheel
        if (io.MouseWheel != 0.0f) {
            canvasZoom *= (1.0f + io.MouseWheel * 0.1f);
            canvasZoom = std::max(0.1f, std::min(canvasZoom, 5.0f));
        }
        
        // Pan with middle mouse button
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
            ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
            cameraX -= delta.x / canvasZoom;
            cameraY -= delta.y / canvasZoom;
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
        }
        
        // Select/move entities with left mouse button
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 canvasPos = ImGui::GetItemRectMin();
            
            float worldX = (mousePos.x - canvasPos.x) / canvasZoom + cameraX;
            float worldY = (mousePos.y - canvasPos.y) / canvasZoom + cameraY;
            
            GameEntity* clickedEntity = GetEntityAtPosition(worldX, worldY);
            selectedEntity = clickedEntity;
        }
        
        // Move selected entity
        if (selectedEntity && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            selectedEntity->x += delta.x / canvasZoom;
            selectedEntity->y += delta.y / canvasZoom;
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }
    }
}

void GameEditor::CleanupTextures() {
    for (const auto& entity : entities) {
        if (entity->texture) {
            SDL_DestroyTexture(entity->texture);
        }
    }
}

void GameEditor::RefreshBrowserEntries() {
    browserEntries.clear();
    
    try {
        // Add parent directory entry if not at root
        if (currentBrowserPath != "/" && currentBrowserPath != ".") {
            std::filesystem::path parentPath = std::filesystem::path(currentBrowserPath).parent_path();
            if (parentPath.empty()) parentPath = ".";
            browserEntries.emplace_back(parentPath);
        }
        
        // Add all entries in current directory
        for (const auto& entry : std::filesystem::directory_iterator(currentBrowserPath)) {
            browserEntries.push_back(entry);
        }
        
        // Sort entries: directories first, then files
        std::sort(browserEntries.begin(), browserEntries.end(), 
            [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b) {
                if (a.is_directory() && !b.is_directory()) return true;
                if (!a.is_directory() && b.is_directory()) return false;
                return a.path().filename() < b.path().filename();
            });
    }
    catch (const std::exception& e) {
        std::cerr << "Error reading directory: " << e.what() << std::endl;
    }
}

bool GameEditor::IsImageFile(const std::string& filename) {
    std::string ext = std::filesystem::path(filename).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    return (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || 
            ext == ".bmp" || ext == ".tga" || ext == ".gif" ||
            ext == ".tiff" || ext == ".webp");
}

void GameEditor::RenderFileBrowser() {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Select Image File", &showFileBrowser)) {
        // Current path display
        ImGui::Text("Current Path: %s", currentBrowserPath.c_str());
        ImGui::Separator();
        
        // File/folder list
        ImGui::BeginChild("FileList", ImVec2(0, -30), true);
        
        for (const auto& entry : browserEntries) {
            std::string filename = entry.path().filename().string();
            bool isDirectory = entry.is_directory();
            bool isParent = false;
            
            // Check if this is parent directory entry
            if (filename.empty() || filename == "..") {
                filename = "..";
                isParent = true;
                isDirectory = true;
            }
            
            // Icon and name
            if (isDirectory) {
                ImGui::Text("📁 %s", filename.c_str());
            } else if (IsImageFile(filename)) {
                ImGui::Text("🖼️ %s", filename.c_str());
            } else {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "📄 %s", filename.c_str());
            }
            
            // Handle double-click
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (isDirectory) {
                    if (isParent) {
                        currentBrowserPath = entry.path().string();
                    } else {
                        currentBrowserPath = entry.path().string();
                    }
                    RefreshBrowserEntries();
                } else if (IsImageFile(filename)) {
                    // Select this file
                    selectedFilePath = entry.path().string();
                    strcpy(importPathBuffer, selectedFilePath.c_str());
                    showFileBrowser = false;
                }
            }
            
            // Handle single-click selection for files
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !isDirectory) {
                if (IsImageFile(filename)) {
                    selectedFilePath = entry.path().string();
                }
            }
            
            // Highlight selected file
            if (!isDirectory && entry.path().string() == selectedFilePath) {
                ImVec2 itemMin = ImGui::GetItemRectMin();
                ImVec2 itemMax = ImGui::GetItemRectMax();
                ImGui::GetWindowDrawList()->AddRectFilled(
                    itemMin, itemMax, IM_COL32(100, 150, 255, 50));
            }
        }
        
        ImGui::EndChild();
        
        // Bottom buttons
        ImGui::Separator();
        
        if (ImGui::Button("Select", ImVec2(80, 0))) {
            if (!selectedFilePath.empty() && IsImageFile(selectedFilePath)) {
                strcpy(importPathBuffer, selectedFilePath.c_str());
                showFileBrowser = false;
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80, 0))) {
            showFileBrowser = false;
            selectedFilePath.clear();
        }
        
        ImGui::SameLine();
        ImGui::Text("Selected: %s", selectedFilePath.empty() ? "None" : 
                   std::filesystem::path(selectedFilePath).filename().string().c_str());
    }
    ImGui::End();
}
