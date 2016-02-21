
#include <ruby.h>

#include "server_ext.h"

void Init_indicate_bind ()
{
    Init_server_ext();
    Init_indicator_ext();
}
