#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "board.h"
#include "timex.h"
#include "ztimer.h"

#include "sx127x.h"
#include "sx127x_netdev.h"
#include "sx127x_params.h"

#include "net/loramac.h"     /* core loramac definitions */
#include "semtech_loramac.h" /* package API */

#include "hts221.h"
#include "hts221_params.h"

#include "cayenne_lpp.h"

#include "parametres_devices.h"

/*-------------------------------------------------------------*/

#define EVER (;;)

/*-------------------------------------------------------------*/