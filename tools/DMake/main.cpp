#include <DMakeLib/DMake.hpp>

#include <cmCommandLineArgument.h>
#include <cmExecutionStatus.h>
#include <cmMakefile.h>
#include <cmProjectCommand.h>
#include <cmake.h>

#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QtQml/QQmlExtensionPlugin>

Q_IMPORT_QML_PLUGIN(DMakeLibPlugin)

// using Command = std::function<bool(std::vector<cmListFileArgument> const&,
//                                    cmExecutionStatus&)>;

std::function<bool(std::string const &value)> getShowCachedCallback(bool &show_flag,
                                                                    bool *help_flag = nullptr,
                                                                    std::string *filter = nullptr)
{
    return [=, &show_flag](std::string const &value) -> bool {
        show_flag = true;
        if (help_flag) {
            *help_flag = true;
        }
        if (filter) {
            *filter = value;
        }
        return true;
    };
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QQmlApplicationEngine qmlApplicationEngine;

    cmsys::Encoding::CommandLineArguments args = cmsys::Encoding::CommandLineArguments::Main(argc,
                                                                                             argv);
    auto ac = args.argc();
    auto av = args.argv();

    cmSystemTools::InitializeLibUV();
    cmSystemTools::FindCMakeResources(av[0]);

    // TODO: Wrap this into some struct and provide a function that returns is as the result.
    bool wizard_mode = false;
    bool sysinfo = false;
    bool list_cached = false;
    bool list_all_cached = false;
    bool list_help = false;
    // (Regex) Filter on the cached variable(s) to print.
    std::string filter_var_name;
    bool view_only = false;
    cmState::Role role = cmState::Role::Project;
    std::vector<std::string> parsedArgs;

    using CommandArgument = cmCommandLineArgument<bool(std::string const &value)>;
    std::vector<CommandArgument> arguments = {
        CommandArgument{"-i",
                        CommandArgument::Values::Zero,
                        [&wizard_mode](std::string const &) -> bool {
                            /* clang-format off */
        qCritical() <<
          "The \"cmake -i\" wizard mode is no longer supported.\n"
          "Use the -D option to set cache values on the command line.\n"
          "Use cmake-gui or ccmake for an interactive dialog.\n";
                            /* clang-format on */
                            wizard_mode = true;
                            return true;
                        }},
        CommandArgument{"--system-information",
                        CommandArgument::Values::Zero,
                        CommandArgument::setToTrue(sysinfo)},
        CommandArgument{"-N", CommandArgument::Values::Zero, CommandArgument::setToTrue(view_only)},
        CommandArgument{"-LAH",
                        CommandArgument::Values::Zero,
                        getShowCachedCallback(list_all_cached, &list_help)},
        CommandArgument{"-LA",
                        CommandArgument::Values::Zero,
                        getShowCachedCallback(list_all_cached)},
        CommandArgument{"-LH",
                        CommandArgument::Values::Zero,
                        getShowCachedCallback(list_cached, &list_help)},
        CommandArgument{"-L", CommandArgument::Values::Zero, getShowCachedCallback(list_cached)},
        CommandArgument{"-LRAH",
                        CommandArgument::Values::One,
                        getShowCachedCallback(list_all_cached, &list_help, &filter_var_name)},
        CommandArgument{"-LRA",
                        CommandArgument::Values::One,
                        getShowCachedCallback(list_all_cached, nullptr, &filter_var_name)},
        CommandArgument{"-LRH",
                        CommandArgument::Values::One,
                        getShowCachedCallback(list_cached, &list_help, &filter_var_name)},
        CommandArgument{"-LR",
                        CommandArgument::Values::One,
                        getShowCachedCallback(list_cached, nullptr, &filter_var_name)},
        CommandArgument{"-P",
                        "No script specified for argument -P",
                        CommandArgument::Values::One,
                        CommandArgument::RequiresSeparator::No,
                        [&](std::string const &value) -> bool {
                            role = cmState::Role::Script;
                            parsedArgs.emplace_back("-P");
                            parsedArgs.push_back(value);
                            return true;
                        }},
        CommandArgument{"--find-package",
                        CommandArgument::Values::Zero,
                        [&](std::string const &) -> bool {
                            role = cmState::Role::FindPackage;
                            parsedArgs.emplace_back("--find-package");
                            return true;
                        }},
        CommandArgument{"--list-presets",
                        CommandArgument::Values::ZeroOrOne,
                        [&](std::string const &value) -> bool {
                            role = cmState::Role::Help;
                            parsedArgs.emplace_back("--list-presets");
                            parsedArgs.emplace_back(value);
                            return true;
                        }},
    };

    std::vector<std::string> inputArgs;
    inputArgs.reserve(ac);
    cm::append(inputArgs, av, av + ac);

    for (decltype(inputArgs.size()) i = 0; i < inputArgs.size(); ++i) {
        std::string const &arg = inputArgs[i];
        bool matched = false;

        // Only in script mode do we stop parsing instead
        // of preferring the last mode flag provided
        if (arg == "--" && role == cmState::Role::Script) {
            parsedArgs = inputArgs;
            break;
        }
        for (auto const &m : arguments) {
            if (m.matches(arg)) {
                matched = true;
                if (m.parse(arg, i, inputArgs)) {
                    break;
                }
                return 1; // failed to parse
            }
        }
        if (!matched) {
            parsedArgs.emplace_back(av[i]);
        }
    }

    if (wizard_mode) {
        return 1;
    }

    // qDebug() << CMAKE_DATA_DIR;
    DMakeLib::DMake dmake(&qmlApplicationEngine, cmState::Role::Project);
    int res = dmake.run(parsedArgs, view_only);

    if (list_cached || list_all_cached) {
        qInfo() << "-- Cache values";
        std::vector<std::string> keys = dmake.GetState()->GetCacheEntryKeys();
        cmsys::RegularExpression regex_var_name;
        if (!filter_var_name.empty()) {
            regex_var_name.compile(filter_var_name);
        }
        for (std::string const &k : keys) {
            if (regex_var_name.is_valid() && !regex_var_name.find(k)) {
                continue;
            }

            cmStateEnums::CacheEntryType t = dmake.GetState()->GetCacheEntryType(k);
            if (t != cmStateEnums::INTERNAL && t != cmStateEnums::STATIC
                && t != cmStateEnums::UNINITIALIZED) {
                cmValue advancedProp = dmake.GetState()->GetCacheEntryProperty(k, "ADVANCED");
                if (list_all_cached || !advancedProp) {
                    if (list_help) {
                        cmValue help = dmake.GetState()->GetCacheEntryProperty(k, "HELPSTRING");
                        qInfo() << "// " << (help ? *help : "");
                    }
                    qInfo() << k << ":" << cmState::CacheEntryTypeToString(t) << "="
                            << dmake.GetState()->GetSafeCacheEntryValue(k);
                    if (list_help) {
                        qInfo() << "";
                    }
                }
            }
        }
    }

    // Always return a non-negative value (except exit code from SCRIPT_MODE).
    // Windows tools do not always interpret negative return values as errors.
    if (res != 0) {
        auto scriptModeExitCode = dmake.HasScriptModeExitCode() ? dmake.GetScriptModeExitCode() : 0;
        res = scriptModeExitCode ? scriptModeExitCode : 1;
#ifdef CMake_ENABLE_DEBUGGER
        dmake.StopDebuggerIfNeeded(res);
#endif
        return res;
    }
#ifdef CMake_ENABLE_DEBUGGER
    dmake.StopDebuggerIfNeeded(0);
#endif

    // Quit without running event loop if no QML file was specified.
    if (qmlApplicationEngine.rootObjects().empty()) {
        return EXIT_SUCCESS;
    }

    return app.exec();
}
