#pragma once

#pragma comment (lib, "ksr_d_mt")
#pragma comment (lib, "ksrx_d_mt")

#if _MSC_VER >= 1300 // MSVC.NET
	#ifdef _DEBUG
		#pragma comment (lib, "KeaysTypes7mdD")
		#pragma comment (lib, "KeaysMath7mdD")
		#pragma comment (lib, "keays_registry7D")
	#else
		#pragma comment (lib, "KeaysTypes7md")
		#pragma comment (lib, "KeaysMath7md")
		#pragma comment (lib, "keays_registry7")
	#endif
#else
	#if _MSC_VER > 1000 // MSVC6
		#ifdef _DEBUG
			#pragma comment (lib, "keays_typesD")
			#pragma comment (lib, "keays_mathD")
		#else
			#pragma comment (lib, "keays_types")
			#pragma comment (lib, "keays_math")
		#endif
	#else
		#error "\tCould not determine which keays types and math libraries to use for this version of Visual Studio"
	#endif
#endif

#include <ksr.h>
#include <ksrx.h>

#include <vector>
#include <list>
#include <string>

#include <keays_math.h>

using namespace std;

enum
{
	POINT_CROSS,
	POINT_DIAMOND,
};

namespace KSR
{
namespace utils
{
inline Vector2 VD2toV2( const keays::types::VectorD2 &vd2 )	
									{ return Vector2( (float)vd2.x, (float)vd2.y ); }
inline Vector2 VD3toV2( const keays::types::VectorD3 &vd3 )	
									{ return Vector2( (float)vd3.x, (float)vd3.y ); }
inline Vector2 V3toV2( const Vector3 &v3 )			
									{ return Vector2( v3.x, v3.y ); }

inline Vector3 VD2toV3( const keays::types::VectorD2 &vd2, const float z = 0.0f )	
									{ return Vector3( (float)vd2.x, (float)vd2.y, z ); }
inline Vector3 VD3toV3( const keays::types::VectorD3 &vd3 )	
									{ return Vector3( (float)vd3.x, (float)vd3.y, (float)vd3.z ); }
inline Vector3 V2toV3( const Vector2 &v2, const float z = 0.0f ) 
									{ return Vector3( v2.x, v2.y, z ); }


//-----------------------------------------------------------------------------
inline keays::types::VectorD2 V2toVD2( const Vector2 &v2 )
				{ return keays::types::VectorD2( ktf::SafeFloatToDouble( v2.x ), ktf::SafeFloatToDouble( v2.y ) ); }
inline keays::types::VectorD2 V3toVD2( const Vector3 &v3 )
				{ return keays::types::VectorD2( ktf::SafeFloatToDouble( v3.x ), ktf::SafeFloatToDouble( v3.y ) ); }

inline keays::types::VectorD3 V2toVD3( const Vector2 &v2, const float z = 0.0f )
				{ return keays::types::VectorD3( ktf::SafeFloatToDouble( v2.x ), ktf::SafeFloatToDouble( v2.y ), z ); }
inline keays::types::VectorD3 V3toVD3( const Vector3 &v3 )
				{ return keays::types::VectorD3( ktf::SafeFloatToDouble( v3.x ), ktf::SafeFloatToDouble( v3.y ), ktf::SafeFloatToDouble( v3.z ) ); }
}
}

// EOF