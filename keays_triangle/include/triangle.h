/*! 
	\file 	ksrxfile.h
	Keays Triangle Data types adapted from ksrxfile.h from the Keays Simulation & Rendering Extensions API 
	file & I/O header.

	\author Julian McKinlay
	\author Karl Janke
	\author Andrew Hickey
	\date April 2005
*/

#pragma once

#include <keays_math.h>

#include <set>		// std::set
#include <vector>	// std::vector

#ifdef KEAYS_TRIANGLE_EXPORTS
#define KEAYS_TRIANGLE_API __declspec(dllexport)
#else
#define KEAYS_TRIANGLE_API __declspec(dllimport)
#endif

namespace keays
{
namespace triangle
{

//#ifdef _DEBUG
//extern KEAYS_TRIANGLE_API FILE **g_pLogFile;
//#endif
void KEAYS_TRIANGLE_API AttachLogFile(FILE **pLogFile);
void KEAYS_TRIANGLE_API DetachLogFile();

/*-----------------------------------------------------------------------
	Enumerations
	---------------------------------------------------------------------*/
enum KEAYS_TRIANGLE_API eEdgeFlags
{
	eUT_EF_NONE				=	0,			// 0x00,		//   0 - 0000 0000 0000 0000
	eUT_EF_BREAKLINE		=	(1<<0),		// 0x01,		//   1 - 0000 0000 0000 0001
	eUT_EF_XBREAKLINE		=	(1<<1),		// 0x02,		//   2 - 0000 0000 0000 0010
	eUT_EF_ACTIVE			=	(1<<2),		// 0x04,		//   4 - 0000 0000 0000 0100
	eUT_EF_BOUNDARY			=	(1<<3),		// 0x08,		//   8 - 0000 0000 0000 1000
	eUT_EF_INTERNAL			=	(1<<4),		// 0x10,		//  16 - 0000 0000 0001 0000 
	eUT_EF_POLYGON			=	(1<<5),		// 0x20,		//  32 - 0000 0000 0010 0000
	eUT_EF_CATCH			=	(1<<6),		// 0x40,		//  64 - 0000 0000 0100 0000
	eUT_EF_STREAM 			=	(1<<7),		// 0x80,		// 128 - 0000 0000 1000 0000
	eUT_EF_CAT_STREAM_BREAK	=	(eUT_EF_STREAM | eUT_EF_CATCH | eUT_EF_BREAKLINE),
											// 0xC1,		// 193 - 0000 0000 1100 0001
};

enum KEAYS_TRIANGLE_API eTriangleFlags
{
	eUT_TF_NONE					= 0,			// 0x00,
	eUT_TF_ACTIVE				= (1<<0),		// 0x01
	eUT_TF_HIDDEN 				= (1<<1),		// 0x02
	eUT_TF_LOCKED 				= (1<<2),		// 0x04
	eUT_TF_FROZEN = (eUT_TF_HIDDEN | eUT_TF_LOCKED),
};

/*-----------------------------------------------------------------------
	Structures
	---------------------------------------------------------------------*/
/*!
	\brief define a VectorD3 as a UT Point for consistency with old typename
 */
typedef KEAYS_TRIANGLE_API keays::types::VectorD3 UTPoint;


struct KEAYS_TRIANGLE_API CutSectionNode : public UTPoint
{	//#region
	CutSectionNode();
	CutSectionNode(const CutSectionNode &orig);

	const CutSectionNode &operator=(const CutSectionNode &rhs);
	const CutSectionNode &operator=(const UTPoint &pt);

