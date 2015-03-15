/*
 * Filename: StringData.h
 * Date: Feburary 2004
 * Author: Karl Janke, Keays Software
 *
 * The file contains the internal representation of string data
 * in memory. The version of this data is that defined as
 * Versions 2.2 and higher. It uses structures to store each record as it
 * was determined they were significantly faster than using classes.
 *
 * To use this class in your program include both the Data.h and
 * Data.cpp in your project.
 *
 * It belongs to the keays::stringfile namespace
 */
#ifndef _STRING_DATA_H
#define _STRING_DATA_H

#pragma warning (disable: 4786)

#include <vector>        // std::vector
#include <string>        // std::string
#include <set>            // std::set

#include <assert.h>        // assert

#include <keays_math.h>    // keays::math library

/**
 * keays namespace
 */
namespace keays
{
/**
 * stringfile namespace
 */
namespace stringfile
{
    const int cRECORD_LENGTH            =    56;    /**< Complete length of a Point structure */
    const int cNUMBER_LENGTH            =    32; /**< Length of number part of a Point structure */
    const int cTEXT_LENGTH                =    20; /**< Length of text part of a Point structure */
    const int cTITLE_LENGTH                =    40; /**< Length of title as stored in UR files */

    const int cNOTE_LENGTH                =    12; /**< Length of notes field of Point record */

    // the plot code length and offset
    const int cPLOT_CODE_OFFSET            =    32; /**< Offset of Plot Code in Point record */
    const int cPLOT_CODE_LENGTH            =    4;  /**< Length of Plot Code in Point record */

    // These are offsets into the plotting modes
    const int cCONTOUR_POINT_OFFSET        =    1;
    const int cCONTOUR_STRING_OFFSET    =    2;
    const int cFEATURE_POINT_OFFSET        =    3;
    const int cFEATURE_STRING_OFFSET    =    4;

    // Where to look for the contour/feature plotting stuff
    const int cPLOT_MODE_OFFSET            =    48;
    const int cPLOT_MODE_LENGTH            =    4;

    /**
     * \brief Possible flags to pass to find operations.
     *
     * You must pass:
     *        - One of eFF_CONTOUR, eFF_FEATURE or eFF_ANY
     *        - One of eFF_SORT_MAX or eFF_SORT_MIN
     *        - One of eFF_IGNORE_X, eFF_IGNORE_Y or eFF_IGNORE_Z
     *
     * This is always true except where noted in the find methods documentation.
     */
    enum
    {
        // must pass one of these to indicate the type of record
        eFF_CONTOUR    = 0x00000001,    /**< Only search for points that will be plotted in contour mode only */
        eFF_FEATURE    = 0x00000002,    /**< Only search for points that will be plotted in feature mode only */
        eFF_BOTH    = 0x00000004,    /**< Only search for points that will be plotted in both modes */
        eFF_ANY        = 0x00000200,    /**< Only search for points that will be plotted in at least one mode */

        // must pass one of these to indicate the sorting to apply
        eFF_SORT_MAX= 0x00000008,    /**< Return the maximum point in the ignored axis */
        eFF_SORT_MIN= 0x00000010,    /**< Return the minimum point in the ignored axis */

        // must pass one of these to indicate the axis to ignore
        eFF_IGNORE_X= 0x00000020,    /**< Ignore the specifed X values searching with respect only to Y and Z */
        eFF_IGNORE_Y= 0x00000040,    /**< Ignore the specifed Y values searching with respect only to X and Z */
        eFF_IGNORE_Z= 0x00000080,    /**< Ignore the specifed Z values searching with respect only to X and Y */

        // pass this if you only want single points returned
        eFF_POINT    = 0x00000100,    /**< Only search on single points, i.e. not part of a string */
    };

    // constants for plot modes
    const char cPlotPoint        = 'Y';    /**< Yes. Plot this point */
    const char cNoPlotPoint        = 'N';    /**< No. Don't plot this point */
    const char cSelectString    = 'S';    /**< Selected. Plot the string this point is part of. */
    const char cNoSelectString    = 'U';    /**< Unselected. Plot the string this point is part of. */

