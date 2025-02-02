/***********************************************************************
USBContext - Class representing USB library contexts.
Copyright (c) 2010 Oliver Kreylos

This file is part of the Kinect 3D Video Capture Project (Kinect).

The Kinect 3D Video Capture Project is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Kinect 3D Video Capture Project is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Kinect 3D Video Capture Project; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef USBCONTEXT_INCLUDED
#define USBCONTEXT_INCLUDED

#include <stddef.h>
#include <Threads/Thread.h>

/* Forward declarations: */
struct libusb_context;

class USBContext
	{
	/* Elements: */
	private:
	libusb_context* context; // USB context handle from the USB library
	Threads::Thread eventHandlingThread; // Thread to process and dispatch USB events in the background
	
	/* Private methods: */
	void* eventHandlingThreadMethod(void); // Background event handling method
	
	/* Constructors and destructors: */
	public:
	USBContext(void); // Creates a new USB context
	~USBContext(void); // Destroys the USB context
	
	/* Methods: */
	void setDebugLevel(int newDebugLevel); // Sets the USB library's debugging level
	libusb_context* getContext(void) const // Returns a pointer to the USB context handle
		{
		return context;
		}
	void startEventHandling(void); // Starts background event handling
	void stopEventHandling(void); // Stops background event handling
	void processEvents(void); // Processes any asynchronous transfer events that occurred in the context
	};

#endif
