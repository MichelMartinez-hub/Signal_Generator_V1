#include "bsp/system.h"
#include "bsp/gpio.h"
#include "app/app.h"
#include "protocol/cli.h"

int main(void)
{
    System_Init();
    GPIO_Init();

    App_Init();
    CLI_Init();

    while (1)
    {
        App_Run();
        CLI_Run();
    }
}