# SpaRcle Engine

![image](https://user-images.githubusercontent.com/47291246/183247603-04a0e820-8b32-49f1-a728-cec67b53a5e7.png)

## Supported compilers
- [x] MSVC (Stable)
- [x] MinGW
- [ ] Cygwin
- [x] Clang
- [ ] GCC

## Supported platforms
- [x] Windows 10
- [ ] Ubuntu
- [ ] Arch
- [ ] Redhat
- [ ] Android

## Building
 1. Install [Vulkan SDK/Runtime](https://vulkan.lunarg.com/sdk/home) 1.3.211.0
 2. Install CMake >=  3.16
 3. Run command "git submodule update --init --recursive"
 4. Generate system build files with CMake
 5. Build and run Engine.exe with command line argument "-resources path-to-your-repository/Resources"

## Used libraries
  * glm
  * OpenGL / GLFW / GLEW / glad
  * Bullet3
  * [ImGUI](https://github.com/ocornut/imgui)
  * [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo)
  * [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)
  * [imgui-node-editor](https://github.com/thedmd/imgui-node-editor)
  * stbi
  * json
  * [EvoScript](https://github.com/Monika0000/EvoScript)
  * [EvoVulkan](https://github.com/Monika0000/EvoVulkan)
  * Assimp

## Exists systems
  * Types
      * Thread - обертка над стандартным классом, с некоторыми дополнениями
      * SafePtr - универсальный умный указатель, обеспечивает сохранность данных в многопоточной среде
      * Singleton
  * Math
      * Quaternion
      * Vector4
      * Vector3
      * Vector2
      * Matrix4x4
  * Graphics
      * Node editor
      * Mesh-cluster
      * GUI 
      * Model loader
      * Texturing
        * Compressing BC1-BC7 (Only Vulkan)
      * Shaders
      * Materials
  * Physics (Не начато)
      * Rigidbody (Base component)
  * Scripting (Полностью сделано через EvoScript)
  * Audio (Не начато)
  * World
      * Scene 
      * GameObject 
         * Transform
         * Hierarchy-transformation (80%)
         * Components
      * Chunk-System - чанковая система, которая разбивает мир на чанки и регионы, позволяя удаляться на огромные расстояния от центра мира без каких либо артефактов, производя для этого "сдвиги"
  * Input system (Только стандартый ввод Windows)
  * Memory management
      * Dynamic resource reloading
      * ResourceManager
      * IResource - interface for control resource units
  * Debug and logging
  * Command Manager
 
## Editor
  * ~~Grid (Only OpenGL)~~
  * Manipulation tools
  * Inspector 
  * Hierarchy
  * Asset explorer
  * World edit
  * ~~Visual Evo Script~~
