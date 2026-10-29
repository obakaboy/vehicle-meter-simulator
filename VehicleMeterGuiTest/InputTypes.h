#pragma once

#include <vector>

enum class Command
{
    TogglePower,
    ToggleAcc,
    Reset,
    Quit,
    IncreaseAccTarget,
    DecreaseAccTarget,
    ChangeDriveMode
};

struct InputFrame
{
    bool accelerate = false;
    bool brake = false;
    std::vector<Command> commands;
};