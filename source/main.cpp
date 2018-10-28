#include "game.h"

int main(int argc, char** argv)
{
    #ifndef NODEBUG
    consoleDebugInit(debugDevice_SVC);
    #endif

    auto program = std::make_unique<Game>(argc, argv);

    while(appletMainLoop() && program->running)
    {
        program->update();
    }

    return 0;
}
