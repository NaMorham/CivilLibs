/*
 * Filename: wxWatchEvents.cpp
 * Author: Karl Janke
 * Date: November 2004
 *
 * This file provides the implementation of a wxEvent that can be
 * used in conjunction with the WatchManager callback function.
 * Example:
 *
	UINT TheWatcherFunction( UINT id, const std::string & path )
	{
		wxWatchEvent event(wxEVT_WATCH_CHANGED);
		event.SetFullname( path.c_str() );
		event.SetId( id );

		::wxPostEvent( pPostToHere, event );
		return 0;
	}
 *
 */

#ifdef _HAVE_WXWIDGETS

/* undef this if you don't have wxWidgets and therefore 
 * don't want to build this file */
#define __WXMSW__	

#ifdef __WXMSW__

#include "../include/wxWatchEvent.h"

#include <LeakWatcher.h>

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC_CLASS(wxWatchEvent, wxEvent)

// create some new events
const wxEventType wxEVT_WATCH_CHANGED		= wxNewEventType();

#endif	// __WXMSW__

#endif // #ifdef _HAVE_WXWIDGETS
