#include <nrf_temp.h>
#include "ble_temp_sensor.h"
#include "temp.h"

#define TEMP_BUFFER_SIZE 10
#define TEMP_ERROR_VALUE 0x7FFF

typedef struct ringbuffer {
    uint8_t head;
    uint8_t size;
    int16_t* buf;
}RB;

static RB rbuffer;
static int16_t buffer[TEMP_BUFFER_SIZE];
static struct os_mutex ringbuffer_mutex;

void init_temp(void)
{
    /* Prepare the internal temperature module for measurement */
    nrf_temp_init();
    assert(os_mutex_init(&ringbuffer_mutex) == 0);

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
push_temp_rbuffer(int16_t data)
{
    os_mutex_pend(&ringbuffer_mutex, OS_TIMEOUT_NEVER);
   
    rbuffer.buf[rbuffer.head] = data;
    rbuffer.head = (rbuffer.head + 1) % TEMP_BUFFER_SIZE;
    
    if(rbuffer.size != TEMP_BUFFER_SIZE)
    {
        rbuffer.size++;
    }

    os_mutex_release(&ringbuffer_mutex);
}

/* returns the most recent tempature measurement */
int16_t
pop_temp_rbuffer(void)
{
    os_mutex_pend(&ringbuffer_mutex, OS_TIMEOUT_NEVER);

    if(rbuffer.size == 0)
    {
        os_mutex_release(&ringbuffer_mutex);
        return TEMP_ERROR_VALUE;
    }

    /* Read out LIFO (freshest to oldest) */
    rbuffer.head = (rbuffer.head - 1 + TEMP_BUFFER_SIZE) % TEMP_BUFFER_SIZE;
    int16_t data = rbuffer.buf[rbuffer.head];
    rbuffer.size--;

    os_mutex_release(&ringbuffer_mutex);
    return data;
}

bool
empty_temp_rbuffer(void)
{
    os_mutex_pend(&ringbuffer_mutex, OS_TIMEOUT_NEVER);

    bool emptyFlag = false;
    
    if(rbuffer.size == 0) {
        emptyFlag = true;
    }

    os_mutex_release(&ringbuffer_mutex);

    return emptyFlag;

}

