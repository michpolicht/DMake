#include "Project.hpp"

#include <cmProjectCommand.h>

namespace DMakeLib {

Project::Project(QObject *parent)
    : CMakeCommand{parent}
    , m_commands(this, &m_commandList)
{}

QString Project::name() const
{
    return m_name;
}

void Project::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
    }
}

QQmlListProperty<CMakeCommand> Project::commands() const
{
    return m_commands;
}

void Project::execute(cmExecutionStatus &executionStatus)
{
    cmProjectCommand({m_name.toStdString()}, executionStatus);
    for (auto &&command : m_commandList) {
        command->execute(executionStatus);
    }
}

} // namespace DMakeLib