	double				dist;
	int					triangleID;
	int					layerNum; 
	unsigned char		triFlags;
	//#endregion
};

/*	
//#region
class KEAYS_TRIANGLE_API CutSectionList : public std::vector<CutSectionNode>
{
public:
	inline bool CalcBatterPoint(const double &startHeight, const double &maxWidth, 
								 const double &grade, VectorD3 *pResult) const
	{
		return CalcBatterPoint(startHeight, maxWidth, grade, grade, pResult);
	}

	bool CalcBatterPoint(const double &startHeight, const double &maxWidth, 
						  const double &cutGrade, const double &fillGrade, 
						  VectorD3 *pResult) const;
};
//#endregion
/*/
//#region
typedef KEAYS_TRIANGLE_API std::vector<CutSectionNode> CutSectionList;
bool KEAYS_TRIANGLE_API CalcBatterPoint(const CutSectionList &csList, const double &startHeight, const double &maxWidth, 
										 const double &cutGrade, const double &fillGrade, VectorD3 *pResult, VectorD2 *pProfile = NULL);

inline bool CalcBatterPoint(const CutSectionList &csList, const double &startHeight, const double &maxWidth, 
							 const double &grade, VectorD3 *pResult, VectorD2 *pProfile = NULL)
{
	return CalcBatterPoint(csList, startHeight, maxWidth, grade, grade, pResult, pProfile);
}
//#endregion
//*/


/*!
	\brief
 */
struct KEAYS_TRIANGLE_API UTTriangle
{	//#region
	long			links[3];		//<! index of links to other triangles
	char			back[3];		//<! edge of the linked triangle
	unsigned char	layer;			//<! the color of the triangle
	long			vertices[3];	//<! pointers to the x,y,z data in UTPoints
	unsigned char	eflags[3];		//<! edge flags
	unsigned char	tflags;			//<! visible or not

	// state checks
	bool IsActive() const { return (tflags & eUT_TF_ACTIVE) != 0; }
	void Activate() { tflags |= eUT_TF_ACTIVE; }
	void Deactivate() { tflags &= ~eUT_TF_ACTIVE; }
	bool ToggleActive() { tflags = tflags ^ eUT_TF_ACTIVE; return IsActive(); }

	bool IsLocked() const { return (tflags & eUT_TF_LOCKED) != 0; }
	void Lock() { tflags |= eUT_TF_LOCKED; }
	void Unlock() { tflags &= ~eUT_TF_LOCKED; }
	bool ToggleLocked() { tflags = tflags ^ eUT_TF_LOCKED; return IsLocked(); }

	bool IsHidden() const { return (tflags & eUT_TF_HIDDEN) != 0; }
	void Hide() { tflags |= eUT_TF_HIDDEN; }
	void Unhide() { tflags &= ~eUT_TF_HIDDEN; }
	bool ToggleHidden() { tflags = tflags ^ eUT_TF_HIDDEN; return IsLocked(); }

	bool IsFrozen() const { return (tflags & eUT_TF_FROZEN) != 0; }
	void Freeze() { tflags |= eUT_TF_FROZEN; }
	void Thaw() { tflags &= ~eUT_TF_FROZEN; }
	bool ToggleFrozen() { tflags = tflags ^ eUT_TF_FROZEN; return IsLocked(); }

/*
	// value checks
	const double CalcFaceNormal(const double *points, const int numPoints) const;

	const double Grade(const double *points, const int numPoints) const;
//*/
	//#endregion
};

/*
__inline bool IsSet(bitvector_t &bvt, bitvector_t bit)		{ return (bvt & bit) != 0; }
__inline void SetBit(bitvector_t &bvt, bitvector_t bit)		{ bvt |= bit; }
__inline void ClearBit(bitvector_t &bvt, bitvector_t bit)	{ bvt &= ~bit; }
__inline void ToggleBit(bitvector_t &bvt, bitvector_t bit)	{ bvt = bvt ^ bit; }
*/

/*!
	\brief Store information about all triangles that belong to the same layer
 */
struct KEAYS_TRIANGLE_API TriLayerInfo
{	//#region
	/*!
		\brief
	*/
	TriLayerInfo();
	/*!
		\brief
	*/
	TriLayerInfo(const TriLayerInfo & rhs);
	/*!
		\brief
	*/
	~TriLayerInfo();

	/*!
		\brief
	*/
	const TriLayerInfo & operator = (const TriLayerInfo & rhs);