    const char cBoundaryString    = 'B';    /**< Boundary. The string this point is part of forms a boundary string. Only applies to contour plotting. */
    const char cInternalPolygon    = 'I';    /**< Internal. The string this point is part of forms a internal string. Only applies to contour plotting. */

    /* while not part of the file format as such it is passed around to signal
     * that the current value should be inverted, i.e Y -> N, S -> U & visa-versa
     */
    const char cPlotInvert        = 'X';

/**
    \brief tStringRec structure represents the basic unit: the string record.

    It is typedef'd and refered to as Point.
 */
struct tStringRec
{
    /**
     * \brief The string number.
     *
     * If multiple StringStructs have the  same number then
     * they are on the same line. If's 0 than this is not part of a string, but
     * a point only.
     */
    unsigned int stringNo;

    /**
     * \brief The point number.
     *
     * These should increase sequentially along a line. However this is not a required
     * condition and the string will be drawn in the order the points occur in the file.
     */
    unsigned int pointNo;

    /*
     * The actual point data. Really should be called longtitude, latitude
     * & altitude. When drawing in a computer world, y and z need to be
     * inverted before being drawn.
     */

    /**
     * \brief The position this point lies on in an East/West direction.
     *
     * Also known as Easting or Longtitude.
     */
    double x;

    /**
     * \brief The position this point lies on in an North/South direction
     *
     * Also known as Northing or Latitude.
     */
    double y;    // aka latitude

    /**
     * \brief The position this point lies on above the Earth's surface.
     *
     * Also known as Altitude or RL.
     */
    double z;

    /**
     * \brief The Plot Code for this point.
     *
     * Four characters descriptor that helps to identify strings
     */
    char plotCode[cPLOT_CODE_LENGTH];

    /**
     * \brief Extra Notes
     *
     * Other notes for further information. A generous 12 characters
     * is provided.
     */
    char notes[cNOTE_LENGTH];

    /**
     * \brief Flag to determine if this point should be plotted in contour mode.
     *
     * 'Y' to draw in contour mode, 'N' to not
     */
    char contourPlot;

    /**
     * \brief Flag to determine if the string this point lies on should be plotted in contour mode.
     *
     * 'S' to draw in contour mode, 'U' to not. 'B' for boundary string, 'I' internal polygon.
     */
    char contourString;

    /**
     * \brief Flag to determine if this point should be plotted in feature mode.
     *
     * 'Y' to draw in feature mode, 'N' to not
     */
    char featurePlot;

    /**
     * \brief Flag to determine if the string this point lies on should be plotted in feature mode.
     *
     * 'S' to draw in contour mode, 'U' to not
     */
    char featureString;

    /**
     * \brief A unique ID for this point.
     *
     * Every record must have a unique id. However this does not need to be in order
     * or continuous between records. This attribute is not saved with the rest of the
     * information in this structure.
     */
    unsigned long id;

    /**
     * \brief Convert to type VectorD3
     *
     * Converts the x, y and z values of this point to a keays::types::VectorD3
     */
    operator keays::types::VectorD3 () const
    {
        return keays::types::VectorD3( x, y, z );
    }

    /**
     * \brief Convert to type VectorD2
     *
     * Converts the x and y values of this point to a keays::types::VectorD2
     */
    keays::types::VectorD2 XY() const
    {
        return keays::types::VectorD2( x, y);
    }

    /**
     * \brief Convert from type VectorD3
     *
     * Sets the x, y and z values of this points to the values of the specified keays::types::VectorD3
     */
    void FromVD3( const keays::types::VectorD3 & vd3 )
    {
        x = vd3.x;
        y = vd3.y;
        z = vd3.z;
    }
};

// typedefs
typedef tStringRec                Point;

typedef std::vector<Point *>    vector;
typedef vector::iterator        iterator;
typedef vector::const_iterator    const_iterator;

/**
 * Allocates and return enough memory for a Point structure. If allocation fails
 * then exit(1) is called.
 * \param pCopyMe Pointer to a Point structure to copy.
 * \return The allocated memory. Will never return NULL.
 */
Point * GetPoint( const Point * pCopyMe = NULL );

/**
 * \brief A collection of Point structures.
 *
 * This class stores a collection of Points. It provides methods to manipulate
 * and query this data.
 */
class Data
{
public:

    /**
     * \brief Constructor.
     */
    Data();

