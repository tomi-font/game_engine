#pragma once

#include <Event/Event.hh>

class EventManager;

class EventUser
{
public:

	EventUser() {}
	EventUser(EventManager*);
	virtual ~EventUser() {}

	void setEventManager(EventManager*);

	virtual void listenToEvents();

	// Callback for when an event whose type was subscribed to is triggered.
	virtual void triggered(const Event&);

protected:

	// Start listening to the given type of event.
	void listen(Event::Type);

	void broadcast(const Event&) const;

private:

	EventManager* m_eventManager = nullptr;
};
