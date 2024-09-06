//
// Created by Nikita on 11.07.2021.
//

#ifndef SR_ENGINE_SREVOSCRIPT_H
#define SR_ENGINE_SREVOSCRIPT_H

#include <Scripting/Base/Script.h>
#include <EvoScript/Script.h>
#include <Utils/FileSystem/Path.h>

namespace SR_SCRIPTING_NS {
    class Compiler;

    class SR_DEPRECATED EvoScriptImpl : public Script {
        using Super = Script;
    public:
        EvoScriptImpl(Compiler* compiler, const std::string& name, const SR_UTILS_NS::Path& path)
            : Super(compiler, name, path.ToString())
        { }

        ~EvoScriptImpl() override = default;

    public:
        bool Compile() override;

    private:
        EvoScript::Script* m_script = nullptr;

    };
}

#endif //SR_ENGINE_SREVOSCRIPT_H
