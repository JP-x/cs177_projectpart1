/*
 * Name: Jonathan Padilla
 * SID: 860999947
 * */
#include <iostream>
#include "cpp.h"
#include <string.h>
#include <cstdio>
using namespace std;
#define NUM_CELLS 120		// number of cells on road

#define TINY 1.e-20		// a very small time period AKA EPSILON
#define MOVING_LENGTH 3 //moving car is considered to occupy 3 cells
#define STOPPED_LENGTH 2//stopped car occupies 2 cells

#define GREEN 0
#define YELLOW 1
#define RED 2

#define SIM_LENGTH 10000

facility_set *road;
string driver_id = "A"; //character incremented after a call to new_driver
//process generators
void add_traffic();	
void new_driver(int starting_cell); //pass in cell to start at
void new_driver(); //find open space and place self into spot 
//helper functions for different calculations
void traffic_light();

int next_cell(int current_cell);
void init(); //sets cell values to -1 and init speed array
bool is_empty(int desired_cell);
//array containing all departure times
double D[NUM_CELLS];//default -1 means unoccupied cell?
int d_id_movement[5];//each slot matches driver process successful movements
double speed[6];

//other globals for tracking
int LIGHT_STATE = 0;


extern "C" void sim()		// main process
{
	create("sim");
	//arrivals();		// start a stream of arriving customers
	//std::printf("Enter number of vending machines: " );
    //std::scanf("%l",&n_machines); 
    cout << "Initializing arrays.\n" << endl;
    init();
    //HARD CODED VALUE SINCE INPUT HAS ISSUES FOR SOME REASON
    road = new facility_set("road", NUM_CELLS);//dynamic facility set
    cout << "Creating traffic. \n" << endl;
    add_traffic();		// start a stream of departing customers
	hold (SIM_LENGTH);		// wait for a whole day (in minutes) to pass
	//report();
}
//set all cells to be unoccupied
void init()
{
    for(int i = 0; i < NUM_CELLS ; i++)
    {
        D[i] = -1;
    }

    //times that a car occupies at speed[x]
    //given 'speeds' in specifications
    
    //may have problems with this speed (can't really use it for waiting)
    speed[0] = 0.0;
    
    speed[1] = 3.0;
    speed[2] = (11.0/6.0);
    speed[3] = 1.0;
    speed[4] = (2.0/3.0);
    speed[5] = (0.5);
    //might have misinterpreted instructions
    //
}

