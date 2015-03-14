/*! \file */
/*-----------------------------------------------------------------------
	types.h

	Description: Type definitions and data structures
	Author:		 Julian McKinlay
	---------------------------------------------------------------------*/

#pragma once

typedef D3DXVECTOR4		Vector4;
typedef D3DXVECTOR3		Vector3;
typedef D3DXVECTOR2		Vector2;
typedef D3DXMATRIX		Matrix;
typedef D3DXPLANE		Plane;
typedef D3DXQUATERNION  Quaternion;

/*
struct Matrix : public D3DXMATRIX		
{
private:
	const Matrix &operator =(const Matrix &rhs)
	{
		memcpy(this, &rhs, sizeof(D3DXMATRIX));
		return *this;
	}
};
*/

//! \enum
enum PROGRESSCALLBACKTYPE
{
	PC_UNKNOWN = 0,
	PC_SCENE_SPACEPARTITIONING,
	PC_SCENE_OPTIMISECHUNKS,
	PC_SCENE_PVS,
};

//! \enum
enum REFRESHTYPE
{ 
	RT_NONE = 0, 
	RT_ONCE, 
	RT_LOOP 
};

//! \enum
enum TEXTURETYPE
{ 
	TT_RENDERTARGET = 1,
	TT_ARGB32		= 2,
	TT_TEXTURE		= 4,
	TT_CUBE			= 8,
	TT_VOLUME		= 16,
};

//! \enum
enum DEVICESTATE
{
	DS_READY	= 1,
	DS_NOTRESET	= 2,
	DS_LOST		= 4,
};

//! \enum
enum FLEXIBLEVERTEXFORMATCOMPONENT
{
	XYZ = D3DFVF_XYZ,
	DIFFUSE = D3DFVF_DIFFUSE,
	NORMAL = D3DFVF_NORMAL,
	TEX1 = D3DFVF_TEX1,
	TEX2 = D3DFVF_TEX2,
};

//! \enum
enum FLEXIBLEVERTEXFORMAT
{
	FVF_DIFFUSE_TEX1		= XYZ | DIFFUSE | TEX1,
	FVF_NORMAL_TEX1			= XYZ | NORMAL  | TEX1,
	FVF_NORMAL_TEX2			= XYZ | NORMAL  | TEX2,
	FVF_NORMAL_DIFFUSE_TEX1 = XYZ | NORMAL  | DIFFUSE | TEX1,
	FVF_DIFFUSE_TEX2		= XYZ | DIFFUSE | TEX2,
	FVF_NORMAL_DIFFUSE_TEX2	= XYZ | NORMAL  | DIFFUSE | TEX2,
	FVF_DIFFUSE				= XYZ | DIFFUSE,
	FVF_NORMAL_DIFFUSE		= XYZ | NORMAL | DIFFUSE,
};

struct VERTEX_DIFFUSE_TEX1;
struct VERTEX_NORMAL_TEX1;
struct VERTEX_NORMAL_DIFFUSE_TEX1;
struct VERTEX_DIFFUSE_TEX2;
struct VERTEX_NORMAL_TEX2;
struct VERTEX_NORMAL_DIFFUSE_TEX2;
struct VERTEX_DIFFUSE;
struct VERTEX_NORMAL_DIFFUSE;

/*! \struct VERTEX_DIFFUSE_TEX1 
Vertex structure containing position, colour and 1 set of texture coordinates. */
struct VERTEX_DIFFUSE_TEX1
{
	VERTEX_DIFFUSE_TEX1(const Vector3 &pos = Vector3(0, 0, 0), DWORD col = 0xffffffff, const Vector2 &uv0 = Vector2(0, 0))
		: 	position(pos), color(col), uv(uv0) {}

	VERTEX_DIFFUSE_TEX1(const VERTEX_DIFFUSE_TEX1 &rhs)
		:	position(rhs.position), color(rhs.color), uv(rhs.uv) {}