	unsigned char		m_Layer;
	uInt				m_Size;
	std::vector<int>	*m_pStartIndex;
	std::vector<int>	*m_pLength;
	//#endregion
};

/*!
	\brief Return values for Generate batter strings
 */
enum eBatterReturns
{
	S_SUCCESS = 0,		//!< Batter was successfully generated.
	E_TOO_FEW_POINTS,	//!< The source polyline does not have enough points.
	E_INVALID_DISTANCE,	//!< The max distance is too small.
	E_GEN_PERPS_FAILED,	//!< Generating the perpendicular vectors failed.
	E_PERP_INEQUALITY,	//!< There is an inequality between the number of perpendicular vectors and source points.
	E_PERP_SECTION,		//!< The process failed when generating a perpendicular section
	E_CALC_BATTER,		//!< The process failed when calculating the batter line
	E_NO_TRI_POINTS,	//!< No points were found
	E_NO_TRIANGLES,		//!< No triangles were found
	E_FAILED,			//!< Batter generation failed.
};
/*!
	\brief Data structure for Generate batter strings
 */
class KEAYS_TRIANGLE_API CBatterFlags
{	//#region
public:
	CBatterFlags(const keays::math::eSideSelections side = keays::math::SIDE_LEFT, 
				  const double &maxWidth = 0.0, const double &cutGrade = 1.0, const double &fillGrade = -1.0, 
				  const bool drape = true, const bool close = true, const bool capStart = true, 
				  const bool capEnd = true);
	CBatterFlags(const CBatterFlags &orig);

	const keays::math::eSideSelections &Side() const { return m_side; }
	const double &MaxWidth() const { return m_maxWidth; }
	const double &CutGrade() const { return m_cutGrade; }
	const double &FillGrade() const { return m_fillGrade; }
	const bool Drape() const { return m_drape; }
	const bool CapStart() const { return m_capStart; }
	const bool CapEnd() const { return m_capEnd; }
	const bool Close() const { return m_close; }

	const keays::math::eSideSelections &Side(keays::math::eSideSelections side);
	const double &MaxWidth(const double &maxWidth);
	const double &CutGrade(	const double &cutGrade);
	const double &FillGrade(const double &fillGrade);
	const bool Drape(bool drape);
	void Cap(bool cap);
	const bool CapStart(bool cap);
	const bool CapEnd(bool cap);
	const bool Close(bool close);

	const CBatterFlags &operator=(const CBatterFlags &rhs);
private:
	keays::math::eSideSelections m_side; 
	double m_maxWidth;
	double m_cutGrade; 
	double m_fillGrade; 
	bool m_drape;
	bool m_capStart;
	bool m_capEnd;
	bool m_close;
	//#endregion
};

/*!
	\brief Keays Triangles class (handler for combined points and triangles records).
 */
class KEAYS_TRIANGLE_API Triangles
{	//#region
	friend class UTFile;

public:
	/*!
		\brief
	*/
	Triangles();
	/*!
		\brief
	*/
	~Triangles();

	/*!
		\brief
	*/
	const keays::math::Cube &CalcExtents();

	/*!
		\brief
	*/
	const keays::math::Cube &GetExtents() const { return m_extents; }

	/*!
		\brief
	*/
	const keays::math::Cube &GetVisibleExtents() const { return m_visExtents; }

	/*!
		\brief
	*/
	void Translate(double x, double y, double z);

	/*!
		\brief
	*/
	void CalculateVertexNormals(pFnProgressUpdate pfnProgressUpdate = NULL, int numUpdates = 20, void *pProgressPayload = NULL);

	/*!
		\brief Locate which triangle contains a given position

		\param            pPoint [In]  - a constant pointer to a UTPoint specifying the location to find.
		\param seedTriangleIndex [Out] - a reference to an integer to receive the triangle index
		\param     allowInactive [In]  - a boolean flag indicating if an inactive triangle is allowable
		\param    startSeedIndex [In]  - a constant integer specifying the seed index to start with, if 
										 given -1 it will use the default (recommended)

		\return true if the triangle containing that point was located, otherwise false
	 */
	bool Locate(const UTPoint *pPoint, int &seedTriangleIndex, bool allowInactive = false, const int startSeed = -1) const;
	/*!
		\overload
		\param pt [In]  - a constant reference to a keays::types::VectorD2 specifying the location to find.
	 */
	bool Locate(const VectorD2 &pt, int &seedTriangleIndex, bool allowInactive = false, const int startSeed = -1) const;
	/*!
		\overload
		\param x [In]  - a constant double specifying the x component of the location to find.
		\param y [In]  - a constant double specifying the y component of the location to find.
	 */
	bool Locate(const double &x, const double &y, int &seedTriangleIndex, bool allowInactive = false, const int startSeed = -1) const;

