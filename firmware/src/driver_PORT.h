#ifndef DRIVER_PORT_H
#define DRIVER_PORT_H

#include <stdint.h>

/* Types */
typedef enum {
    PIN_DIR_IN,
    PIN_DIR_OUT
} PINDIR_t;

typedef enum {
    PIN_CFG_SET,
    PIN_CFG_CLR
} PINCFG_t;

typedef enum {
    PIN_DRV_CLR,
    PIN_DRV_SET,
    PIN_DRV_TGL
} PINDRV_t;

/*! @brief   Configure the ports.
 *           Ports for peripherals are configured in their setup functions
 */
void portSetup();

/*! @brief Sets a pin as input or output
 *  @param [in] pin : PIN number. For SAMD10, there is only a single group
 *  @param [in] mode: PIN_IN for input, PIN_OUT for output
 */
void portPinDir(unsigned int pin, PINDIR_t mode);

/*! @brief Sets the mux for pin alternate function
 *  @param [in] pin : Pin number
 *  @param [in] mux : Mux mode
 */
void portPinMux(unsigned int pin, unsigned int mux);

/*! @brief Sets the pin configuration
 *  @param [in] pin : Pin number
 *  @param [in] cfg : configuration option
 *  @param [in] cs  : clear or set configuration
 */
void portPinCfg(unsigned int pin, unsigned int cfg, PINCFG_t cs);

/*! @brief Sets the pin driver value
 *  @param [in] pin : Pin number
 *  @param [in] drv : Clear, set, or toggle pin
 */
void portPinDrv(unsigned int pin, PINDRV_t drv);

/*! @brief Returns the pin value
 *  @param [in] pin : Pin number
 */
unsigned int portPinValue(unsigned int pin);


#endif
