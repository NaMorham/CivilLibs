// keays_kerb.cpp : Defines the entry point for the DLL application.
//
#pragma warning(disable:4786)

#include <exception>
#include <windows.h>

#include "..\include\keays_kerb.h"

using namespace keays::math;

namespace keays
{

namespace types
{

const int MAX_KERB_NAME_SIZE = 16;

const keays::Version G_CURRENT_KERB_VERSION( 1, 0, 0, 0 );

//-----------------------------------------------------------------------------
//#region global utility functions
inline const int ScreenSizeFromWorld( const double ratio, const double worldSize ) 
{  return int( worldSize * ratio ); }

const bool ReadKerbFromFile( FILE *file, Kerb *pKerb )
{	//#region
//*	//#region
	if( !file || !pKerb )
		return false;

	char buf[256], kerbName[MAX_KERB_NAME_SIZE+1];
	int kerbNum, numPoints, result;

	if( !fgets( buf, 256, file ) )
		return false;

	memset( kerbName, 0, MAX_KERB_NAME_SIZE + 1 );
	kerbNum = 0;
	numPoints = 0;
	result = sscanf( buf, "%d %d %16s", &kerbNum, &numPoints, kerbName );
	if( result < 2 )
	{
		return false; // do not need a name
	}

	if( kerbNum < 1 )
		return false;

	pKerb->SetNumber( kerbNum );
	pKerb->SetName( kerbName );

	std::list<keays::types::VectorD2> points;
	// we now have the data to read
	for( int i = 0; i < numPoints; i++ )
	{
		double x, y;
		if( !fgets( buf, 256, file ) )
			return false;

		result = sscanf( buf, "%lf %lf", &x, &y );
		if( result < 2 )
			return false;

		keays::types::VectorD2 pt;
		pt.x = x;
		pt.y = y;
		points.push_back( pt );
		//pKerb->AddPoint( pt );
	}

	// the files should store rightside designs, so we need to reverse the points
	double area = keays::math::CalcPolygonArea( points );
	if( area < 0 )
	{ 
		// reverse them order
		points.reverse();
	}

	std::list<keays::types::VectorD2>::iterator itr;
	for( itr = points.begin(); itr != points.end(); itr++ )
	{
		pKerb->AddPoint( *itr );
	}

	pKerb->RecalcExtents();

	return true;
	//#endregion
/*/
	return pKerb && pKerb->ReadFromFile( 
//*/
	//#endregion
}

const bool WriteKerbToFile( FILE *file, const Kerb &kerb )
{	//#region
	if( !file )
		return false;

	int kerbNum = kerb.GetNumber();
	if( kerbNum < 1 )
		return false;
	int numPts = (int)kerb.GetNumPoints();
	if( numPts < 2 )
		return false;

	// write the information
	fprintf( file, "%5d%5d %16.16s\n", kerbNum, numPts, kerb.GetName().c_str() );
	for( int j = 0; j < numPts; j++ )
	{
		// write the points
		keays::types::VectorD2 pt;
		kerb.GetPoint( j, pt );
		fprintf( file, "%10.3f%10.3f\n", pt.x, pt.y );
	}

	return true;
	//#endregion
}

const bool WriteKerbToString( std::string &result, const Kerb &kerb )
{	//#region
	int kerbNum = kerb.GetNumber();
	if( kerbNum < 1 )
		return false;
	int numPts = (int)kerb.GetNumPoints();
	if( numPts < 2 )
		return false;

	result = "";

	// write the information
	char buf[64];
	_snprintf( buf, 64, "%5d%5d %16.16s\n", kerbNum, numPts, kerb.GetName().c_str() );
	result = buf;
	for( int j = 0; j < numPts; j++ )
	{
		// write the points
		keays::types::VectorD2 pt;
		kerb.GetPoint( j, pt );
		_snprintf( buf, 64, "%10.3f%10.3f\n", pt.x, pt.y );
		result = result + buf;
	}

	return true;
	//#endregion
}
//#endregion

//-----------------------------------------------------------------------------
/* //#region Local Axes implementation - moved to keays types
LocalAxes::LocalAxes()
{
	m_i.Set( 1, 0, 0 );
	m_j.Set( 0, 1, 0 );
	m_k.Set( 0, 0, 1 );
}

LocalAxes::LocalAxes( const keays::types::VectorD3 &i, const keays::types::VectorD3 &j, const keays::types::VectorD3 &k )
{
	m_i = i;
	m_j = j;
	m_k = k;
}

// mutators
const keays::types::VectorD3 &LocalAxes::I( const keays::types::VectorD3 &i )
{
	return m_i;
}

const keays::types::VectorD3 &LocalAxes::J( const keays::types::VectorD3 &j )
{
	return m_j;
}

const keays::types::VectorD3 &LocalAxes::K( const keays::types::VectorD3 &k )
{
	return m_k;
}

const keays::types::VectorD3 &LocalAxes::CalcI()
{
	return m_i;
}

const keays::types::VectorD3 &LocalAxes::CalcJ()
{
	return m_j;
}

const keays::types::VectorD3 &LocalAxes::CalcK()
{
	return m_k;
}

const LocalAxes &LocalAxes::operator=( const LocalAxes &rhs )
{
	m_i = rhs.m_i;
	m_j = rhs.m_j;
	m_k = rhs.m_k;

	return *this;
}

size_t LocalAxes::WriteFile( FILE *file ) const
{
	double temp;
	size_t bytes = 0;

	if( !file )
		return 0;

	temp = m_i.x;
	bytes += fwrite( &temp, sizeof( temp ), 1, file );
	temp = m_i.y;
	bytes += fwrite( &temp, sizeof( temp ), 1, file );
	temp = m_i.z;
	bytes += fwrite( &temp, sizeof( temp ), 1, file );

	temp = m_j.x;
	bytes += fwrite( &temp, sizeof( temp ), 1, file );
	temp = m_j.y;
	bytes += fwrite( &temp, sizeof( temp ), 1, file );
	temp = m_j.z;
	bytes += fwrite( &temp, sizeof( temp ), 1, file );

	temp = m_k.x;
	bytes += fwrite( &temp, sizeof( temp ), 1, file );
	temp = m_k.y;
	bytes += fwrite( &temp, sizeof( temp ), 1, file );
	temp = m_k.z;
	bytes += fwrite( &temp, sizeof( temp ), 1, file );

	return bytes;
}
//#endregion	//*/

//-----------------------------------------------------------------------------
//#region 3D Kerb class implementation
Kerb3D::Kerb3D() 
{	//#region
	OutputDebugString( "Kerb3D - Default Constructor BEGIN\n" );
	m_pPts = new keays::types::Polyline3D; 
	//#endregion
	OutputDebugString( "Kerb3D - Default Constructor END\n" );
}

Kerb3D::Kerb3D( const Kerb3D &orig )
{	//#region
	OutputDebugString( "Kerb3D - Copy Constructor BEGIN\n" );
//	Kerb3D();
	m_pPts = new keays::types::Polyline3D;

	Polyline3D::const_iterator itr;
	
	m_plane = orig.m_plane;
	m_pPts->clear();
	for( itr = orig.m_pPts->begin(); itr != orig.m_pPts->end(); itr++ )
		m_pPts->push_back( *itr );
	//#endregion
	OutputDebugString( "Kerb3D - Copy Constructor END\n" );
}

Kerb3D::~Kerb3D() 
{	//#region
	OutputDebugString( "Kerb3D - Destructor BEGIN\n" );
	m_pPts->clear();
	if( m_pPts )
		delete m_pPts; 
	m_pPts = NULL; 
	//#endregion
	OutputDebugString( "Kerb3D - Destructor END\n" );
}

const Kerb3D &Kerb3D::operator=( const Kerb3D &rhs )
{	//#region
	OutputDebugString( "Kerb3D - operator= BEGIN\n" );
	Polyline3D::const_iterator itr;
	
	m_plane = rhs.m_plane;
	m_pPts->clear();
	for( itr = rhs.m_pPts->begin(); itr != rhs.m_pPts->end(); itr++ )
		m_pPts->push_back( *itr );

	OutputDebugString( "Kerb3D - operator= END\n" );
	return *this;
	//#endregion
}

size_t Kerb3D::WriteFile( FILE *file ) const
{//#region
	double temp;
	size_t bytes = 0;

	if( !file )
		return 0;

	keays::types::VectorD3 axis( m_plane.I() );
	temp = axis.x;
	bytes += fwrite( &temp, sizeof( temp ), 1, file );
	temp = axis.y;
	bytes += fwrite( &temp, sizeof( temp ), 1, file );
	temp = axis.z;
	bytes += fwrite( &temp, sizeof( temp ), 1, file );

	axis = m_plane.J();
	temp = axis.x;
	bytes += fwrite( &temp, sizeof( temp ), 1, file );
	temp = axis.y;
	bytes += fwrite( &temp, sizeof( temp ), 1, file );
	temp = axis.z;
	bytes += fwrite( &temp, sizeof( temp ), 1, file );

	axis = m_plane.K();
	temp = axis.x;
	bytes += fwrite( &temp, sizeof( temp ), 1, file );
	temp = axis.y;
	bytes += fwrite( &temp, sizeof( temp ), 1, file );
	temp = axis.z;
	bytes += fwrite( &temp, sizeof( temp ), 1, file );

//	bytes = m_plane.WriteFile( file );

	// now write the points
	Polyline3D::const_iterator itr;
	for( itr = m_pPts->begin(); itr != m_pPts->end(); itr++ )
	{
		temp = (*itr).x;
		bytes += fwrite( &temp, sizeof( double ), 1, file );
		temp = (*itr).y;
		bytes += fwrite( &temp, sizeof( double ), 1, file );
		temp = (*itr).z;
		bytes += fwrite( &temp, sizeof( double ), 1, file );
	}

	return bytes;
	//#endregion
}
//#endregion

//-----------------------------------------------------------------------------
//#region Base Kerb class implementation
// This is the constructor of a class that has been exported.
// see keays_kerb.h for the class definition
Kerb::Kerb( bool createBasePoints /*=true*/ )
{	//#region
	m_pPts = new KerbPtsList;
	m_pExtents = new keays::math::RectD;
	m_pName = new std::string;
//	m_pVersion = new keays::Version( G_CURRENT_KERB_VERSION );

	m_pPts->clear();
	m_pName->erase();
	m_kerbNum = -1;
	m_needsRecalc = true;

	m_allowEndPoints = createBasePoints;
	if( m_allowEndPoints )
	{
		m_pPts->push_back( keays::types::VectorD2( 0, 0 ) ); // the first and last points must be 0,0
		m_pPts->push_back( keays::types::VectorD2( 0, 0 ) );

#ifdef _DEBUG
		char buf[512];
		memset( buf, 0, 512 );
		_snprintf( buf, 511, "Kerb::Kerb( %s ) - numPoints = %d\n", (m_allowEndPoints ? "true" : "false"), m_pPts->size() );
		OutputDebugString( buf );
#endif
	}
	//#endregion
}

Kerb::Kerb( const Kerb &orig )
{	//#region
	OutputDebugString( "Kerb:: Kerb( const kerb & )\n" );
	// create
	m_pExtents = new keays::math::RectD;
	m_pPts = new KerbPtsList;
	m_pName = new std::string;
//	m_pVersion = new keays::Version;

	// and copy
	DuplicateKerb( orig );
	//#endregion
}

const int Kerb::DuplicateKerb( const Kerb &src )
{	//#region
	OutputDebugString( "Kerb:: DuplicateKerb(...)\n" );

	//(*m_pVersion) = src.Version();
	m_kerbNum = src.GetNumber();
	(*m_pName) = src.GetName();
	m_pPts->clear();
	m_allowEndPoints = src.m_allowEndPoints;

	KerbPtsList::const_iterator itr;
	const KerbPtsList &otherPts = src.GetPointsRef();
	for( itr = otherPts.begin(); itr != otherPts.end(); itr++ )
	{
		m_pPts->push_back( *itr );
	}

	RecalcExtents();

	return 0;
	//#endregion
}

const Kerb &Kerb::operator=( const Kerb &rhs )
{	//#region
	DuplicateKerb( rhs );

	return *this;
	//#endregion
}

const KerbPtsList &Kerb::GetPointsRef() const
{	//#region
	return *m_pPts;
	//#endregion
}

Kerb::~Kerb()
{	//#region
	delete m_pPts;				m_pPts = NULL;
	delete m_pExtents;			m_pExtents = NULL;
	delete m_pName;				m_pName = NULL;
//	delete m_pVersion;			m_pVersion = NULL;
	//#endregion
}

void Kerb::AddPoint( const keays::types::VectorD2 &pt )
{	//#region
	if( m_pPts->size() < 1 )
	{
//		*m_pOuterPointItr = m_pPts->begin();
	}

	// the recalc takes care of the outer point if it is not the first point
	m_pPts->push_back( pt );

	m_needsRecalc = true;
	//#endregion
}

const bool Kerb::GetPoint( const int index, keays::types::VectorD2 &pt ) const
{	//#region
	if( ( index < 0 ) || ( index >= (int)GetNumPoints() ) )
		return false;

	pt = (*m_pPts)[ index ];

	return true;
	//#endregion
}

const bool Kerb::RemovePoint( const int index, keays::types::VectorD2 *pPt )
{	//#region
	// test to ensure we are withing the appropriate range, 
	// NOTE: YOU CANNOT REMOVE THE FIRST POINT, to do this you should remove the whole kerb
	if( ( index < 0 ) || ( index >= (int)GetNumPoints() ) )
		return false;

	// we need to move the data for each point after up by one index,
	// this would be alot faster if we use a list, but the convienience 
	// of random access is much more useful
	int last = (int)m_pPts->size() - 1;
	if( last != index )
	{
		// we are not removing the last point so we need to adjust
		int i, j;
		//last--; // stop 1 before the last
		for( i = index, j = i+1; i < last; i++, j++ )
		{
			(*m_pPts)[i] = (*m_pPts)[j];
		}
	}
	m_pPts->pop_back();

	m_needsRecalc = true;

	return true;
	//#endregion
}

bool Kerb::InsertPointBefore( const int index, const keays::types::VectorD2 &pt )
{	//#region
	// test to ensure we are withing the appropriate range, 
	// NOTE: YOU CANNOT INSERT BEFORE THE FIRST POINT, it is ALWAYS at 0,0 with an index of 0
	if( ( index <= 0 ) || ( index >= (int)GetNumPoints() ) )
		return false;

	// duplicate the last point
	int last = (int)m_pPts->size()-1;
	m_pPts->push_back( m_pPts->at( last ) );

	// starting from the end and working backwards
	int i, j;
	for( i = last, j = i-1; i > index; j--, i-- )
	{
		(*m_pPts)[i] = (*m_pPts)[j];
	}
	(*m_pPts)[index] = pt;

	m_needsRecalc = true;

	return true;
	//#endregion
}

bool Kerb::InsertPointAfter( const int index, const keays::types::VectorD2 &pt )
{	//#region
	// test to ensure we are withing the appropriate range, 
	// NOTE: if you specify a point beyond the end of the pts, it will add onto the end
	if( index < 0 )
		return false;

	m_needsRecalc = true;

	int last = (int)m_pPts->size()-1; // the index of the last point
	// duplicate...
	m_pPts->push_back( m_pPts->at( last ) );
	if( index >= last )
	{
		// we are adding to the end
		AddPoint( pt );
		return true;
	} else
	{
		// we are adding in the middle, so we need to duplicate the last point, and shift the others

		// ...now shift from the insert point
		int i, j;
		for( i = last, j = i-1; i >= index+1; i--, j-- )
		{
			(*m_pPts)[i] = (*m_pPts)[j];
		}
	}
	(*m_pPts)[index+1] = pt;

	return true;
	//#endregion
}

const bool Kerb::ReplacePoint( const int index, const keays::types::VectorD2 &replacementPt, keays::types::VectorD2 *pRemovedPt )
{	//#region
	if( ( index < 0 ) || ( index > (int)GetNumPoints() ) )
		return false;

	if( pRemovedPt )
		*pRemovedPt = (*m_pPts)[index];

	(*m_pPts)[index] = replacementPt;

	m_needsRecalc = true;

	return true;
	//#endregion
}

const int Kerb::GetNumber() const
{	//#region
	return m_kerbNum;
	//#endregion
}

void Kerb::SetNumber( const int kerbNum )
{	//#region
	m_kerbNum = kerbNum;
	//#endregion
}

const std::string &Kerb::GetName() const
{	//#region
	return (*m_pName);
	//#endregion
}

void Kerb::SetName( const char *name )
{	//#region
	m_pName->assign( name );
	//#endregion
}

void Kerb::SetName( const std::string &name )
{	//#region
	(*m_pName) = name;
	//#endregion
}

const size_t Kerb::GetNumPoints() const
{	//#region
	return m_pPts->size();
	//#endregion
}

const double Kerb::TotalWidth() const
{	//#region
	if( m_needsRecalc )
		return 0.0;
//		RecalcExtents();

	if( m_pPts->size() )
		return m_pExtents->GetWidth();
	else 
		return 0.0;
	//#endregion
}

const double Kerb::TotalHeight() const
{	//#region
	if( m_needsRecalc )
		return 0.0;
//		RecalcExtents();

	if( m_pPts->size() )
		return m_pExtents->GetHeight();
	else 
		return 0.0;
	//#endregion
}

// these are simple since the model is ALWAYS built around [0,0]
const double Kerb::GetInsideWidth() const
{	//#region
	if( m_needsRecalc )
		return 0.0;
//		RecalcExtents();

	if( m_pPts->size() )
		return m_pExtents->m_left;
	else 
		return 0.0;
	//#endregion
}
const double Kerb::GetOutsideWidth() const
{	//#region
	if( m_needsRecalc )
		return 0.0;
//		RecalcExtents();

	if( m_pPts->size() )
		return m_pExtents->m_right;
	else 
		return 0.0;
	//#endregion
}
const double Kerb::GetHeight() const
{	//#region
	if( m_needsRecalc )
		return 0.0;
//		RecalcExtents();

	if( m_pPts->size() )
		return m_pExtents->m_top;
	else 
		return 0.0;
	//#endregion
}
const double Kerb::GetDepth() const
{	//#region
	if( m_needsRecalc )
		return 0.0;
//		RecalcExtents();

	if( m_pPts->size() )
		return m_pExtents->m_bottom;
	else 
		return 0.0;
	//#endregion
}

/*	//#region
const int Kerb::GetScreenPoints( const RECT &screenRect, POINT *pts, const int ptsSize, 
								 const keays::math::eSideSelections side, const Kerb::eKerbProfile profile, 
								 double *pHRatio, double *pVRatio ) const
{	//#region
	if( m_needsRecalc ) // added due to paranoia
		return 0;
//		RecalcExtents();

#ifdef _DEBUG
	char buf[256];
#endif

	double width, height, hRatio, vRatio;
	int ptCount, scnWidth, scnHeight, scnHOffset, scnVOffset;
	int ptIdx, numPts;

	Polyline2D kerbPts;
	//keays::math::RectD rectD;
	if( !GetPoints( &kerbPts, profile, side == keays::math::SIDE_LEFT, NULL ) ) //&rectD ) )
	{
		OutputDebugString ( "Kerb::GetScreenPoints(...) - FAILED - Unable to get points\n" );
		return 0;
	}

	numPts = (int)kerbPts.size();
#ifdef _DEBUG
	_snprintf( buf, 256, "GetPoints returned %d points\n", numPts );
	OutputDebugString( buf );
#endif

//	width = rectD.GetWidth();
	width = m_pExtents->GetWidth();
//	height = rectD.GetHeight();
	height = m_pExtents->GetHeight();
	ptCount = 0;
	scnWidth = screenRect.right - screenRect.left;
	scnHeight = screenRect.bottom - screenRect.top;

	if( width <= 0.0 )
	{
		width = 100.0;
	}
	if( height <= 0.0 )
	{
		height = 100.0;
	}

#if 0
  // do the setup
	if( side == keays::math::SIDE_LEFT )
	{
		// work out our scaling ratio
		hRatio = double( scnWidth ) / -width; // all the layouts are for right side in direction of travel, so flip for left side
		vRatio = double( scnHeight ) / height;
		scnHOffset = ScreenSizeFromWorld( hRatio, GetInsideWidth() );
	} else if ( side == keays::math::SIDE_RIGHT )
	{
		hRatio = double( scnWidth ) / width;
		vRatio = double( scnHeight ) / height;
		scnHOffset = ScreenSizeFromWorld( hRatio, GetOutsideWidth() );
	} else
	{
		OutputDebugString( "Kerb::GetScreenPoints(...) - FAILED - invalid side\n" );
		return 0;
	}
#endif

	hRatio = double( scnWidth ) / width;
	vRatio = double( scnHeight ) / height;
//	scnHOffset = ScreenSizeFromWorld( hRatio, rectD.GetLeft() );
	if( side == keays::math::SIDE_LEFT )
	{
		scnHOffset = ScreenSizeFromWorld( hRatio, m_pExtents->GetRight() ) * -1;
	} else
	{
		scnHOffset = ScreenSizeFromWorld( hRatio, m_pExtents->GetLeft() );
	}

//	scnVOffset = ScreenSizeFromWorld( vRatio, rectD.GetTop() );
	scnVOffset = ScreenSizeFromWorld( vRatio, m_pExtents->GetTop() );

	if( pHRatio ) *pHRatio = hRatio;
	if( pVRatio ) *pVRatio = vRatio;

	ptCount = 0;
#if 0
	for( itr = m_pPts->begin(), ptIdx = 0; itr != m_pPts->end(), ptIdx < ptsSize; 
		 itr++, ptIdx++ )
	{
		// process the points
		const keays::types::VectorD2 &pt = (*itr);
		pts[ptIdx].x = ScreenSizeFromWorld( hRatio, pt.x ) - scnHOffset;
//		pts[ptIdx].x = scnWidth - ScreenSizeFromWorld( hRatio, pt.x );
		pts[ptIdx].y = scnVOffset + ScreenSizeFromWorld( vRatio, -pt.y ); // stupid screen co-ords
		ptCount++;
	}
#endif

	Polyline2D::iterator itr;
	OutputDebugString( "Copy to result vector\n" );
	for( ptIdx = 0, itr = kerbPts.begin(); itr != kerbPts.end(); itr++, ptIdx++ )
	{
		if( ptIdx >= ptsSize )
		{
			OutputDebugString( "About to go out of bounds - exiting\n" );
			break;
		}
		const keays::types::VectorD2 &pt = *itr;

		pts[ptIdx].x = ScreenSizeFromWorld( hRatio, pt.x ) - scnHOffset;
		pts[ptIdx].y = scnVOffset + ScreenSizeFromWorld( vRatio, -pt.y ); // stupid screen co-ords

#ifdef _DEBUG
		_snprintf( buf, 256, "PT( %d ) [%d, %d]\n", ptIdx, pts[ptIdx].x, pts[ptIdx].y );
		OutputDebugString( buf );
#endif
		ptCount++;
	}
#ifdef _DEBUG
	sprintf( buf, "Kerb::GetScreenPoints(...) - SUCCESS - %d points\n", ptCount );
	OutputDebugString( buf );
#endif

	return ptCount;
	//#endregion
}
//#endregion	//*/

// assumes that the points are wound in the right direction
const keays::types::Polyline2D *
Kerb::GetPoints( keays::types::Polyline2D *pPoints, const Kerb::eKerbProfile profile,
				 const bool isLeftSide, keays::math::RectD *pRect /*= NULL*/ ) const
{	//#region
#ifdef _DEBUG
	char buf[ 256 ];
#endif

	if( !pPoints )
	{
		OutputDebugString( "Kerb::GetPoints(...) - Returning NULL, NULL pointer passed as destination\n" );
		return NULL;
	}
	size_t numPoints = m_pPts->size();
	if( numPoints < 1 )
	{
		OutputDebugString( "Kerb::GetPoints(...) - Returning NULL, no points to get\n" );
		return NULL;
	}
	if( m_needsRecalc )
	{
		OutputDebugString( "Kerb::GetPoints(...) - Returning NULL, Not calculated\n" );
		return NULL;
	}

	pPoints->clear();
	if( pRect )
		pRect->MakeInvalid();
	size_t i;

//	TOP SURFACE WHEN DOING RIGHT SIDE IS FROM 
	int interval = 1;
	int start = 0;
	int end = 0;

	std::list<keays::types::VectorD2> ptsList;

	if( profile == KP_BOTTOM )
	{	//#region
		
		//#endregion
	} else if( profile == KP_TOP )
	{	//#region

		//#endregion
	} else	// the full outline
	{	//#region
		for( i = 0; i < numPoints; i++ )
		{
#ifdef _DEBUG
			_snprintf( buf, 256, "Gen Points for all: pt( %d/%d ) [%7.3f, %7.3f]\n",
								i, numPoints, (*m_pPts)[i].x, (*m_pPts)[i].y );
			OutputDebugString( buf );
#endif
			ptsList.push_back( (*m_pPts)[i] );
		}
		//#endregion
	}
/* //#region old version
		//RecalcExtents();
	if( ( m_outerPointIndex < 0 ) || ( m_outerPointIndex >= numPoints ) )
	{
		OutputDebugString( "Kerb::GetPoints(...) - Returning NULL, outer point out of range\n" );
		//RecalcExtents();
		return NULL;
	}


	//	NOTE: I am using (*m_pPts)[] since it is much faster but not as safe as using an 
	//	iterator or the at() function, the safety checks should be taken care of at the start.
	if( profile == KP_BOTTOM )
	{	//#region
//		ptsList.push_back( m_basePoint ); // add the base point to the start of the list
		for( i = 0; i <= m_outerPointIndex; i++ )
		{
#ifdef _DEBUG
			_snprintf( buf, 256, "Gen Points on bottom: pt( %d/%d ) [%7.3f, %7.3f]\n",
								i, m_outerPointIndex, (*m_pPts)[i].x, (*m_pPts)[i].y );
			OutputDebugString( buf );
#endif
			ptsList.push_back( (*m_pPts)[i] );
		}
		//#endregion
	} else if( profile == KP_TOP )
	{	//#region
		for( i = m_outerPointIndex; i < numPoints; i++ )
		{
#ifdef _DEBUG
			_snprintf( buf, 256, "Gen Points on top: pt( %d/%d ) [%7.3f, %7.3f]\n",
								i, numPoints - m_outerPointIndex, (*m_pPts)[i].x, (*m_pPts)[i].y );
			OutputDebugString( buf );
#endif
			ptsList.push_back( (*m_pPts)[i] );
		}
		//ptsList.push_back( m_basePoint ); // add the base point to the end of the list
		//#endregion
	} else
	{	//#region
		//ptsList.push_back( m_basePoint ); // add the base point to the start of the list
		for( i = 0; i < numPoints; i++ )
		{
#ifdef _DEBUG
			_snprintf( buf, 256, "Gen Points for all: pt( %d/%d ) [%7.3f, %7.3f]\n",
								i, numPoints, (*m_pPts)[i].x, (*m_pPts)[i].y );
			OutputDebugString( buf );
#endif
			ptsList.push_back( (*m_pPts)[i] );
		}
		//ptsList.push_back( m_basePoint ); // add the base point to the end of the list
		//#endregion
	}
//#endregion	//*/

	// we reverse the order if we are doing the left side
	if( isLeftSide )
	{
		ptsList.reverse();
	}

	std::list<keays::types::VectorD2>::iterator itr;
	for( itr = ptsList.begin(); itr != ptsList.end(); itr++ )
	{
		keays::types::VectorD2 pt = (*itr);
		if( isLeftSide )
			pt.x = pt.x * -1;
		if( pRect )
			pRect->IncludePoint( pt );
		pPoints->push_back( pt );
	}

	return pPoints;
	//#endregion
}

void Kerb::RecalcExtents()
{	//#region
	int i = 0, j = 0;
	m_pExtents->MakeInvalid();
	m_pExtents->IncludePoint( 0, 0 );
	m_needsRecalc = false;
	if( m_pPts->size() < 1 )
		return;

	KerbPtsList::const_iterator itr;
	for( itr = m_pPts->begin(); itr != m_pPts->end(); itr++ )
	{
		m_pExtents->IncludePoint( *itr );
	}

	if( CalcPolygonArea( *m_pPts ) < 0.0 )
	{
		int numPts = (int)m_pPts->size();
		int lastPtIdx = numPts - 1;

		// it needs to have winding reversed
		for( i = 0, j = lastPtIdx; ( i < numPts/2 ) && ( i < j ); i++, j-- )
		{
			VectorD2 pt;
			pt = (*m_pPts)[i];
			(*m_pPts)[i] = (*m_pPts)[j];
			(*m_pPts)[j] = pt;
		}
	}
	RemoveDuplicates();

	VectorD2 minPt, maxPt;
	minPt = maxPt = VectorD2( 0, 0 );
	for( i = 0, itr = m_pPts->begin(); itr != m_pPts->end(); itr++, i++	)
	{
		const VectorD2 &pt = (*itr);

		if( ( pt.x > maxPt.x ) || ( ( pt.x == maxPt.x ) && ( pt.y >= maxPt.y ) ) )
		{
			m_topEnd = m_bottomEnd = i;
			maxPt = pt;
		}
		if( ( pt.x < minPt.x ) || ( ( pt.x == minPt.x ) && ( pt.y >= minPt.y ) ) )
		{
			m_topStart = m_bottomStart = i;
			minPt = pt;
		}
	}
	//#endregion
}

void Kerb::RemoveDuplicates()
{	//#region
	if( m_pPts->size() > 2 )
	{
		KerbPtsList::iterator pt, nextPt;
		pt = m_pPts->begin();
		nextPt = pt; nextPt++;

		for( ; pt != m_pPts->end() && nextPt != m_pPts->end(); pt++, nextPt++ )
		{
			while( ( pt->x == nextPt->x ) && ( pt->y == nextPt->y ) && ( pt != m_pPts->end() ) && ( nextPt != m_pPts->end() ) )
			{
				nextPt = m_pPts->erase( nextPt );
				//nextPt = pt;
				//nextPt++;
			}
		}
	}

	RecalcOuterPoint();
	//#endregion
}

void Kerb::RecalcOuterPoint()
{	//#region
	KerbPtsList::iterator itr;

	// reset to the base point
	if( m_pPts->size() < 1 )
	{
		return;
	} else 
	{
	}

	// now test them all
	/*
		the outer point is the highest point, if there are 2 of equal height, 
		then it is is the outermost of those points
	 */
	int i;
	for( i = 0, itr = m_pPts->begin(); itr != m_pPts->end(); itr++, i++ )
	{
	}
	//#endregion
}

const bool Kerb::Write( const char *fileName ) const
{ 	//#region
	FILE *file = fopen( fileName, "w" );
	return ( file && Write( file ) );
	//#endregion
}

const bool Kerb::Write( FILE *outFile ) const
{	//#region
	if( !outFile )
		return false;

	int kerbNum = GetNumber();
	if( kerbNum < 1 )
		return false;
	int numPts = (int)GetNumPoints();
	if( numPts < 2 )
		return false;

	// write the information
	fprintf( outFile, "%5d%5d %16.16s %d %d %d %d %08x %08x\n", 
			 kerbNum, numPts, GetName().c_str(), 
			 m_topStart, m_topEnd, m_bottomStart, m_bottomEnd,
			 m_lineColour, m_fillColour );
	KerbPtsList::const_iterator itr;
	keays::types::VectorD2 lastPoint( INVALID_ANGLE, INVALID_ANGLE );
	for( itr = m_pPts->begin(); itr != m_pPts->end(); itr++ )
	{
		// write the points
		const keays::types::VectorD2 &pt = (*itr);
		if( pt != lastPoint )
			fprintf( outFile, "%10.3f%10.3f\n", pt.x, pt.y );
		lastPoint = pt;
	}

	return true;
	//#endregion
}

const bool Kerb::Read( const char *fileName )
{	//#region
	FILE *file = fopen( fileName, "r" );
	return ( file && Read( file ) );
	//#endregion
}

const bool Kerb::Read( FILE *inFile )
{	//#region
	if( !inFile )
		return false;

	char buf[256], kerbName[MAX_KERB_NAME_SIZE+1];
	int kerbNum, numPoints, result;

	if( !fgets( buf, 256, inFile ) )
		return false;

	memset( kerbName, 0, MAX_KERB_NAME_SIZE + 1 );
	kerbNum = 0;
	numPoints = 0;
	unsigned int	topStart, topEnd, bottomStart, bottomEnd;
	DWORD			outlineColour = 0xffff0000, 
					fillColour = 0x00000000;

	result = sscanf( buf, "%d %d %s %u %u %u %u %x %x",		//"%d %d %*.*s %u %u %u %u %x %x", 
					 &kerbNum, &numPoints, kerbName,		//MAX_KERB_NAME_SIZE, MAX_KERB_NAME_SIZE, kerbName,
					 &topStart, &topEnd, &bottomStart, &bottomEnd, 
					 &outlineColour, &fillColour ); // these last 2 leave me cold :(
	if( result < 2 )
	{
		return false; // do not need a name
	}

//	if( kerbNum < 1 )
//		return false;

	m_pPts->clear();

	SetNumber( kerbNum );
	SetName( kerbName );

	std::list<keays::types::VectorD2> points, allPoints;
	// we now have the data to read
//	if( !fgets( buf, 256, inFile ) )
//		return false;
	double x, y;
	keays::types::VectorD2 pt;

	for( int i = 0; i < numPoints; i++ )
	{
		if( !fgets( buf, 256, inFile ) )
			return false;

		result = sscanf( buf, "%lf %lf", &x, &y );
		if( result < 2 )
			return false;

		pt.x = x;
		pt.y = y;
		points.push_back( pt );
	}
/*
	if( !fgets( buf, 256, inFile ) )
		return false;
	result = sscanf( buf, "%lf %lf", &x, &y );
	if( result < 2 )
		return false;
	pt.x = x;
	pt.y = y;
	allPoints.push_back( pt );
//*/

	// the files should store rightside designs, so we need to reverse the points
	double area = keays::math::CalcPolygonArea( points );
	if( area < 0 )
	{ 
		// reverse them order
		points.reverse();
	}

	std::list<keays::types::VectorD2>::iterator itr;
	for( itr = points.begin(); itr != points.end(); itr++ ) 
	{
		AddPoint( *itr );
	}

	RecalcExtents();

	return true;
	//#endregion
}

//enum eSpecialPoints { SP_TOP_START, SP_TOP_END, SP_BOTTOM_START, SP_BOTTOM_END };
bool Kerb::SpecialPointIndex( const eSpecialPoints pt, unsigned int index )
{
	if( ( index < 0 ) || ( index >= m_pPts->size() ) )
		return false;
	m_specialPts[pt] = index;
	return true;
}

bool Kerb::SpecialPointIndex( const unsigned int tsIdx, const unsigned int teIdx, 
						const unsigned int bsIdx, const unsigned int beIdx )
{
	bool result = true;
	const size_t numPts = m_pPts->size();
	if( ( tsIdx < 0 ) || ( tsIdx >= numPts ) )
		result &= false;
	else 
		m_topStart = tsIdx;

	if( ( teIdx < 0 ) || ( teIdx >= numPts ) )
		result &= false;
	else 
		m_topEnd = teIdx;

	if( ( bsIdx < 0 ) || ( bsIdx >= numPts ) )
		result &= false;
	else 
		m_bottomStart = bsIdx;
	
	if( ( beIdx < 0 ) || ( beIdx >= numPts ) )
		result &= false;
	else
		m_bottomEnd = teIdx;

	return result;
}

const unsigned int Kerb::SpecialPointIndex( const eSpecialPoints pt ) const
{
	return ( pt >= SP_TOP_START && pt < SP_BOTTOM_END ? m_specialPts[pt] : -1 );
}

void Kerb::SpecialPointIndex( unsigned int *pTsIdx, unsigned int *pTeIdx, 
							  unsigned int *pBsIdx, unsigned int *pBeIdx ) const
{
	if( pTsIdx )
		*pTsIdx = m_topStart;
	if( pTeIdx )
		*pTeIdx = m_topEnd;
	if( pBsIdx )
		*pBsIdx = m_bottomStart;
	if( pBeIdx )
		*pBeIdx = m_bottomEnd;
}

//#endregion

//------------------------------------------------------------------------
//#region Kerb File Implementation
KerbFile::KerbFile()
{	//#region
	m_pKerbs = new KerbList;
	m_pKerbsID = new IdIndexMap;
	//#endregion
}
KerbFile::KerbFile( const char *kerbFileName )
{
	m_pKerbs = new KerbList;
	m_pKerbsID = new IdIndexMap;
}
KerbFile::~KerbFile()
{
	delete m_pKerbs;
	m_pKerbs = NULL;

	delete m_pKerbsID;
	m_pKerbsID = NULL;

}

int KerbFile::AddKerb( const Kerb &newKerb, bool allowReplace /*= false*/, Kerb *replacedKerb /*= NULL*/ )
{
/*
	if( m_pKerbsID->find( newKerb.GetNumber() ) == m_pKerbsID->end() )
	{
		if( !allowReplace )
			return -1;
		// remove it
	}

	// we add it to the vector...
	m_pKerbs->push_back( newKerb );
	int index = (int)m_pKerbs->size()-1;

	// ...and set set up the mapping
	std::pair<int, int> idIndexPair( newKerb.GetNumber(), index );
	m_pKerbsID->insert( idIndexPair );

	return index;
//*/
	return 0;
}

bool KerbFile::RemoveKerbByID( const int id, Kerb *pKerb /*= NULL*/ )
{
	bool found = false;
/*

	KerbListItr itr;
	for( itr = m_pKerbs->begin(); itr != m_pKerbs->end(); itr++ )
	{
		Kerb &k = (*itr);
		// test if we've found it
		if( k.GetNumber() == id )
		{
			// mark, and copy if required
			found = true;
			if( pKerb )
				*pKerb = k;

			// remove from the map...
			IdIndexMapItr mItr = m_pKerbsID->find( k.GetNumber() );
			if( mItr != m_pKerbsID->end() )
				m_pKerbsID->erase( mItr );
			// ... and the vector
			m_pKerbs->erase( itr );
			break;
		}
	}
//*/	
	return found;
}

bool KerbFile::RemoveKerbByIndex( const int index, Kerb *pKerb /*= NULL*/ )
{
/*
	if( index >= (int)m_pKerbs->size() )
		return false;

	int count = 0;

	KerbListItr itr;
	for( itr = m_pKerbs->begin(); itr != m_pKerbs->end(); itr++ )
	{
		// test if we've found it
		if( count == index )
		{
			Kerb &k = (*itr);
			// copy if required
			if( pKerb )
				*pKerb = k;

			// remove from the map...
			IdIndexMapItr mItr = m_pKerbsID->find( k.GetNumber() );
			if( mItr != m_pKerbsID->end() )
				m_pKerbsID->erase( mItr );
			// ... and the vector
			m_pKerbs->erase( itr );
			break;
		}
	}
//*/

	return true;
}

const Kerb *KerbFile::GetKerbByID( const int id, Kerb *pKerb ) const
{
/*
	if( !pKerb )
		return NULL;

	// we have an id, so use the map
	IdIndexMapItr itr;
	itr = m_pKerbsID->find( id );
	if( itr == m_pKerbsID->end() )
		return NULL;

	
//*/
	return pKerb;
}
const Kerb *KerbFile::GetKerbByIndex( const int index, Kerb *pKerb ) const
{
	return NULL;
}
//#endregion

};	// namespace types

}; // namespace keays