    /**
     * \brief Deconstructor.
     */
    ~Data();

    /** Methods to manipulate the data **/

    /*
     * Delete num records at position line. Return a pointer to
     * the removed data. Delete if not required.
     */
    std::vector<Point *> * Delete ( int line, int num );

    /*
     * Delete point that is the same as s. Returns the index where
     * the record was.
     */
    Point * Delete ( Point * s, unsigned long *oIndex );

    /* Delete String with number stringNo*/
    bool Delete ( int stringNo );

    vector * Insert ( int line, int number );

    bool Insert ( unsigned int line, vector *toInsert, bool bFixId = true )
    {
        std::vector<Point *>::iterator it;

        if ( data.size() == 0 ) it = data.end();
        else if ( line >= data.size() ) it = data.end();
        else if ( line <= 0 ) it = data.begin();
        else it = &(data.at( line ));

        return Insert( it, toInsert, bFixId );
    }

    void Append( Point * s );

    bool Clear ();    // erase everything & Free memory !

    bool Insert ( iterator, vector *toInsert, bool bFixId = true );

    /*
     * Get the structure at position i in the vector with s
     */
    void Replace ( int i, Point * s );

    /* Set all the contour/feature bits of the string at row to be
       equals to value (or invert if value == 'I') */
    void FixSelectedStrings(int row, char value, bool contour);

    /*
     * Check the file to see if it is in a valid state, ie:
     * 1) There are no blank lines remaining from an insert etc
     * 2) All points on a line are continuous
     * ??
     */
    bool MakeValidState();

    void SetMaxStringNum( unsigned long i ) { m_MaxStringNum = i; }
    void SetNumStrings( int num ) { numStrings = num; }
    void SetChanged( bool b) { m_bHasChanged = b; }

    // Methods to modify individual data members
    /*
     * Note that these are dumb methods, i.e. don't consider if part of string etc
     */
    void SetStringNo( unsigned long i, unsigned long stringNo )
        { assert( i < data.size() ) ; ( data.at(i) )->stringNo = stringNo; }
    void SetPointNo( unsigned long i, unsigned long pointNo )
        { assert( i < data.size() ) ; ( data.at(i) )->pointNo = pointNo; }
    void SetX( unsigned long i, double x )
        { assert( i < data.size() ) ; ( data.at(i) )->x = x; }
    void SetY( unsigned long i, double y )
        { assert( i < data.size() ) ; ( data.at(i) )->y = y; }
    void SetZ( unsigned long i, double z )
        { assert( i < data.size() ) ; ( data.at(i) )->z = z; }
    void SetPlotCode( unsigned long i, char code [cPLOT_CODE_LENGTH] )
        { assert( i < data.size() ) ; memcpy( data.at(i)->plotCode, code, cPLOT_CODE_LENGTH ); }
    void SetPlotNotes( unsigned long i, char notes [cNOTE_LENGTH] )
        { assert( i < data.size() ) ; memcpy( data.at(i)->notes, notes, cNOTE_LENGTH ); }

    void SetContourPoint( unsigned long i, char c )
        { assert( i < data.size() ) ; data.at(i)->contourPlot = c; }
    void SetContourString( unsigned long i, char c )
        { assert( i < data.size() ) ; data.at(i)->contourString = c; }
    void SetFeaturePoint( unsigned long i, char c )
        { assert( i < data.size() ) ; data.at(i)->featurePlot = c; }
    void SetFeatureString( unsigned long i, char c )
        { assert( i < data.size() ) ; data.at(i)->featureString = c; }


    char GetContourPoint( unsigned long i )
        { assert( i < data.size() ) ; return data.at(i)->contourPlot; }
    char GetContourString( unsigned long i )
        { assert( i < data.size() ) ; return data.at(i)->contourString; }
    char GetFeaturePoint( unsigned long i )
        { assert( i < data.size() ) ; return data.at(i)->featurePlot; }
    char GetFeatureString( unsigned long i )
        { assert( i < data.size() ) ; return data.at(i)->featureString; }

    //

    /* STL wrappers */

    /**
     * \brief Behaves exactly like an insert() on an STL vector
     */
    void insert ( iterator it, Point * s ) { data.insert( it, s ); }

