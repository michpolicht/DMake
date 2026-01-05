#include "AddExecutable.hpp"

#include <cmAddExecutableCommand.h>

namespace DMakeLib {

AddExecutable::AddExecutable(QObject *parent)
    : CMakeCommand{parent}
{}

QString AddExecutable::name() const
{
    return m_name;
}

void AddExecutable::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
    }
}

QStringList AddExecutable::sources() const
{
    return m_sources;
}

void AddExecutable::setSources(const QStringList &sources)
{
    if (m_sources != sources) {
        m_sources = sources;
        emit sourcesChanged();
    }
}

void AddExecutable::execute(cmExecutionStatus &executionStatus)
{
    std::vector<std::string> args;
    args.push_back(m_name.toStdString());
    for (auto &&source : m_sources) {
        args.push_back(source.toStdString());
    }
    cmAddExecutableCommand(args, executionStatus);
}

} // namespace DMakeLib