	/*!
		\brief Allocate and set the number of triangles.
		This function will free any memory currently used by triangles, and reallocate

		\param numTris [In]  - a long integer specifying the desired number of triangles
	 */
	void SetNumberTriangles(long numTris);
	/*!
		\brief
	 */
	unsigned long GetNumberTriangles() const { return m_NumberTriangles; }
	
	/*!
		\brief Set the number of visible triangles.

		\param numVisibleTri [In]  - a long integer specifying the number of visible triangles
	 */
	void SetNumberVisibleTriangles(long numVisibleTri) { m_NumberVisibleTriangles = numVisibleTri; }
	/*!
		\brief
	 */
	unsigned long GetNumberVisibleTriangles() const { return m_NumberVisibleTriangles; }

	/*!
		\brief Allocate and set the number of points used.
		This function will free any memory currently used by points, and reallocate

		\param numPoints [In]  - a long integer specifying the desired number of points
	 */
	void SetNumberPoints(long numPoints);
	/*!
		\brief
	 */
	unsigned long GetNumberPoints() const { return m_NumberPoints; }

	/*!
		\brief
	 */
	const UTPoint	 *GetPoint(unsigned long pointNo) const;
	/*!
		\brief
	 */
	const UTTriangle *GetTriangle(unsigned long triNo) const;

	/*!
		\brief
	 */
	const UTTriangle *GetTriangles() const	{ return m_pTriangles; }

	/*!
		\brief
	 */
	const UTPoint	*GetPoints() const		{ return m_pPoints; }

	/*!
		\brief
	 */
	const UTPoint *GetVertexNormals() const { return m_pVertexNormals; }

	/*!
		\brief Calculate the face normal for the given triangle id.

		\param triangleID [In]  - a constant unsigned long integer specifying the id of the triangle to 
								  calculate for.
		\return a VectorD3 representing the normalised face normal, or keays::types::BAD_POINT 
				if the id is out of range.
	 */
	const UTPoint CalcFaceNormal(const unsigned long triangleID) const;
	/*!
		\overload calculates the face normal without the safety checks
	 */
	const UTPoint CalcFaceNormalNC(const unsigned long triangleID) const 
	{
		keays::types::VectorD3 normal(
			keays::math::Cross(
				(m_pPoints[m_pTriangles[triangleID].vertices[1]] - m_pPoints[m_pTriangles[triangleID].vertices[0]]),
				(m_pPoints[m_pTriangles[triangleID].vertices[2]] - m_pPoints[m_pTriangles[triangleID].vertices[0]])));
		return normal.Normalise();
	}

	/*!
		\brief Calculate the grade <b>up</b> the face of the given triangle id.

		\param triangleID [In]  - a constant unsigned long integer specifying the id of the triangle to 
								  calculate for.
		\return a const double precision value representing the normalised face normal, or 
				keays::types::INVALID_ANGLE if the id is out of range.
	 */
	const double Grade(const unsigned long triangleID) const;
	/*!
		\overload Calculate the grade <b>up</b> the face without safety checks.
	 */
	const double GradeNC(const unsigned long triangleID) const
	{
		return Grade(triangleID);
	}

	/*!
		\brief Calculate the geometric centroid of the given triangle id.

		\param triangleID [In]  - a constant unsigned long integer specifying the id of the triangle to 
								  calculate for.
		\return a VectorD3 representing the geometric centroid, or keays::types::BAD_POINT 
				if the id is out of range.
	 */
	const UTPoint GeometricCentroid(const unsigned long triangleID) const;
	/*!
		\overload calculates the geometric centroid without the safety checks
	 */
	const UTPoint GeometricCentroidNC(const unsigned long triangleID) const 
	{
		return UTPoint((m_pPoints[m_pTriangles[triangleID].vertices[0]] + 
						  m_pPoints[m_pTriangles[triangleID].vertices[1]] + 
						  m_pPoints[m_pTriangles[triangleID].vertices[2]]) / 3.0);
	}

	/*!
		\brief
	 */
	bool HeightAtPoint(const keays::types::VectorD2 &pt, double *pHeight, int *pTriIndex = NULL, bool allowInactive = false) const;
	/*!
		\overload
	 */
	bool HeightAtPoint(const double &x, const double &y, double *pHeight, int *pTriIndex = NULL, bool allowInactive = false) const
	{
		return HeightAtPoint(keays::types::VectorD2(x, y), pHeight, pTriIndex, allowInactive);
	}

