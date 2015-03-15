//#define __WXMSW__

// disable annoying STL name warning
#pragma warning (disable: 4786)

/* Includes */
#ifdef __WXMSW__
#include <wx/log.h>        // bad, evil etc
#endif

#include <vector>
#include <string>
#include <math.h>

// Keays base libraries
#include <keays_math.h>      // Keays Maths Library
#include <keays_types.h>  // Keays Types Library

#include <Data.h>


#include <LeakWatcher.h>

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


namespace km = keays::math;
namespace kt = keays::types;

using namespace std;

namespace keays
{
namespace stringfile
{

// Because of rounding errors
static const float cAboutZero = 0.001f;

Point * GetPoint( const Point * copyMe )
{
    Point *s;

    s = (Point *)(malloc( sizeof(Point) ));

    if ( NULL == s ) exit(1);

    if ( NULL != copyMe ) *s = *copyMe;

    return s;
}


Data::Data()
{
    numStrings = 0;
    numPoints = 0;
    m_CurrentId = 0;
    m_MaxStringNum = 0;
}

Data::~Data()
{
    Clear();
}

/*
 * Return the StrictStruct structure at
 * the position i in the vector data.
 */
Point * Data::Get ( int i )
{
    return (Point *)data[i];
}

const Point * Data::GetFromId( unsigned int id, iterator * hintIt )
{
    iterator it;
    bool b2ndPass;

    if ( *hintIt !=  NULL )
    {
        it = *hintIt;
        b2ndPass = true;
    }
    else
    {
        it = data.begin();
        b2ndPass = false;
    }

    for ( it; it != data.end(); it++ )
        if ( id == (*it)->id )
        {
            *hintIt = it;
            return *it;
        }

    if (b2ndPass)
        for ( it = data.begin(); it != data.end(); it++ )
            if ( id == (*it)->id )
            {
                *hintIt = it;
                return *it;
            }

    *hintIt = NULL;
    return NULL;
}

/*
 * Replace the StringStruct as index with s.
 */
void Data::Replace ( int index, Point * s )
{
    memcpy(data[index]->notes, s->notes, cNOTE_LENGTH);
    memcpy(data[index]->plotCode, s->plotCode, cPLOT_CODE_LENGTH);
    data[index]->pointNo = s->pointNo;
    data[index]->stringNo = s->stringNo;
    data[index]->x = s->x;
    data[index]->y = s->y;
    data[index]->z = s->z;
    data[index]->id = s->id;

    m_bHasChanged = true;
}

void Data::FixMaxStringNum ()
{
    unsigned long maxStringNo = 0;
    iterator it;

    for ( it = data.begin(); it != data.end(); it++ )
        if ( (*it)->stringNo > maxStringNo )
            maxStringNo = (*it)->stringNo;

    SetMaxStringNum( maxStringNo );
}

vector * Data::Delete ( int startLine, int count )
{
    iterator it;
    iterator endIt;
    iterator startIt;

    vector * deleted = new vector();

#ifdef __WXMSW__
    wxASSERT( startLine >= 0 );
    wxASSERT( count >= 1 );
#endif

    endIt = data.end(); // for deleting last entry

    startIt = &(data.at(startLine));
    endIt = &(data.at(startLine + count - 1));
    endIt++; // 1 beyond

    for ( it = startIt; it != endIt; it++ )
    {
        deleted->push_back( *it );
    }
    /*
    for ( it = startIt; it !=endIt; it++ ) {
        free(*it);
    }
    */

    data.erase(startIt, endIt);

    numPoints = numPoints - count;
    m_bHasChanged = true;
    return deleted;
}

Point * Data::Delete ( Point *s, unsigned long *oIndex )
{
    iterator it;
    Point * delPoint = NULL;
    unsigned long index = 0;

    numPoints--;
    for ( it = data.begin(); it != data.end(); it++ )
    {
        if ( (*it)->x == s->x &&
             (*it)->y == s->y &&
             (*it)->z == s->z )
        {
            delPoint = *it;
            data.erase(it);
            m_bHasChanged = true;
            *oIndex = index;
            return delPoint;;
        }

        index++;
    }

    return NULL;

}

// BROKEN - how can this possibly work?
bool Data::Delete ( int stringNo )
{
    iterator it;
    bool found = false;

    numStrings--;

    for ( it = data.begin(); it != data.end(); it++ )
    {
        if ( (*it)->stringNo == stringNo ) {
            found = true;
            free(*it);
            data.erase(it);
        } else {
            if ( found )
            {
                m_bHasChanged = true;
                return true;
            }
        }
        numPoints--;

    }
    return false;
}

/*
 * Should not modify m_bHasChanged - used when files are first loaded only
 */
void Data::Append( Point * s )
{
    s->id = m_CurrentId;
    m_CurrentId++;
    data.push_back( s );
    numPoints++;
}

//bool Data::Insert ( int line, StringRecord d )
//{

//    return true;
//}

vector * Data::Insert ( int line, int number )
{

    vector * insertVector = new vector();
    iterator it;

    if ( 0 == line ) it = data.begin();
    else if ( data.size() == line ) it = data.end();
    else it = &(data.at(line));

    /* Insert the required number of lines */
    vector tmpVec;

    for ( int i = 0; i < number; i++ ) {
        Point *tmp = (Point *) malloc ( cRECORD_LENGTH );
        memset( tmp, '\0', cNUMBER_LENGTH );
        memset( (void *)&(((char *)tmp)[cNUMBER_LENGTH]), ' ', cTEXT_LENGTH );
        tmp->pointNo = m_CurrentId;    // give it the current id
        tmp->stringNo = 0;            // by default make it a single point
        tmp->id = m_CurrentId;
        m_CurrentId++;
        tmpVec.push_back(tmp);
        insertVector->push_back( tmp );
        numPoints++;
    }

    data.insert(it, tmpVec.begin(), tmpVec.end());

    m_bHasChanged = true;
    return insertVector;
}

/*
 * Iterator to insert before. Vector to insert
 */
bool Data::Insert ( iterator it, vector *pToInsert, bool bFixId )
{
    iterator iIt;

    for ( iIt = pToInsert->begin(); iIt != pToInsert->end(); iIt++ )
    {
        if ( bFixId )
        {
#ifdef __WXMSW__
            //wxLogDebug("Insert->Updating ID %d => ID %d", (*iIt)->id, m_CurrentId );
#endif
            (*iIt)->id = m_CurrentId;
            m_CurrentId++;
        }
    }

    numPoints = numPoints + pToInsert->size();
    data.insert( it, pToInsert->begin(), pToInsert->end() );

    m_bHasChanged = true;

    return true;
}



bool Data::Clear ( )
{
    iterator it;
    for ( it = data.begin(); it != data.end(); it++ )
    {
        free(*it);
    }
    data.clear();
    numPoints = 0;
    numStrings = 0;
    m_CurrentId = 0;

    m_bHasChanged = false;
    return true;
}

void Data::Dump ()
{
/*#ifdef __WXMSW__
    iterator it;


    //wxLogDebug("-- Start Data Dump --");

    for ( it = data.begin(); it != data.end(); it++ )
    {
        Point *s = *it;
        //wxLogDebug ("(%d) %d %d %f %f %f", // %s %s |%c|%c|%c|%c",
                     //s->id, s->stringNo, s->pointNo, s->x, s->z, s->y );
                     //s->notes, s->plotCode, s->contourPlot, s->contourString,
                     //s->featurePlot, s->featureString );
    }

    wxLogDebug("-- End Data Dump --");
#endif*/
}

std::vector<std::string> Data::GetUniquePlotCodes ( )
{
    std::vector<std::string> strings;
    iterator it;
    std::vector<std::string>::iterator it2;
    bool add = true;
    char plotCode[cPLOT_CODE_LENGTH+1];

    for ( it = data.begin(); it != data.end(); it++ )
    {
        memset( plotCode, 0, cPLOT_CODE_LENGTH+1 );
        strncpy(plotCode, (*it)->plotCode, cPLOT_CODE_LENGTH);
        for ( it2 = strings.begin(); it2 != strings.end(); it2++ )
        {
            if ( (*it2) == plotCode )
            {
                add = false;
                break;
            }
        }
        if (add) strings.push_back(plotCode);
        add = true;
    }
    return strings;
}

unsigned long Data::FindClosestPoint( float x, float y, float z,
                                          int flags )
{
    iterator it;
    unsigned long index = 0;
    unsigned long closestIndex = 0;
    Point *s;
    double d,    // distance
           cd = 99e99;    // closest distance

    if ( data.size() == 0 ) return -1;
    if ( data.size() == 1 ) return -1;
    CheckFlags( flags );

    int layerFlags = flags & ( eFF_CONTOUR|eFF_FEATURE|eFF_ANY|eFF_BOTH );

    it = data.begin();
    s = *it;

    while ( (flags & eFF_POINT) && s->stringNo > 0 )
    {
        it++;
        index++;
        if ( data.end() == it ) return -1;
        s = *it;
    }

    if ( flags & eFF_IGNORE_X )
    {
        for ( ; it != data.end(); it++ )
        {
            s = *it;
            if (

             ( (s->stringNo == 0 && s->contourPlot == cPlotPoint && s->featurePlot == cNoPlotPoint ) ||
               (s->stringNo > 0 && s->contourString != cNoSelectString && s->featureString == cNoSelectString) ||
               (layerFlags & ~(eFF_CONTOUR)) )

                &&

             ( (s->stringNo == 0 && s->featurePlot == cPlotPoint && s->contourPlot == cNoPlotPoint ) ||
               (s->stringNo > 0 && s->featureString == cSelectString && s->contourString != cSelectString ) ||
               (layerFlags & ~(eFF_FEATURE)) )

                &&

             ( (s->stringNo == 0 && s->featurePlot == cPlotPoint && s->contourPlot == cPlotPoint ) ||
               (s->stringNo > 0 && s->featureString == cSelectString && s->contourString != cNoSelectString ) ||
               (layerFlags & ~(eFF_BOTH)) )

                &&

             ( (s->stringNo == 0 && (s->featurePlot == cPlotPoint || s->contourPlot == cPlotPoint) ) ||
               (s->stringNo > 0 && (s->featureString == cSelectString || s->contourString != cNoSelectString) ) ||
               (layerFlags & ~(eFF_ANY)) )

                &&

                 (((eFF_POINT & flags) && s->stringNo == 0 ) ||
                 !(eFF_POINT & flags))
                 )
            {
                d = sqrt( pow(z - s->z, 2) + pow(y - s->y, 2) );
                if ( d < cd )
                {
                    cd = d;
                    closestIndex = index;
                }
            }
            index++;
        }
    }
    else if ( flags & eFF_IGNORE_Y )
    {
        for ( ; it != data.end(); it++ )
        {
            s = *it;
            if (
               ( (s->stringNo == 0 && s->contourPlot == cPlotPoint && s->featurePlot == cNoPlotPoint ) ||
               (s->stringNo > 0 && s->contourString != cNoSelectString && s->featureString == cNoSelectString) ||
               (layerFlags & ~(eFF_CONTOUR)) )

                &&

             ( (s->stringNo == 0 && s->featurePlot == cPlotPoint && s->contourPlot == cNoPlotPoint ) ||
               (s->stringNo > 0 && s->featureString == cSelectString && s->contourString != cSelectString ) ||
               (layerFlags & ~(eFF_FEATURE)) )

                &&

             ( (s->stringNo == 0 && s->featurePlot == cPlotPoint && s->contourPlot == cPlotPoint ) ||
               (s->stringNo > 0 && s->featureString == cSelectString && s->contourString != cNoSelectString ) ||
               (layerFlags & ~(eFF_BOTH)) )

                &&

             ( (s->stringNo == 0 && (s->featurePlot == cPlotPoint || s->contourPlot == cPlotPoint) ) ||
               (s->stringNo > 0 && (s->featureString == cSelectString || s->contourString != cNoSelectString) ) ||
               (layerFlags & ~(eFF_ANY)) )

                    &&

                 (((eFF_POINT & flags) && s->stringNo == 0 ) ||
                 !(eFF_POINT & flags))

                 )
            {
                d = sqrt( pow(x - s->x, 2) + pow(z - s->z, 2) );
                if ( d < cd )
                {
                    cd = d;
                    closestIndex = index;
                }
            }
            index++;
        }
    }
    else if ( flags & eFF_IGNORE_Z )
    {
        for ( ; it != data.end(); it++ )
        {
            s = *it;
            if (

             ( (s->stringNo == 0 && s->contourPlot == cPlotPoint && s->featurePlot == cNoPlotPoint ) ||
               (s->stringNo > 0 && s->contourString != cNoSelectString && s->featureString == cNoSelectString) ||
               (layerFlags & ~(eFF_CONTOUR)) )

                &&

             ( (s->stringNo == 0 && s->featurePlot == cPlotPoint && s->contourPlot == cNoPlotPoint ) ||
               (s->stringNo > 0 && s->featureString == cSelectString && s->contourString != cSelectString ) ||
               (layerFlags & ~(eFF_FEATURE)) )

                &&

             ( (s->stringNo == 0 && s->featurePlot == cPlotPoint && s->contourPlot == cPlotPoint ) ||
               (s->stringNo > 0 && s->featureString == cSelectString && s->contourString != cNoSelectString ) ||
               (layerFlags & ~(eFF_BOTH)) )

                &&

             ( (s->stringNo == 0 && (s->featurePlot == cPlotPoint || s->contourPlot == cPlotPoint) ) ||
               (s->stringNo > 0 && (s->featureString == cSelectString || s->contourString != cNoSelectString) ) ||
               (layerFlags & ~(eFF_ANY)) )

                    &&

                 (((eFF_POINT & flags) && s->stringNo == 0 ) ||
                 !(eFF_POINT & flags))
                 )
            {
                d = sqrt( pow(x - s->x, 2) + pow(y - s->y, 2) );
                if ( d < cd )
                {
                    cd = d;
                    closestIndex = index;
                }
            }
            index++;
        }
    }

    if ( (eFF_POINT & flags) && (data.at(closestIndex)->stringNo > 0) )    // can't return a stringed point
        return -1;

    return closestIndex;
}

unsigned long Data::FindPoint( float x, float y, float z,
                                   float tolerance, int flags )
{
    iterator it;
    unsigned long index = 0;
    CheckFlags( flags );

    for ( it = data.begin(); it != data.end(); it++ )
    {
        Point *s = *it;


        if ( ( (x >= s->x - tolerance &&
              x <= s->x + tolerance) || ( flags & eFF_IGNORE_X ) )

                 &&

             ( (y >= s->y - tolerance &&
                y <= s->y + tolerance) || ( flags & eFF_IGNORE_Y ) )

                 &&

             ( (z >= s->z - tolerance &&
                z <= s->z + tolerance) || ( flags & eFF_IGNORE_Z )  )

                &&

             ( (s->stringNo == 0 && s->contourPlot == cPlotPoint) ||
               (s->stringNo > 0 && s->contourString != cNoSelectString ) ||
               (flags & eFF_FEATURE) )

                &&

             ( (s->stringNo == 0 && s->featurePlot == cPlotPoint) ||
               (s->stringNo > 0 && s->featureString == cSelectString ) ||
               (flags & eFF_CONTOUR) )
          )
        {
            return index;
        }

        index++;
    }
    index++;
    return index;
}


/*
 * Preconditon: x,y,z must specify the smaller values (as opposed to x2,y2,z2)
 */
std::vector<unsigned long> Data::FindPoints( double x, double y, double z,
                                      double x2, double y2, double z2,
                                      double tolerance, int flags )
{
    std::vector<unsigned long> foundPoints;
    iterator it;
    unsigned long index = 0;
    CheckFlags( flags );

    // apply tolerance
    x  = x  - tolerance;
    x2 = x2 + tolerance;
    y  = y  - tolerance;
    y2 = y2 + tolerance;
    z  = z  - tolerance;
    z2 = z2 + tolerance;

    int layerFlags = flags & ( eFF_CONTOUR | eFF_FEATURE | eFF_BOTH | eFF_ANY );

    for ( it = data.begin(); it != data.end(); it++ )
    {
        Point *s = *it;

        if ( ( (s->x >= x && s->x <= x2) || eFF_IGNORE_X & flags )

                 &&

             ( (s->y >= y && s->y <= y2) || eFF_IGNORE_Y & flags )

                 &&

             ( (s->z >= z && s->z <= z2) || eFF_IGNORE_Z & flags )

                &&

             ( (s->stringNo == 0 && s->contourPlot == cPlotPoint && s->featurePlot == cNoPlotPoint ) ||
               (s->stringNo > 0 && s->contourString != cNoSelectString && s->featureString == cNoSelectString) ||
               (layerFlags & ~(eFF_CONTOUR)) )

                &&

             ( (s->stringNo == 0 && s->featurePlot == cPlotPoint && s->contourPlot == cNoPlotPoint ) ||
               (s->stringNo > 0 && s->featureString == cSelectString && s->contourString != cSelectString ) ||
               (layerFlags & ~(eFF_FEATURE)) )

                &&

             ( (s->stringNo == 0 && s->featurePlot == cPlotPoint && s->contourPlot == cPlotPoint ) ||
               (s->stringNo > 0 && s->featureString == cSelectString && s->contourString != cNoSelectString ) ||
               (layerFlags & ~(eFF_BOTH)) )

                &&

             ( (s->stringNo == 0 && (s->featurePlot == cPlotPoint || s->contourPlot == cPlotPoint) ) ||
               (s->stringNo > 0 && (s->featureString == cSelectString || s->contourString != cNoSelectString) ) ||
               (layerFlags & ~(eFF_ANY)) )
            )
        {
            foundPoints.push_back( index );
        }
        index++;
    }

    return foundPoints;
}

unsigned long Data::FindLine( double x, double y, double z,
                                  double tolerance, int flags )
{
    iterator it, nIt;
    Point *s1, *s2;
    unsigned long index = 0;
    bool    xOk = false, yOk = false, zOk = false;
    float    dX, dY, dZ, m;
    float    minX, maxX, minY, maxY, minZ, maxZ;

    CheckFlags( flags );

    for ( it = data.begin(); it != data.end(); it++ )
    {
        s1 = *it;
        nIt = it;
        nIt++;
        if ( nIt == data.end() ) break;
        s2 = *nIt;

        if ( s1->stringNo != 0 && s2->stringNo != 0 && s1->stringNo == s2->stringNo)
        {    // it is a line
            nIt = it;
            nIt++;

            // its current seen
            if ( !( (s1->contourString != cNoSelectString || flags & eFF_FEATURE ) &&
                    (s1->featureString == cSelectString || flags & eFF_CONTOUR ) ) )
            {
                index++;
                continue;
            }

            dX = s2->x - s1->x;
            dY = s2->y - s1->y;
            dZ = s2->z - s1->z;

            // determine the minimum values of s1 & s2
            if ( s1->x > s2->x ) { minX = s2->x; maxX = s1->x; }
            else { minX = s1->x; maxX = s2->x; }

            if ( s1->y > s2->y ) { minY = s2->y; maxY = s1->y; }
            else { minY = s1->y; maxY = s2->y; }

            if ( s1->z > s2->z ) { minZ = s2->z; maxZ = s1->z; }
            else { minZ = s1->z; maxZ = s2->z; }

            // Check if the inputs are inside this particular lines bounds
            if ( (x >= (minX - tolerance) && x <= (maxX + tolerance)) ||
                 (eFF_IGNORE_X & flags) )
            {
                xOk = true;
            }

            if ( (y >= (minY - tolerance) && y <= (maxY + tolerance)) ||
                 (eFF_IGNORE_Y & flags) )
            {
                yOk = true;
            }

            if ( (z >= (minZ -tolerance) && z <= (maxZ + tolerance)) ||
                 (eFF_IGNORE_Z & flags) )
            {
                zOk = true;
            }


            if ( xOk && yOk && zOk )
            {    // its inside the 'big box'

                float realY, realZ;

                // substitue in 'x' in get correct 'y' value
                if ( flags & eFF_IGNORE_Z )    // Top
                {
/*                    wxLogDebug( "Testing a line!!");
                    wxLogDebug( "----------------------------------------");
                    wxLogDebug( "From Line %d, Point %d to Line %d, Point %d ",
                                 s1->stringNo, s1->pointNo, s2->stringNo, s2->pointNo );
                    wxLogDebug( "Points: 1: %f %f | 2: %f %f", s1->x, s1->y,
                                s2->x, s2->y);
*/

                    if ( fabs(dX) < cAboutZero ||
                         fabs(dY) < cAboutZero )
                    {
//                            wxLogDebug( "**** TEST SUCCEEDED (dX||dY==0) ****");
//                            wxLogDebug( "----------------------------------------");
                         return index;
                    }

                    m = dY/dX;

                    realY = (m * (x-s1->x)) + s1->y;    // normalized Y
/*#ifdef __WXMSW__
                    wxLogDebug( "Actual Normalized Y: %f, Clicked Normalized Y: %f", realY, y );
#endif*/
                    if ( y >= realY - tolerance && y <= realY + tolerance )
                    {
                        //wxLogDebug( "**** TEST SUCCEEDED ****");
                        //wxLogDebug( "----------------------------------------");
                        return index;
                    }
                    //wxLogDebug( "**** TEST FAILED ****");
                    //wxLogDebug( "----------------------------------------");
                }
                else if ( flags & eFF_IGNORE_Y )
                {
                    if ( fabs(dZ) < cAboutZero ||
                         fabs(dY) < cAboutZero )
                    {
                        return index;
                    }

                    m = dZ/dX;

                    realZ = (m * (x-s1->x)) + s1->z;    // normalized Z

                    //wxLogDebug( "From Line %d, Point %d to Line %d, Point %d ",
                                 //s1->stringNo, s1->pointNo, s2->stringNo, s2->pointNo );
                    //wxLogDebug( "Normalization Values: %f %f", s1->x, s1->y );
                    //wxLogDebug( "Actual Normalized Y: %f, Clicked Normalized Y: %f", realZ, z );

                    if ( z >= realZ - tolerance && z <= realZ + tolerance ) return index;
                }
                else if ( flags & eFF_IGNORE_X )    // Left
                {
                    //if ( fabs(dY) < cAboutZero )
                    //    return index;
                    //else
                    //
                    if ( fabs(dZ) < cAboutZero ||
                         fabs(dY) < cAboutZero )
                    {
                        return index;
                    }

                    m = dZ/dY;

                    realZ = (m * (y-s1->y)) + s1->z;    // normalized X

                    /*
                    wxLogDebug( "From Line %d, Point %d to Line %d, Point %d ",
                                 s1->stringNo, s1->pointNo, s2->stringNo, s2->pointNo );
                    wxLogDebug( "Normalization Values: %f %f", s1->y, s1->z );
                    wxLogDebug( "Actual Normalized Y: %f, Clicked Normalized Y: %f", realZ, z );
                    */

                    if ( z >= realZ - tolerance && z <= realZ + tolerance ) return index;
                }    // if
            }    // if xOk && yOk && zOk

            xOk = yOk = zOk = false;
        }    // if ( s1->stringNo != 0 && s2->stringNo != 0)
        index++;
    }    // for loop

    return data.size();
}

bool Data::CheckFlags( int flags )
{
    bool bOk = true;
    int cnt = 0;

    if ( flags & eFF_CONTOUR ) cnt++;
    if ( flags & eFF_FEATURE ) cnt++;
    if ( flags & eFF_ANY ) cnt++;
    if ( flags & eFF_BOTH ) cnt++;

    if ( cnt > 1 )
    {
/*#ifdef __WXMSW__
        wxLogDebug( "Too many of: eFF_CONTOUR, eFF_ANY, eFF_BOTH, eFF_FEATURE specified!" );
#endif*/
        bOk = false;
    }

    if ( !(flags & (eFF_SORT_MAX|eFF_SORT_MIN)) )
    {
/*#ifdef __WXMSW__
        wxLogDebug( "Both eFF_SORT_MAX & eFF_SORT_MIN specified!" );
#endif*/
        bOk = false;
    }

    if ( !(flags & (eFF_IGNORE_X|eFF_IGNORE_Y|eFF_IGNORE_Z)) )
    {
/*#ifdef __WXMSW__
        wxLogDebug( "Two many of: eFF_IGNORE_X, eFF_IGNORE_Y and eFF_IGNORE_Z specified!" );
#endif*/
        bOk = false;
    }

    if ( !(flags & (eFF_CONTOUR|eFF_FEATURE|eFF_ANY|eFF_BOTH)) )
    {
/*#ifdef __WXMSW__
        wxLogDebug( "Need to specify one of: eFF_CONTOUR | eFF_FEATURE | eFF_ANY | eFF_BOTH!" );
#endif*/
        bOk = false;
    }

    if ( !( (flags & eFF_SORT_MAX) || (flags & eFF_SORT_MIN) ) )
    {
/*#ifdef __WXMSW__
        wxLogDebug( "Need to specify one of: eFF_SORT_MAX | eFF_SORT_MIN!" );
#endif*/
        bOk = false;
    }

    if ( !( (flags & eFF_IGNORE_X) || (flags & eFF_IGNORE_Y)
                                   || (flags & eFF_IGNORE_Z) ) )
    {
/*#ifdef __WXMSW__
        wxLogDebug( "Need to specify one of: eFF_IGNORE_X | eFF_IGNORE_Y | eFF_IGNORE_Z!" );
#endif*/
        bOk = false;
    }

    if ( !bOk )
    {
/*#ifdef __WXMSW__
        wxLogError( "Incorrect combination of flags given to Find* function!");
#endif*/
    }

    return bOk;
}

//
// Precondition - Assumes data is sorted in increasing order!!!! ( ie x < x2, y < y2, z < z2 )
//
std::vector<unsigned long> Data::FindLines( double x, double y, double z,
                                                double x2, double y2, double z2,
                                                double tolerance, int flags )
{
    std::vector<unsigned long> lines;
    iterator it, nIt;
    unsigned long index = 0;
    Point *s1, *s2;
    bool    xOk = false, yOk = false, zOk = false;
    bool    addPoint = false;
    float    dX, dY, dZ;//, m;
    float    minX, maxX, minY, maxY, minZ, maxZ;

    CheckFlags( flags );

    int layerFlags = flags & ( eFF_CONTOUR | eFF_FEATURE | eFF_BOTH | eFF_ANY );

    // apply tolerance
    x -= tolerance;
    y -= tolerance;
    z -= tolerance;

    x2 += tolerance;
    y2 += tolerance;
    z2 += tolerance;

/*#ifdef __WXMSW__
    wxLogDebug("Entering FindLines()!");


    // Check preconditions
    wxASSERT( x <= x2 );
    wxASSERT( y <= y2 );
    wxASSERT( z <= z2 );
#endif __WXMSW__*/

    for ( it = data.begin(); it != data.end(); it++ )
    {
        s1 = *it;
        nIt = it;
        nIt++;
        if ( nIt == data.end() ) break;
        s2 = *nIt;

        if ( s1->stringNo != 0 && s2->stringNo != 0 && s1->stringNo == s2->stringNo)
        {    // it is a string, and its the same string in both points
            nIt = it;
            nIt++;

            // its current seen
            switch ( layerFlags )
            {
                case eFF_CONTOUR:
                    if ( (s1->contourString == cNoSelectString || s1->featureString != cNoSelectString ) )
                    {
                        index++;
                        continue;
                    }
                break;

                case eFF_FEATURE:
                    if ( (s1->featureString == cNoSelectString || s1->contourString != cNoSelectString ) )
                    {
                        index++;
                        continue;
                    }
                break;

                case eFF_ANY:
                    if ( (s1->featureString == cNoSelectString && s1->contourString == cNoSelectString ) )
                    {
                        index++;
                        continue;
                    }
                break;

                case eFF_BOTH:
                    if ( (s1->featureString == cNoSelectString || s1->contourString == cNoSelectString ) )
                    {
                        index++;
                        continue;
                    }
                break;
            }

            // First check if its quite impossible for these points to be in the box. If so
            // then break out of any more calculations
            if ( ((s1->x < x && s2->x < x) && !(eFF_IGNORE_X & flags)) ||
                 ((s1->x > x2 && s2->x > x2 ) && !(eFF_IGNORE_X & flags)) ||
                 ((s1->y < y && s2->y < y ) && !(eFF_IGNORE_Y & flags)) ||
                 ((s1->y > y2 && s2->y > y2) && !(eFF_IGNORE_Y & flags)) ||
                 ((s1->z < z && s2->z < z ) && !(eFF_IGNORE_Z & flags)) ||
                 ((s1->z > z2 && s2->z > z2) && !(eFF_IGNORE_Z & flags))
               )
            {
                index++; continue;
            }

            dX = s1->x - s2->x;
            dY = s1->y - s2->y;
            dZ = s1->z - s2->z;

            // Quick Tests - Are either of the points s1,s2 inside the selection box?
            if ( ((s1->x >= x && s1->x <= x2) || eFF_IGNORE_X & flags ) &&
                 ((s1->y >= y && s1->y <= y2) || eFF_IGNORE_Y & flags ) &&
                 ((s1->z >= z && s1->z <= z2) || eFF_IGNORE_Z & flags )

                 ||

                 ((s2->x >= x && s2->x <= x2) || eFF_IGNORE_X & flags ) &&
                 ((s2->y >= y && s2->y <= y2) || eFF_IGNORE_Y & flags ) &&
                 ((s2->z >= z && s2->z <= z2) || eFF_IGNORE_Z & flags ) )
            {
                //wxLogDebug("**!!Point contained case:");
                // Short circuit evaluation for efficieny
                lines.push_back( index );
                index++; continue;
            }


            if ( s1->x > s2->x ) { minX = s2->x; maxX = s1->x; }
            else { minX = s1->x; maxX = s2->x; }

            if ( s1->y > s2->y ) { minY = s2->y; maxY = s1->y; }
            else { minY = s1->y; maxY = s2->y; }

            if ( s1->z > s2->z ) { minZ = s2->z; maxZ = s1->z; }
            else { minZ = s1->z; maxZ = s2->z; }

            /*
             *      |
             *      |
             *   y+---+
             * ---|   |---
             *  y2+---+
             *    x | x2
             *      |
             *
             */
            if (
                 // x / y cases
                 ((minX > x && maxX < x2) && (minY < y && maxY > y2) && eFF_IGNORE_Z & flags) ||    // vertical
                 ((minX < x && maxX > x2) && (minY > y && maxY < y2) && eFF_IGNORE_Z & flags) || // horizontal

                 // x / z cases
                 ((minX > x && maxX < x2) && (minZ < z && maxZ > z2) && eFF_IGNORE_Y & flags) ||    // vertical
                 ((minX < x && maxX > x2) && (minZ > z && maxZ < z2) && eFF_IGNORE_Y & flags) || // horizontal

                 // y / z cases
                 ((minY > y && maxY < y2) && (minZ < z && maxZ > z2) && eFF_IGNORE_X & flags) ||    // vertical
                 ((minY < y && maxY > y2) && (minZ > z && maxZ < z2) && eFF_IGNORE_X & flags)  // horizontal
                )
            {
                //wxLogDebug("**!!Horiz/Vert Case: Point 1: %f %f %f, Point 2: %f %f %f",
                //            s1->x, s1->y, s1->z, s2->x, s2->y, s2->z );
                // Short circuit evaluation for efficieny
                lines.push_back( index );
                index++; continue;
            }

            /* Finally we need to check for a line the enters via a side, and exits
             * via a side (or visa-versa)
             * To do this will require line-line intersect
             */
            kt::VectorD2 p1, p2;
            float slope;

            switch (flags & (eFF_IGNORE_X|eFF_IGNORE_Y|eFF_IGNORE_Z))
            {
                case eFF_IGNORE_X:
                {
                    p1 = kt::VectorD2( s1->z, s1->y );
                    p2 = kt::VectorD2( s2->z, s2->y );
                    slope = (s2->y - s1->y) / (s2->z - s1->z);
                };
                case eFF_IGNORE_Y:
                {
                    p1 = kt::VectorD2( s1->x, s1->z );
                    p2 = kt::VectorD2( s2->x, s2->z );
                    slope = (s2->z - s1->z) / (s2->x - s1->x);
                };
                case eFF_IGNORE_Z:
                {
                    p1 = kt::VectorD2( s1->x, s1->y );
                    p2 = kt::VectorD2( s2->x, s2->y );
                    slope = (s2->y - s1->y) / (s2->x - s1->x);
                };
            };

            kt::VectorD2 p3( x, y );
            kt::VectorD2 p4( x2, y2 );

            if ( slope > 0 )
            {
                p3 = kt::VectorD2( x, y2 );
                p4 = kt::VectorD2( x2, y );
            }

            kt::VectorD2 rp;

            int result = km::LineLineIntersect( p1, p2, p3, p4, rp );

            if ( km::S_INTERSECT == result )
            {
/*#ifdef __WXMSW__
                wxLogDebug("**!!We have teh intersection");
#endif*/

                /*s = new Point();
                s->x =rp.x;
                s->y =rp.y;
                s->z = 0;
                s->contourPlot = 'Y';
                s->featurePlot = 'Y';
                s->contourString = 'S';
                s->featureString = 'S';
                s->stringNo = 0;
                s->pointNo = 100;
                */
                if ( rp.x >= x && rp.x <= x2 && rp.y >= y && rp.y <= y2 )
                {
                    //wxLogDebug("**!!Last Case: Point 1: %f %f %f, Point 2: %f %f %f. "
                    //        " Intersection at: %f %f",
                    //        s1->x, s1->y, s1->z, s2->x, s2->y, s2->z, rp.x, rp.y );
                    lines.push_back( index );
                }
            }
        }

        index++;
    }

    //Append( s );
/*#ifdef __WXMSW__
    wxLogDebug("Exiting FindLines()!");
#endif*/
    return lines;
}

/*
 * x - the on screen x coordinate (pixels)
 * y - the on screen y corrdinate (pixels)
 * maxX - the width of the displayed area in pixels
 * maxY - the height of the displayed area in pixels
 * minX - the world X coordinate of the upper left hand corner
 * minY - the world Y coordinate of the upper left hand corner
 * deltaX - the world width of the displayed area
 * deltaY - the world height of the displayed area
 */
Point * Data::Find( int x, int y, int maxX, int maxY, double minX,
                   double minY, double deltaX, double deltaY )
{
    iterator it;

    for ( it = data.begin(); it != data.end(); it++ )
    {
        Point *s = *it;
        int a = (int)((s->x - minX) / deltaX * maxX);
        int c = (int)((s->y - minY) / deltaY * maxY);

        if ( ( a >= x - 2 &&
               a <= x + 2) &&

             ( c >= y - 2 &&
               c <= y + 2) )
        {
            return s;
        }
    }

    return NULL;
}

/*
int Data::FindLine( int x, int y, int maxX, int maxY, double minX,
                   double minY, double deltaX, double deltaY )
{
    vector<Point *>::iterator it;

    for ( it = data.begin(); it != data.end(); it++ )
    {
        Point *s = *it;
        int a = (int)((s->x - minX) / deltaX * maxX);
        int c = (int)((s->y - minY) / deltaY * maxY);

        if ( ( a >= x - 2 &&
               a <= x + 2) &&

             ( c >= y - 2 &&
               c <= y + 2) )
        {
            return s->stringNo;
        }
    }

    return NULL;
}
*/


/* Make sure all points in a string have a consistent state
 * in their string field (ie selected or unselected)
 */
void Data::FixSelectedStrings(int row, char value, bool contour)
{
    unsigned int stringNo;
    unsigned int curRow;
    char orgVal;
    Point *s = data.at(row);
    stringNo = s->stringNo;
    curRow = row;

    if ( stringNo == 0 )
    {
/*#ifdef __WXMSW__
        wxLogDebug( "Data::FixSelectedStrings - You should NOT be here!");
#endif*/
        return;
    }

    if ( contour )
        orgVal = data.at(row)->contourString;
    else
        orgVal = data.at(row)->featureString;

    /* find start of string */
    while (s->stringNo == stringNo && curRow > 0)
    {
        s = data.at(--curRow);
    }

    if (curRow > 0)
        s = data.at(++curRow);

    /* go through and set the string to be (un)selected */
    while (s->stringNo == stringNo && curRow < (data.size() - 1))
    {
        if (contour)
        {
            if (value != 'I')
                s->contourString = value;
            else
                if (orgVal == 'S')
                    s->contourString = 'U';
                else
                    s->contourString = 'S';
        } else {
            if (value != 'I')
                s->featureString = value;
            else
                if (orgVal == 'S')
                    s->featureString = 'U';
                else
                    s->featureString = 'S';
        }

        s = data.at(++curRow);
    }

    if (curRow == data.size() - 1)
    {
        if (contour)
        {
            if (value != 'I')
                s->contourString = value;
            else
                if (orgVal == 'S')
                    s->contourString = 'U';
                else
                    s->contourString = 'S';
        } else {
            if (value != 'I')
                s->featureString = 'S';
            else
                if (orgVal == 'S')
                    s->featureString = 'U';
                else
                    s->featureString = 'S';
        }
    }
    m_bHasChanged = true;


/***** ******/
}

keays::math::Cube Data::FindMinMax()
{
    keays::math::Cube minMax;
    iterator it = data.begin();

    if ( data.size() == 0 )
    {
        memset( &minMax, 0, sizeof(keays::math::Cube) );
        return minMax;
    }

    double x, y, z;

    x = (*it)->x;
    y = (*it)->y;
    z = (*it)->z;

    minMax = keays::math::Cube( x,x,y,y,z,z );
    //minMax.SetX( = minMax.minX = x;
    //minMax.maxY = minMax.minY = y;
    //minMax.maxZ = minMax.minZ = z;

    /* Loop throught the whole model.... */
    for ( ; it != data.end(); it++ )
    {
        x = (*it)->x;
        y = (*it)->y;
        z = (*it)->z;

        if ( x < minMax.GetLeft() ) minMax.Left( x );
        if ( y < minMax.GetBottom() ) minMax.Bottom( y );
        if ( z < minMax.Base() ) minMax.Base( z );

        if ( x > minMax.GetRight() ) minMax.Right( x );
        if ( y > minMax.GetTop() ) minMax.Top( y );
        if ( z > minMax.Roof() ) minMax.Roof( z );
    }

    return minMax;
}

set<unsigned long> * Data::GetUniqueStringNumbers()
{
    iterator it;
    set<unsigned long> * strings = new set<unsigned long>();

    for (it = data.begin(); it != data.end(); it++)
        strings->insert( (*it)->stringNo );

    return strings;
}

/* Everything below here is buggy/slow/useless */
vector * Data::GetLine ( int lineNum, unsigned long & startAt )
{
    unsigned long lastNum = 0;
    unsigned long lineCnt = 0;
    bool    bFound = false;
    iterator it;
    vector * retVec = new vector();

    if ( 0 == lineNum ) return NULL;

    for ( it = data.begin(); it != data.end(); it++ )
    {
        if ( (*it)->stringNo == lineNum )
        {
            if ( !bFound ) startAt = lineCnt;
            if ( bFound && lastNum != lineNum )
            {
/*#ifdef __WXMSW__
                wxLogDebug("**Non continuous string found: %d!", lineNum);
#endif*/
            }
            retVec->push_back(*it);
            bFound = true;
        }
        lastNum = lineNum;
        lineCnt++;
    }
    return retVec;
}

bool Data::ReIDString( int stringNumber )
{
    unsigned long lastNum = 0;
    bool    bFound = false;
    iterator it;

    if ( 0 == stringNumber ) return false;

/*#ifdef __WXMSW__
    wxLogDebug("At start ReIDString, String %d: m_CurrentId = %d", stringNumber, m_CurrentId );
#endif*/

    for ( it = data.begin(); it != data.end(); it++ )
    {
        if ( (*it)->stringNo == stringNumber )
        {
            if ( bFound && lastNum != stringNumber )
            {
/*#ifdef __WXMSW__
                wxLogDebug("**Non continuous string found: %d!", stringNumber);
#endif*/
            }
/*#ifdef __WXMSW__
            wxLogDebug("Updating ID %d => ID %d", (*it)->id, m_CurrentId );
#endif*/

            (*it)->id = m_CurrentId;
            m_CurrentId += 1;
            bFound = true;
        }
        lastNum = stringNumber;
    }
    return true;
}

/*

  This that this should fix:
  1) Strings scattered across the file
  2) State of feature & contour plot bits on strings
  3)

 */
bool Data::MakeValidState()
{
    iterator it;
    set<unsigned long>::iterator it2;
    iterator curPos;
    set<unsigned long> * stringNos;
    Point *s;
    Point *copyPoint;
    int curStringNo;
    bool stringDone;

    for ( it = data.begin(); it != data.end(); it++ )
    {
        s = *it;
        if ( s->pointNo < 0 || s->stringNo < 0 )    // invalid
        {
            // Delete(s); <-- FIX LATER
            it--;
        }
    }

    stringNos = GetUniqueStringNumbers();

    curPos = data.begin();

    for ( it2 = stringNos->begin(); it2 != stringNos->end(); it2++ )
    {
        curStringNo = *it2;
        stringDone = false;
        for (it = curPos; it != data.end(); it++)
        {
            s = *it;
            if ((s->stringNo != curStringNo && !stringDone)
                    || (s->stringNo == curStringNo && stringDone))
            {
                if (!stringDone)
                {
                    stringDone = true;
                    curPos = it;
                }
                else // move string as out of sequence
                {
                    copyPoint = *it;
                    data.erase(it);
                    it = data.insert(curPos, copyPoint);
                    it++;
                    curPos = it;
                    //data.insert(curPos, *it);
                    //it = data.erase(it);
                    //it--;
                }
            }
        }
    }

    delete stringNos;

    return true;
}

} // namespace stringfile
} // namespace keays