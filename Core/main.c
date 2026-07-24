#include "app.h"

int main(void)
{
    App_Init();

    for (;;)
    {
        App_ProcessHighestPriorityEvents();
        App_ProcessRemoteInput();
        App_ProcessStateMachine();
        App_ProcessDisplay();
        App_Idle();
    }
}