	const VERTEX_DIFFUSE_TEX1 &operator =(const VERTEX_DIFFUSE_TEX1 &rhs);
	const VERTEX_DIFFUSE_TEX1 &operator =(const VERTEX_NORMAL_TEX1 &rhs);
	const VERTEX_DIFFUSE_TEX1 &operator =(const VERTEX_NORMAL_DIFFUSE_TEX1 &rhs);
	const VERTEX_DIFFUSE_TEX1 &operator =(const VERTEX_DIFFUSE_TEX2 &rhs);
	const VERTEX_DIFFUSE_TEX1 &operator =(const VERTEX_NORMAL_TEX2 &rhs);
	const VERTEX_DIFFUSE_TEX1 &operator =(const VERTEX_NORMAL_DIFFUSE_TEX2 &rhs);
	const VERTEX_DIFFUSE_TEX1 &operator =(const VERTEX_DIFFUSE &rhs);
	const VERTEX_DIFFUSE_TEX1 &operator =(const VERTEX_NORMAL_DIFFUSE &rhs);

	const VERTEX_DIFFUSE_TEX1 &operator =(const Vector3 &pos) { position = pos; return *this; }
	const VERTEX_DIFFUSE_TEX1 &operator =(const DWORD col) { color = col; return *this; }
	const VERTEX_DIFFUSE_TEX1 &operator =(const Vector2 &tex) { uv = tex; return *this; }

	Vector3 position;	/**< Position in XYZ coordinates. */
	DWORD color;		/**< Colour, in A8R8G8B8. */
	Vector2 uv;			/**< Texture coordinates. */
};

/*! \struct VERTEX_NORMAL_TEX1 
Vertex structure containing position, vertex normal and 1 set of texture coordinates. */
struct VERTEX_NORMAL_TEX1
{
	VERTEX_NORMAL_TEX1(const Vector3 &pos = Vector3(0, 0, 0), const Vector3 &norm = Vector3(0, 1, 0),  
					   const Vector2 &uv0 = Vector2(0, 0))
		:	position(pos), normal(norm), uv(uv0) {}

	VERTEX_NORMAL_TEX1(const VERTEX_NORMAL_TEX1 &rhs)
		:	position(rhs.position), normal(rhs.normal), uv(rhs.uv) {}

	const VERTEX_NORMAL_TEX1 &operator =(const VERTEX_NORMAL_TEX1 &rhs);
	const VERTEX_NORMAL_TEX1 &operator =(const VERTEX_DIFFUSE_TEX1 &rhs);
	const VERTEX_NORMAL_TEX1 &operator =(const VERTEX_NORMAL_DIFFUSE_TEX1 &rhs);
	const VERTEX_NORMAL_TEX1 &operator =(const VERTEX_DIFFUSE_TEX2 &rhs);
	const VERTEX_NORMAL_TEX1 &operator =(const VERTEX_NORMAL_TEX2 &rhs);
	const VERTEX_NORMAL_TEX1 &operator =(const VERTEX_NORMAL_DIFFUSE_TEX2 &rhs);
	const VERTEX_NORMAL_TEX1 &operator =(const VERTEX_DIFFUSE &rhs);
	const VERTEX_NORMAL_TEX1 &operator =(const VERTEX_NORMAL_DIFFUSE &rhs);

	const VERTEX_NORMAL_TEX1 &operator =(const Vector3 &pos) { position = pos; return *this; }
	const VERTEX_NORMAL_TEX1 &operator =(const Vector2 &tex) { uv = tex; return *this; }

	Vector3 position;	/**< Position in XYZ coordinates. */
	Vector3 normal;		/**< Vertex Normal, in XYZ coordinates. */
	Vector2 uv;			/**< Texture coordinates. */
};

