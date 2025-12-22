#include <QCoreApplication>

#include <cmProjectCommand.h>
#include <cmMakefile.h>
#include <cmake.h>
#include <cmExecutionStatus.h>

// using Command = std::function<bool(std::vector<cmListFileArgument> const&,
//                                    cmExecutionStatus&)>;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    cmake cm(cmState::Role::Project);
    cm.AddCMakePaths();

    qDebug() << "CreateAndSetGlobalGenerator: " << cm.CreateAndSetGlobalGenerator("Ninja");
    cmMakefile makefile(cm.GetGlobalGenerator(), cm.GetCurrentSnapshot());
    cmExecutionStatus executionStatus(makefile);

    auto result = cmProjectCommand({"test"}, executionStatus);
    if (!result) {
        qDebug() << executionStatus.GetError();
    }

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
