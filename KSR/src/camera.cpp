#include "../include/ksr.h"

#include "../include/leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

//---------------------------------------------------------------------
	Camera::Camera()
		:	m_transformMode(TT_FREE), m_translationMode(TT_CAMERA), m_rotationMode(TT_CAMERA), m_orbitMode(TT_CAMERA),
			m_position(Vector3(0, 0, 0)), m_orbit(Vector3(0, 0, 0)), m_targetPosition(Vector3(0, 0, 0)),
			m_upVector(Vector3(0, 1, 0)), m_forwardVector(Vector3(0, 0, 1)), m_rightVector(Vector3(1, 0, 0)),
			m_orbitRadius(0), m_numWaypoints(0), m_lastWaypointPos(Vector3(0, 0, 1)), m_lastWaypointRot(Vector3(0, 0, 1))
	//-----------------------------------------------------------------
	{
		AddUsedMemory(sizeof(Camera), _T("Camera::Camera()"));

		D3DXQuaternionIdentity(&m_qOrientation);
	}


//---------------------------------------------------------------------
	Camera::~Camera()
	//-----------------------------------------------------------------
	{
		Logf(_T("Destroying Camera..."));

		FreeUsedMemory(sizeof(Camera), _T("Camera::~Camera()"));
	}


//---------------------------------------------------------------------
	void Camera::SetRotation(const Vector3 &r)
	//-----------------------------------------------------------------
	{
		Quaternion pitch, yaw, roll;

		D3DXQuaternionRotationAxis(&pitch, &m_rightVector, D3DXToRadian(r.x));
		D3DXQuaternionRotationAxis(&yaw, &m_upVector, D3DXToRadian(r.y));
		D3DXQuaternionRotationAxis(&roll, &m_forwardVector, D3DXToRadian(r.z));

		m_qOrientation = yaw * pitch * roll;

		if (m_transformMode == TT_ORBIT)
			m_orbitRadius = Math::Length(m_position - m_targetPosition);
	}


//---------------------------------------------------------------------
	void Camera::SetCoordSystemMode(DWORD mode, const Vector3 &up, const Vector3 &forward)
	//-----------------------------------------------------------------
	{
		m_coordSystemMode = mode;

		m_upVector = Math::Normalise(up);
		m_forwardVector = Math::Normalise(forward);
		m_rightVector = Math::Normalise(Math::Cross(m_upVector, m_forwardVector));

		if (m_coordSystemMode == VS_RIGHTHANDED)
			m_rightVector *= -1;
	}


//---------------------------------------------------------------------
	void Camera::SetTransformMode(DWORD mode)
	//-----------------------------------------------------------------
	{
		m_transformMode = mode;

		if (m_transformMode == TT_ORBIT)
		{
			Vector3 ctXZ = Math::Normalise(m_position - m_targetPosition);
			Vector3 ctYZ = Math::Normalise(Math::Cross(ctXZ, m_upVector));

			Vector3 r = m_rightVector;

			if (m_coordSystemMode == VS_RIGHTHANDED)
				r *= -1;

			float rXZ = (float)acos(Math::Dot(ctYZ, r));
			float rYZ = (float)acos(Math::Dot(ctXZ, m_upVector));

			if (Math::Dot(r, ctXZ) >= 0)
				rXZ = D3DX_PI + (D3DX_PI - rXZ);
			else
				rXZ = rXZ;

			if (Math::Dot(m_upVector, ctXZ) >= 0)
				rYZ = D3DX_PI/2 - rYZ;
			else
				rYZ = -(rYZ - D3DX_PI/2);

			m_orbit = Vector3(D3DXToDegree(rYZ), D3DXToDegree(rXZ), 0);
			m_orbitRadius = Math::Length(m_position - m_targetPosition);
		}
	}


