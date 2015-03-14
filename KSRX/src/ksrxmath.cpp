#include <ksr.h>
#include "..\include\ksrx.h"

using namespace KSR;

//-----------------------------------------------------------------------
	KSR::Vector3 KSRX::Unproject(KSR::PVIEWPORT pViewport, const KSR::Vector2 &v)
	//-------------------------------------------------------------------
	{
		KSR::ViewportSettings vps;
		pViewport->GetSettings(&vps);

		KSR::Matrix view = pViewport->GetCamera()->GetViewMatrix();

		return KSR::Math::Unproject(v, view, (float)vps.width, (float)vps.height, D3DXToRadian(vps.fov), vps.aspect, vps.nearClipPlane, vps.coordSystemMode);
	}


//-----------------------------------------------------------------------
	KSR::Vector2 KSRX::Project(KSR::PVIEWPORT pViewport, const KSR::Vector3 &v)
	//-------------------------------------------------------------------
	{
		KSR::ViewportSettings vps;
		pViewport->GetSettings(&vps);

		KSR::Matrix view, projection;
		view = pViewport->GetCamera()->GetViewMatrix();
		projection = pViewport->GetProjectionMatrix(0);

		return KSR::Math::Project(v, view, projection, (float)vps.width, (float)vps.height, vps.nearClipPlane, vps.farClipPlane);
	}


//-----------------------------------------------------------------------
	const DWORD KSRX::DivideARGB(const DWORD colour, const unsigned char divisor, bool modifyAlpha)
	//-------------------------------------------------------------------
	{
		unsigned char a = (unsigned char)((colour & 0xff000000) >> 24);
		unsigned char r = (unsigned char)((colour & 0x00ff0000) >> 16);
		unsigned char g = (unsigned char)((colour & 0x0000ff00) >> 8);
		unsigned char b = (unsigned char)(colour & 0x000000ff);

		if (divisor == 0)
		{
			r = g = b = 0;

			if (modifyAlpha)
				a = 0xff;
		}
		else
		{
			r /= divisor;
			g /= divisor;
			b /= divisor;

			if (modifyAlpha)
				a /= divisor;
		}

		return (a << 24) | (r << 16) | (g << 8) | b;
	}


//-----------------------------------------------------------------------
	const DWORD KSRX::MultiplyARGB(const DWORD colour, const unsigned char scalar, bool modifyAlpha)
	//-------------------------------------------------------------------
	{
		unsigned char a = (unsigned char)(colour & 0xff000000) >> 24;
		unsigned char r = (unsigned char)(colour & 0x00ff0000) >> 16;
		unsigned char g = (unsigned char)(colour & 0x0000ff00) >> 8;
		unsigned char b = (unsigned char)(colour & 0x000000ff);

		r *= scalar;
		g *= scalar;
		b *= scalar;

		if (modifyAlpha)
		{
			a *= scalar;
		}

		return (a << 24) | (r << 16) | (g << 8) | b;
	}

// EOF