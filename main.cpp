// header
#include <iostream>
#include <queue>
#include <ctime>
#include <cmath>
#include <random>
#include <chrono>

using namespace std;


// classes and structures
int lambda = 0;
double sys_clock = 0.0;
double completed_processes = 0.0;
double num_processes = 0.0;
double turn_around_time = 0.0;

struct Process
{
    int id; // auto generate an id
    double arrival_time; // time created
    double prev_departure; // departing time from previous component
	double prob_done; // if <= .4 then assign data else process is completed
	
    // constructors
    Process()
    {
        id = -1;
        arrival_time = 0.0;
        prev_departure = 0.0;
		prob_done = 0.0;
    }
	
    Process(int i, double t)
    {
        id = i;
        arrival_time = t;
        prev_departure = 0.0;
		prob_done = 0.0;
    }
	
    // overload < operator for priority queue
    bool operator<(const Process& rhs) const
    {
        return rhs.arrival_time < arrival_time; // invoking object compared to referenced
    }
};

double gen_inter_arrival(double y) { return sys_clock + ( (-1.0 / lambda) * log(y) ); }

class Component
{
private:
    // private member variables and constants
    int type; // 0 = cpu, 1 = disk
    double service_rate; // CPU = .02 DISK = .06
    double time_active; // aggregate time servicing processes
    double service_time; // generated time at Xn arrival
    double departure_time;
    // if activated = false then no arrivals yet
	bool idle;
    Process current_process; // process departing

public:
	// constructors
	Component()
    {
        type = -1;
        service_rate = 0.0;
        time_active = 0.0;
        service_time = 0.0;
        departure_time = 0.0;
        idle = true;
    }
	
	Component(int component_type, double t_avg)
    {
        type = component_type;
        service_rate = t_avg;
        time_active = 0.0;
        service_time = 0.0;
        departure_time = 0.0;
        idle = true;
    }

    // functions
    void gen_service_time (double y) { service_time = (-1.0 * service_rate) * log(y); }
    bool is_idle() { return idle; }
    double get_departure() { return departure_time; }
    double get_prev_departure() { return current_process.prev_departure; }
    double get_curr_prob() { return current_process.prob_done; }
    double get_curr_arrival() { return current_process.arrival_time; }
    double get_time_active() { return time_active; }

    void start(Process entry, double y) // used only in the cpu
    {
        idle = false;
        gen_service_time(y);
        current_process = entry;
        departure_time = entry.arrival_time + service_time;
        time_active = time_active + service_time;
    }

    void departure(queue<Process>& ready_queue, queue<Process>& disk_queue, Component& destination, uniform_real_distribution<double>& unif, mt19937_64& rng)
    {
        if (type == 0)// if cpu then destination = disk
        {
            double temp = unif(rng);
            if (temp + get_curr_prob() <= .4) // schedule disk queue arrival
            {
                current_process.prob_done = temp;
                current_process.prev_departure = departure_time;
                destination.arrivals(current_process, ready_queue, disk_queue, unif, rng);
            }
            else if (temp + get_curr_prob() > .4)
            {
                current_process.prob_done = temp;
                turn_around_time = departure_time - current_process.arrival_time; 
                completed_processes++;
            }
            if (sys_clock >= destination.get_departure() && !destination.is_idle()) // if process in disk departs before next arrival
            {
                destination.departure(ready_queue, disk_queue, *this, unif, rng);
            }
            // either ask if ready_queue is occupied
            if (!ready_queue.empty())
            {  
                current_process = ready_queue.front();
                ready_queue.pop();
                gen_service_time(unif(rng));
                time_active = time_active + service_time;
                if (get_curr_arrival() >= departure_time)
                {
                    departure_time = get_curr_arrival() + service_time; 
                }
                else 
                {
                    departure_time = departure_time + service_time; // schedule new departure time
                }
            } 
        }
        else if (type == 1) // if disk then destination = cpu
        {
            current_process.prev_departure = departure_time;
            ready_queue.push(current_process); // push disk's current process into ready queue
            if (!disk_queue.empty())
            {
                current_process = disk_queue.front();
                disk_queue.pop();
                gen_service_time(unif(rng));
                departure_time = departure_time + service_time; // new departure time
                time_active = time_active + service_time;
            }
            else if (disk_queue.empty())
            {
                idle = true;
            }
        }
    }

void arrivals(Process entry, queue<Process>& ready_queue, queue<Process>& disk_queue,
                             uniform_real_distribution<double>& unif, mt19937_64& rng)
{
    if (type == 0) // cpu
    {
        ready_queue.push(entry);
    }  
    else // disk
    {
        if (idle == false) // disk is occupied
        {
            disk_queue.push(entry);
        }
        else // disk is idle
        {
            idle = false;
            gen_service_time(unif(rng));
            current_process = entry;
            time_active = time_active + service_time;

            // arriving process is later than disk's recent departure
            if(entry.prev_departure > departure_time)
                departure_time = entry.prev_departure + service_time; // service starts process arrival time
            else // arriving process arrives at disk's departure
                departure_time = departure_time + service_time;
        }
    }
}
};

