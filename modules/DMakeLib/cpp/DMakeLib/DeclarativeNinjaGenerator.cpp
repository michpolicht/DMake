#include "DeclarativeNinjaGenerator.hpp"

#include <CMakeCommand.hpp>
#include <cmAddExecutableCommand.h>
#include <cmExecutionStatus.h>
#include <cmMakefile.h>
#include <cmProjectCommand.h>
#include <cmStateDirectory.h>
#include <cmake.h>

#include <QQmlApplicationEngine>
#include <QtDebug>

namespace DMakeLib {

template<typename T>
class GlobalGeneratorQmlEngineFactory : public cmGlobalGeneratorFactory
{
public:
    GlobalGeneratorQmlEngineFactory(QQmlApplicationEngine *engine)
        : m_engine(engine)
    {}

    /** Create a GlobalGenerator */
    std::unique_ptr<cmGlobalGenerator> CreateGlobalGenerator(std::string const &name,
                                                             cmake *cm) const override
    {
        if (name != T::GetActualName()) {
            return std::unique_ptr<cmGlobalGenerator>();
        }
        return std::unique_ptr<cmGlobalGenerator>(cm::make_unique<T>(cm, m_engine));
    }

    /** Get the documentation entry for this factory */
    cmDocumentationEntry GetDocumentation() const override { return T::GetDocumentation(); }

    /** Get the names of the current registered generators */
    std::vector<std::string> GetGeneratorNames() const override { return {T::GetActualName()}; }

    /** Determine whether or not this generator supports toolsets */
    bool SupportsToolset() const override { return T::SupportsToolset(); }

    /** Determine whether or not this generator supports platforms */
    bool SupportsPlatform() const override { return T::SupportsPlatform(); }

    /** Get the list of supported platforms name for this generator */
    std::vector<std::string> GetKnownPlatforms() const override
    {
        // default is no platform supported
        return {};
    }

    std::string GetDefaultPlatformName() const override { return {}; }

private:
    QPointer<QQmlApplicationEngine> m_engine;
};

std::unique_ptr<cmGlobalGeneratorFactory> DeclarativeNinjaGenerator::NewFactory(
    QQmlApplicationEngine *qmlApplicationEngine)
{
    return std::unique_ptr<cmGlobalGeneratorFactory>(
        new GlobalGeneratorQmlEngineFactory<DeclarativeNinjaGenerator>(qmlApplicationEngine));
}

std::string DeclarativeNinjaGenerator::GetActualName()
{
    return "DNinja";
}

DeclarativeNinjaGenerator::DeclarativeNinjaGenerator(cmake *cm,
                                                     QQmlApplicationEngine *qmlApplicationEngine)
    : cmGlobalNinjaGenerator(cm)
    , m_qmlApplicationEngine(qmlApplicationEngine)
{}

void DeclarativeNinjaGenerator::Configure()
{
    qDebug() << "DeclarativeNinjaGenerator is configuring the project...";

    this->FirstTimeProgress = 0.0f;
    // this->ClearGeneratorMembers();
    // this->NextDeferId = 0;

    cmStateSnapshot snapshot = this->CMakeInstance->GetCurrentSnapshot();
    snapshot.GetDirectory().SetCurrentSource(this->CMakeInstance->GetHomeDirectory());
    snapshot.GetDirectory().SetCurrentBinary(this->CMakeInstance->GetHomeOutputDirectory());

    auto dirMfu = cm::make_unique<cmMakefile>(this, snapshot);
    auto *dirMf = dirMfu.get();
    this->Makefiles.push_back(std::move(dirMfu));
    dirMf->SetRecursionDepth(this->RecursionDepth);
    // this->IndexMakefile(dirMf);

    // this->BinaryDirectories.insert(this->CMakeInstance->GetHomeOutputDirectory());

    // if (this->ExtraGenerator && !this->CMakeInstance->GetIsInTryCompile()) {
    //     this->CMakeInstance
    //         ->IssueMessage(MessageType::DEPRECATION_WARNING,
    //                        cmStrCat("Support for \"Extra Generators\" like\n  ",
    //                                 this->ExtraGenerator->GetName(),
    //                                 "\nis deprecated and will be removed from a future version "
    //                                 "of CMake.  IDEs may use the cmake-file-api(7) to view "
    //                                 "CMake-generated project build trees."));
    // }

    // now do it
    // dirMf->Configure();
    // dirMf->EnforceDirectoryLevelRules();

    // qDebug() << "CreateAndSetGlobalGenerator: " << cm.CreateAndSetGlobalGenerator("Ninja");
    // cmMakefile makefile(dmake.GetGlobalGenerator(), dmake.GetCurrentSnapshot());
    cmExecutionStatus executionStatus(*dirMf);

    auto projectFilePath = GetCMakeInstance()->GetCMakeListFile(
        snapshot.GetDirectory().GetCurrentSource());
    qDebug() << "Project file:" << projectFilePath << "...";
    QObject::connect(m_qmlApplicationEngine,
                     &QQmlApplicationEngine::objectCreated,
                     m_qmlApplicationEngine,
                     [this, dirMf](QObject *object, const QUrl &ur) {
                         qDebug() << "Project object" << object << "loaded";
                         cmExecutionStatus executionStatus(*dirMf);
                         auto cmakeCommandObject = qobject_cast<CMakeCommand *>(object);
                         if (cmakeCommandObject) {
                             cmakeCommandObject->execute(executionStatus);
                         }

                         emit m_qmlApplicationEngine->quit();
                     });
    m_qmlApplicationEngine->load(QString::fromStdString(projectFilePath));

    // Put a copy of each global target in every directory.
    {
        std::vector<GlobalTargetInfo> globalTargets;
        this->CreateDefaultGlobalTargets(globalTargets);

        for (auto const &mf : this->Makefiles) {
            for (GlobalTargetInfo const &globalTarget : globalTargets) {
                this->CreateGlobalTarget(globalTarget, mf.get());
            }
        }
    }

    this->ReserveGlobalTargetCodegen();

    // update the cache entry for the number of local generators, this is used
    // for progress
    this->GetCMakeInstance()->AddCacheEntry("CMAKE_NUMBER_OF_MAKEFILES",
                                            std::to_string(this->Makefiles.size()),
                                            "number of local generators",
                                            cmStateEnums::INTERNAL);
}

std::string DeclarativeNinjaGenerator::GetName() const
{
    return cmGlobalNinjaGenerator::GetActualName();
}

} // namespace DMakeLib
