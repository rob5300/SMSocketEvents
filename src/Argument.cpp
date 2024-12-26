#include "Argument.h"

void Argument::ClearValue()
{
    value.reset();
    type = EventArgumentType_NONE;
    length = 0;
}