/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#include <xc.h>        /* XC8 General Include File */
#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */
#include <string.h>

#include "system.h"
#include "user.h"
#include "battery.h"

/******************************************************************************/
/* User Global Variable Declaration                                           */
/******************************************************************************/


enum State cur_State;
PROGRAM cur_Program;
ACTION cur_Action;
Battery battery;

unsigned short long precondition_Time;
unsigned short long charge_Time;
tPIParams PI; // Data Structure for PI controller
unsigned short timeout;
long seconds;


/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/

void main(void)
{
    short res;
    short precondition;
    char end[10];
    char msg_Error[100];

    /* Configure the oscillator for the device */
    ConfigureOscillator();

    /* Initialize
     *  I/O and Peripherals for application */
    InitApp();

    while(1)
    {
        res = INDEFINED;
        switch (cur_State)
        {
            case POWERUP :
                res = initialize();
                if (res != OK)
                {
                    strcpy(msg_Error,"Charger Error");
                    cur_State = CHARGE_ERROR;
                }
                else
                {
                    cur_State = INITIALIZERD;
                }
                break;

            case INITIALIZERD :
                res = list_Battery();
                if (res != OK)
                {
                    strcpy(msg_Error,"Battery list Error");
                    cur_State = CHARGE_ERROR;
                }
                else
                {
                    cur_State = BATTERIES_LISTED;
                }
                break;

            case BATTERIES_LISTED :
                select_Battery();
                cur_State = BATTERY_SELECTED;
                break;

            case BATTERY_SELECTED :
                res = list_Action();
                if (res != OK)
                {
                    strcpy(msg_Error,"Action list Error");
                    cur_State = CHARGE_ERROR;
                }
                else
                {
                    cur_State = ACTION_LISTED;
                }
                break;

            case ACTION_LISTED :
                select_Action();
                cur_State = ACTION_SELECTED;
                break;

            case ACTION_SELECTED :
                switch (cur_Action)
                {
                    case RUN :
                        cur_State = RUN_SELECTED;
                    break;

                    case EDIT :
                        cur_State = EDIT_SELECTED;
                    break;

                    case DELETE :
                        cur_State = DELETE_SELECTED;
                    break;
                }
                break;

            case RUN_SELECTED :
                res = list_Program();
                if (res != OK)
                {
                    strcpy(msg_Error,"Program list Error");
                    cur_State = CHARGE_ERROR;
                }
                else
                {
                    cur_State = PROGRAM_LISTED;
                }
                break;

            case PROGRAM_LISTED :
                select_Program();
                cur_State = PROGRAM_SELECTED;
                break;

            case PROGRAM_SELECTED :
                switch (cur_Program)
                {
                    case CHARGE :
                            /* LIPO */
                            if (strncmp(battery.battery_type, algo_Name[0], 5))
                            {
                                res = check_Lipo();
                                if (res != OK)
                                {
                                    strcpy(msg_Error,"Liop check failure");
                                    cur_State = CHARGE_ERROR;
                                }
                                else
                                {
                                    cur_State = LIPO_ALGO_STARTED;
                                }
                            }
                            /* NIMH */
                            else if (strncmp(battery.battery_type, algo_Name[1], 5))
                            {

                            }
                            /* PB */
                            else if (strncmp(battery.battery_type, algo_Name[2], 5))
                            {

                            }
                        break;

                    case STORAGE :

                        break;
                    case DISCHARGE :

                        break;
                    case CYCLE :

                        break;
                    case BALANCE :

                        break;

                    default :

                        break;
                }
                break;

            case LIPO_ALGO_STARTED :
                res = check_Precondition(&precondition);
                if (res != OK)
                {
                    strcpy(msg_Error,"Precondition check failure");
                    cur_State = CHARGE_ERROR;
                }
                else
                {
                    cur_State = PRECONDITION_SELECTED;
                }
                break;

            case PRECONDITION_SELECTED :
                if (precondition)
                {
                    res = start_Precondition();
                    if (res != OK)
                    {
                        strcpy(msg_Error,"Precondition starting failure");
                        cur_State = CHARGE_ERROR;
                    }
                    else
                    {
                        cur_State = PRECONDITION_STARTED;
                    }
                }
                else
                {
                    res = start_CC();
                    if (res != OK)
                    {
                        strcpy(msg_Error,"CC starting failure");
                        cur_State = CHARGE_ERROR;
                    }
                    else
                    {
                        cur_State = CC_PHASE_STARTED;
                    }
                }
                break;

            case PRECONDITION_STARTED :
            case PRECONDITION_VERIFICATION :
                res = verifie_Precondition(end);
                if (res != OK)
                {
                    strcpy(msg_Error,"precondition verification failure");
                    cur_State = CHARGE_ERROR;
                }
                else
                {
                    if (strncmp(end,"END",3))
                    {
                        cur_State = PRECONDITION_ENDED;
                    }
                    else if (strncmp(end,"TIMER",5))
                    {
                        strcpy(msg_Error,"Precondition safety timer ended");
                        cur_State = CHARGE_ERROR;
                    }
                    else if (strncmp(end,"PROGRESS",8))
                    {
                        cur_State = PRECONDITION_VERIFICATION;
                    }
                }
                break;

            case PRECONDITION_ENDED :
                res = start_CC();
                if (res != OK)
                {
                    strcpy(msg_Error,"CC starting failure");
                    cur_State = CHARGE_ERROR;
                }
                else
                {
                    cur_State = CC_PHASE_STARTED;
                }
                break;

            case CC_PHASE_STARTED :
            case VBAT_VERIFICATION :
                res = verifie_Vbat(end);
                if (res != OK)
                {
                    strcpy(msg_Error,"CC verification failure");
                    cur_State = CHARGE_ERROR;
                }
                else
                {
                    if (strncmp(end,"END",3))
                    {
                        cur_State = CC_PHASE_ENDED;
                    }
                    else if (strncmp(end,"TIMER",5))
                    {
                        strcpy(msg_Error,"CC safety timer ended");
                        cur_State = CHARGE_ERROR;
                    }
                    else if (strncmp(end,"PROGRESS",8))
                    {
                        cur_State = VBAT_VERIFICATION;
                    }
                }
                break;

            case CC_PHASE_ENDED :
                res = start_CV();
                if (res != OK)
                {
                    strcpy(msg_Error,"CV starting failure");
                    cur_State = CHARGE_ERROR;
                }
                else
                {
                    cur_State = CV_PHASE_STARTED;
                }
                break;

            case CV_PHASE_STARTED :
            case IBAT_VERIFICATION :
                res = verifie_Ibat(end);
                if (res != OK)
                {
                    strcpy(msg_Error,"CV verification failure");
                    cur_State = CHARGE_ERROR;
                }
                else
                {
                    if (strncmp(end,"END",3))
                    {
                        cur_State = CV_PHASE_ENDED;
                    }
                    else if (strncmp(end,"TIMER",5))
                    {
                        strcpy(msg_Error,"CV safety timer ended");
                        cur_State = CHARGE_ERROR;
                    }
                    else if (strncmp(end,"PROGRESS",8))
                    {
                        cur_State = IBAT_VERIFICATION;
                    }
                }
                break;

            case CV_PHASE_ENDED :
                res = stop_CV();
                if (res != OK)
                {
                    strcpy(msg_Error,"CV stoping failure");
                    cur_State = CHARGE_ERROR;
                }
                else
                {
                    cur_State = CHARGE_ENDED;
                }
                break;
                
            case CHARGE_ENDED :
                end_Charge();
                break;

            case CHARGE_ERROR :

                break;
            default :

                break;
        }
    }

}

