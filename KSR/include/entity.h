/*! \file */
/*-----------------------------------------------------------------------
	entity.h

	Description: KSREntity class declaration
	Author:		 Julian McKinlay
	---------------------------------------------------------------------*/

#pragma once

//! \enum
enum COLLISIONTYPE
{
	CT_BODYWORLD = 0,	/**< A collision between a body and static geometry. */
	CT_BODYBODY,		/**< A collision between two bodies. */
};

struct CollisionInfo;
struct BoundingVolume;
struct SurfaceMaterial;
struct RigidBody;
struct Entity;
struct OutputEntity;

//! \struct
struct CollisionInfo
{
	DWORD m_type;

	Vector3 m_contactPointBody[2];
	Vector3 m_contactPointWorld;
	Vector3 m_relativeVelocity;
	Vector3 m_collisionNormal;
	Vector3 m_collisionTangent;

	float m_relativeNormalVelocity;

	int m_bodyID[2];
	Entity *m_pBody[2];
};

struct BoundingVolume
{
	BoundingVolume();

	float m_sphereRadius;

	float m_cylinderHeight;
	float m_cylinderRadius;

	Vector3 m_boundingBox;
	Vector3 m_offset;
};


struct SurfaceMaterial
{
	float m_friction;
	float m_restitution;
};


struct RigidBody
{
public:
	RigidBody();
	~RigidBody();

	Vector3 m_lastPosition;
	Vector3 m_lastRotation;

	Vector3 m_velocity;
	Vector3 m_rotVelocity;

	Vector3 m_acceleration;
	Vector3 m_rotAcceleration;

	Vector3 m_force;
	Vector3 m_torque;

	Vector3 m_centerOfMass;

	float m_mass;
	float m_totalBoundingRadius;

	std::vector<BoundingVolume> m_boundingVolumes;
};


//! \struct Entity
struct Entity : public MemObject
{
public:
	Entity();
	virtual ~Entity();

	HRESULT Serialise(OutputEntity *pOutputEntity, std::vector< std::pair<Entity *, int> > &entityMap);
	HRESULT Reassemble(OutputEntity *pOutputEntity, std::vector< std::pair<Entity *, int> > &entityMap);

	virtual HRESULT Update(const float deltaTime);

	//! Attaches the parent of this Entity
		/*! \param pParent [in] Address of an Entity structure, representing the entity to set.
			\param propagate True if the event should propagate to its children. False otherwise. Defaults to true.*/
	virtual void AttachParent(Entity *pParent, bool propagate = true);

	//! Detaches the parent of this Entity
		/*!	\param propagate True if the event should propagate to its children. False otherwise. Defaults to true.*/
	virtual void DetachParent(bool propagate = true);

	//! Attaches a specified child to this Entity
		/*! \param pParent [in] Address of an Entity structure, representing the entity to attach.
			\param propagate True if the event should propagate to its parent. False otherwise. Defaults to true.*/
	virtual void AttachChild(Entity *pChild, bool propagate = true);

	//! Detaches a specified child from this Entity
		/*! \param pParent [in] Address of an Entity structure, representing the entity to detach.
			\param propagate True if the event should propagate to its parent. False otherwise. Defaults to true.*/
	virtual void DetachChild(Entity *pChild, bool propagate = true);

	//! Detaches all children from this Entity
	virtual void DetachChildren();

	virtual HRESULT SetCollisionGeometry(PGEOMETRY pGeometry);
	virtual HRESULT AddBoundingVolume(BoundingVolume *pBoundingVolume);

	virtual void ApplyImpulse(const Vector3 &position, const Vector3 &direction);

	virtual void AddVelocity(const Vector3 &v);
	virtual void AddAngularVelocity(const Vector3 &v);

	virtual void SetPosition(const Vector3 &p);
	virtual void SetRotation(const Vector3 &r);
	virtual void SetScale(float s);
	virtual void SetScale(const Vector3 &s);
	virtual void SetVelocity(const Vector3 &v);
	virtual void SetAngularVelocity(const Vector3 &v);
	virtual void SetForce(const Vector3 &force);
	virtual void SetTorque(const Vector3 &torque);

	virtual void SetMass(float mass);

	virtual void SetRedraw(DWORD r);
	virtual void SetUpdate(DWORD u);

	virtual void SetVisualID(int id);

	virtual const Vector3 &GetPosition() const;
	virtual const Vector3 &GetRotation() const;
	virtual Vector3 GetVelocity() const;
	virtual Vector3 GetAngularVelocity() const;

	virtual Vector3 GetForce() const;
	virtual Vector3 GetTorque() const;

	virtual float GetMass() const;

	virtual DWORD GetRedraw() const;
	virtual DWORD GetUpdate() const;

	virtual int GetVisualID() const;

	virtual RigidBody *GetRigidBody() const;

	virtual const Matrix &GetLocalTransformMatrix();
	virtual const Matrix &GetAbsoluteTransformMatrix();

	Entity *m_pParent;
	std::list<Entity *> m_children;

private:
	void CalculateLocalTransformMatrix();
	void CalculateAbsoluteTransformMatrix();

	DWORD m_redraw,
		  m_update;

	int m_visualId;

	Vector3 m_position;
	Vector3 m_rotation;
	Vector3 m_velocity;
	Vector3 m_angularVelocity;
	Vector3 m_scale;

	Matrix m_localTransform;
	Matrix m_absoluteTransform;

	bool m_localTransformRequiresUpdate;

	RigidBody *m_pBody;
};

//! \typedef Entity *PENTITY
typedef Entity *PENTITY;

// EOF