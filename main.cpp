#include <sstream>
#include <iostream>
#include <string>


// the number of hours to run the simulation.
const int nHour = 48;


// turning on / off the electrical appliance
#define on(bit) state = (switches)(state | (bit));
#define off(bit) state = (switches)(state & ~(bit));
// checking the power on the appliance
#define switchedOn(bit) (state & (bit))
// event handling
#define flag_event(bit)  if( stateCopy & (bit) )


// data global
enum switches
{
    ALL_TURN_OFF     =   0,  // all turn off.
    ALL_POWER_SUPPLY =   1,  // turning on the power of the whole house.
    MAIN_SOCKETS     =   2,  // main sockets in the house.
    LIGHT_INSIDE     =   4,  // light inside the house.
    LIGHT_OUTSIDE    =   8,  // outdoor light in the garden.
    HOME_HEATING     =  16,  // heating in the room.
    PUMP_HEATING     =  32,  // heating of the water supply coming from the well with a pump.
    AIR_CONDITIONER  =  64,  // air conditioning in the room.
    MOTION_SENSOR    = 128   // Motion Sensor
}state,stateCopy;  // switchboard status
double t1;         // temperature sensor outside the house.
double t2;         // temperature sensor inside the house.
double light1;     // lighting




/*!
 * @brief Execute a command line.
 * @param commandLine - command line.
 * @return Returns true if the string is valid, false otherwise.
 */
bool commandExecution(std::string &commandLine)
{
    std::stringstream buffer;
    std::string stateMoveStr, stateLightStr;

    // parsing
    buffer.clear();
    buffer.str(commandLine);
    buffer >> t1 >> t2 >> stateMoveStr >> stateLightStr;

    // analysis
    if( stateMoveStr == "yes" ) on(MOTION_SENSOR)
    else if( stateMoveStr == "no"  ) off(MOTION_SENSOR)
    else return false;

    if( stateLightStr == "on" ) on(LIGHT_INSIDE)
    else if( stateLightStr == "off"  ) off(LIGHT_INSIDE)
    else return false;

    // return on successful execution of commands
    return true;
}

/*!
 * @brief Subroutine to perform automation work for the current time (hour).
 * @param hour - current time.
 */
void automationWork(int hour)
{
    // plumbing heating
    if( t1 < 0.0 ) on(PUMP_HEATING)
    if( t1 > 5.0 ) off(PUMP_HEATING)

    // outdoor Lighting
    if( !((hour>=5) && (hour<=16)) && switchedOn(MOTION_SENSOR) )
        on(LIGHT_OUTSIDE)
    else
        off(LIGHT_OUTSIDE)

    // indoor heating
    if( t2 < 22.0 ) on(HOME_HEATING)
    if( t2 >= 25.0 ) off(HOME_HEATING)

    // air conditioner
    if( t2 >= 30.0 ) on(AIR_CONDITIONER)
    if( t2 <= 25.0 ) off(AIR_CONDITIONER)

    // indoor lighting
    if( switchedOn(LIGHT_INSIDE) )
    {
        const int hour1 = 16;
        const int hour2 = 20;
        const double light_high = 5000;
        const double light_low = 2700;

        if( hour > hour2 )
            light1 = light_low;
        else if( hour >= hour1 )
            light1 = light_high - (light_high-light_low)/(hour2-hour1) * (hour-hour1+1);
        else //if( hour >= 0 )
            light1 = light_high;
    }
}

/*!
 * @brief Output of the time (hour) and the events that the automation performed at that time.
 * @param hour - current time.
 */
void eventOutput(int hour)
{
    int i = 0;

    // detection of changes
    stateCopy = (switches)(stateCopy ^ state);

    // displaying event messages
    std::cout
        << "\nTime: " << hour << std::endl
        << "Events that have taken place:\n";
    flag_event(ALL_POWER_SUPPLY) {
        std::cout << (++i) << ") ";
        if (switchedOn(ALL_POWER_SUPPLY))
            std::cout << "Whole house meals included.\n";
        else
            std::cout << "The power to the whole house is turned off.\n";
    }
    flag_event(MAIN_SOCKETS) {
        std::cout << (++i) << ") ";
        if (switchedOn(MAIN_SOCKETS))
            std::cout << "The main sockets of the house are included.\n";
        else
            std::cout << "The main sockets of the house are switched off.\n";
    }
    flag_event(LIGHT_INSIDE) {
        std::cout << (++i) << ") ";
        if (switchedOn(LIGHT_INSIDE))
            std::cout << "The light inside the house is on. Colour temperature: " << light1 << std::endl;
        else
            std::cout << "The lights inside the house are off.\n";
    }
    flag_event(LIGHT_OUTSIDE) {
        std::cout << (++i) << ") ";
        if (switchedOn(LIGHT_OUTSIDE))
            std::cout << "Outdoor light in the garden is on.\n";
        else
            std::cout << "The outside light in the garden is off.\n";
    }
    flag_event(HOME_HEATING) {
        std::cout << (++i) << ") ";
        if (switchedOn(HOME_HEATING))
            std::cout << "Heating in the room is on.\n";
        else
            std::cout << "The heating in the room is turned off.\n";
    }
    flag_event(PUMP_HEATING) {
        std::cout << (++i) << ") ";
        if (switchedOn(PUMP_HEATING))
            std::cout << "Heating water supply coming from a well with a pump included.\n";
        else
            std::cout << "Heating water supply coming from a well with a pump turned off.\n";
    }
    flag_event(AIR_CONDITIONER) {
        std::cout << (++i) << ") ";
        if (switchedOn(AIR_CONDITIONER))
            std::cout << "Air conditioning in the room is on.\n";
        else
            std::cout << "The air conditioner in the room is off.\n";
    }
    flag_event(MOTION_SENSOR) {
        std::cout << (++i) << ") ";
        std::cout << "Motion Sensor: ";
        if (switchedOn(MOTION_SENSOR))
            std::cout << "there is movement.\n";
        else
            std::cout << "no movement.\n";
    }
    std::cout
        << (++i) << ") End of event enumeration."
        << "(note: if this message is in the first line, then no events have occurred in the system.)\n\n";
}



int main() {
    // initialization
    state = ALL_TURN_OFF;

    // simulation organization
    for(int hour=0; hour<nHour; hour++)
    {
        std::string commandLine;
        bool inputSuccess;

        // copying the system state before executing commands
        stateCopy = state;

        // command input
        do {
            std::cout << ">";
            std::getline(std::cin, commandLine);

            inputSuccess = commandExecution(commandLine);

            if ( !inputSuccess )
                std::cout << "Error input. Try again.\n";
        }while( !inputSuccess );

        // work
        automationWork(hour % 24);
        eventOutput(hour);
    }

    return 0;
}
