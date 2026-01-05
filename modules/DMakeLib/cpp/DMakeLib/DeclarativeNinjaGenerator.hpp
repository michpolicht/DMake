#pragma once

#include "autogen/ApiMacro.hpp"

#include <cmGlobalNinjaGenerator.h>

#include <QQmlApplicationEngine>

namespace DMakeLib {

class DMAKE_LIB_API DeclarativeNinjaGenerator : public cmGlobalNinjaGenerator
{
public:
    static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory(
        QQmlApplicationEngine *qmlApplicationEngine);

    static std::string GetActualName();

    DeclarativeNinjaGenerator(cmake *cm, QQmlApplicationEngine *engine);

    void Configure() override;

    std::string GetName() const override;

private:
    QPointer<QQmlApplicationEngine> m_qmlApplicationEngine;
};

} // namespace DMakeLib
