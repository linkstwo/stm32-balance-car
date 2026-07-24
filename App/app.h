#ifndef APP_APP_H
#define APP_APP_H

void App_Init(void);
void App_ProcessHighestPriorityEvents(void);
void App_ProcessRemoteInput(void);
void App_ProcessStateMachine(void);
void App_ProcessDisplay(void);
void App_Idle(void);

#endif
