/*! \file */
/*-----------------------------------------------------------------------
    light.h

    Description: KSRLight class declaration
    Author:         Julian McKinlay
    ---------------------------------------------------------------------*/

#pragma once

enum LIGHTTYPE
{
    LT_DIRECTIONAL = D3DLIGHT_DIRECTIONAL,    //!< Directional Light. For more information, see \ref lighting.
    LT_POINT = D3DLIGHT_POINT,                //!< Point Light. For more information, see \ref lighting.
    LT_SPOT = D3DLIGHT_SPOT,                //!< Spot Light. For more information, see \ref lighting.
};

//! \struct Light
struct Light
{
    // Directional constructor
    Light(const Vector3 &direction = Vector3(0, -1, 0), DWORD diffuseColour = 0xffafafaf,
          DWORD specularColour = 0xffffffff, DWORD ambientColour = 0xff7f7f7f, LIGHTTYPE type = LT_DIRECTIONAL);

    // Point constructors
    Light(const Vector3 &position, DWORD diffuseColour, DWORD specularColour, DWORD ambientColour, float range,
          float attenuation0, float attenuation1, float attenuation2, LIGHTTYPE type = LT_POINT);

    Light(const Vector3 &position, DWORD diffuseColour, DWORD specularColour, DWORD ambientColour, float range,
          const Vector3 &attenuation, LIGHTTYPE type = LT_POINT);

    // Spot constructors
    Light(const Vector3 &position, const Vector3 &direction, DWORD diffuseColour, DWORD specularColour, DWORD ambientColour,
          float range, float falloff, float attenuation0, float attenuation1, float attenuation2, float innerConeAngle, float outerConeAngle,
          LIGHTTYPE type = LT_SPOT);

    Light(const Vector3 &position, const Vector3 &direction, DWORD diffuseColour, DWORD specularColour, DWORD ambientColour,
          float range, float falloff, const Vector3 &attenuation, float innerConeAngle, float outerConeAngle,
          LIGHTTYPE type = LT_SPOT);

    Light(const Light &rhs);
    const Light &operator =(const Light &rhs);

    operator D3DLIGHT9 *() { return &m_light; }
    operator const D3DLIGHT9 *() const { return &m_light; }

    operator D3DLIGHT9 &() { return m_light; }
    operator const D3DLIGHT9 &() const { return m_light; }

private:

    D3DLIGHT9 m_light;

    /*Vector3 m_position,
            m_direction;

    LIGHTTYPE m_type;

    DWORD m_diffuseColour,
          m_specularColour,
          m_ambientColour;

    float m_range,
          m_falloff,
          m_attenuation0,
          m_attenuation1,
          m_attenuation2,
          m_theta,
          m_phi;*/
};

//! \typedef Light *PLIGHT
typedef Light *PLIGHT;

// EOF