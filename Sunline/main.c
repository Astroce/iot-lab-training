#include "main.h"

static sx127x_t sx127x;   

static semtech_loramac_t loramac; 

static hts221_t hts221;

static cayenne_lpp_t lpp;

const uint8_t appeui[LORAMAC_APPEUI_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const uint8_t deveui[5][LORAMAC_DEVEUI_LEN] =
{
    {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x04, 0xD2, 0x38},
    {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x04, 0xDC, 0x6C},
    {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x04, 0xDC, 0xBD},
    {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x04, 0xDC, 0xBE},
    {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x04, 0xDC, 0xBF}
};

const uint8_t appkey[5][LORAMAC_APPKEY_LEN] =
{
    {0x68, 0x7A, 0x6F, 0xF8, 0xE7, 0x2F, 0xE0, 0x43, 0x4F, 0x5A, 0xBF, 0x61, 0xF1, 0x2F, 0x79, 0xCF},
    {0x96, 0xEE, 0x0D, 0x5A, 0x98, 0xBA, 0x34, 0x95, 0x56, 0xAB, 0x90, 0xFB, 0xAB, 0x46, 0x5A, 0x0A},
    {0x2E, 0x83, 0xE8, 0x2F, 0xB1, 0x3A, 0xBD, 0x7C, 0x2D, 0x94, 0x4E, 0x5B, 0x0C, 0x7F, 0xBC, 0x55},
    {0x09, 0xAD, 0x65, 0x88, 0x60, 0xED, 0x83, 0x3C, 0x09, 0xFB, 0x27, 0x65, 0x49, 0xAD, 0x23, 0x98},
    {0x70, 0x3F, 0x33, 0x62, 0x47, 0x42, 0x40, 0x72, 0xFF, 0xB0, 0x5E, 0x86, 0x29, 0xA0, 0x97, 0x7C}
};

int main (void)
{ 
    uint8_t count;
    
    if (hts221_init(&hts221, &hts221_params[0]) != HTS221_OK) 
    {
        puts("Sensor initialization failed");
        return 1;
    }

    if (hts221_power_on(&hts221) != HTS221_OK) 
    {
        puts("Sensor initialization power on failed");
        return 1;
    }

    if (hts221_set_rate(&hts221, hts221.p.rate) != HTS221_OK) 
    {
        puts("Sensor continuous mode setup failed");
        return 1;
    }
    /* initialize the radio driver */

    sx127x_setup(&sx127x, &sx127x_params[0], 0);
    loramac.netdev = &sx127x.netdev;
    loramac.netdev->driver = &sx127x_driver;
    
    /* initialize loramac stack */
    
    semtech_loramac_init(&loramac);

    /* configure the device parameters */

    semtech_loramac_set_appeui(&loramac, appeui);
    semtech_loramac_set_deveui(&loramac, &deveui[0][LORAMAC_DEVEUI_LEN]);
    semtech_loramac_set_appkey(&loramac, &appkey[0][LORAMAC_APPKEY_LEN]);

    /* change datarate to DR5 (SF7/BW125kHz) */

    semtech_loramac_set_dr(&loramac, 5);
    
    /* start the OTAA join procedure */
    
    if (semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA) != SEMTECH_LORAMAC_JOIN_SUCCEEDED)
    {    
        
        for(count = 1; count < 6; count ++)
        {
            printf("Counter : %d\n\r", count);

            semtech_loramac_set_appeui(&loramac, appeui);
            semtech_loramac_set_deveui(&loramac, &deveui[count][LORAMAC_DEVEUI_LEN]);
            semtech_loramac_set_appkey(&loramac, &appkey[count][LORAMAC_APPKEY_LEN]);

            semtech_loramac_set_dr(&loramac, 5);
            
            ztimer_sleep(ZTIMER_MSEC, 2 * MS_PER_SEC);
            
            if(semtech_loramac_join(&loramac, LORAMAC_JOIN_OTAA) == SEMTECH_LORAMAC_JOIN_SUCCEEDED)
            {
                 puts("procedure join with new APPKEY");
                
                break;
            }
            
            else
            {
                 puts("echec join with new APPKEY");
            }
        }   
    }   
    
    for EVER
    {
            /* do some measurements */
        uint16_t humidity = 0;
        int16_t temperature = 0;
        
        if (hts221_read_humidity(&hts221, &humidity) != HTS221_OK) 
        {
            puts("Cannot read humidity!");
        }
        
        if (hts221_read_temperature(&hts221, &temperature) != HTS221_OK) 
        {
            puts("Cannot read temperature!");
        }

        printf("H: %d.%d%%, T:%d.%dC\n",
                (humidity / 10), (humidity % 10),
                (temperature / 10), (temperature % 10));
        
        cayenne_lpp_add_temperature(&lpp, 0, (float)temperature / 10);
        cayenne_lpp_add_relative_humidity(&lpp, 1, (float)humidity / 10);

        printf("Sending LPP data\n");
        
        uint8_t ret = semtech_loramac_send(&loramac, lpp.buffer, lpp.cursor);
        
        if (ret != SEMTECH_LORAMAC_TX_DONE) 
        {
            printf("Cannot send lpp message, ret code: %d\n", ret);
        }

        cayenne_lpp_reset(&lpp);

        ztimer_sleep(ZTIMER_MSEC, 5 * MS_PER_SEC);
    }   
    return 0; 
}    