//---------------------------------------------------------------------
	void Camera::Transform(DWORD type, const Vector3 &transform)
	//-----------------------------------------------------------------
	{
		switch (type)
		{
		case TT_TRANSLATE:
			if (m_transformMode == TT_FREE)
			{
				if (m_translationMode == TT_CAMERA)
				{
					Vector3 v;
					Matrix mOrientation;

					D3DXMatrixRotationQuaternion(&mOrientation, &m_qOrientation);

					D3DXVec3TransformNormal(&v, &transform, &mOrientation);

					m_position += v;
				}
				else
					m_position += transform;
			}

			break;

		case TT_ROTATE:
			if (m_transformMode == TT_FREE)
			{
				if (m_rotationMode == TT_CAMERA)
				{
					Vector3 axisX = m_rightVector,
							axisY = m_upVector,
							axisZ = m_forwardVector;

					Matrix mOrientation;
					D3DXMatrixRotationQuaternion(&mOrientation, &m_qOrientation);

					D3DXVec3TransformNormal(&axisX, &axisX, &mOrientation);
					D3DXVec3TransformNormal(&axisZ, &axisZ, &mOrientation);

					axisX = Math::Normalise(axisX);
					axisZ = Math::Normalise(axisZ);

					Quaternion rotX, rotY, rotZ;
					D3DXQuaternionRotationAxis(&rotX, &axisX, D3DXToRadian(transform.x));
					D3DXQuaternionRotationAxis(&rotY, &axisY, D3DXToRadian(transform.y));
					D3DXQuaternionRotationAxis(&rotZ, &axisZ, D3DXToRadian(transform.z));

					m_qOrientation = m_qOrientation * rotX * rotY * rotZ;
				}
				else
				{
					Quaternion rot;
					D3DXQuaternionRotationYawPitchRoll(&rot, D3DXToRadian(transform.y), D3DXToRadian(transform.x), D3DXToRadian(transform.z));

					m_qOrientation = rot * m_qOrientation;
				}
			}
			else if (m_transformMode == TT_ORBIT)
			{
				if (m_orbitMode == TT_CAMERA)
				{
					m_orbit += transform;

					float rXZ = D3DXToRadian(m_orbit.y);
					float rYZ = D3DXToRadian(m_orbit.x);

					Vector3 axisXZ = m_upVector;
					Vector3 axisYZ = m_rightVector;

					Matrix rotX, rotY;
					D3DXMatrixRotationX(&rotX, rYZ);
					D3DXMatrixRotationY(&rotY, rXZ);
					D3DXVec3TransformNormal(&axisXZ, &m_upVector, &rotX);
					//D3DXVec3TransformNormal(&axisYZ, &rightVector, &rotY);

					Quaternion qXZ, qYZ;
					D3DXQuaternionRotationAxis(&qXZ, &axisXZ, rXZ);
					D3DXQuaternionRotationAxis(&qYZ, &axisYZ, rYZ);

					m_position = m_forwardVector * -m_orbitRadius;

					Matrix rot, rotXZ, rotYZ;
					D3DXMatrixRotationAxis(&rotXZ, &axisXZ, rXZ);
					D3DXMatrixRotationAxis(&rotYZ, &axisYZ, rYZ);

					rot = rotYZ * rotXZ;

					D3DXVec3TransformCoord(&m_position, &m_position, &rot);

					m_position += m_targetPosition;
				}
				else
				{
					m_orbit += transform;

					float rXZ = D3DXToRadian(m_orbit.y);
					float rYZ = D3DXToRadian(m_orbit.x);

					if (m_coordSystemMode == VS_RIGHTHANDED)
						rYZ *= -1;

					Quaternion qXZ, qYZ;
					D3DXQuaternionRotationAxis(&qXZ, &m_upVector, rXZ);
					D3DXQuaternionRotationAxis(&qYZ, &m_rightVector, rYZ);

					m_qOrientation = qYZ * qXZ;

					m_position = m_forwardVector * -m_orbitRadius;

					Matrix rot, rotXZ, rotYZ;
					D3DXMatrixRotationAxis(&rotXZ, &m_upVector, rXZ);
					D3DXMatrixRotationAxis(&rotYZ, &m_rightVector, rYZ);

					rot = rotYZ * rotXZ;

					D3DXVec3TransformCoord(&m_position, &m_position, &rot);

					m_position += m_targetPosition;
				}
			}

			break;
		}
	}


//---------------------------------------------------------------------
	const Vector3 Camera::GetDirection() const
	//-----------------------------------------------------------------
	{
		return Math::GetZAxis(GetOrientation());
	}