/*! \struct VERTEX_NORMAL_DIFFUSE_TEX1 
Vertex structure containing position, vertex normal, colour and 1 set of texture coordinates. */
struct VERTEX_NORMAL_DIFFUSE_TEX1
{
	VERTEX_NORMAL_DIFFUSE_TEX1(const Vector3 &pos = Vector3(0, 0, 0), const Vector3 &norm = Vector3(0, 1, 0), 
							   DWORD col = 0xffffffff, const Vector2 &uv0 = Vector2(0, 0))
		: 	position(pos), normal(norm), color(col), uv(uv0) {}

	VERTEX_NORMAL_DIFFUSE_TEX1(const VERTEX_NORMAL_DIFFUSE_TEX1 &rhs)
		:	position(rhs.position), normal(rhs.normal), color(rhs.color), uv(rhs.uv) {}

	const VERTEX_NORMAL_DIFFUSE_TEX1 &operator =(const VERTEX_NORMAL_DIFFUSE_TEX1 &rhs);
	const VERTEX_NORMAL_DIFFUSE_TEX1 &operator =(const VERTEX_DIFFUSE_TEX1 &rhs);
	const VERTEX_NORMAL_DIFFUSE_TEX1 &operator =(const VERTEX_NORMAL_TEX1 &rhs);
	const VERTEX_NORMAL_DIFFUSE_TEX1 &operator =(const VERTEX_NORMAL_TEX2 &rhs);
	const VERTEX_NORMAL_DIFFUSE_TEX1 &operator =(const VERTEX_NORMAL_DIFFUSE_TEX2 &rhs);
	const VERTEX_NORMAL_DIFFUSE_TEX1 &operator =(const VERTEX_DIFFUSE &rhs);
	const VERTEX_NORMAL_DIFFUSE_TEX1 &operator =(const VERTEX_NORMAL_DIFFUSE &rhs);

	const VERTEX_NORMAL_DIFFUSE_TEX1 &operator =(const Vector3 &pos) { position = pos; return *this; }
	const VERTEX_NORMAL_DIFFUSE_TEX1 &operator =(const DWORD col) { color = col; return *this; }
	const VERTEX_NORMAL_DIFFUSE_TEX1 &operator =(const Vector2 &tex) { uv = tex; return *this; }

	Vector3 position;	/**< Position in XYZ coordinates. */
	Vector3 normal;		/**< Vertex Normal, in XYZ coordinates. */
	DWORD color;		/**< Colour, in A8R8G8B8. */
	Vector2 uv;			/**< Texture coordinates. */
};

/*! \struct VERTEX_DIFFUSE_TEX2
Vertex structure containing position, colour and 2 sets of texture coordinates. */
struct VERTEX_DIFFUSE_TEX2
{
	VERTEX_DIFFUSE_TEX2(const Vector3 &pos = Vector3(0, 0, 0), DWORD col = 0xffffffff,
						const Vector2 &uv0 = Vector2(0, 0), const Vector2 &uv1 = Vector2(0, 0))
		:	position(pos), color(col) { uv[0] = uv0; uv[1] = uv1; }

	VERTEX_DIFFUSE_TEX2(const VERTEX_DIFFUSE_TEX2 &rhs)
		:	position(rhs.position), color(rhs.color) { uv[0] = rhs.uv[0]; uv[1] = rhs.uv[1]; }

	const VERTEX_DIFFUSE_TEX2 &operator =(const VERTEX_DIFFUSE_TEX2 &rhs);
	const VERTEX_DIFFUSE_TEX2 &operator =(const VERTEX_DIFFUSE_TEX1 &rhs);
	const VERTEX_DIFFUSE_TEX2 &operator =(const VERTEX_NORMAL_TEX1 &rhs);
	const VERTEX_DIFFUSE_TEX2 &operator =(const VERTEX_NORMAL_DIFFUSE_TEX1 &rhs);
	const VERTEX_DIFFUSE_TEX2 &operator =(const VERTEX_NORMAL_TEX2 &rhs);
	const VERTEX_DIFFUSE_TEX2 &operator =(const VERTEX_NORMAL_DIFFUSE_TEX2 &rhs);
	const VERTEX_DIFFUSE_TEX2 &operator =(const VERTEX_DIFFUSE &rhs);
	const VERTEX_DIFFUSE_TEX2 &operator =(const VERTEX_NORMAL_DIFFUSE &rhs);

