#include "game.h"

int main(int argc, char** argv)
{
    consoleDebugInit(debugDevice_SVC);

    auto program = std::make_unique<Game>(argc, argv);

    while(appletMainLoop() && program->running)
    {
        program->update();
    }

    return 0;
}
