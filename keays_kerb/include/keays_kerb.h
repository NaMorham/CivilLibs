/*!
    \file keays_kerb.h
    \brief Classes and Structures used in managing Keays Software kerb templates.

    \author Andrew Hickey, Keays Software
    \date Thursday, November 25th, 2004
 */

#pragma once

#pragma warning(disable:4786)

#include <keays_types.h>
#include <keays_math.h>
#include <keays_version.h>
#include <vector>
#include <string>
#include <map>
#include <tinyXML.h>

#ifdef KEAYS_KERB_EXPORTS
#define KEAYS_KERB_EXPORTS_API __declspec(dllexport)
#else
#define KEAYS_KERB_EXPORTS_API __declspec(dllimport)
#endif

#ifdef _DEBUG
    #if _MSC_VER < 1300
        #pragma comment( lib, "keays_typesD" )
        #pragma comment( lib, "keays_mathD" )
        #pragma comment( lib, "keays_versionD" )
    #else
        #pragma comment( lib, "keays_types7D" )
        #pragma comment( lib, "keays_math7D" )
        #pragma comment( lib, "keays_version7D" )
    #endif // _MSVC_6_
#else
    #if _MSC_VER < 1300
        #pragma comment( lib, "keays_types")
        #pragma comment( lib, "keays_math")
        #pragma comment( lib, "keays_version" )
    #else
        #pragma comment( lib, "keays_types7")
        #pragma comment( lib, "keays_math7")
        #pragma comment( lib, "keays_version7" )
    #endif // _MSVC_6_
#endif

/*!
    \brief General Keays Software Namespace
    Used to prevent conflicts with existing functions/ variables
 */
namespace keays
{

/*!
    \brief type specific namespace
    Used to prevent conflicts with existing functions/ variables
 */
namespace types
{

/*!
    \brief Maximum length of the name for the kerb.
    \note the buffer for the name should be MAX_KERB_NAME_SIZE+1 to allow for NULL termination.
 */
extern KEAYS_KERB_EXPORTS_API const int MAX_KERB_NAME_SIZE;

//! define a point list
typedef Polyline2D KerbPtsList;//std::list<VectorD2> KerbPtsList;

//-----------------------------------------------------------------------------
/*//#region Local Axes Declaration - moved to keays_types
class KEAYS_KERB_EXPORTS_API LocalAxes
{
public:
    LocalAxes();
    LocalAxes( const keays::types::VectorD3 &i, const keays::types::VectorD3 &j, const keays::types::VectorD3 &k );

    // accessors
    const keays::types::VectorD3 &I() const { return m_i; }
    const keays::types::VectorD3 &J() const { return m_j; }
    const keays::types::VectorD3 &K() const { return m_k; }

    // mutators
    const keays::types::VectorD3 &I( const keays::types::VectorD3 &i );
    const keays::types::VectorD3 &J( const keays::types::VectorD3 &j );
    const keays::types::VectorD3 &K( const keays::types::VectorD3 &k );

    const keays::types::VectorD3 &CalcI();
    const keays::types::VectorD3 &CalcJ();
    const keays::types::VectorD3 &CalcK();

    const LocalAxes &operator=( const LocalAxes &rhs );
    size_t WriteFile( FILE *file ) const;

private:
    keays::types::VectorD3 m_i, m_j, m_k;
};
//#endregion //*/

//-----------------------------------------------------------------------------
//#region 3D Kerb class declaration
/*!
    \brief A Kerb profile in 3D
 */
struct KEAYS_KERB_EXPORTS_API Kerb3D
{
    Kerb3D();
    Kerb3D( const Kerb3D &orig );
    ~Kerb3D();

    const Kerb3D &operator=( const Kerb3D &rhs );

    size_t WriteFile( FILE *file ) const;

    keays::types::Polyline3D    *m_pPts;
    keays::types::LocalAxes        m_plane;
};
//#endregion

//-----------------------------------------------------------------------------
//#region 2D Base Kerb class declaration
/*!
    \brief Base kerb template class.
 */
class KEAYS_KERB_EXPORTS_API Kerb
{
public:

    enum eKerbProfile { KP_FULL, KP_TOP, KP_BOTTOM };

    //! \brief Default constructor.
    Kerb( bool createBasePoints = true );

    Kerb( const Kerb &orig );
    //! \brief Destructor.
    ~Kerb();

    const int DuplicateKerb( const Kerb &src );

    /*!
        \brief Add a point to the Kerb
        The point is added to the end
     */
    void AddPoint( const keays::types::VectorD2 &pt );
    const bool GetPoint( const int index, keays::types::VectorD2 &pt ) const;
    const bool RemovePoint( const int index, keays::types::VectorD2 *pPt );
    const bool ReplacePoint( const int index, const keays::types::VectorD2 &replacementPt, keays::types::VectorD2 *pRemovedPt );

    /*!
        \brief Insert a point after the specifed index
     */
    bool InsertPointAfter( const int index, const keays::types::VectorD2 &pt );

    /*!
        \brief Insert a point before the specifed index
     */
    bool InsertPointBefore( const int index, const keays::types::VectorD2 &pt );


    const int GetNumber() const;
    void SetNumber( const int kerbNum );

    const std::string &GetName() const;
    void SetName( const char *name );
    void SetName( const std::string &name );