    /**
     * \brief Behaves exactly like an erase() on an STL vector
     */
    void erase ( iterator it ) { data.erase( it ); }

    /**
     * \brief Behaves exactly like a begin() on an STL vector
     */
    iterator begin() { return data.begin(); }

    /**
     * \brief Behaves exactly like a end() on an STL vector
     */
    iterator end() { return data.end(); }

    /**
     * \brief Behaves exactly like an at() on an STL vector
     */
    Point * at( int i ) { return data.at( i ); }

    /**
     * \brief Behaves exactly like a size() on an STL vector
     */
    long size() { return data.size(); }

    /** Methods to get information from the data **/

    // HACK //
    int GetIndex( iterator fIt )
    {
        unsigned int i = 0;
        iterator it = data.begin();

        for ( it; *it != *fIt && i < data.size(); it++ )
            i++;

        return i;
    }

    /*
     * Getters for various objects/values
     */

    unsigned int GetRecordCount () { return data.size(); }
    vector * GetDataRef ( ) { return &data; }
    unsigned int GetNumPoints() { return numPoints; };
    unsigned int GetNumStrings() { return numStrings; };
    unsigned long GetMaxStringNum() { return m_MaxStringNum; }

    /*
     * Get the structure at position i in the vector
     */
    Point * Get ( int i );

    const Point * GetFromId( unsigned int id, iterator * hintIt );

    /*
     * Return a pointer to a vector of pointers that contain the lane for the
     * line with number lineNum
     */
    vector * GetLine ( int lineNum, unsigned long & foundAt );

    /*
     * Give the string defined by stringNumber a new set of IDs so that it is
     * contiguous
     */
    bool ReIDString( int stringNumber );

    /*
     * Return a vector of strings which are the unique plot codes for
     * this model.
     */
    std::vector<std::string> GetUniquePlotCodes ( );

    /*
     * Guaranteed to return a point. Will find the closest point to the position
     * x, y, z
     */
    unsigned long FindClosestPoint( float x, float y, float z,
                                        int flags );

    unsigned long FindPoint( float x, float y, float z,
                                 float tolerance, int flags );

    /*
     * Return the index of the record that is the start of the string (if any).
     * If a string can't be found then return size()
     */
    unsigned long FindLine( double x, double y, double z,
                                double tolerance, int flags );

    std::vector<unsigned long> FindPoints( double x, double y, double z,
                                          double x2, double y2, double z2,
                                          double tolerance, int flags );

    std::vector<unsigned long> FindLines( double x, double y, double z,
                                             double x2, double y2, double z2,
                                             double tolerance, int flags );

    /* Find the first point in the vector that when converted is equal to x & y */
    Point * Find( int x, int y, int maxX, int maxY, double minX,
                         double minY, double deltaX, double deltaY );

    /* Find the first line in the vector which contains this point */
    /*int FindLine( int x, int y, int maxX, int maxY, double minX,
                         double minY, double deltaX, double deltaY );
    */
    keays::math::Cube FindMinMax();

    /*
     * True if the file has changed since it was last loaded from disk/saved
     */
    bool HasChanged() { return m_bHasChanged; }

    void FixMaxStringNum( void );    // Make sure m_MaxStringNum is in sync with the data

    std::set<unsigned long> * GetUniqueStringNumbers(); // return the string numbers

    void        SetTitle( std::string & title ) { m_Title = title; m_bHasChanged = true; }
    std::string GetTitle() { return m_Title; }

    void        SetCompany( std::string & company ) { m_Company = company; m_bHasChanged = true; }
    std::string GetCompany() { return m_Company; }

    /* Debug Methods */

    void Dump ();

    /* End Debug Methods */

private:
    bool        CheckFlags( int flags );

protected:

    vector                data;            // vector to store all the Points

    // numRecs can != numPoints when there are invalid points in the model
    long                numPoints;        // number of strings in the model
    long                numStrings;        // number of valid points in the model
    std::string            m_Title;        // title of this file
    std::string            m_Company;        // company name

    bool                m_bHasChanged;    // true if the data in memory has changed since last save/load operation

    unsigned long    m_CurrentId;    // the next created point will have this id
    unsigned long    m_MaxStringNum; // current maximum string number
};

} // namespace stringfile
} // namespace keays

#endif