#pragma once

#include "autogen/ApiMacro.hpp"

#include <cmake.h>

#include <QObject>
#include <QQmlApplicationEngine>

namespace DMakeLib {

class DMAKE_LIB_API DMake : public QObject,
                            public cmake
{
    Q_OBJECT
    QML_UNCREATABLE("DMake object can not be created from QML")

public:
    DMake(QQmlApplicationEngine *qmlApplicationEngine,
          cmState::Role role,
          cmState::TryCompile isTryCompile = cmState::TryCompile::No,
          QObject *parent = nullptr);

    int run(std::vector<std::string> const& args, bool noconfigure = false);

private:
    QPointer<QQmlApplicationEngine> m_qmlApplicationEngine;

    // Member variables we can't access from outside of cmake class.
    std::string CheckStampList;
    std::string CheckStampFile;
#ifndef CMAKE_BOOTSTRAP
    bool SarifFileOutput = false;
#endif
    bool FreshCache = false;
};

}
