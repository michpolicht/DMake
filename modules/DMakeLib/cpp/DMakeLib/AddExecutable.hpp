#pragma once

#include "CMakeCommand.hpp"

#include <QQmlEngine>

namespace DMakeLib {

class AddExecutable : public CMakeCommand
{
    Q_OBJECT
    QML_ELEMENT

public:
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)

    Q_PROPERTY(QStringList sources READ sources WRITE setSources NOTIFY sourcesChanged)

    explicit AddExecutable(QObject *parent = nullptr);

    QString name() const;

    void setName(const QString &name);

    QStringList sources() const;

    void setSources(const QStringList &sources);

    void execute(cmExecutionStatus &executionStatus) override;

signals:
    void nameChanged();

    void sourcesChanged();

private:
    QString m_name;
    QStringList m_sources;
};

} // namespace DMakeLib
