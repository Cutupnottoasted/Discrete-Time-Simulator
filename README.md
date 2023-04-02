# Discrete-Time-Simulator
A discrete time simulator that simulates a scheduler, cpu, disk, cpu queue, and disk queue.  
Designed by Cody Hoang
Date: April 1st, 2023

How to run:
Using linux command line enter g++ main.cpp and then type ./a.out to run the program.
The program will prompt you to enter Lambda rate, CPU/Disk average service rate.

Overview:
This program simulates a system made up of:
A scheduler (event_queue)
Cpu Queue (ready_queue)
Disk Queue (disk_queue)
Component CPU (cpu)
Component Disk (disk)

Scheduler:
The scheduler schedules future events (processes) to be handled by the queues and components. The queue is a Priority Queue that orders the events by its arrival time. The earlier the arrival time, the higher the priority.

CPU/Disk Queue:
The CPU and Disk queue both function though FIFO. As each process arrives, the first process in the queue will be set to arrive at the departure time of the current process in the Component.
However, if the first process is also the first event then the process will go straight to the CPU to be processed.

CPU/Disk Component:
The CPU and Disk both function as a staging area for each process. When the process first arrives at the Component they are assigned to its current process. The component then generates the service time based on the Component’s service rate and processes it. Depending on which Component the process has departed from will determine its action.

When a process departs from the CPU there is a .6 chance that the process will exit the system but if the process does not exit the system then it will enter into the disk queue. If the disk queue is idle then the process will immediately begin service, otherwise it will be placed into the disk queue.

When a process departs from the Disk, it will go directly into the ready queue. If the Disk’s departure time is earlier than the most recent event arrival time, then the process will enter the ready queue before the event arrival. 


Structures and variables:
Globals:
Double Lambda: Represents the rate of arrival
Double Sys_clock: Represents the current time
Double Completed_processes: Represents the number of completed processes
Double Num_processes: Represents the number of processes created
Double turn_around_time: Used to aggregate turnaround time.
Function:
Double gen_inter_arrival: returns the arrival rate of each event created.

struct Process{}
Int id: Holds a randomly generated name.
Double arrival_time: Holds the creation time of the process
Double prev_departure: Holds the previous departure time from the last Component.
Double prob_done: If the process does not exit immediately then this holds the remaining percentage done.
Functions:
Bool operator<: Overloaded function to use Priority Queue library as the method to hold processes.

Class Component{}
Int type: Represents the CPU (0) and the Disk (1)
Double service_rate: The user given rate of the Component
Double time_active: The total time spent servicing processes
Double service_time: The current service time for current process
Double departure_time: The departure time for current process
Bool idle: Represents if the Component is busy or not
Process current_process: Holds the process currently being serviced



Functions: 
Void gen_service_time: Generates the service time for current process
Bool is_idle: returns idle
Double get_departure: returns departure_time
Double get_prev_departure: returns current_process.prev_departure
Double get_curr_prob: returns current_process.arrival_time
Double get_time_active: returns Component’s time active
Void start: This function is used once to start the system when the first event arrives
Void Departure: This functions processed departures for each Component. The invoking Component has an assigned type that is used to determine its actions.
Using the sys_clock 
Void arrivals: This function processes each arrival after the first. The invoking Component has an assigned type that is used to determine its actions. 


Main Function:
Void check_earliest_event: This function is used to loop and ensure that the current event arrival time is the latest event. The function loops and checks both the Disk and the CPU until the departure times for both events are after the event’s current arrival time.

Main variables:
Mt19937_64 rng: An engine designed to create real numbers using 64 bits

uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
seed_seq ss { uint32_t (timeSeed & 0xffffffff), uint32_t(timeSeed >> 32) };
rng.seed(ss);

This above is used to set a seed based on the current time.

Uniform_real_distribution<> unif(0, 1): Used to ensure an inclusive generation between 0 and 1

double avg_size_cpu: Used to calculate average ready queue size 
double avg_size_disk: Used to calculate average disk queue size
double avg_util_cpu: Used to calculate average CPU utility
double avg_util_disk: Used to calculate average Disk utility
Double service_rate: Used to hold user input for Components. 

