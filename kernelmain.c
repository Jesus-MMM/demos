#include "io.h"
#include "splash.h"

int kernel_main()
{
    style_cursor(DISABLE);
    animate_splash();
    return 0;
}