//---------------------------------------------------------------------
	void Camera::LookAt(const Vector3 &p)
	//-----------------------------------------------------------------
	{
		Vector3 ctXZ = Math::Normalise(m_position - p);
		Vector3 ctYZ = Math::Normalise(Math::Cross(ctXZ, m_upVector));

		Vector3 r = m_rightVector;

		if (m_coordSystemMode == VS_RIGHTHANDED)
			r *= -1;

		float rXZ = (float)acos(Math::Dot(ctYZ, r));
		float rYZ = (float)acos(Math::Dot(ctXZ, m_upVector));

		if (Math::Dot(r, ctXZ) >= 0)
			rXZ = D3DX_PI + (D3DX_PI - rXZ);
		else
			rXZ = rXZ;

		if (Math::Dot(m_upVector, ctXZ) >= 0)
			rYZ = D3DX_PI/2 - rYZ;
		else
			rYZ = -(rYZ - D3DX_PI/2);

		if (m_coordSystemMode == VS_RIGHTHANDED)
			rYZ *= -1;

		Quaternion qXZ, qYZ;
		D3DXQuaternionRotationAxis(&qXZ, &m_upVector, rXZ);
		D3DXQuaternionRotationAxis(&qYZ, &m_rightVector, rYZ);

		m_qOrientation = qYZ * qXZ;
	}


//---------------------------------------------------------------------
	Matrix Camera::GetOrientation() const
	//-----------------------------------------------------------------
	{
		Matrix mOrientation;

		D3DXMatrixRotationQuaternion(&mOrientation, &m_qOrientation);

		return mOrientation;
	}


//---------------------------------------------------------------------
	Matrix Camera::GetViewMatrix() const
	//-----------------------------------------------------------------
	{
		Matrix view, mOrientation;
		D3DXMatrixRotationQuaternion(&mOrientation, &m_qOrientation);

		Vector3 from = m_position;

		Vector3 at = m_forwardVector;
		Vector3 up = m_upVector;

		D3DXVec3TransformNormal(&at, &at, &mOrientation);
		D3DXVec3TransformNormal(&up, &up, &mOrientation);

		at = m_position + at;

		if (m_coordSystemMode == VS_LEFTHANDED)
			D3DXMatrixLookAtLH(&view, &from, &at, &up);
		else if (m_coordSystemMode == VS_RIGHTHANDED)
			D3DXMatrixLookAtRH(&view, &from, &at, &up);

		return view;
	}


//---------------------------------------------------------------------
	void Camera::SetFrustum(const Matrix &cameraFrustum)
	//-----------------------------------------------------------------
	{
		m_frustum = cameraFrustum;

		m_frustumPlanes[0] = Plane(m_frustum._14 + m_frustum._11,
										m_frustum._24 + m_frustum._21,
										m_frustum._34 + m_frustum._31,
										m_frustum._44 + m_frustum._41);
		m_frustumPlanes[1] = Plane(m_frustum._14 - m_frustum._11,
										m_frustum._24 - m_frustum._21,
										m_frustum._34 - m_frustum._31,
										m_frustum._44 - m_frustum._41);
		m_frustumPlanes[2] = Plane(m_frustum._14 - m_frustum._12,
										m_frustum._24 - m_frustum._22,
										m_frustum._34 - m_frustum._32,
										m_frustum._44 - m_frustum._42);
		m_frustumPlanes[3] = Plane(m_frustum._14 + m_frustum._12,
										m_frustum._24 + m_frustum._22,
										m_frustum._34 + m_frustum._32,
										m_frustum._44 + m_frustum._42);
		m_frustumPlanes[4] = Plane(m_frustum._13,
										m_frustum._23,
										m_frustum._33,
										m_frustum._43);
		m_frustumPlanes[5] = Plane(m_frustum._14 + m_frustum._13,
										m_frustum._24 + m_frustum._23,
										m_frustum._34 + m_frustum._33,
										m_frustum._44 + m_frustum._43);
	}


//---------------------------------------------------------------------
	bool Camera::CheckPointFrustum(const Vector3 &point) const
	//-----------------------------------------------------------------
	{
		return Math::PointFrustumCollision(point, m_frustumPlanes);
	}


//---------------------------------------------------------------------
	bool Camera::CheckBoxFrustum(const Vector3 &min, const Vector3 &max) const
	//-----------------------------------------------------------------
	{
		return Math::AABBFrustumCollision(Math::AABB(min, max), m_frustumPlanes);
	}


//---------------------------------------------------------------------
	bool Camera::CheckSphereFrustum(const Vector3 &position, const float &radius) const
	//-----------------------------------------------------------------
	{
		for (int i = 0; i < 6; i++)
		{
			//float d = D3DXVec3Dot(&D3DXVECTOR3(frustumPlanes[i].x, frustumPlanes[i].y, frustumPlanes[i].z), &position);
			float d = 0;

			if (d < -radius)
				return false;

			if ((float)fabs(d) < radius)
				return true;
		}

		return true;
	}

// EOF