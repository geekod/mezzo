/*
	Mezzo Mesoscopic Traffic Simulation 
	Copyright (C) 2008  Wilco Burghout

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef VEHICLE_HH
#define VEHICLE_HH
#include "route.h"
#include "od.h"
//#include "PVM.h"
#include "signature.h"
#include <list>
#include "vtypes.h"
#include "busline.h"

class ODpair;
class Route;
class Link;
//class PVM;

class Vehicle
{
  public:
   Vehicle();
   //Vehicle(Vclass* vclass_):vclass(vclass_) {Vehicle();}
   //void init (const int id_, const int type_, const double length_, Route* const route_, ODpair*const  odpair_, const double time_); // OLD
   void init (const int id_, Vclass* const vclass_, Vtype* const type_, Route* const route_, ODpair*const  odpair_, const double time_); // NEW with vclass
   const double get_length() const {return length;}
   const double get_exit_time() const {return exit_time;}
   const double get_start_time() const {return start_time;}
   const ODVal get_odids () const  ;
   void set_exit_time(const double time){exit_time=time;}
   void set_entry_time(const double time){entry_time=time;}
   void set_alternative_route (Route* const  route_) {route=route_; switched=1;}
   void set_switched(const int i) {switched=i;}
   const double get_entry_time() const  {return entry_time;}
   void set_curr_link(Link* const curr_link_);
   Link* const  get_curr_link() const ;
   Route* const get_route() const {return route;}
   Link* const nextlink() const ;

   
   const int get_id() const {return id;}
   const int get_type_id() const {return type->get_id();}
   Vtype const * const get_type() const {return type;}

   Vclass const * const get_vclass() const {return vclass;}

   const  int get_oid() const ;
   const int get_did() const ;
 //  const int get_nextlink_id() const {return next_link_id;}
	void set_entered(); 
	void add_meters(const int meters_) {meters+=meters_;}
	void set_meters(const int meters_) {meters=meters_;}
	const int get_meters () const {return meters;}
	void report(const double time);

  protected:
	int id;
	//int next_link_id;
	Route* route;
	ODpair * odpair;
	double start_time;
	Vtype* type; 
	Vclass* vclass; // Vehicle class
	double length;
 	double entry_time;
	double exit_time; 	
	double arrival_time;
	Link* curr_link;
	
	vector <Link*> ::const_iterator next_link_iter;
	bool entered;
	int switched;
	int meters;
};

class Bus : public Vehicle
{
public:
	Bus():Vehicle() {occupancy=0;}
	//Bus(const int id_, const int type_, const double length_,Route* const route_, ODpair* const odpair_, const double time_) :
		//Vehicle(id_, type_,length_,route_,odpair_,time_) {} // Old, now use Vehicle() and init(...)
	const int get_occupancy() const {return occupancy;}
	void set_occupancy (const int occup) {occupancy=occup;}
	
protected:
	int occupancy;

};


class VehicleRecycler
	//!< Vehicle Factory that returns new vehicles and recycles old ones (via addvehicle(Vehicle*))
{
 public:
 	virtual ~VehicleRecycler(); //!< deletes all vehicles in the container
	Vehicle* const  newVehicle() ; //!< returns a new vehicle
	void addVehicle(Vehicle*const  veh){recycled.push_back( veh);}
	 
	 Bus* const newBus() {	 	if (recycled_busses.empty())
     								return new Bus();
     							else
     							{
     								Bus* bus=recycled_busses.front();
     								recycled_busses.pop_front();
     								return bus;
     							}	
     						}
							
     void addBus(Bus*const  bus){recycled_busses.push_back( bus);}

	 void register_vclass(Vclass* vclass_) { vehicleclasses.insert(pair<int,Vclass*>(vclass_->get_id(),vclass_));}

 private:
	list <Vehicle*> recycled;
	list <Bus*> recycled_busses;
	map <int,Vclass*> vehicleclasses;
};

//static VehicleRecycler recycler;
extern VehicleRecycler recycler;

		

#endif
