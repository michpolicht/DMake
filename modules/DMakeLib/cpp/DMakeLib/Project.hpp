#pragma once

#include <QQmlEngine>
#include "CMakeCommand.hpp"

namespace DMakeLib {

class Project : public CMakeCommand
{
    Q_OBJECT
    QML_ELEMENT

public:
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)

    Q_PROPERTY(QQmlListProperty<CMakeCommand> commands READ commands CONSTANT)

    Q_CLASSINFO("DefaultProperty", "commands")

    explicit Project(QObject *parent = nullptr);

    QString name() const;

    void setName(const QString &name);

    QQmlListProperty<CMakeCommand> commands() const;

    void execute(cmExecutionStatus &executionStatus) override;

signals:
    void nameChanged();

private:
    QString m_name;
    QList<CMakeCommand *> m_commandList;
    QQmlListProperty<CMakeCommand> m_commands;
};

} // namespace DMakeLib
