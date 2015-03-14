/*! \file */
/*-----------------------------------------------------------------------
	settings.h

	Description: Settings Structures
	Author:		 Julian McKinlay
	---------------------------------------------------------------------*/

#pragma once

// Core Settings
enum { CS_HAL = 1, CS_REF };
enum { CS_SINGLE = 1, CS_DOUBLE };

// Scene Settings
enum { SS_NONE = 0, SS_ENABLED };
enum { SS_QUADTREE = 1, SS_OCTTREE };
enum { SS_AXIS_X = 1, SS_AXIS_Y, SS_AXIS_Z };

// Viewport settings
enum { VS_NONE = 0 };
enum { VS_ORTHO = 1, VS_PERSPECTIVE };
enum { VS_ZBUFFER = 1, VS_WBUFFER };
enum { VS_LEFTHANDED = 1, VS_RIGHTHANDED };

enum SPACEPARTITIONMODE
{
	VS_EDGE = 1,
	VS_FACE = 2,
	VS_BOTH = VS_EDGE | VS_FACE,
};


//! \struct CoreSettings
struct CoreSettings
{
	CoreSettings()
	:	createDefaultResources(false), forceSoftwareVertexProcessing(false),
		vertexShaderDebugging(false), pixelShaderDebugging(false),
		minLogLevel(0), maxLogLevel(0),
		screenWidth(1), screenHeight(1),
		deviceType(CS_HAL), floatPrecision(CS_SINGLE)
	{};

	bool createDefaultResources,		/**< Creates default textures and materials. True to enable, False to disable.*/
		 forceSoftwareVertexProcessing, /**< Forces Software VP, even if Hardware VP is available.*/
		 vertexShaderDebugging,			/**< Enables VS Debugging.*/
		 pixelShaderDebugging;			/**< Enables PS Debugging.*/

	int minLogLevel,
	    maxLogLevel;

	int screenWidth,
		screenHeight;

	DWORD deviceType,				/**< Desired Rendering Device Type. Must be one of CS_HAL, CS_REF or CS_SW */
		  floatPrecision;			/**< Desired Floating Point Precision level. Must be one of CS_SINGLE or CS_DOUBLE */
};

//! \typedef CoreSettings *PCORESETTINGS
typedef CoreSettings *PCORESETTINGS;

//! \struct SceneSettings
struct SceneSettings
{
	SceneSettings()
	:	spacePartitionMode(SS_NONE), spacePartitionColour(0xffffffff), 
		spacePartitionAxis(SS_AXIS_Y), visibilityMode(SS_NONE),
		optimiseChunks(false), polygonDepth(0), sizeDepth(0), subdivisionDepth(0), 
		collisionTolerance(0.9f), collisionPrecision(3)
	{};

	DWORD spacePartitionMode,		/**< Space Partitioning Mode. Must be one of SS_NONE, SS_QUADTREE or SS_OCTTREE.
										 Defaults to SS_NONE. */
		  spacePartitionColour,		/**< Space Partitioning Colour if partition outline rendering is enabled. 
										 Defaults to 0xffffffff. */
		  spacePartitionAxis,		/**< Space Partitioning Axis, normal to the plane used to subdivide along.
									     Must be one of SS_AXIS_X, SS_AXIS_Y or SS_AXIS_Z. Defaults to SS_AXIS_Y. */
		  visibilityMode;			/**< Potential Visibility Set Mode. Must be one of CS_NONE or CS_ENABLED. 
										 Defaults to SS_NONE. */

	bool optimiseChunks;			/**< Optimise the Chunks to more closely fit the space partition. True to Enable,
										 False to Disable. Defaults to False. */

	int	polygonDepth,				/**< Polygon Depth used for space partitioning calculations. Maximum resulting number 
										 of polygons that may be contained within one leaf. Defaults to 0. */
		sizeDepth,					/**< Size Depth used for space partitioning calculations. 
										 Maximum resulting size of one leaf. Defaults to 0. */
		subdivisionDepth;			/**< Subdivision Depth used for space partitioning calculations.
										 Maximum number of times a node may be subdivided. Defaults to 0. */
	float collisionTolerance,		/**< Penetration distance to tolerate before detecting a collision. */
		  collisionPrecision;		/**< Number of decimal places to consider during collision detection calculations. */
};

//! \typedef SceneSettings *PSCENESETTINGS
typedef SceneSettings *PSCENESETTINGS;

//! \struct ViewportSettings
struct ViewportSettings
{
	ViewportSettings()
	:	width(640), height(480), effectID(-1), numDepthLevels(1), viewWidth(1.0f), viewHeight(1.0f),
		fov(45.0f), aspect(1.0f), nearClipPlane(FLT_MIN), farClipPlane(FLT_MAX), depthLevelDistance(1),
		boundingEdgeEffectId(-1), boundingFaceEffectId(-1),
		multiSampling(false), drawSpacePartition(false), drawVisualBoundingBox(false),
		projectionMode(VS_PERSPECTIVE), depthStencilMode(VS_ZBUFFER), coordSystemMode(VS_LEFTHANDED), 
		spacePartitionMode(VS_EDGE), bgColour(0xffffffff)
	{};

	int width,					/**< Width of the Viewport, in pixels. */
		height,					/**< Height of the Viewport, in pixels. */
		effectID,				/**< Index of the default effect for this viewport. */
		numDepthLevels,			/**< Number of depth levels between coplanar primitives. Must be greater or equal to 1. 
									 Defaults to 1. */
		boundingEdgeEffectId,	/**< Effect ID of the Space Partition edges. Defaults to -1. */
		boundingFaceEffectId;	/**< Effect ID of the Space Partition faces. Defaults to -1. */

	float viewWidth,			/**< Width of the view volume. Only affects the Viewport while projectionMode is equal to VS_ORTHO. */
		  viewHeight,			/**< Height of the view volume. Only affects the Viewport while projectionMode is equal to VS_ORTHO. */
		  fov,					/**< Field of view, in degrees. */
		  aspect,				/**< Aspect ration of view. */
		  nearClipPlane,		/**< Distance to the near clip plane. */
		  farClipPlane,			/**< Distance to the far clip plane. */
		  depthLevelDistance;	/**< Distance that seperates depth levels. */

	bool multiSampling,			/**< True to enable multi sampling, false to disable it. Default value is false. */
		 drawSpacePartition,	/**< True to enable drawing of the space partition outline, false to disable it. 
									 Default value is false. */
		 drawVisualBoundingBox; /**< True to enable drawing of Visual Bounding Boxes, false to disable it.
									 Default value is false. */

	DWORD projectionMode,		/**< Filling mode. One member of the VS_PROJECTIONMODE enumerated type. Default value is VS_PERSPECTIVE. */
		  depthStencilMode,		/**< Filling mode. One member of the VS_FOGMODE enumerated type. Default value is VS_LINEAR. */
		  coordSystemMode,		/**< Coordinate System mode. One member of the VS_COORDSYSTEMMODE. Default value is VS_LEFTHANDED. */
		  spacePartitionMode,	/**< Space Partition Rendering Mode. One member of VS_SPACEPARTITIONMODE. Default value is VS_EDGE. */
		  bgColour;				/**< Default background colour of this Viewport. 32-bit colour value in 0xAARRGGBB. Default value is 0xFFFFFFFF. */
};

//! \typedef ViewportSettings *PVIEWPORTSETTINGS
typedef ViewportSettings *PVIEWPORTSETTINGS;

// EOF