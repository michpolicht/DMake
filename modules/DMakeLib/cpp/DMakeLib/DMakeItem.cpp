#include "DMakeItem.hpp"

namespace DMakeLib {

DMakeItem::DMakeItem(QObject *parent)
    : QObject{parent}
{}

void DMakeItem::execute(cmExecutionStatus &status) {}

} // namespace DMakeLib
