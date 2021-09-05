//
// Error: handling / reporting of system errors
//

#include "stm32g4xx_hal.h"
#include "error.h"


// Private variables
static uint32_t err_reg = 0;
static uint32_t err_time[ERR_MAX] = {0};
static uint32_t err_last_time = 0;


// Assert an error: sets err register bit and records timestamp
void error_assert(error_t err)
{
    // Return on invalid error
	if(err >= ERR_MAX)
		return;

    // Record time at which error was reported
	err_time[err] = HAL_GetTick();
    
    // Record time of latest error that occurred
    err_last_time = HAL_GetTick(); 
    
    // Set error bit in register
	err_reg |= (1 << err);
}


// Get the systick at which an error last occurred, or 0 otherwise
uint32_t error_timestamp(error_t err)
{
    // Return on invalid error
	if(err >= ERR_MAX)
		return 0;

	return err_time[err];
}


// Return timestamp of last asserted error or 0 if none asserted
uint32_t error_last_timestamp(void)
{
    return err_last_time;
}


// Returns 1 if the error has occurred since boot
uint8_t error_occurred(error_t err)
{
    // Return on invalid error    
	if(err >= ERR_MAX)
		return 0;

	return (err_reg & (1 << err)) > 0;
}


// Return value of error register
uint32_t error_reg(void)
{
	return err_reg;
}
