#include "CMakeCommand.hpp"

namespace DMakeLib {

CMakeCommand::CMakeCommand(QObject *parent)
    : QObject{parent}
{}

void CMakeCommand::execute(cmExecutionStatus &executionStatus) {}

} // namespace DMakeLib
