#ifndef _PASSENGER
#define _PASSENGER

#include "parameters.h"
#include "busline.h"
#include "od_stops.h"
#include "Random.h"

class Bustrip;
class Busline;
class Busstop;
class ODstops;
class ODzone;
class Day;

typedef pair<Busstop*,Busline*> StopLine; //new data type Stop+Line for the Memory.Stop recording
typedef pair<ODstops*,Busline*> LegLine; //preliminary for the next one
typedef pair<Busstop*,Day*> StopDay;  //new data type Stop+Day for the Memory.Stop recording

class Passenger : public Action
{
public:
	Passenger ();
	~Passenger ();
	void init (int pass_id, double start_time_, ODstops* OD_stop_);
	void init_zone (int pass_id, double start_time_, ODzone* origin_, ODzone* destination_);
	void reset();
	
	// Gets and sets:
	int get_id () {return passenger_id;}
	double get_start_time () {return start_time;}
	void set_end_time (double end_time_) {end_time = end_time_;}
	double get_end_time () {return end_time;}
	ODstops* get_OD_stop () {return OD_stop;}
	ODzone* get_o_zone () {return o_zone;}
	ODzone* get_d_zone () {return d_zone;}
	void set_origin_walking_distances (map<Busstop*,double> origin_walking_distances_) {origin_walking_distances = origin_walking_distances_;}
	void set_destination_walking_distances (map<Busstop*,double> destination_walking_distances_) {destination_walking_distances = destination_walking_distances_;}
	double get_origin_walking_distance (Busstop* stop) {return origin_walking_distances[stop];}
	double get_destination_walking_distance (Busstop* stop) {return destination_walking_distances[stop];}
	Busstop* get_original_origin () {return original_origin;}
	int get_nr_boardings () {return nr_boardings;}
	vector <Busstop*> get_chosen_path_stops () {return selected_path_stops;}
	void set_ODstop (ODstops* ODstop_) {OD_stop = ODstop_;}
	void add_to_selected_path_stop (Busstop* stop) {selected_path_stops.push_back(stop);}
	bool get_this_is_the_last_stop () {return this_is_the_last_stop;}
	bool get_has_network_rti () {return has_network_rti;}
	void add_to_travel_time (double time) {in_vehicle_time += time;}
	void add_to_waiting_time (double time) {waiting_time += time;}
	double get_walking_time () {return walking_time;}
	void add_to_walking_time (double walking_time_) {walking_time += walking_time_;}
	double get_waiting_time () {return waiting_time;}
	double get_in_vehicle_time () {return in_vehicle_time;}
	void set_expected_waiting_time (Busstop* stop, double expected_waiting_time_) {expected_waiting_times[stop] = expected_waiting_time_;}
	void set_at_stop_indicator (bool at_stop_) { at_stop = at_stop_;}
	void set_last_board_time (double last_board_time_) {last_board_time = last_board_time_;}
	void set_last_arrival_time_at_stop (double last_arrival_time_at_stop_) {last_arrival_time_at_stop = last_arrival_time_at_stop_;}
	double get_last_board_time () {return last_board_time;}
	double get_last_arrival_time_at_stop () {return last_arrival_time_at_stop;}
	void add_event_to_pass (Eventlist *eventlist, double time) {eventlist->add_event(time,this);}
	bool execute(Eventlist* eventlist, double time); // called every time passengers choose to walk to another stop (origin/transfer), puts the passenger at the waiting list at the right timing
	double get_min_waiting_time_by_RTI (Busstop* stop, double time);
	double get_min_waiting_time_by_headway (Busstop* stop, double time);

	// Passenger decision processes
	bool make_boarding_decision (Bustrip* arriving_bus, double time); // boarding decision making 
	Busstop* make_alighting_decision (Bustrip* boarding_bus, double time); // alighting decision making 
	Busstop* make_connection_decision (double time); // connection link decision (walking between stops)

	// Demand in terms of zones
	/*
	map<Busstop*,double> sample_walking_distances (ODzone* zone);
	Busstop* make_first_stop_decision (double time); // deciding at which stop to initiate the trip
	double calc_boarding_probability_zone (Busline* arriving_bus, Busstop* o_stop, double time);
	Busstop* make_alighting_decision_zone (Bustrip* boarding_bus, double time); 
	Busstop* make_connection_decision_zone (double time);
	bool stop_is_in_d_zone (Busstop* stop);
	*/

	// output-related 
	void write_selected_path(ostream& out);

