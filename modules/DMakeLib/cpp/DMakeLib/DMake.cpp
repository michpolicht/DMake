#include "DMake.hpp"

#include <cmGlobalFastbuildGenerator.h>
#include <cmMessenger.h>
#include <cmFileTimeCache.h>
#include <cmCommandLineArgument.h>
#include <cmSarifLog.h>

#include <iostream>

namespace {

#if !defined(CMAKE_BOOTSTRAP)
using JsonValueMapType = std::unordered_map<std::string, Json::Value>;
#endif

auto IgnoreAndTrueLambda = [](std::string const&, cmake*) -> bool {
    return true;
};

using CommandArgument =
    cmCommandLineArgument<bool(std::string const& value, cmake* state)>;

#ifndef CMAKE_BOOTSTRAP
void cmWarnUnusedCliWarning(std::string const& variable, int /*unused*/,
                            void* ctx, char const* /*unused*/,
                            cmMakefile const* /*unused*/)
{
    cmake* cm = reinterpret_cast<cmake*>(ctx);
    cm->MarkCliAsUsed(variable);
}
#endif

bool cmakeCheckStampFile(std::string const& stampName)
{
    // The stamp file does not exist.  Use the stamp dependencies to
    // determine whether it is really out of date.  This works in
    // conjunction with cmLocalVisualStudio7Generator to avoid
    // repeatedly re-running CMake when the user rebuilds the entire
    // solution.
    std::string stampDepends = cmStrCat(stampName, ".depend");
#if defined(_WIN32) || defined(__CYGWIN__)
    cmsys::ifstream fin(stampDepends.c_str(), std::ios::in | std::ios::binary);
#else
    cmsys::ifstream fin(stampDepends.c_str());
#endif
    if (!fin) {
        // The stamp dependencies file cannot be read.  Just assume the
        // build system is really out of date.
        std::cout << "CMake is re-running because " << stampName
                  << " dependency file is missing.\n";
        return false;
    }

    // Compare the stamp dependencies against the dependency file itself.
    {
        cmFileTimeCache ftc;
        std::string dep;
        while (cmSystemTools::GetLineFromStream(fin, dep)) {
            int result;
            if (!dep.empty() && dep[0] != '#' &&
                (!ftc.Compare(stampDepends, dep, &result) || result < 0)) {
                // The stamp depends file is older than this dependency.  The
                // build system is really out of date.
                /* clang-format off */
        std::cout << "CMake is re-running because " << stampName
                  << " is out-of-date.\n"
                     "  the file '" << dep << "'\n"
                     "  is newer than '" << stampDepends << "'\n"
                     "  result='" << result << "'\n";
                /* clang-format on */
                return false;
            }
        }
    }

    // The build system is up to date.  The stamp file has been removed
    // by the VS IDE due to a "rebuild" request.  Restore it atomically.
    std::ostringstream stampTempStream;
    stampTempStream << stampName << ".tmp" << cmSystemTools::RandomNumber();
    std::string stampTemp = stampTempStream.str();
    {
        // TODO: Teach cmGeneratedFileStream to use a random temp file (with
        // multiple tries in unlikely case of conflict) and use that here.
        cmsys::ofstream stamp(stampTemp.c_str());
        stamp << "# CMake generation timestamp file for this directory.\n";
    }
    std::string err;
    if (cmSystemTools::RenameFile(stampTemp, stampName,
                                  cmSystemTools::Replace::Yes, &err) ==
        cmSystemTools::RenameResult::Success) {
        // CMake does not need to re-run because the stamp file is up-to-date.
        return true;
    }
    cmSystemTools::RemoveFile(stampTemp);
    cmSystemTools::Error(
        cmStrCat("Cannot restore timestamp \"", stampName, "\": ", err));
    return false;
}

bool cmakeCheckStampList(std::string const& stampList)
{
    // If the stamp list does not exist CMake must rerun to generate it.
    if (!cmSystemTools::FileExists(stampList)) {
        std::cout << "CMake is re-running because generate.stamp.list "
                     "is missing.\n";
        return false;
    }
    cmsys::ifstream fin(stampList.c_str());
    if (!fin) {
        std::cout << "CMake is re-running because generate.stamp.list "
                     "could not be read.\n";
        return false;
    }

    // Check each stamp.
    std::string stampName;
    while (cmSystemTools::GetLineFromStream(fin, stampName)) {
        if (!cmakeCheckStampFile(stampName)) {
            return false;
        }
    }
    return true;
}

} // namespace

