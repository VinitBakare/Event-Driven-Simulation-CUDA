#ifndef EVENT_H
#define EVENT_H

class Event
{ 
public:
	int netID;
	char value;
	int time;
	Event(int ID,char val,int time);
};



#endif