    const double TotalWidth() const;
    const double TotalHeight() const;
    const double GetInsideWidth() const;
    const double GetOutsideWidth() const;
    const double GetHeight() const;
    const double GetDepth() const;

    const size_t GetNumPoints() const;

    void RecalcExtents();
    void RemoveDuplicates();

    const bool Write( const char *fileName ) const;
    const bool Write( std::string &output ) const;
    const bool Write( FILE *outFile ) const;

    const bool Read( const char *fileName );
    const bool Read( const std::string &input );
    const bool Read( FILE *inFile );

    const Kerb &operator=( const Kerb &rhs );
    const Kerb &operator+=( const keays::types::VectorD2 &newPoint ) { AddPoint( newPoint ); return *this; }

    const bool operator< ( const Kerb &rhs ) const { return m_kerbNum <  rhs.GetNumber(); }
    const bool operator> ( const Kerb &rhs ) const { return m_kerbNum >  rhs.GetNumber(); }
    const bool operator==( const Kerb &rhs ) const { return m_kerbNum == rhs.GetNumber(); }
    const bool operator<=( const Kerb &rhs ) const { return m_kerbNum <= rhs.GetNumber(); }
    const bool operator>=( const Kerb &rhs ) const { return m_kerbNum >= rhs.GetNumber(); }
    const bool operator!=( const Kerb &rhs ) const { return m_kerbNum != rhs.GetNumber(); }

    /*!
        \brief Get the points in 2D world co-ordinates.
     */
    const keays::types::Polyline2D *GetPoints( Polyline2D *pPoints, const Kerb::eKerbProfile profile,
                                            const bool isLeftSide, keays::math::RectD *pRect = NULL ) const;
    /*!
        \brief Get the points in screen co-ordinates
     */
/*    //#region
    const int GetScreenPoints( const RECT &screenRect, POINT *pts, const int ptsSize,
                    const keays::math::eSideSelections side, const Kerb::eKerbProfile profile = KP_FULL,
                    double *pHRatio = NULL, double *pVRatio = NULL ) const;
*/    //#endregion

    const KerbPtsList &GetPointsRef() const;

    enum eSpecialPoints { SP_TOP_START, SP_TOP_END, SP_BOTTOM_START, SP_BOTTOM_END };
    bool SpecialPointIndex( const eSpecialPoints pt, unsigned int index );
    bool SpecialPointIndex( const unsigned int tsIdx, const unsigned int teIdx,
                            const unsigned int bsIdx, const unsigned int beIdx );
    const unsigned int SpecialPointIndex( const eSpecialPoints pt ) const;
    void SpecialPointIndex( unsigned int *pTsIdx, unsigned int *pTeIdx,
                            unsigned int *pBsIdx, unsigned int *pBeIdx ) const;

protected:
    void RecalcOuterPoint();
    bool m_allowEndPoints;

//private:
    int                m_kerbNum;
    std::string        *m_pName;
    KerbPtsList        *m_pPts;
    keays::math::RectD
                    *m_pExtents;
    bool            m_needsRecalc;
//    keays::Version    *m_pVersion;    //TODO: STOP USING CRAP PLAIN TEXT FORMATS THEN WRITE AN XML PARSER

    union
    {
        struct
        {
            unsigned int    m_topStart;
            unsigned int    m_topEnd;
            unsigned int    m_bottomStart;
            unsigned int    m_bottomEnd;
        };
        unsigned int m_specialPts[4];
    };

    DWORD            m_lineColour;
    DWORD            m_fillColour;
    bool            m_filled;
};
//#endregion

//extern int nKeays_kerb;

//int fnKeays_kerb( void );

//KEAYS_KERB_EXPORTS_API const bool ReadKerbFromFile( FILE *file, Kerb *pKerb );
//const bool ReadKerbFromString( const std::string &str, Kerb *pKerb );

///KEAYS_KERB_EXPORTS_API const bool WriteKerbToFile( FILE *file, const Kerb &kerb );
KEAYS_KERB_EXPORTS_API const bool WriteKerbToString( std::string &string, const Kerb &kerb );

typedef std::map<int, int> IdIndexMap;                // find a kerb by id
typedef std::map<std::string, int> NameIndexMap;    // find a kerb by name
typedef std::vector<Kerb> KerbList;                    // a vector because we do not sort it

//-----------------------------------------------------------------------------
//#region KerbFile class declaration
class KEAYS_KERB_EXPORTS_API KerbFile
{
public:
    KerbFile();
    KerbFile( const char *kerbFileName );
    ~KerbFile();

    int AddKerb( const Kerb &newKerb, bool allowReplace = false, Kerb *replacedKerb = NULL );
    bool RemoveKerbByID( const int id, Kerb *pKerb = NULL );
    bool RemoveKerbByIndex( const int index, Kerb *pKerb = NULL );

    const Kerb *GetKerbByID( const int id, Kerb *pKerb ) const;
    const Kerb *GetKerbByIndex( const int index, Kerb *pKerb ) const;

    const size_t NumKerbs() const { return m_pKerbs->size(); }
private:
    KerbList *m_pKerbs;
    IdIndexMap *m_pKerbsID;

};
//#endregion

};    // namespace types

}; // namespace keays