namespace DMakeLib {

DMake::DMake(cmState::Role role, cmState::TryCompile isTryCompile, QObject *parent)
    : QObject(parent),
    cmake(role, isTryCompile)
{
}

int DMake::run(const std::vector<std::string> &args, bool noconfigure)
{
    // Process the arguments
    this->SetArgs(args);
    if (cmSystemTools::GetErrorOccurredFlag()) {
        return -1;
    }
    if (GetState()->GetRole() == cmState::Role::Help) {
        return 0;
    }

#ifndef CMAKE_BOOTSTRAP
    // Configure the SARIF log for the current run
    cmSarif::LogFileWriter sarifLogFileWriter(
        this->GetMessenger()->GetSarifResultsLog());
    if (!sarifLogFileWriter.ConfigureForCMakeRun(*this)) {
        return -1;
    }
#endif

    // Log the trace format version to the desired output
    if (this->GetTrace()) {
        this->PrintTraceFormatVersion();
    }

    // If we are given a stamp list file check if it is really out of date.
    if (!this->CheckStampList.empty() &&
        cmakeCheckStampList(this->CheckStampList)) {
        return 0;
    }

    // If we are given a stamp file check if it is really out of date.
    if (!this->CheckStampFile.empty() &&
        cmakeCheckStampFile(this->CheckStampFile)) {
        return 0;
    }

    if (GetState()->GetRole() == cmState::Role::Project) {
        if (this->FreshCache) {
            this->DeleteCache(this->GetHomeOutputDirectory());
        }
        // load the cache
        if (this->LoadCache() < 0) {
            cmSystemTools::Error("Error executing cmake::LoadCache(). Aborting.\n");
            return -1;
        }
#ifndef CMAKE_BOOTSTRAP
        // If no SARIF file has been explicitly specified, use the default path
        if (!this->SarifFileOutput) {
            // If no output file is specified, use the default path
            // Enable parent directory creation for the default path
            sarifLogFileWriter.SetPath(
                cm::filesystem::path(this->GetHomeOutputDirectory()) /
                    std::string(cmSarif::PROJECT_DEFAULT_SARIF_FILE),
                true);
        }
#endif
    } else {
        if (this->FreshCache) {
            cmSystemTools::Error("--fresh allowed only when configuring a project");
            return -1;
        }
        this->AddCMakePaths();
    }

#ifndef CMAKE_BOOTSTRAP
    this->ProcessPresetVariables();
    this->ProcessPresetEnvironment();
#endif
    // Add any cache args
    if (!this->SetCacheArgs(args)) {
        cmSystemTools::Error("Run 'cmake --help' for all supported options.");
        return -1;
    }
#ifndef CMAKE_BOOTSTRAP
    if (this->GetLogLevel() == Message::LogLevel::LOG_VERBOSE ||
        this->GetLogLevel() == Message::LogLevel::LOG_DEBUG ||
        this->GetLogLevel() == Message::LogLevel::LOG_TRACE) {
        this->PrintPresetVariables();
        this->PrintPresetEnvironment();
    }
#endif

    // In script mode we terminate after running the script.
    if (GetState()->GetRole() != cmState::Role::Project) {
        if (cmSystemTools::GetErrorOccurredFlag()) {
            return -1;
        }
        return this->HasScriptModeExitCode() ? this->GetScriptModeExitCode() : 0;
    }

#ifndef CMAKE_BOOTSTRAP
    // CMake only responds to the SARIF variable in normal mode
    this->MarkCliAsUsed(cmSarif::PROJECT_SARIF_FILE_VARIABLE);
#endif

    // If MAKEFLAGS are given in the environment, remove the environment
    // variable.  This will prevent try-compile from succeeding when it
    // should fail (if "-i" is an option).  We cannot simply test
    // whether "-i" is given and remove it because some make programs
    // encode the MAKEFLAGS variable in a strange way.
    if (cmSystemTools::HasEnv("MAKEFLAGS")) {
        cmSystemTools::PutEnv("MAKEFLAGS=");
    }

    this->PreLoadCMakeFiles();

    if (noconfigure) {
        return 0;
    }

    // now run the global generate
    // Check the state of the build system to see if we need to regenerate.
    if (!this->CheckBuildSystem()) {
        return 0;
    }
    // After generating fbuild.bff, FastBuild sees rebuild-bff as outdated since
    // it hasn’t built the target yet. To make it a no-op for future runs, we
    // trigger a dummy fbuild invocation that creates this marker file and runs
    // CMake, marking rebuild-bff as up-to-date.
    std::string const FBuildRestatFile =
        cmStrCat(this->GetHomeOutputDirectory(), '/', FASTBUILD_RESTAT_FILE);
    if (cmSystemTools::FileExists(FBuildRestatFile)) {
        cmsys::ifstream restat(FBuildRestatFile.c_str(),
                               std::ios::in | std::ios::binary);
        std::string const file((std::istreambuf_iterator<char>(restat)),
                               std::istreambuf_iterator<char>());
        // On Windows can not delete file if it's still opened.
        restat.close();
        cmSystemTools::Touch(file, true);
        cmSystemTools::RemoveFile(FBuildRestatFile);
        return 0;
    }

#ifdef CMake_ENABLE_DEBUGGER
    if (!this->StartDebuggerIfEnabled()) {
        return -1;
    }
#endif

    int ret = this->Configure();
    if (ret) {
#if defined(CMAKE_HAVE_VS_GENERATORS)
        if (!this->VSSolutionFile.empty() && this->GlobalGenerator) {
            // CMake is running to regenerate a Visual Studio build tree
            // during a build from the VS IDE.  The build files cannot be
            // regenerated, so we should stop the build.
            cmSystemTools::Message("CMake Configure step failed.  "
                                   "Build files cannot be regenerated correctly.  "
                                   "Attempting to stop IDE build.");
            cmGlobalVisualStudioGenerator& gg =
                cm::static_reference_cast<cmGlobalVisualStudioGenerator>(
                    this->GlobalGenerator);
            gg.CallVisualStudioMacro(cmGlobalVisualStudioGenerator::MacroStop,
                                     this->VSSolutionFile);
        }
#endif
        return ret;
    }
    ret = this->Generate();
    if (ret) {
        cmSystemTools::Message("CMake Generate step failed.  "
                               "Build files cannot be regenerated correctly.");
        return ret;
    }
    std::string message = cmStrCat("Build files have been written to: ",
                                   this->GetHomeOutputDirectory());
    this->UpdateProgress(message, -1);
    return ret;
}

}
