/*
 * Name: Jonathan Padilla
 * SID: 860999947
 * */
#include <iostream>
#include "cpp.h"
#include <string.h>
#include <cstdio>
#include <time.h> //not sure if conflicts with csim
using namespace std;
#define NUM_CELLS 120		// number of cells on road

#define TINY 1.e-20		// a very small time period AKA EPSILON
#define MOVING_LENGTH 3 //moving car is considered to occupy 3 cells
#define STOPPED_LENGTH 2//stopped car occupies 2 cells
#define CAR_LEN 2.0 //default size of car
#define CROSSWALK1 118
#define CROSSWALK2 119
#define GREEN 0
#define YELLOW 1
#define RED 2
#define STOPPED 0
#define MOVING 1
#define SIM_LENGTH 2000

facility_set *road;
string driver_id = "A"; //character incremented after a call to new_driver
//process generators
void add_traffic();	
void new_driver(int starting_cell); //pass in cell to start at
void new_driver(); //find open space and place self into spot 
void target_speed_generator(); //randomly generate new target speeds for every car every 1-2 minutes
//helper functions for different calculations
void traffic_light();

int next_cell(int current_cell);
void init(); //sets cell values to -1 and init speed array
bool is_empty(int desired_cell);
void accelerate(int &speed, int car_id);
void brake(int &speed, int ahead_speed);
//return car speed
int infront_speed(int c_id);
int rand_speed();
//array containing all departure times
double D[NUM_CELLS];//default -1 means unoccupied cell?

//MAY REMOVE
int d_id_movement[5];//each slot matches driver process successful movements
int d_id_taillocations[100];//used for checking when moving to another cell

int d_id_speeds[100];//max 100 cars. ID 0 refers to car A, ID 1 refers to car B
int d_id_targetspeeds[100];//used for randomly generated speeds
double speed[6];

//other globals for tracking
int LIGHT_STATE = GREEN;
int NUM_CARS = 1;
int car_ids = 0;//used for d_id_speeds

extern "C" void sim()		// main process
{
	create("sim");
    srand(time(NULL));
    cout << "Initializing arrays.\n" << endl;
    init();
    //HARD CODED VALUE SINCE INPUT HAS ISSUES FOR SOME REASON
    road = new facility_set("road", NUM_CELLS);//dynamic facility set
    cout << "Creating traffic. \n" << endl;

    //set number of cars
    cout << "Enter number of cars on the road: ";
    cin >> NUM_CARS;


    add_traffic();		// start a stream of departing customers
    target_speed_generator();
    //traffic_light();
	hold (SIM_LENGTH);		// wait for a whole day (in minutes) to pass
	//report();
}
//set all cells to be unoccupied
void init()
{
    //initialize departure times
    for(int i = 0; i < NUM_CELLS ; i++)
    {
        D[i] = -1;
    }

    //int d_id_speeds[100]
    //initialize driver speeds
    for(int i = 0 ; i < 100; i++)
    {
        d_id_speeds[i] = -1;
    }
    //times that a car occupies at speed[x]
    //given 'speeds' in specifications
    
    //may have problems with this speed (can't really use it for waiting)
    speed[0] = 0.0;
    
    speed[1] = 3.0/CAR_LEN;
    speed[2] = (11.0/6.0)/CAR_LEN;
    speed[3] = 1.0/CAR_LEN;
    speed[4] = (2.0/3.0)/CAR_LEN;
    speed[5] = (0.5)/CAR_LEN;
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
    int cur_cell = (current_cell+1)%NUM_CELLS; //prevent out of range access

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
        //not sure if look ahead is needed
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
    int nxt_cell =  (current_cell+1)%NUM_CELLS;
    //118 and 119 are crosswalk 
    //might need to change value if light is red
    //handled in look_ahead
    return nxt_cell;
}

void add_traffic()		// this model segment spawns departing customers
{
	create("add_traffic");
    //set light
    //UNCOMMENT FOR PART 2

    //new_driver(1);//initial car starts at 1 so that the tail of the car is at zero
    
    //new_driver(2);
    //car length is 2 since all cars are stopped
    
    
    int start_pos = NUM_CARS*2;
    int car_count = NUM_CARS;
    for(int i = NUM_CARS ; i > 0 ; i--)
    {
        new_driver(start_pos);
        car_count--;
        start_pos = car_count*2;
    }
    
}

//Repeat experiment 1 when cells 118 amd 119 represent 
//a pedestrian cross-walk controlled by a traffic light (described below)

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
                cout << "GREEN LIGHT!" << endl;
                hold(rand_time);//hold for 2 minutes (120 sim time)
                //change light after waiting
                LIGHT_STATE = YELLOW;
            }
            else if(LIGHT_STATE == YELLOW)
            {
                rand_time = 10;
                light_change_time = rand_time + clock;
                cout << "YELLOW LIGHT!" << endl;
                hold(10);
                
                //THIS MAY BE A PROBLEM WHERE
                //CARS ARE CONSTANTLY MOVING
                //SO THE LIGHT MAY NEVER CHANGE
                //change light after waiting
                if(is_empty(CROSSWALK1) && is_empty(CROSSWALK2))
                {
                    LIGHT_STATE = RED;
                    //reserve spaces for crosswalk
                    (*road)[CROSSWALK1].reserve();
                    (*road)[CROSSWALK2].reserve();
                }
            }
            else if(LIGHT_STATE == RED)
            {
                rand_time = uniform(30,90);
                light_change_time = rand_time + clock;
                cout << "RED LIGHT!" << endl;
                hold(rand_time);
                //change light after waiting
                LIGHT_STATE = GREEN;
                //light is green release crosswalks
                (*road)[CROSSWALK1].release();
                (*road)[CROSSWALK2].release();
            }
        }
        else
        {
            hold(1);
        }
    }
}

