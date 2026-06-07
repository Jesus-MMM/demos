#include "io.h"
#include "types.h"
#include "util_lib.h"

int kernel_main()
{

    const char *text = "Welcome to DemOS";

    write_to_screen(text, strlen(text));
    
    return 0;
    
}