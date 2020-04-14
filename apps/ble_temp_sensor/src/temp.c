#include <nrf_temp.h>
#include "ble_temp_sensor.h"
#include "temp.h"

#define TEMP_BUFFER_SIZE 10
#define TEMP_ERROR_VALUE 0x7FFF

typedef struct ring_buffer {
    uint8_t head;
    uint8_t size;
    int16_t* buf;
}RB;

static RB rbuffer;
static int16_t buffer[TEMP_BUFFER_SIZE];
//static os_sr_t sr;

void init_temp(void)
{
    /* Prepare the internal temperature module for measurement */
    nrf_temp_init();
    
    /* Set our ring buffer values */
    rbuffer.head = 0;
    rbuffer.size = 0;
    rbuffer.buf = buffer;
}

/* Returns the internal temperature of the nRF52 in degC (2 decimal places, scaled) */
int16_t
get_temp_measurement(void)
{
    int16_t temp;
    /* Start the temperature measurement. */
    NRF_TEMP->TASKS_START = 1;
    while(NRF_TEMP->EVENTS_DATARDY != TEMP_INTENSET_DATARDY_Set) {};
    /* Temp reading is in units of 0.25degC, so divide by 4 to get in units of degC
     * (scale by 100 to avoid representing as decimal). */
    temp = (nrf_temp_read() * 100) / 4.0;

    return temp;
}

/* periodic timer event triggered which means we need to push a new tempature value */
void
push_temp_rbuffer(void)
{
    /* TODO: investigate potenial thread saftey issues. 
    ** It seems that the BLE read chactersitic could interrupt this in the middle of pushing 
    ** a new value to the ring buffer. This could be problematic. 
    */
    //OS_ENTER_CRITICAL(sr);

    rbuffer.buf[rbuffer.head] = get_temp_measurement();
    
    /* Debug message of each tempature value being pushed to the circular queue */
    //LOG(DEBUG, "value pushed %d\n", rbuffer.buf[rbuffer.head]);
    rbuffer.head = (rbuffer.head + 1) % TEMP_BUFFER_SIZE;
    
    if(rbuffer.size != TEMP_BUFFER_SIZE)
    {
        rbuffer.size++;
    }

    //OS_EXIT_CRITICAL(sr);
}

/* returns the most recent tempature measurement */
int16_t
pop_temp_rbuffer(void)
{
    if(rbuffer.size == 0)
    {
        return TEMP_ERROR_VALUE;
    }

    /* Read out LIFO (freshest to oldest) */
    rbuffer.head = (rbuffer.head - 1 + TEMP_BUFFER_SIZE) % TEMP_BUFFER_SIZE;
    rbuffer.size--;
    return rbuffer.buf[rbuffer.head];
}

bool
empty_temp_rbuffer(void)
{
    return rbuffer.size == 0;
}

