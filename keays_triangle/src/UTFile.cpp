
#include "..\include\UTFile.h"

#include <mathhelp.h>

#include <FCNTL.H>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>

#include <LeakWatcher.h>
#include <Windows.h>

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static TCHAR THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable : 4786) // ignore the long name warning associated with stl stuff

namespace keays
{
namespace triangle
{

bool UTFile::Read(LPCTSTR filename, Triangles &triangles, pFnProgressUpdate pfnProgressUpdate /*= NULL*/)
{
	if (!filename)
	{
//		OutputDebugString("NULL filename passed to UTFile::Read(...)\n");
		return false;
	}

	// Load geometry from UT
	FILE *pointsFile,
		 *trianglesFile;
	int fileNameLen = strlen(filename);

	if (fileNameLen < 5)
	{
//		OutputDebugString("filename too short in UTFile::Read ...)\n");
		return false;
	}

	std::string pointsFilename = filename;
	std::string triangleFilename = filename;
	pointsFilename[fileNameLen - 2] = 'p';

	pointsFile = keays::math::FileOpen(pointsFilename.c_str(), "rb");
	trianglesFile = keays::math::FileOpen(triangleFilename.c_str(), "rb");

	if (pointsFile == NULL) 
		return false;
	if (trianglesFile == NULL) 
		return false;

	UTPoint		*pPoints = NULL;
	UTTriangle	*pTriangles = NULL;
	UTTriangle	*pVisibleTriangles = NULL;

	int numPoints = 0,
		numTriangles = 0,
		numVisibleTriangles = 0;

	struct _stat fileStat;
	_fstat(_fileno(pointsFile), &fileStat);
	int pointsFileLen = fileStat.st_size;
	numPoints = pointsFileLen / sizeof(UTPoint);

	_fstat(_fileno(trianglesFile), &fileStat);
	int trianglesFileLen = fileStat.st_size;
	numTriangles = trianglesFileLen / sizeof(UTTriangle);

	if (0 == numPoints || 0 == numTriangles) 
		return false;

	triangles.SetNumberPoints(numPoints);			// this allocate the memory
	triangles.SetNumberTriangles(numTriangles);	// as above

	pPoints		= triangles.m_pPoints;
	pTriangles	= triangles.m_pTriangles;

	int pfl = fread((void *)pPoints, pointsFileLen, 1, pointsFile);
	int trl = fread((void *)pTriangles, trianglesFileLen, 1, trianglesFile);

	// get number of visible triangles
	for (int i = 0; i < numTriangles; i++)
	{
		if (eUT_TF_ACTIVE == (pTriangles[i].tflags & eUT_TF_ACTIVE))
		{
			numVisibleTriangles = numVisibleTriangles + 1;
		}
	}

	triangles.SetNumberVisibleTriangles(numVisibleTriangles);
	triangles.CalculateVertexNormals(pfnProgressUpdate);
	
	fclose(pointsFile);
	fclose(trianglesFile);

	triangles.CalcExtents();

	return true;
}

bool UTFile::ReadV2(LPCTSTR filename, Triangles &triangles, pFnProgressUpdate pfnProgressUpdate /*= NULL*/)
{
	FILE *pFile = keays::math::FileOpen(filename, "rb");
	if (!pFile)
		return false;

	// read the header information
	V2UTFileHeader header;
	fread(&header, sizeof(V2UTFileHeader), 1, pFile);

	if (header.Version() != G_V2_KUT_FILE_VERSION)
	{
		// wrong version so we close and quit
		fclose(pFile);
		return false;
	}

	UTPoint		*pPoints = NULL;
	UTTriangle	*pTriangles = NULL;

	pPoints		= triangles.m_pPoints;
	pTriangles	= triangles.m_pTriangles;

	// allocate then read the triangles
	triangles.SetNumberTriangles(header.Triangles().Count());
	fread(triangles.m_pTriangles, header.Triangles().Count() * header.Triangles().RecordSize(), 1, pFile);

	// calculate the visible triangles
	long visCount = 0;
	for (unsigned int i = 0; i < header.Triangles().Count(); i++)
	{
		if (triangles.m_pTriangles[i].IsActive())
		{
			++visCount;
		}
	}
	triangles.SetNumberVisibleTriangles(visCount);

	// allocate then read the points
	triangles.SetNumberPoints(header.Points().Count());
	fread(triangles.m_pPoints, header.Points().Count() * header.Points().RecordSize(), 1, pFile);

	// allocate then read the normals
	// this does not have a function because it should only be done by calc normals
	if (triangles.m_pVertexNormals)
		delete[] triangles.m_pVertexNormals;
	triangles.m_pVertexNormals = new UTPoint[header.VertexNormals().Count()];
	fread(triangles.m_pVertexNormals, header.VertexNormals().Count() * header.VertexNormals().RecordSize(), 1, pFile);

	// some extra work needs to be done to ensure the bounds are fine
	triangles.CalcExtents();

	fclose(pFile);

	return true;
}

bool UTFile::Save(LPCTSTR filename, const Triangles *pTriangles, const unsigned char version)
{
	switch (version)
	{
	case 1:
		return SaveV1(filename, pTriangles);
	case 2:
		return SaveV2(filename, pTriangles);
	default:
		return SaveV1(filename, pTriangles);
	};
}

bool UTFile::SaveV1(LPCTSTR filename, const Triangles *pTriangles)
{
	if (!pTriangles || !filename)
		return false;

	std::string triangleFilename(filename);
	std::string pointsFilename;
	std::string triangleBakFilename;
	std::string pointsBakFilename;

	TCHAR drv[_MAX_DRIVE];
	TCHAR path[_MAX_PATH];
	TCHAR fname[_MAX_FNAME]; 
	TCHAR ext[_MAX_EXT];
	TCHAR buf[_MAX_PATH];
	keays::math::SplitPath(filename, drv, path, fname, ext);
	if ((_strnicmp(ext, ".ut", 3) != 0) || (strlen(ext) != 4))
		return false;	// invalid filename

	ext[2] = 'p';	// points file
	keays::math::MakePath(buf, drv, path, fname, ext);
	pointsFilename = buf;

	ext[2] = 'u';	// triangles backup file
	keays::math::MakePath(buf, drv, path, fname, ext);
	triangleBakFilename = buf;

	ext[2] = 'o';	// points backup file
	keays::math::MakePath(buf, drv, path, fname, ext);
	pointsBakFilename = buf;

	// write triangles
	// backup first
	MoveFileEx(triangleFilename.c_str(), triangleBakFilename.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
	
	FILE *pTriFile = keays::math::FileOpen(triangleFilename.c_str(), "wb");
	const UTTriangle *pRawTriangles = pTriangles->GetTriangles();
	const unsigned int numTriangles = pTriangles->GetNumberTriangles();
	if (!pTriFile || !pRawTriangles || (numTriangles < 1))
	{
		// restore the backup file and delete backup
		MoveFileEx(triangleBakFilename.c_str(), triangleFilename.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		return false;
	}

	// now we have the file pointer, the data pointer, and the record count, 
	// time to write the data
	fwrite((void*)pRawTriangles, numTriangles*sizeof(UTTriangle), 1, pTriFile);
	fclose(pTriFile);

	// write points
	// backup first
	MoveFileEx(pointsFilename.c_str(), pointsBakFilename.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);

	FILE *pPointsFile = keays::math::FileOpen(pointsFilename.c_str(), "wb");
	const UTPoint *pRawPoints = pTriangles->GetPoints();
	const unsigned int numPoints = pTriangles->GetNumberPoints();
	if (!pPointsFile || !pRawPoints || (numPoints < 3))
	{
		// restore the triangle file and delete backup
		MoveFileEx(triangleBakFilename.c_str(), triangleFilename.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);

		// restore the points file and delete backup
		MoveFileEx(pointsBakFilename.c_str(), pointsFilename.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		return false;
	}

	fwrite((void*)pRawPoints, numPoints*sizeof(UTPoint), 1, pPointsFile);
	fclose(pPointsFile);

	return true;
}

bool UTFile::SaveV2(LPCTSTR filename, const Triangles *pTriangles)
{
	if (!pTriangles)
		return false;
	if (!filename)
		return false;

	unsigned long numTriangles = pTriangles->GetNumberTriangles();
	unsigned long numPoints = pTriangles->GetNumberPoints();
	if ((numTriangles < 1) || (numPoints < 3))
		return false;

	// backup the existing file (if required)
	std::string bakFilename(filename);
	bakFilename += ".bak";
	if (_access(filename, 06) == 0)
		MoveFileEx(filename, bakFilename.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);

	// create and calculate the header
	V2UTFileHeader header;
	header.Triangles() = V2UTFileHeaderRecord(sizeof(V2UTFileHeader), 
											   numTriangles * sizeof(UTTriangle),
											   sizeof(UTTriangle), numTriangles);
	header.Points() = V2UTFileHeaderRecord(header.Triangles().Offset() + header.Triangles().Size(), 
											   numPoints * sizeof(UTPoint),
											   sizeof(UTPoint), numPoints);
	// not there is a vertex normal for every point
	header.VertexNormals() = V2UTFileHeaderRecord(header.Points().Offset() + header.Points().Size(), 
											   numPoints * sizeof(UTPoint),
											   sizeof(UTPoint), numPoints);

	// we have the header information (an are given the content) so we can write the file
	FILE *pFile = keays::math::FileOpen(filename, "wb");
	if (!pFile)
	{
		// restore the backup
		MoveFileEx(bakFilename.c_str(), filename, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		return false;
	}

	// header
	fwrite(&header, sizeof(V2UTFileHeader), 1, pFile);
	// triangles
	fwrite(pTriangles->GetTriangles(), sizeof(UTTriangle) * numTriangles, 1, pFile);
	fwrite(pTriangles->GetPoints(), sizeof(UTPoint) * numPoints, 1, pFile);
	fwrite(pTriangles->GetVertexNormals(), sizeof(UTPoint) *numPoints, 1, pFile);

	fclose(pFile);

	return true;
}

bool UTFile::ReadV2Text(LPCTSTR filename, Triangles &triangles, pFnProgressUpdate pfnProgressUpdate /*= NULL*/)
{
	return false;
}

bool UTFile::SaveText(LPCTSTR filename, const Triangles *pTriangles, const unsigned char version)
{
	return false;
}

bool UTFile::SaveV2Text(LPCTSTR filename, const Triangles *pTriangles)
{
	return false;
}

bool UTFile::CanLoadExt(const std::string & ext)
{
	return false;
}

};
};
