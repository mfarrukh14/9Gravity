# 9Gravity Game Engine

A basic 2D game engine built with SDL3 in C++. This engine provides essential functionality for creating 2D games with support for graphics, audio, input handling, physics, and asset management.


You only need:
- **C++17 compatible compiler**
- **CMake** (recommended) or **Make**

**Windows:**
- MinGW-w64 or Visual Studio

**Linux:**
- GCC or Clang (usually pre-installed)
```bash
# Ubuntu/Debian
sudo apt install build-essential cmake

# Fedora
sudo dnf install gcc-c++ make cmake
```

**macOS:**
- Xcode Command Line Tools
```bash
xcode-select --install
brew install cmake  
```

### Building (Easy Way)

#### Windows
```cmd
build.bat
```

#### Linux/macOS
```bash
./build.sh
```

### 🔧 Building (Manual)

#### Option 1: CMake (Recommended)
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

#### Option 2: Make
```bash
make
```

#### Option 3: Manual Compilation
```bash
# Windows
g++ -std=c++17 -Iinclude -ISDL/x86_64-w64-mingw32/include src/*.cpp -o 9Gravity.exe -LSDL/x86_64-w64-mingw32/lib -lSDL3

# Linux/macOS (if you need to build without Make/CMake)
g++ -std=c++17 -Iinclude src/*.cpp -o 9Gravity `pkg-config --cflags --libs sdl3 2>/dev/null || echo "-lSDL3"`
```



## Usage Example

```cpp
#include "Engine.h"
#include "Scene.h"

class MyGame : public Engine {
public:
    bool Initialize() {
        if (!Engine::Initialize("My Game", 800, 600)) {
            return false;
        }
        
        // Setup your game here
        return true;
    }
    
protected:
    void Update(float deltaTime) override {
    }
    
    void Render() override {
        GetRenderer()->Clear();
        GetRenderer()->Present();
    }
};

int main() {
    MyGame game;
    if (game.Initialize()) {
        game.Run();
    }
    return 0;
}
```

## Creating Game Objects

```cpp
class Player : public GameObject {
public:
    Player(float x, float y) {
        position = Vector2(x, y);
        // Load texture
        texture = GetEngine()->GetAssetManager()->LoadTexture("player", "assets/textures/player.png");
    }
    
    void Update(float deltaTime) override {
        // Handle input
        if (GetEngine()->GetInputManager()->IsKeyDown(SDLK_LEFT)) {
            position.x -= speed * deltaTime;
        }
    }
    
    void Render(Renderer* renderer) override {
        if (texture) {
            renderer->DrawTexture(texture.get(), position);
        }
    }
    
private:
    std::shared_ptr<Texture> texture;
    float speed = 200.0f;
};
```

## Loading Assets

```cpp
// Load textures
auto texture = assetManager->LoadTexture("player", "assets/textures/player.png");

// Load sounds
auto sound = audioManager->LoadSound("jump", "assets/audio/jump.wav");
auto music = audioManager->LoadMusic("background", "assets/audio/music.ogg");

// Play audio
audioManager->PlaySound("jump");
audioManager->PlayMusic("background");
```

## Physics System

```cpp
// Create physics body
Physics::Body body;
body.position = Vector2(100, 100);
body.velocity = Vector2(50, 0);
body.mass = 1.0f;

// Update physics
Physics::ApplyGravity(body, Vector2(0, 500)); // Gravity
Physics::UpdateBody(body, deltaTime);

// Collision detection
Physics::AABB box1(position1, width1, height1);
Physics::AABB box2(position2, width2, height2);
if (Physics::CheckCollision(box1, box2)) {
    Physics::ResolveCollision(body1, body2, box1, box2);
}
```

## Input Handling

```cpp
auto input = engine->GetInputManager();

// Keyboard
if (input->IsKeyPressed(SDLK_SPACE)) {
    // Key just pressed this frame
}
if (input->IsKeyDown(SDLK_LEFT)) {
    // Key held down
}

// Mouse
if (input->IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
    // Mouse button clicked
}
int mouseX = input->GetMouseX();
int mouseY = input->GetMouseY();
```

## Troubleshooting

### Build Issues

1. **Compiler not found:**
   - **Windows**: Install MinGW-w64 or Visual Studio
   - **Linux**: Install build-essential or development tools
   - **macOS**: Install Xcode Command Line Tools

2. **CMake not found:**
   - Install CMake from cmake.org or use package manager
   - Alternative: Use Make instead (`build.bat make` or `./build.sh make`)

3. **Architecture mismatch (Windows):**
   - The project auto-detects architecture (x86_64 or i686)
   - Ensure your compiler matches your system architecture

### Runtime Issues

1. **SDL3.dll not found (Windows):**
   - The build system automatically copies SDL3.dll
   - If issues persist, manually copy `SDL/x86_64-w64-mingw32/bin/SDL3.dll` to your executable directory

2. **Permission denied (Linux/macOS):**
   - Make the build script executable: `chmod +x build.sh`
   - Run with: `./build.sh`

3. **Assets not loading:**
   - Check that the assets folder is in the same directory as your executable
   - Verify file paths are correct (case-sensitive on Linux/macOS)

## Requirements

- **C++17** compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **CMake 3.16+** (recommended) or Make
- **Operating System**: Windows 10+, Linux, or macOS 10.14+


## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! This is an educational engine that can be extended with:

- Animation system
- Particle effects  
- Tilemap support
- UI system
- Scripting support
- More physics features
- Advanced audio features

## Support

If you encounter issues:

1. Check the [Troubleshooting](#🛠️-troubleshooting) section
2. Ensure all [Requirements](#📋-requirements) are met
3. Try the build scripts: `build.bat` (Windows) or `./build.sh` (Linux/macOS)
4. Open an issue with your error details and system information

---

**Happy coding! 🎮**