bool is_empty(int desired_cell)
{
    
    if((*road)[desired_cell].status() == FREE)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/* 
a moving car monitors the status of the roadway up 
to 4 car-lengths (or 8 cells) ahead of its 
current position, depending on its speed

probably going to add speed as an argument later on
*/
bool look_ahead(int current_cell, int current_speed)
{
    int look_ahead_length = 0;
    int cur_cell = current_cell;

    if(current_speed == 5){
        look_ahead_length = 8;
    }
    else if(current_speed == 4){
        look_ahead_length = 6;
    }
    else if(current_speed == 3 || current_speed == 2){
        look_ahead_length = 4;
    }
    else if(current_speed == 1){
        look_ahead_length = 2 ;
    }
    else 
    {
        //wut
        look_ahead_length = 2;
    }

    for(int i = 0; i < look_ahead_length ; i++)
    {
        if((*road)[cur_cell].status() == BUSY)
        {
            return false;
        }
        //increment cell, modulo to stay within range
        cur_cell = (cur_cell+1)%NUM_CELLS;
    }
    return true;

}

//return index of needed cell
int next_cell(int current_cell)
{
    return (current_cell+1)%NUM_CELLS;
}

long group_size();
//uniform(3,5)
//long cur_q_size = 0;
//cur_q_size = (*regularMachines)[i].qlength();

void add_traffic()		// this model segment spawns departing customers
{
	create("add_traffic");
    //set light
    traffic_light();
    new_driver(0);
    //new_driver(2);
    /*
	while(clock < 1440.)	//
	{
		hold(expntl(10)); // exponential interarrivals, mean 10 minutes
		//long group = group_size(); random group size (leftover from lab3)
		long group = 2;
        //add 2 drivers to the road
        for (long i=0;i<group;i++)
			new_driver();	// new driver appears on road
	}*/
}

//traffic light process
//waits for random times and switches light state
void traffic_light() 
{
    create("light");
    //start light at green
    LIGHT_STATE = GREEN;
    double light_change_time = clock + 120;
    double rand_time = 0.0;
    while(clock < SIM_LENGTH)//keep going in circles until time ends
    {
        if(light_change_time < clock)
        {
            if(LIGHT_STATE == GREEN)
            {
                rand_time = expntl(120);
                light_change_time = rand_time + clock;
                hold(rand_time);//hold for 2 minutes (120 sim time)
                //change light after waiting
                LIGHT_STATE = YELLOW;
            }
            else if(LIGHT_STATE == YELLOW)
            {
                rand_time = 10;
                light_change_time = rand_time + clock;
                hold(10);
                //change light after waiting
                LIGHT_STATE = RED;
            }
            else if(LIGHT_STATE == RED)
            {
                rand_time = uniform(30,90);
                light_change_time = rand_time + clock;
                hold(rand_time);
                //change light after waiting
                LIGHT_STATE = GREEN;
            }
        }
        else
        {
            hold(1);
        }
    }
}

//ASSUMPTION: GIVEN CELL IS AVAILABLE
void new_driver(int starting_cell)
{
    cout << "Placing driver at cell: " << starting_cell << endl;
    string driver_process_id = "";
    driver_process_id = "new_driver"+driver_id;
    driver_id[0] = driver_id[0] + 1;
    
    ///////////CREATE DRIVER PROCESS//////////////////
    create(driver_process_id.c_str());
    /////////////////////////////////
    //trace_on();
    //
    cout << "lolwut" << endl;
    double R = 0.0; //remaining time in cell
    double departure_time = 0.0;
    int needed_cell = 0;
    int current_cell;
    int number_movements = 0;
    //calculate departure time
    R = expntl(1);
    departure_time = clock + R;
    //set cell departure time in global array
    D[starting_cell] = departure_time;
    //car takes spot in cell
    (*road)[starting_cell].reserve();
    current_cell = starting_cell;
    while(clock < SIM_LENGTH)//keep going in circles until time ends
    {
        //cout << "current_clock: " << clock << endl;
        //if time to leave
        //release current cell and move to next_cell
        //generate new departure time (may change once speed is added
        if(departure_time < clock)
        {
            //release current_cell
            //determine cell to travel to next (needed cell j)
            needed_cell = next_cell(current_cell);
            if(is_empty(needed_cell))
            {
                //cout << driver_process_id << " moving to cell: " << needed_cell << endl;
                (*road)[current_cell].release(); //release cell in facility
                D[current_cell] = -1; //reset departure time in D
                //set new departure time
                R = expntl(1);
                departure_time = clock + R;
                D[needed_cell] = departure_time;
                //update cell
                current_cell = needed_cell;
                (*road)[current_cell].reserve();
                number_movements++;
                cout << process_name() << " moved " << number_movements << " times.\n";
                hold(R);
            }
            else //needed_cell is occupied  can't move
            {
                //determine new departure_time
                double new_time1 = 0.0;
                double new_time2 = 0.0; 
                new_time1 = D[needed_cell] + TINY;
                new_time2 = clock + R;
                //take max of 2 new times
                //cout << "Cell: " << needed_cell << " occupied.\n";
                if(new_time1 > new_time2)
                {
                    departure_time = new_time1;
                }
                else
                {
                    departure_time = new_time2;
                }
                //cout << "New departure time: " << departure_time << endl;
                D[current_cell] = departure_time;
                //hold for new time
                if(number_movements > 24)
                cout << process_name() << " moved successfully " << number_movements << " times.\n";
                
                number_movements = 0;
                hold(D[current_cell]-clock);
            }
        }
        else
        {
            hold(1);
        }
    }
    //cout << "DONE";
   //cout << process_name() << "Number of movements: " << number_movements << endl; 
}

long group_size()	// function gives the number of customers in a group
{
	double x = prob();
	if (x < 0.3) return 1;
	else
	{
		if (x < 0.7) return 2;
		else
			return 4;
	}
}
