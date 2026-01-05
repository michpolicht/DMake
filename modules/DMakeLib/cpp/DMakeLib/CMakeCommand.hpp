#pragma once

#include "autogen/ApiMacro.hpp"

#include <QObject>
#include <QQmlEngine>

#include <cmExecutionStatus.h>

namespace DMakeLib {

class DMAKE_LIB_API CMakeCommand : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit CMakeCommand(QObject *parent = nullptr);

    virtual void execute(cmExecutionStatus &executionStatus);

signals:
};

} // namespace DMakeLib