	const VERTEX_DIFFUSE_TEX2 &operator =(const Vector3 &pos) { position = pos; return *this; }
	const VERTEX_DIFFUSE_TEX2 &operator =(const DWORD col) { color = col; return *this; }
	const VERTEX_DIFFUSE_TEX2 &operator =(const Vector2 &tex) { uv[0] = tex; return *this; }
	const VERTEX_DIFFUSE_TEX2 &operator =(const Vector2 *texCoords) { uv[0] = texCoords[0]; uv[1] = texCoords[1]; return *this; }

	Vector3 position;	/**< Position in XYZ coordinates. */
	DWORD color;		/**< Colour, in A8R8G8B8. */
	Vector2 uv[2];		/**< Array of 2 Texture coordinate vectors. */
};

/*! \struct VERTEX_NORMAL_TEX2 
Vertex structure containing position, vertex normal and 2 sets of texture coordinates. */
struct VERTEX_NORMAL_TEX2
{
	VERTEX_NORMAL_TEX2(const Vector3 &pos = Vector3(0, 0, 0), const Vector3 &norm = Vector3(0, 1, 0), 
					   const Vector2 &uv0 = Vector2(0, 0), const Vector2 &uv1 = Vector2(0, 0))
	:	position(pos), normal(norm) { uv[0] = uv0; uv[1] = uv1; }

	VERTEX_NORMAL_TEX2(const VERTEX_NORMAL_TEX2 &rhs)
	:	position(rhs.position), normal(rhs.normal) { uv[0] = rhs.uv[0]; uv[1] = rhs.uv[1]; }

	const VERTEX_NORMAL_TEX2 &operator =(const VERTEX_NORMAL_TEX2 &rhs);
	const VERTEX_NORMAL_TEX2 &operator =(const VERTEX_DIFFUSE_TEX1 &rhs);
	const VERTEX_NORMAL_TEX2 &operator =(const VERTEX_NORMAL_TEX1 &rhs);
	const VERTEX_NORMAL_TEX2 &operator =(const VERTEX_NORMAL_DIFFUSE_TEX1 &rhs);
	const VERTEX_NORMAL_TEX2 &operator =(const VERTEX_DIFFUSE_TEX2 &rhs);
	const VERTEX_NORMAL_TEX2 &operator =(const VERTEX_NORMAL_DIFFUSE_TEX2 &rhs);
	const VERTEX_NORMAL_TEX2 &operator =(const VERTEX_DIFFUSE &rhs);
	const VERTEX_NORMAL_TEX2 &operator =(const VERTEX_NORMAL_DIFFUSE &rhs);

	const VERTEX_NORMAL_TEX2 &operator =(const Vector3 &pos) { position = pos; return *this; }
	const VERTEX_NORMAL_TEX2 &operator =(const Vector2 &tex) { uv[0] = tex; return *this; }
	const VERTEX_NORMAL_TEX2 &operator =(const Vector2 *texCoords) { uv[0] = texCoords[0]; uv[1] = texCoords[1]; return *this; }

	Vector3 position;	/**< Position in XYZ coordinates. */
	Vector3 normal;		/**< Vertex Normal, in XYZ coordinates. */
	Vector2 uv[2];		/**< Array of 2 Texture coordinate vectors. */
};

