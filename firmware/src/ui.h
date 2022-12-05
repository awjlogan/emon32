#ifndef UI_H
#define UI_H

/* A very simple state machine to track the button press. States:
 * Reset->SW_OPEN->SW_PRESSED->SW_CLOSED->SW_RELEASED->SW_OPEN
 */
typedef enum {
    SW_OPEN,    /* Not pressed */
    SW_CLOSED,  /* Pressed */
    SW_PRESSED, /* Last state was CLOSED, now confirmed closed */
    SW_RELEASED /* Last state was OPEN, now confirmed open */
} SwitchState_t;

/* Contains the states that are available to the energy monitor */
typedef enum {
    EMON_IDLE,      /* Ready to start */
    EMON_ACTIVE,    /* Collecting data */
    EMON_ERROR      /* An error has occured */
} EmonState_t;

/*! @brief Updates the current state of the button, called every millisecond
 */
SwitchState_t uiSWUpdate();

/*! @brief Returns the current state of the button
 */
SwitchState_t uiSWState();

/*! @brief Update the LED state, called every millisecond
 *  @param [in] : State of the energy monitor
 */
void uiUpdateLED(EmonState_t estate);

#endif