	//Day-to-Day---------------------------------------------------------------
	void set_experienced_waiting_time (Busstop* stop, Busline* line, Day* day, double waiting_time); //Passenger memory-stops
	void set_experienced_invehicle_time (ODstops* ODstop, Busline* line, Day* day, double leg_IVT); //Passenger memory-legs
	void set_experienced_crowding_at_stop (Busstop* stop, Day* day, int nr_waiting); //Passenger memory-stops (passengers waiting)
	void set_experienced_crowding_onboard (ODstops* ODstop, Busline* line, Day* day, double crowding_onboard); //Passenger memory-legs (experienced crowdedness weigthed on the leg's IVT)
	void evening_cleaning();
	//-------------------------------------------------------------------------
	map <StopLine, map<Day*,double>> get_experienced_waiting_time(){return experienced_waiting_times;}//waiting time only for the specific line the passenger will take
	map <LegLine, map<Day*,double>> get_experienced_invehicle_time(){return experienced_invehicle_times;}//IVT for each passenger on a leg
	map <Busstop*, map<Day*,int>> get_experienced_crowding_at_stop (){return experienced_crowding_at_stop;}  //experienced crowdedness at the stop, no matter which line
	map <LegLine, map<Day*,double>> get_experienced_crowding_onboard (){return experienced_crowding_onboard;}  //experienced crowdedness weigthed on the leg's IVT
	//-------------------------------------------------------------------------
	bool any_waiting_time(Busstop* stop, Busline* line);
	double strategy_waiting_time(Busstop* stop, Busline* line);
	bool any_invehicle_time(ODstops* ODstop, Busline* line);
	double strategy_invehicle_time(ODstops* ODstop, Busline* line);
	bool any_crowding_at_stop(Busstop* stop);
	int strategy_crowding_at_stop(Busstop* stop);
	bool any_crowding_onboard(ODstops* ODstop, Busline* line);
	double strategy_crowding_onboard(ODstops* ODstop, Busline* line);

	// fuzzy travellers

protected:
	int passenger_id;
	double start_time;
	double end_time;
	double walking_time;
	double waiting_time;
	double in_vehicle_time;
	double last_board_time;
	double last_arrival_time_at_stop;
	Busstop* original_origin;
	ODstops* OD_stop;
	bool boarding_decision;
	bool at_stop; // true- indicates that passenger is waiting at a stop; flase- passenger is on-board
	bool changed_stop;
	Random* random;
	double elapsed_overpassed_time;
	Busstop* elapsed_waiting;
	map<Busstop*, double> expected_waiting_times;
	int nr_boardings; // counts the number of times pass boarded a vehicle
	vector <Busstop*> selected_path_stops;
	bool has_network_rti;

	// Day-to-Day
	map <StopLine, map<Day*,double>> experienced_waiting_times;   //Passenger memory - Stops recording //for each combination stop-line we get the list of all the perceived waiting times in each day
	map <LegLine, map<Day*,double>> experienced_invehicle_times;  //Passenger memory - Legs recording
	map <Busstop*, map<Day*,int>> experienced_crowding_at_stop;   //Passenger memory - Stops recording
	map <LegLine, map<Day*,double>> experienced_crowding_onboard; //Passenger memory - Legs recording
	int crowding_at_stop;   //Passenger memory - Stops recording
	int crowding_onboard;   //Passenger memory - Legs recording
	double segments_load;   //numerator term to calculate leg load weigthed on the leg IVT
	double leg_IVT;         //in-vehicle time for a single leg
	int start_stop;
	int end_stop;

	// fuzzy travellers
	vector <ODstops*> fuzzy_destination_set; // contains all destination choice-set

	// relevant only for OD in terms od zones
	ODzone* o_zone;
	ODzone* d_zone;
	map<Busstop*,double> origin_walking_distances;
	map<Busstop*,double> destination_walking_distances;
	bool this_is_the_last_stop;
};

class PassengerRecycler
{
 public:
 	~PassengerRecycler();
	Passenger* newPassenger() {	 	if (pass_recycled.empty())
     								return new Passenger();
     								else
     								{
     									Passenger* pass=pass_recycled.front();
     									pass_recycled.pop_front();
     									return pass;
     								}	
     							}
							
     void addPassenger(Passenger* pass){pass_recycled.push_back(pass);}
 
private:
	list <Passenger*> pass_recycled;
};
//static PassengerRecycler recycler;
extern PassengerRecycler pass_recycler;

#endif //_Passenger