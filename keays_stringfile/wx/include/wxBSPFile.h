/*
 * Filename: wxBSPFile.h
 * Date: May 2004
 * Author: Karl Janke, Keays Software
 *
 * This file provides the header definition of a class that can load
 * Quake3 BSP Files into a StringData object in memory. It can't save.
 * Also note that a lot of the original BSP Information is discarded, 
 * such as lighting & texture information.
 *
 * It belongs to the keays::stringfile namespace
 */
#ifndef _WX_BSPFILE_H
#define _WX_BSPFILE_H

// Includes
#include <string>		// std::string
#include <File.h>		// keays::stringfile::File

namespace keays
{
namespace stringfile
{
namespace wx
{


struct aVector3
{
	float x,y,z;
};

struct aVector2
{
	float x,y;
};

typedef unsigned char aByte;

// stolen from bsp.h
enum BSPLUMPS
{
	kEntities = 0,
    kTextures,
    kPlanes,
    kNodes,
    kLeafs,
    kLeafFaces,
    kLeafBrushes,
    kModels,
    kBrushes,
    kBrushSides,
    kVertices,
    kMeshVerts,
    kShaders,
    kFaces,
    kLightmaps,
    kLightVolumes,
    kVisData,
    kMaxLumps
};

struct BSPHeader
{
	char id[4];
	int version;
};

struct BSPLump
{
	int offset, length;
};

struct BSPVertex
{
	aVector3 position;
	aVector2 uv[2];
	aVector3 normal;
	aByte	color;
};

struct BSPFace
{
	int textureId,
		effect,
		type,
		startVertex,
		numVerts,
		startMeshVertex,
		numMeshVerts,
		lightmapId,
		lightmapCorner[2],
		lightmapSize[2];
	aVector3 position;
	aVector3 mapVecs[2];
	aVector3 normal;
	int size[2];
};

struct BSPTexture
{
	char filename[64];
	int flags,
		contents;
};

struct BSPLightmap
{
	aByte bits[128][128][3];
};
// end stolen code from bsp.h

class BSPFile : public StringFile
{
public:
	/* Implementation of StringFile interface */
	static bool Read ( const std::string & filename, 
					   keays::stringfile::Data * stringData,
					   bool progress, bool userAbort );
	
	static bool Save ( const std::string & filename, 
					   keays::stringfile::Data * stringData,
					   bool progress, bool userAbort );
	
	static bool CanLoadExt( const std::string & extension );
	/* End Implementation of StringFile interface */
};

} // namespace wx
} // namespace stringfile
} // namespace keays

#endif