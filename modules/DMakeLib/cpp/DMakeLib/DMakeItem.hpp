#pragma once

#include "autogen/ApiMacro.hpp"

#include <QObject>
#include <QQmlEngine>

#include <cmExecutionStatus.h>

namespace DMakeLib {

class DMAKE_LIB_API DMakeItem : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit DMakeItem(QObject *parent = nullptr);

    virtual void execute(cmExecutionStatus &status);

signals:
};

} // namespace DMakeLib
