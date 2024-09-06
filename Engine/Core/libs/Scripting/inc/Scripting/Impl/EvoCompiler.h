//
// Created by Nikita on 11.07.2021.
//

#ifndef SR_ENGINE_EVOCOMPILER_H
#define SR_ENGINE_EVOCOMPILER_H

#include <Scripting/Base/Compiler.h>

#include <Utils/Resources/Xml.h>
#include <EvoScript/Script.h>
#include <EvoScript/Compilation/Compiler.h>
#include <EvoScript/Compilation/AddressTableGen.h>
#include <EvoScript/Compilation/Casting.h>

namespace SR_SCRIPTING_NS {
    class SR_DLL_EXPORT EvoCompiler : public SR_SCRIPTING_NS::Compiler, public EvoScript::Compiler {
    public:
        explicit EvoCompiler(std::string cachePath);
        ~EvoCompiler() override;

    public:
        SR_NODISCARD EvoScript::CastingGen* GetCasting() const { return m_casting; }
        SR_NODISCARD EvoScript::AddressTableGen* GetGenerator() const;

    public:
        bool Init() override;

    private:
        SR_NODISCARD std::string GetGeneratorName(const SR_XML_NS::Node& config) const;

    private:
        EvoScript::AddressTableGen* m_generator = nullptr;
        EvoScript::CastingGen*      m_casting   = nullptr;

    };

    class SR_DLL_EXPORT GlobalEvoCompiler : public SR_UTILS_NS::Singleton<GlobalEvoCompiler>, public EvoCompiler  {
        SR_REGISTER_SINGLETON(GlobalEvoCompiler)
    protected:
        GlobalEvoCompiler();
        ~GlobalEvoCompiler() override = default;

    protected:
        void OnSingletonDestroy() override {
            SR_INFO("GlobalEvoCompiler::OnSingletonDestroy() : free global compiler...");
        }

    };
}

#endif //SR_ENGINE_EVOCOMPILER_H
