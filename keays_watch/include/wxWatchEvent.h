#ifdef __WXMSW__
/*
 * Filename: wxWatchEvent.h
 * Author: Karl Janke
 * Date: November 2004
 *
 * This file provides the definition of a wxEvent that can be
 * used in conjunction with the WatchManager callback function.
 * See wxWatchEvents.cpp for example usage.
 */

#ifndef _FILE_WATCHER_EVENTS
#define _FILE_WATCHER_EVENTS

#ifdef _HAVE_WXWIDGETS
#include <wx/wx.h>

// Not always defined
#ifndef UINT
#define UINT unsigned int
#endif

/*
 * wxWatchEvent class. Extents from wxEvent and carries information
 * about the file or directory that changed.
 */ 
class wxWatchEvent : public wxEvent
{
public:
	DECLARE_DYNAMIC_CLASS(wxWatchEvent)
	
	wxWatchEvent() : wxEvent( wxID_ANY, wxEVT_NULL ) 
	{ 
		m_Fullname = wxEmptyString; 
		m_WatchId = 0; 
	}
	
	wxWatchEvent( WXTYPE type ) : wxEvent ( wxID_ANY, type ) 
	{ 
		m_Fullname = wxEmptyString;
		m_WatchId = 0; 
	}

	wxWatchEvent( const wxWatchEvent & rhs ) : wxEvent(rhs)
	{
		m_WatchId = rhs.m_WatchId;
		m_Fullname = rhs.m_Fullname;
	}

	~wxWatchEvent( ) {}

    virtual wxEvent *Clone() const { return new wxWatchEvent(*this); }
	
	void	 SetFullname( const wxString & name ) { m_Fullname = name; }
	wxString GetFullname( ) { return m_Fullname; }

	void	SetId( UINT id ) { m_WatchId = id; }
	UINT	GetId() { return m_WatchId; }

private:
	UINT		m_WatchId;		// id of file/directory that changed
	wxString	m_Fullname;		// full path to file/directory that changed
};

extern const wxEventType wxEVT_WATCH_CHANGED;

typedef void (wxEvtHandler::*wxWatchEventFunction)(wxWatchEvent&);

/* Event Table Macro */
#define EVT_WATCH_CHANGED(fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_WATCH_CHANGED, -1, -1, \
        (wxObjectEventFunction)(wxEventFunction)(wxWatchEventFunction)&fn, \
        (wxObject *) NULL \
    ),

#endif // #ifdef _HAVE_WXWIDGETS

#endif	//#ifndef _FILE_WATCHER_EVENTS
#endif	//#ifdef __WXMSW__