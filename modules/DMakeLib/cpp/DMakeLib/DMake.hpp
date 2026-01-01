#pragma once

#include <QObject>
#include <QQmlEngine>

#include <cmake.h>

#include "autogen/ApiMacro.hpp"

namespace DMakeLib {

class DMAKE_LIB_API DMake : public QObject,
                            public cmake
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit DMake(cmState::Role role,
                   cmState::TryCompile isTryCompile = cmState::TryCompile::No, QObject *parent = nullptr);

    int run(std::vector<std::string> const& args, bool noconfigure = false);

private:
    // Member variables we can't access from outside of cmake class.
    std::string CheckStampList;
    std::string CheckStampFile;
#ifndef CMAKE_BOOTSTRAP
    bool SarifFileOutput = false;
#endif
    bool FreshCache = false;
};

}