	/*!
		\brief
	 */
	const CutSectionList *Section(const UTPoint &pt0, const UTPoint &pt1, 
								   CutSectionList *pCutList, double *pStartDistance, 
								   bool genEndPoint = true, bool breaklinesOnly = false,
								   int *pNumCutRet = NULL, int *pMaxCut = NULL) const;

	/*!
		\brief
	 */
	const CutSectionList *Section(const std::vector< UTPoint > &pts, CutSectionList *pCutList, 
								   bool breaklinesOnly = false,
								   int *pNumCutRet = NULL, int *pMaxCut = NULL) const;

	/*!
		\brief Generate a batter string on the side of the polyline indicated

		\param polyline [In]  - an keays::types::Polyline3D with the original polyline to batter
		\param  pResult [Out] - 
		\param    flags [In]  - 
	 */
	int GenerateBatterString(const keays::types::Polyline3D &polyline, 
							  keays::types::Polyline3D *pResult, const CBatterFlags &flags) const;

	// modify an individual triangle
	void Activate(const unsigned long triangleID) 
	{ 
		if (m_pTriangles && (triangleID < m_NumberTriangles)) 
			m_pTriangles[triangleID].Activate();
	}

	void Deactivate(const unsigned long triangleID)
	{ 
		if (m_pTriangles && (triangleID < m_NumberTriangles)) 
			m_pTriangles[triangleID].Deactivate();
	}

	bool ToggleActive(const unsigned long triangleID)
	{ 
		return (m_pTriangles && (triangleID < m_NumberTriangles)) ?
				 m_pTriangles[triangleID].ToggleActive() : false;
	}

	void Lock(const unsigned long triangleID)
	{ 
		if (m_pTriangles && (triangleID < m_NumberTriangles)) 
			m_pTriangles[triangleID].Lock();
	}
	void Unlock(const unsigned long triangleID)
	{ 
		if (m_pTriangles && (triangleID < m_NumberTriangles)) 
			m_pTriangles[triangleID].Unlock();
	}
	bool ToggleLocked(const unsigned long triangleID)
	{ 
		return (m_pTriangles && (triangleID < m_NumberTriangles)) ?
				 m_pTriangles[triangleID].ToggleLocked() : false;
	}

	void Hide(const unsigned long triangleID)
	{ 
		if (m_pTriangles && (triangleID < m_NumberTriangles)) 
			m_pTriangles[triangleID].Hide();
	}
	void Unhide(const unsigned long triangleID)
	{ 
		if (m_pTriangles && (triangleID < m_NumberTriangles)) 
			m_pTriangles[triangleID].Unhide();
	}
	bool ToggleHidden(const unsigned long triangleID)
	{ 
		return (m_pTriangles && (triangleID < m_NumberTriangles)) ?
				 m_pTriangles[triangleID].ToggleHidden() : false;
	}

	void SetTriangleLayer(const unsigned long triangleID, const unsigned char layer)
	{
		if (m_pTriangles && (triangleID < m_NumberTriangles)) 
			m_pTriangles[triangleID].layer = layer;
	}

private:
	/*
		agfdhsd
	 */
	double AreaParallelogram(UTPoint *pA, UTPoint *pB, UTPoint *pC);

	/*
		agfdhsd
	 */
	void SetTriangles(UTTriangle * pT, long numTri);
	/*
		agfdhsd
	 */
	void SetPoints(UTPoint	* pPm, long numPoints);
	
	/*
		agfdhsd
	 */
	const UTPoint CalculateAverageNormal(unsigned int triIndex, UTPoint * pThePoints,
										  UTPoint *pFaceNormals,
										  std::set<int> *pVisitedTris);

	// --- member variables ---
	unsigned int m_NumberTriangles;
	unsigned int m_NumberVisibleTriangles;
	unsigned int m_NumberPoints;

	UTPoint		*m_pPoints;
	UTTriangle	*m_pTriangles;
	UTPoint		*m_pVertexNormals;

	mutable int				m_seedTriangleIndex;
	keays::math::Cube		m_extents;
	keays::math::Cube		m_visExtents;
	//#endregion
};

};	// namespace keays
};	// namespace triangle

// EOF