/*! \struct VERTEX_NORMAL_DIFFUSE_TEX2 
Vertex structure containing position, vertex normal, colour and 2 sets of texture coordinates. */
struct VERTEX_NORMAL_DIFFUSE_TEX2
{
	VERTEX_NORMAL_DIFFUSE_TEX2(const Vector3 &pos = Vector3(0, 0, 0), const Vector3 &norm = Vector3(0, 1, 0),
							   DWORD col = 0xffffffff, const Vector2 &uv0 = Vector2(0, 0), 
							   const Vector2 &uv1 = Vector2(0, 0))
		:	position(pos), normal(norm), color(col) { uv[0] = uv0; uv[1] = uv1; }
	VERTEX_NORMAL_DIFFUSE_TEX2(const VERTEX_NORMAL_DIFFUSE_TEX2 &rhs)
		:	position(rhs.position), normal(rhs.normal), color(rhs.color) { uv[0] = rhs.uv[0]; uv[1] = rhs.uv[1]; }

	const VERTEX_NORMAL_DIFFUSE_TEX2 &operator =(const VERTEX_NORMAL_DIFFUSE_TEX2 &rhs);
	const VERTEX_NORMAL_DIFFUSE_TEX2 &operator =(const VERTEX_DIFFUSE_TEX1 &rhs);
	const VERTEX_NORMAL_DIFFUSE_TEX2 &operator =(const VERTEX_NORMAL_TEX1 &rhs);
	const VERTEX_NORMAL_DIFFUSE_TEX2 &operator =(const VERTEX_NORMAL_DIFFUSE_TEX1 &rhs);
	const VERTEX_NORMAL_DIFFUSE_TEX2 &operator =(const VERTEX_DIFFUSE_TEX2 &rhs);
	const VERTEX_NORMAL_DIFFUSE_TEX2 &operator =(const VERTEX_NORMAL_TEX2 &rhs);
	const VERTEX_NORMAL_DIFFUSE_TEX2 &operator =(const VERTEX_DIFFUSE &rhs);
	const VERTEX_NORMAL_DIFFUSE_TEX2 &operator =(const VERTEX_NORMAL_DIFFUSE &rhs);

	const VERTEX_NORMAL_DIFFUSE_TEX2 &operator =(const Vector3 &pos) { position = pos; return *this; }
	const VERTEX_NORMAL_DIFFUSE_TEX2 &operator =(const DWORD col) { color = col; return *this; }
	const VERTEX_NORMAL_DIFFUSE_TEX2 &operator =(const Vector2 &tex) { uv[0] = tex; return *this; }
	const VERTEX_NORMAL_DIFFUSE_TEX2 &operator =(const Vector2 *texCoords) { uv[0] = texCoords[0]; uv[1] = texCoords[1]; return *this; }

	Vector3 position;	/**< Position in XYZ coordinates. */
	Vector3 normal;		/**< Vertex Normal, in XYZ coordinates. */
	DWORD color;		/**< Colour, in A8R8G8B8. */
	Vector2 uv[2];		/**< Array of 2 Texture coordinate vectors. */
};

/*! \struct FVF_DIFFUSE
Vertex structure containing position and colour.*/
struct VERTEX_DIFFUSE
{
	VERTEX_DIFFUSE(const Vector3 &pos = Vector3(0, 0, 0), DWORD col = 0xffffffff)
		:	position(pos), color(col) {}

	VERTEX_DIFFUSE(const VERTEX_DIFFUSE &rhs)
		:	position(rhs.position), color(rhs.color) {}

	const VERTEX_DIFFUSE &operator =(const VERTEX_DIFFUSE &rhs);
	const VERTEX_DIFFUSE &operator =(const VERTEX_DIFFUSE_TEX1 &rhs);
	const VERTEX_DIFFUSE &operator =(const VERTEX_NORMAL_TEX1 &rhs);
	const VERTEX_DIFFUSE &operator =(const VERTEX_NORMAL_DIFFUSE_TEX1 &rhs);
	const VERTEX_DIFFUSE &operator =(const VERTEX_DIFFUSE_TEX2 &rhs);
	const VERTEX_DIFFUSE &operator =(const VERTEX_NORMAL_TEX2 &rhs);
	const VERTEX_DIFFUSE &operator =(const VERTEX_NORMAL_DIFFUSE_TEX2 &rhs);
	const VERTEX_DIFFUSE &operator =(const VERTEX_NORMAL_DIFFUSE &rhs);

