#include "../include/ksr.h"

#include "../include/leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

//-----------------------------------------------------------------------
    Light::Light(const Vector3 &direction, DWORD diffuseColour, DWORD specularColour, DWORD ambientColour, LIGHTTYPE type)
    //-------------------------------------------------------------------
    {
        m_light.Position = Vector3(0, 0, 0);
        m_light.Direction = direction;

        m_light.Diffuse = D3DXCOLOR(diffuseColour);
        m_light.Specular = D3DXCOLOR(specularColour);
        m_light.Ambient = D3DXCOLOR(ambientColour);

        m_light.Type = (D3DLIGHTTYPE)type;

        m_light.Range = m_light.Falloff = 0.0f;
        m_light.Attenuation0 = m_light.Attenuation1 = m_light.Attenuation2 = 0.0f;
        m_light.Theta = m_light.Phi = 0.0f;
    }


//-----------------------------------------------------------------------
    Light::Light(const Vector3 &position, DWORD diffuseColour, DWORD specularColour, DWORD ambientColour, float range,
                 float attenuation0, float attenuation1, float attenuation2, LIGHTTYPE type)
    //-------------------------------------------------------------------
    {
        m_light.Position = position;
        m_light.Direction = Vector3(0, 0, 0);

        m_light.Diffuse = D3DXCOLOR(diffuseColour);
        m_light.Specular = D3DXCOLOR(specularColour);
        m_light.Ambient = D3DXCOLOR(ambientColour);

        m_light.Type = (D3DLIGHTTYPE)type;

        m_light.Range = range;
        m_light.Falloff = 0.0f;

        m_light.Attenuation0 = attenuation0;
        m_light.Attenuation1 = attenuation1;
        m_light.Attenuation2 = attenuation2;
        m_light.Theta = m_light.Phi = 0.0f;
    }


//-----------------------------------------------------------------------
    Light::Light(const Vector3 &position, DWORD diffuseColour, DWORD specularColour, DWORD ambientColour, float range,
                 const Vector3 &attenuation, LIGHTTYPE type)
    //-------------------------------------------------------------------
    {
        m_light.Position = position;
        m_light.Direction = Vector3(0, 0, 0);

        m_light.Diffuse = D3DXCOLOR(diffuseColour);
        m_light.Specular = D3DXCOLOR(specularColour);
        m_light.Ambient = D3DXCOLOR(ambientColour);

        m_light.Type = (D3DLIGHTTYPE)type;

        m_light.Range = range;
        m_light.Falloff = 0.0f;

        m_light.Attenuation0 = attenuation.x;
        m_light.Attenuation1 = attenuation.y;
        m_light.Attenuation2 = attenuation.z;
        m_light.Theta = m_light.Phi = 0.0f;
    }


//-----------------------------------------------------------------------
    Light::Light(const Vector3 &position, const Vector3 &direction, DWORD diffuseColour, DWORD specularColour, DWORD ambientColour,
                 float range, float falloff, float attenuation0, float attenuation1, float attenuation2, float innerConeAngle, float outerConeAngle,
                 LIGHTTYPE type)
    //-------------------------------------------------------------------
    {
        m_light.Position = position;
        m_light.Direction = direction;

        m_light.Diffuse = D3DXCOLOR(diffuseColour);
        m_light.Specular = D3DXCOLOR(specularColour);
        m_light.Ambient = D3DXCOLOR(ambientColour);

        m_light.Type = (D3DLIGHTTYPE)type;

        m_light.Range = range;
        m_light.Falloff = falloff;

        m_light.Attenuation0 = attenuation0;
        m_light.Attenuation1 = attenuation1;
        m_light.Attenuation2 = attenuation2;
        m_light.Theta = innerConeAngle;
        m_light.Phi = outerConeAngle;
    }


//-----------------------------------------------------------------------
    Light::Light(const Vector3 &position, const Vector3 &direction, DWORD diffuseColour, DWORD specularColour, DWORD ambientColour,
                 float range, float falloff, const Vector3 &attenuation, float innerConeAngle, float outerConeAngle,
                 LIGHTTYPE type)
    //-------------------------------------------------------------------
    {
        m_light.Position = position;
        m_light.Direction = direction;

        m_light.Diffuse = D3DXCOLOR(diffuseColour);
        m_light.Specular = D3DXCOLOR(specularColour);
        m_light.Ambient = D3DXCOLOR(ambientColour);

        m_light.Type = (D3DLIGHTTYPE)type;

        m_light.Range = range;
        m_light.Falloff = falloff;

        m_light.Attenuation0 = attenuation.x;
        m_light.Attenuation1 = attenuation.y;
        m_light.Attenuation2 = attenuation.z;
        m_light.Theta = innerConeAngle;
        m_light.Phi = outerConeAngle;
    }


//-----------------------------------------------------------------------
    Light::Light(const Light &rhs)
    //-------------------------------------------------------------------
    {
        m_light.Position = rhs.m_light.Position;
        m_light.Direction = rhs.m_light.Direction;

        m_light.Diffuse = rhs.m_light.Diffuse;
        m_light.Specular = rhs.m_light.Specular;
        m_light.Ambient = rhs.m_light.Ambient;

        m_light.Type = rhs.m_light.Type;

        m_light.Range = rhs.m_light.Range;
        m_light.Falloff = rhs.m_light.Falloff;
        m_light.Attenuation0 = rhs.m_light.Attenuation0;
        m_light.Attenuation1 = rhs.m_light.Attenuation1;
        m_light.Attenuation2 = rhs.m_light.Attenuation2;
        m_light.Theta = rhs.m_light.Theta;
        m_light.Phi = rhs.m_light.Phi;
    }


//-----------------------------------------------------------------------
    const Light &Light::operator =(const Light &rhs)
    //-------------------------------------------------------------------
    {
        m_light.Position = rhs.m_light.Position;
        m_light.Direction = rhs.m_light.Direction;

        m_light.Diffuse = rhs.m_light.Diffuse;
        m_light.Specular = rhs.m_light.Specular;
        m_light.Ambient = rhs.m_light.Ambient;

        m_light.Type = rhs.m_light.Type;

        m_light.Range = rhs.m_light.Range;
        m_light.Falloff = rhs.m_light.Falloff;
        m_light.Attenuation0 = rhs.m_light.Attenuation0;
        m_light.Attenuation1 = rhs.m_light.Attenuation1;
        m_light.Attenuation2 = rhs.m_light.Attenuation2;
        m_light.Theta = rhs.m_light.Theta;
        m_light.Phi = rhs.m_light.Phi;

        return *this;
    }

// EOF