/*! \file */
/*-----------------------------------------------------------------------
    camera.h

    Description: Camera class definition
    Author:         Julian McKinlay
    ---------------------------------------------------------------------*/

#pragma once

enum { TT_FREE = 0, TT_ORBIT, TT_FOLLOWTARGET };
enum { TT_WORLD = 0, TT_CAMERA };
enum { TT_TRANSLATE = 0, TT_ROTATE };

struct CameraWaypoint
{
    CameraWaypoint(const Vector3 &position = Vector3(0, 0, 0), const Vector3 &rotation = Vector3(0, 0, 0), const float &time = 0.0f)
        : m_time(time), m_position(position), m_rotation(rotation) {}

    Vector3 m_position, m_rotation;
    float m_time;
};

struct Camera : public MemObject
{
public:
    Camera();
    ~Camera();

    void AddWaypoint(const CameraWaypoint &waypoint);
    void AddWaypoint(const Vector3 &pos, const Vector3 &rot, const float &t) { AddWaypoint(CameraWaypoint(pos, rot, t)); }
    void ClearWaypoints() { m_numWaypoints = 0; };

    void SetFrustum(const Matrix &cameraFrustum);

    bool CheckPointFrustum(const Vector3 &point) const;
    bool CheckSphereFrustum(const Vector3 &position, const float &radius) const;
    bool CheckBoxFrustum(const Vector3 &min, const Vector3 &max) const;

    void Transform(DWORD type, const Vector3 &transform);

    void SetTransformMode(DWORD mode);
    void SetTarget(const Vector3 &target) { m_targetPosition = target; };
//    void SetTarget(Entity *target) { targetEntity = target; };
    void SetTranslationMode(DWORD mode) { m_translationMode = mode; };
    void SetRotationMode(DWORD mode) { m_rotationMode = mode; };
    void SetOrbitMode(DWORD mode) { m_orbitMode = mode; };
    void SetCoordSystemMode(DWORD coordSystemMode, const Vector3 &up, const Vector3 &forward);

    void SetPosition(const Vector3 &p) { m_position = p; };
    void SetRotation(const Vector3 &r);
    void LookAt(const Vector3 &p);

    const Matrix &GetFrustum() const { return m_frustum; };
    DWORD GetTransformMode() const { return m_transformMode; };
    const Vector3 &GetPosition() const { return m_position; };
    const Vector3 GetDirection() const;
    const Vector3 &GetTarget() const { return m_targetPosition; };
//    Entiity *GetTarget() const { return targetEntity; };
    Matrix GetOrientation() const;
    Matrix GetViewMatrix() const;
    int GetNumWaypoints() const { return m_numWaypoints; };

    const Vector3 &GetUpVector() const        { return m_upVector; };
    const Vector3 &GetForwardVector() const    { return m_forwardVector; };

private:
    DWORD m_transformMode,
          m_translationMode,
          m_rotationMode,
          m_orbitMode,
          m_coordSystemMode;

    Vector3 m_position,
            m_orbit,
            m_targetPosition,
            m_upVector,
            m_forwardVector,
            m_rightVector;

    Quaternion m_qOrientation;

    float m_orbitRadius;
    float m_deceleration, m_spinAmt;

    int m_numWaypoints;
    Vector3 m_lastWaypointPos, m_lastWaypointRot;

    Matrix m_frustum;
    Plane m_frustumPlanes[6];
};

typedef struct Camera *PCAMERA;

// EOF