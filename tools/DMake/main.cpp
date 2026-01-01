#include <QCoreApplication>

#include <cmProjectCommand.h>
#include <cmMakefile.h>
#include <cmake.h>
#include <cmExecutionStatus.h>

#include <DMakeLib/DMake.hpp>

// using Command = std::function<bool(std::vector<cmListFileArgument> const&,
//                                    cmExecutionStatus&)>;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    cmsys::Encoding::CommandLineArguments args =
        cmsys::Encoding::CommandLineArguments::Main(argc, argv);
    auto ac = args.argc();
    auto av = args.argv();

    cmSystemTools::InitializeLibUV();
    cmSystemTools::FindCMakeResources(av[0]);

    // qDebug() << CMAKE_DATA_DIR;
    DMakeLib::DMake dmake(cmState::Role::Project);
    dmake.run({});

    // cmake cm(cmState::Role::Project);
    // cm.AddCMakePaths();

    // qDebug() << "CreateAndSetGlobalGenerator: " << cm.CreateAndSetGlobalGenerator("Ninja");
    // cmMakefile makefile(dmake.GetGlobalGenerator(), dmake.GetCurrentSnapshot());
    // cmExecutionStatus executionStatus(makefile);

    // auto result = cmProjectCommand({"test"}, executionStatus);
    // if (!result) {
    //     qDebug() << executionStatus.GetError();
    // }

    // Set up code that uses the Qt event loop here.
    // Call a.quit() or a.exit() to quit the application.
    // A not very useful example would be including
    // #include <QTimer>
    // near the top of the file and calling
    // QTimer::singleShot(5000, &a, &QCoreApplication::quit);
    // which quits the application after 5 seconds.

    // If you do not need a running Qt event loop, remove the call
    // to a.exec() or use the Non-Qt Plain C++ Application template.

    // return a.exec();
}