//ASSUMPTION: GIVEN CELL IS AVAILABLE
//cars travel at constant speed for one car length
//accelerate = step up speed
//brake lower speed by 2 (if possible)


void new_driver(int starting_cell)
{
    cout << "Placing driver at cell: " << starting_cell << endl;
    string driver_process_id = "";
    driver_process_id = "new_driver"+driver_id;
    driver_id[0] = driver_id[0] + 1;
    //rand()%4 + 2 
    ///////////CREATE DRIVER PROCESS//////////////////
    create(driver_process_id.c_str());
    /////////////////////////////////
    //trace_on();
    //
   
    double departure_time = 0.0;
    int needed_cell = 0;
    int car_state = STOPPED;
    int current_cell;
    int cur_speed = 0;//used for upping speed speed[cur_speed]
    
    //used for keeping track of car speeds
    int car_id = car_ids;
    //increment counter so next car gets new id
    car_ids++;
    //reset after reaching 120 movements
    int number_movements = 0;
    //increment after 120 movements
    int laps = 0;
    //nose1 is set when moving
    int nose1_cell = (starting_cell+1)%NUM_CELLS;//prevent out of range
    int nose_cell = starting_cell;
    int tail_cell = starting_cell-1;
    if(tail_cell == -1)
    {
        tail_cell = 119;
    }
    //calculate departure time
    departure_time = clock + speed[cur_speed];
    //set cell departure time in global array
    D[nose_cell] = departure_time;
    D[tail_cell] = departure_time;
    //car takes reserves 2 cells
    (*road)[nose_cell].reserve();
    (*road)[tail_cell].reserve();
    current_cell = starting_cell;
    d_id_speeds[car_id] = 0;
    
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
            //+1 to compensate for extra cell while moving
            //otherwise look ahead would catch the car that looking ahead
            //to determine that it is being blocked by itself
            if(look_ahead(current_cell+1,cur_speed) && cur_speed <= d_id_targetspeeds[car_id])//clear to accelerate
            {
                //car can move set new state to moving
                car_state = MOVING;
                //increase speed (if not at target)
                if(cur_speed == 0)
                {
                    cout << "car_id: " << car_id <<  "no longer stopped" << endl;
                }
                accelerate(cur_speed, car_id);
                d_id_speeds[car_id] = cur_speed;
                //cout << "increasing speed" << endl;
                //cout <<  "target_speed: " <<  d_id_targetspeeds[car_id] << endl;
                //cout << "current_speed: " <<  cur_speed << endl;
                //cout << driver_process_id << " moving to cell: " << needed_cell << endl;
                //get a new set of cells to move to
                tail_cell = current_cell-1;
                nose_cell = current_cell;
                nose1_cell = (current_cell+1)%NUM_CELLS;//prevent out of range 

                if(tail_cell == -1)// occurs when nose is at 0
                {
                    tail_cell = 119;
                }
                //determine which cells to grab next
                int next_tailcell = next_cell(tail_cell);
                int next_nosecell = next_cell(nose_cell);
                int next_nose1cell = next_cell(nose1_cell);

                //release cells
                (*road)[tail_cell].release(); //release cell in facility
                (*road)[nose_cell].release();
                if(!is_empty(nose1_cell))//case where just starting to move do not want to release an empty cell
                {
                    (*road)[nose1_cell].release();//moving so occupying 3 cells
                }
                D[tail_cell] = -1;
                D[nose_cell] = -1;
                D[nose1_cell] = -1;

                //set new cells
                tail_cell = next_tailcell;
                nose_cell = next_nosecell;
                nose1_cell = next_nose1cell;

                //set new departure time
                departure_time = clock + speed[cur_speed];
                D[tail_cell] = departure_time;
                D[nose_cell] = departure_time;
                D[nose1_cell] = departure_time;

                //update cell
                current_cell = needed_cell;
                (*road)[tail_cell].reserve();
                (*road)[nose_cell].reserve();
                (*road)[nose1_cell].reserve();
                number_movements++;
                //check if completion of lap
                if(number_movements >= 120)
                {
                    laps++;
                    number_movements = 0;
                    cout << "car_id: " << car_id << " completed a lap." << endl;
                }
                else
                {
                    number_movements++;
                }
                //cout << process_name() << " moved " << number_movements << " times.\n";
                //driver 1 second reaction time
                hold(1);
            }
            else //look_ahead determines there is a car within range
            {//change speed because of obstruction
                //driver 1 second reaction time
                hold(1);
                //determine new speed
                int infr_speed = 0;
                if( (LIGHT_STATE == YELLOW || LIGHT_STATE == RED) && nose_cell >= 110 && nose_cell != CROSSWALK1 && nose_cell != CROSSWALK2)//if in range BRAKE FOR LIGHT
                {
                    cout << "car_id: " << car_id << " slowing down for light." << endl;
                    infr_speed = 0;
                }
                else
                {
                    infr_speed = infront_speed(car_id);
                }
                brake(cur_speed, infr_speed);
                //cout << "car_id: " << car_id << "braking." << endl;
                

                //cout << "decreasing speed" << endl;
                //cout <<  "target_speed: " <<  d_id_targetspeeds[car_id] << endl;
                //cout << "current_speed: " <<  cur_speed << endl;


                if(cur_speed == 0)
                {
                    car_state = STOPPED;
                    cout << "car_id: " << car_id <<" STOPPED FOR LIGHT." << endl;
                    //(*road)[nose1_cell].release(); //stopped release nose1cell MIGHT NOT NEED
                }
                else
                {
                    car_state = MOVING;
                }
                //cout << driver_process_id << " moving to cell: " << needed_cell << endl;
                //get a new set of cells to move to
                tail_cell = current_cell-1;
                nose_cell = current_cell;
                nose1_cell = (current_cell+1)%NUM_CELLS;//prevent out of range 

                if(tail_cell == -1)// occurs when nose is at 0
                {
                    tail_cell = 119;
                }
                //determine which cells to grab next
                int next_tailcell = next_cell(tail_cell);
                int next_nosecell = next_cell(nose_cell);
                int next_nose1cell = next_cell(nose1_cell);

                //release cells
                (*road)[tail_cell].release(); //release cell in facility
                (*road)[nose_cell].release();
                //possible problem...
                //may release a cell that does not belong to current car
                if(!is_empty(nose1_cell) && car_state == MOVING)//case where just starting to move do not want to release an empty cell
                {
                    (*road)[nose1_cell].release();//moving so occupying 3 cells
                    D[nose1_cell] = -1;
                }
                D[tail_cell] = -1;
                D[nose_cell] = -1;

                //set new cells
                tail_cell = next_tailcell;
                nose_cell = next_nosecell;
                nose1_cell = next_nose1cell;

                //set new departure time
                departure_time = clock + speed[cur_speed];
                D[tail_cell] = departure_time;
                D[nose_cell] = departure_time;

                //update cell
                current_cell = needed_cell;
                (*road)[tail_cell].reserve();
                (*road)[nose_cell].reserve();
                if(car_state != STOPPED)
                {
                    D[nose1_cell] = departure_time;
                    (*road)[nose1_cell].reserve();
                     number_movements++;
                }
                //check if completion of lap
                if(number_movements >= 120)
                {
                    laps++;
                    number_movements = 0;
                }
                else
                {
                    number_movements++;
                }
                d_id_speeds[car_id] = cur_speed;
                //cout << process_name() << " moved " << number_movements << " times.\n";
                //driver 1 second reaction time
                hold(1);
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

int infront_speed(int c_id)
{
    //case where only 1 car on road
    if(NUM_CARS == 1)
    {
        return 0;
    }


    if(c_id == 0)//lfirst car is looking at the last cars speed (case of overlapping car)
    {
        return d_id_speeds[NUM_CARS-1];
    }
    else if(c_id == NUM_CARS-1) //last car is looking at first cars speed
    {
        return d_id_speeds[0];
    }
    else //other cars just look in front of them 
    {
        //kind of backwards in that -1 refers to the car in front
        //cout << "c_id: " << c_id << " getting speed of " << c_id-1 << endl;
        return d_id_speeds[c_id-1];
    }
}

void brake(int &speed, int ahead_speed)
{
    int lower_speed = speed -2 ;
    if(lower_speed < ahead_speed)
    {
        speed = ahead_speed;
    }
    else
    {
        speed = lower_speed;
    }
}

int rand_speed()
{
    return rand()%4 + 2 ; // range [2,5]
}

void target_speed_generator()
{
    create("target_speed_generator");
    
    //setup random speeds for every car
    for(int i = 0; i < 100 ; i++)
    {
        d_id_targetspeeds[i] = rand_speed();
    }

    double wait_time = uniform(120,240);
    double generation_time = clock + wait_time;
    while(clock < SIM_LENGTH)//keep going in circles until time ends
    {
        if(generation_time < clock)
        {
            //generate a new set of random speeds
            for(int i = 0; i < 100 ; i++)
            {
                d_id_targetspeeds[i] = rand_speed();
            }
            //set new generation time
            wait_time = uniform(120,240);
            generation_time = clock + wait_time;
            hold(wait_time);
        }
        else
        {
            //just wait for another second
            hold(1);
        }
    }
}

void accelerate(int &speed, int car_id)
{
    if(speed < d_id_targetspeeds[car_id] )
    {
        speed++;
    }
    //don't change speed unless needed

}