	const VERTEX_DIFFUSE &operator =(const Vector3 &pos) { position = pos; return *this; }
	const VERTEX_DIFFUSE &operator =(const DWORD col) { color = col; return *this; }

	Vector3 position;	/**< Position in XYZ coordinates. */
	DWORD color;		/**< Colour, in A8R8G8B8. */
};

/*! \struct FVF_NORMAL_DIFFUSE
Vertex structure containing position, normal and colour.*/
struct VERTEX_NORMAL_DIFFUSE
{
	VERTEX_NORMAL_DIFFUSE(const Vector3 &pos = Vector3(0, 0, 0), const Vector3 &norm = Vector3(0, 1, 0), DWORD col = 0xffffffff)
		:	position(pos), normal(norm), color(col) {}
	VERTEX_NORMAL_DIFFUSE(const VERTEX_NORMAL_DIFFUSE &rhs)
		:	position(rhs.position), normal(rhs.normal), color(rhs.color) {}

	const VERTEX_NORMAL_DIFFUSE &operator =(const VERTEX_NORMAL_DIFFUSE &rhs);
	const VERTEX_NORMAL_DIFFUSE &operator =(const VERTEX_DIFFUSE_TEX1 &rhs);
	const VERTEX_NORMAL_DIFFUSE &operator =(const VERTEX_NORMAL_TEX1 &rhs);
	const VERTEX_NORMAL_DIFFUSE &operator =(const VERTEX_NORMAL_DIFFUSE_TEX1 &rhs);
	const VERTEX_NORMAL_DIFFUSE &operator =(const VERTEX_DIFFUSE_TEX2 &rhs);
	const VERTEX_NORMAL_DIFFUSE &operator =(const VERTEX_NORMAL_TEX2 &rhs);
	const VERTEX_NORMAL_DIFFUSE &operator =(const VERTEX_NORMAL_DIFFUSE_TEX2 &rhs);
	const VERTEX_NORMAL_DIFFUSE &operator =(const VERTEX_DIFFUSE &rhs);

	const VERTEX_NORMAL_DIFFUSE &operator =(const Vector3 &pos) { position = pos; return *this; }
	const VERTEX_NORMAL_DIFFUSE &operator =(const DWORD col) { color = col; return *this; }

	Vector3 position;	/**< Position in XYZ coordinates. */
	Vector3 normal;		/**< Vertex Normal, in XYZ coordinates. */
	DWORD color;		/**< Colour, in A8R8G8B8. */
};

struct Scene;
typedef Scene *PSCENE;
typedef std::list<PSCENE> SceneList;

struct Geometry;
//! \typedef Geometry *PGEOMETRY
typedef Geometry *PGEOMETRY;
typedef std::list<PGEOMETRY> GeometryList;

#ifdef UNICODE
	typedef std::wstring String;
//	typedef std::vector<std::wstring> StringArray;
//	typedef std::list<std::wstring> StringList;
#else
	typedef std::string String;
//	typedef std::vector<std::string> StringArray;
//	typedef std::list<std::string> StringList;
#endif
typedef std::vector<String> StringArray;
typedef std::list<String> StringList;

#ifndef ubyte
typedef unsigned char ubyte;
#endif

#ifdef _DEBUG
#define _DEBUGID
	template <class IDTYPE>
	class LWIDServer
	{
	public:
		LWIDServer();
		~LWIDServer();

		const IDTYPE GetID();
		bool ReleaseID(const IDTYPE id);

		LPCTSTR DumpData() const;

	private:
		void Fix();

		IDTYPE m_ID;
		std::set<IDTYPE> *m_pFreedIDs;
	};
#endif

// EOF