void check_earliest_event(priority_queue<Process>& event_queue, queue<Process>& ready_queue, queue<Process>& disk_queue, 
                            Component& cpu, Component& disk, uniform_real_distribution<double>& unif, mt19937_64& rng)
{
    do
    {

        if ( disk.is_idle() ) // if the disk is idle
        {
            if (sys_clock >= cpu.get_departure()) // if cpu departure comes first
                cpu.departure(ready_queue, disk_queue, disk, unif, rng);
        }
        else if ( !disk.is_idle() )
        {
            // if cpu is earlier than both disk departure and current clock
            if (sys_clock >= cpu.get_departure() && cpu.get_departure() < disk.get_departure())
                cpu.departure(ready_queue, disk_queue, disk, unif, rng);
            // if disk is earlier than both cpu departure and current clock
            else if (sys_clock >= disk.get_departure() && disk.get_departure() < cpu.get_departure())
                disk.departure(ready_queue, disk_queue, cpu, unif, rng);
        }
    } while (sys_clock >= cpu.get_departure() || sys_clock >= disk.get_departure() && !cpu.is_idle() && !disk.is_idle());
    
} 


int main()
{
    mt19937_64 rng;
    uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    seed_seq ss { uint32_t (timeSeed & 0xffffffff), uint32_t(timeSeed >> 32) };
    rng.seed(ss);
    uniform_real_distribution<double> unif(0.0, 1.0);

    priority_queue<Process> event_queue;
    queue<Process> ready_queue;
    queue<Process> disk_queue;
    double avg_size_cpu = 0.0;
    double avg_size_disk = 0.0;
    double avg_util_cpu = 0.0;
    double avg_util_disk = 0.0;
    double service_rate;
    cout << "Enter Lambda Rate: ";
    cin >> lambda;
    cout << "\nEnter Service Rate for CPU: ";
    cin >> service_rate;
    Component cpu(0, service_rate);
    cout << "\nEnter Service Rate for DISK: ";
    cin >> service_rate;
    Component disk(1, service_rate);
    
    Process p(rand(), gen_inter_arrival(unif(rng)));
    num_processes++;
    sys_clock = p.arrival_time;
    event_queue.push(p);
    cpu.start(p, unif(rng));
    event_queue.pop();

    do
    {
        avg_size_cpu = avg_size_cpu + ready_queue.size();
        avg_size_disk = avg_size_disk + disk_queue.size();
        Process p(rand(), gen_inter_arrival(unif(rng)));
        num_processes++;
        sys_clock = p.arrival_time;
        event_queue.push(p);
        cpu.arrivals(event_queue.top(), ready_queue, disk_queue, unif, rng);
        event_queue.pop();

        check_earliest_event(event_queue, ready_queue, disk_queue, cpu, disk, unif, rng);  

    } while (completed_processes < 10000);
    sys_clock = cpu.get_departure(); 
    avg_size_cpu = avg_size_cpu/completed_processes;
    avg_size_disk = avg_size_disk/completed_processes;
    avg_util_cpu = cpu.get_time_active()/sys_clock;
    avg_util_disk = disk.get_time_active()/sys_clock;
    turn_around_time = turn_around_time/sys_clock;
    cout << "\nAverage turn around time: " << turn_around_time << endl;
    cout << "Throughput: " << completed_processes / sys_clock << endl;
    cout << "Average CPU util: " << avg_util_cpu << endl;
    cout << "Average DISK util: " << avg_util_disk << endl;
    cout << "Average ready queue size: " << avg_size_cpu << endl;
    cout << "Average disk queue size: " << avg_size_disk << endl;
    return 0; 
}