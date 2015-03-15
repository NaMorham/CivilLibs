#include "../include/ksr.h"

#include "../include/leakwatcher.h"

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace KSR;

//-----------------------------------------------------------------------
    BoundingVolume::BoundingVolume()
        :    m_sphereRadius(0), m_cylinderRadius(0), m_cylinderHeight(0),
            m_boundingBox(Vector3(0, 0, 0)), m_offset(Vector3(0, 0, 0))
    //-------------------------------------------------------------------
    {
    }


//-----------------------------------------------------------------------
    RigidBody::RigidBody()
        :    m_lastPosition(Vector3(0, 0, 0)), m_lastRotation(Vector3(0, 0, 0)),
            m_velocity(Vector3(0, 0, 0)), m_rotVelocity(Vector3(0, 0, 0)),
            m_acceleration(Vector3(0, 0, 0)), m_rotAcceleration(Vector3(0, 0, 0)),
            m_force(Vector3(0, 0, 0)), m_torque(Vector3(0, 0, 0)),
            m_centerOfMass(Vector3(0, 0, 0)),
            m_mass(0)
    //-------------------------------------------------------------------
    {
        m_boundingVolumes.clear();
    }


//-----------------------------------------------------------------------
    RigidBody::~RigidBody()
    //-------------------------------------------------------------------
    {
        m_boundingVolumes.clear();
    }


//-----------------------------------------------------------------------
    Entity::Entity()
    //-------------------------------------------------------------------
    {
        //FreeUsedMemory(sizeof(Entity));
        AddUsedMemory(sizeof(Entity), "Entity::Entity()");

        m_visualId = -1;

        m_redraw = RT_LOOP;
        m_update = RT_LOOP;

        m_position = m_rotation = m_velocity = m_angularVelocity = Vector3(0, 0, 0);

        m_scale = Vector3(1, 1, 1);

        D3DXMatrixIdentity(&m_localTransform);
        m_absoluteTransform = m_localTransform;

        m_pParent = NULL;

        m_localTransformRequiresUpdate = false;

        m_pBody = NULL;
    }


//-----------------------------------------------------------------------
    Entity::~Entity()
    //-------------------------------------------------------------------
    {
        Logf("Destroying Entity...");

        FreeUsedMemory(sizeof(Entity), "Entity::~Entity()");

        if (m_pBody)
        {
            delete m_pBody;
            FreeUsedMemory(sizeof(RigidBody), "Entity::~Entity() - RigidBody");
        }
    }


//-----------------------------------------------------------------------
    HRESULT Entity::Update(const float deltaTime)
    //-------------------------------------------------------------------
    {
        if (m_update == RT_ONCE)
            m_update = RT_NONE;

        m_position += m_velocity * deltaTime;
        m_rotation += m_angularVelocity * deltaTime;

        m_localTransformRequiresUpdate = true;

        std::list<Entity *>::iterator i = m_children.begin();
        for (; i != m_children.end(); i++)
        {
            if (!(*i))
                continue;

            if ((*i)->GetUpdate())
                (*i)->Update(deltaTime);
        }

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Entity::Serialise(OutputEntity *pOutputEntity, std::vector< std::pair<PENTITY, int> > &entityMap)
    //-------------------------------------------------------------------
    {
        if (!pOutputEntity)
            return E_FAIL;

        pOutputEntity->size = sizeof(OutputEntity) + m_children.size() * sizeof(int);

        pOutputEntity->parentID = -1;
        pOutputEntity->numChildren = m_children.size();

        if (pOutputEntity->numChildren)
        {
            pOutputEntity->pChildIDs = new int[m_children.size()];

            memset(pOutputEntity->pChildIDs, -1, sizeof(int) * m_children.size());

            std::vector< std::pair<PENTITY, int> >::iterator e = entityMap.begin();
            for (; e != entityMap.end(); e++)
            {
                if (e->first == m_pParent)
                    pOutputEntity->parentID = e->second;

                std::list<PENTITY>::iterator c = m_children.begin();
                for (int i = 0; c != m_children.end(); i++, c++)
                {
                    if (e->first == (*c))
                        pOutputEntity->pChildIDs[i] = e->second;
                }
            }
        }
        else
            pOutputEntity->pChildIDs = NULL;

        pOutputEntity->position = m_position;
        pOutputEntity->rotation = m_rotation;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Entity::Reassemble(OutputEntity *pOutputEntity, std::vector< std::pair<PENTITY, int> > &entityMap)
    //-------------------------------------------------------------------
    {
        if (!pOutputEntity)
            return E_FAIL;

        m_pParent = NULL;

        std::vector< std::pair<PENTITY, int> >::iterator e = entityMap.begin();
        for (; e != entityMap.end(); e++)
        {
            if (e->second == pOutputEntity->parentID)
                m_pParent = e->first;

            for (int i = 0; i < pOutputEntity->numChildren; i++)
            {
                if (e->second == pOutputEntity->pChildIDs[i])
                {
                    m_children.push_back(e->first);
                    break;
                }
            }
        }

        pOutputEntity->position = m_position;
        pOutputEntity->rotation = m_rotation;

        return S_OK;
    }


//-----------------------------------------------------------------------
    void Entity::AttachParent(Entity *pParent, bool propagate)
    //-------------------------------------------------------------------
    {
        if (!pParent)
            return;

        if (propagate)
            pParent->AttachChild(this, false);

        m_pParent = pParent;
    }


//-----------------------------------------------------------------------
    void Entity::DetachParent(bool propagate)
    //-------------------------------------------------------------------
    {
        if (!m_pParent)
            return;

        if (propagate)
            m_pParent->DetachChild(this, false);

        m_pParent = NULL;
    }


//-----------------------------------------------------------------------
    void Entity::AttachChild(Entity *pChild, bool propagate)
    //-------------------------------------------------------------------
    {
        if (!pChild)
            return;

        if (propagate)
            pChild->AttachParent(this, false);

        m_children.push_back(pChild);
    }


//-----------------------------------------------------------------------
    void Entity::DetachChild(Entity *pChild, bool propagate)
    //-------------------------------------------------------------------
    {
        std::list<Entity *>::iterator i = m_children.begin();

        for (; i != m_children.end();)
        {
            if ((*i) == pChild)
            {
                if (propagate)
                    (*i)->DetachParent(false);

                i = m_children.erase(i);
            }
            else
            {
                i++;
            }
        }
    }


//-----------------------------------------------------------------------
    void Entity::DetachChildren()
    //-------------------------------------------------------------------
    {
        std::list<Entity *>::iterator i = m_children.begin();

        for (; i != m_children.end();)
        {
            (*i)->DetachParent(false);

            i = m_children.erase(i);
        }
    }


//-----------------------------------------------------------------------
    HRESULT Entity::SetCollisionGeometry(PGEOMETRY pGeometry)
    //-------------------------------------------------------------------
    {
        if (!pGeometry)
            return E_FAIL;

        return S_OK;
    }


//-----------------------------------------------------------------------
    HRESULT Entity::AddBoundingVolume(BoundingVolume *pBoundingVolume)
    //-------------------------------------------------------------------
    {
        if (!m_pBody)
            return E_FAIL;

        m_pBody->m_boundingVolumes.push_back(*pBoundingVolume);

        float radius = 0;
        if (pBoundingVolume->m_sphereRadius)
        {
            radius = Math::Length(pBoundingVolume->m_offset) + pBoundingVolume->m_sphereRadius;
        }
        else if (pBoundingVolume->m_cylinderHeight && pBoundingVolume->m_cylinderRadius)
        {
            radius = Math::Length(pBoundingVolume->m_offset + Vector3(pBoundingVolume->m_cylinderRadius,
                                                              pBoundingVolume->m_cylinderHeight, 0));
        }
        else if (pBoundingVolume->m_boundingBox)
        {
            Vector3 min = pBoundingVolume->m_offset - pBoundingVolume->m_boundingBox * 0.5f;
            Vector3 max = pBoundingVolume->m_offset + pBoundingVolume->m_boundingBox * 0.5f;

            Vector3 points[8];
            points[0] = Vector3(min.x, min.y, min.z);
            points[1] = Vector3(max.x, min.y, min.z);
            points[2] = Vector3(min.x, min.y, max.z);
            points[3] = Vector3(max.x, min.y, max.z);
            points[4] = Vector3(min.x, max.y, min.z);
            points[5] = Vector3(max.x, max.y, min.z);
            points[6] = Vector3(min.x, max.y, max.z);
            points[7] = Vector3(max.x, max.y, max.z);

            for (int i = 0; i < 8; i++)
            {
                float length = Math::Length(points[i]);
                if (length > radius)
                    radius = length;
            }
        }

        if (radius > m_pBody->m_totalBoundingRadius)
            m_pBody->m_totalBoundingRadius = radius;

        return S_OK;
    }


//-----------------------------------------------------------------------
    void Entity::ApplyImpulse(const Vector3 &position, const Vector3 &direction)
    //-------------------------------------------------------------------
    {
        if (!m_pBody)
            return;

        m_pBody->m_velocity += direction / m_pBody->m_mass;
    }


//-----------------------------------------------------------------------
    void Entity::SetPosition(const Vector3 &p)
    //-------------------------------------------------------------------
    {
        m_localTransformRequiresUpdate = true;

        m_position = p;

        if (m_pBody)
            m_pBody->m_lastPosition = p;
    }


//-----------------------------------------------------------------------
    void Entity::SetRotation(const Vector3 &r)
    //-------------------------------------------------------------------
    {
        m_localTransformRequiresUpdate = true;

        m_rotation = r;

        if (m_pBody)
            m_pBody->m_lastRotation = r;
    }


//-----------------------------------------------------------------------
    void Entity::SetScale(float s)
    //-------------------------------------------------------------------
    {
        SetScale(Vector3(s, s, s));
    }


//-----------------------------------------------------------------------
    void Entity::SetScale(const Vector3 &s)
    //-------------------------------------------------------------------
    {
        m_localTransformRequiresUpdate = true;

        m_scale = s;
    }


//-----------------------------------------------------------------------
    void Entity::SetVelocity(const Vector3 &v)
    //-------------------------------------------------------------------
    {
        m_velocity = v;
    }


//-----------------------------------------------------------------------
    void Entity::SetAngularVelocity(const Vector3 &v)
    //-------------------------------------------------------------------
    {
        m_angularVelocity = v;
    }


//-----------------------------------------------------------------------
    void Entity::AddVelocity(const Vector3 &v)
    //-------------------------------------------------------------------
    {
        m_velocity += v;
    }


//-----------------------------------------------------------------------
    void Entity::AddAngularVelocity(const Vector3 &v)
    //-------------------------------------------------------------------
    {
        m_angularVelocity += v;
    }


//-----------------------------------------------------------------------
    void Entity::SetForce(const Vector3 &force)
    //-------------------------------------------------------------------
    {
        if (m_pBody)
            m_pBody->m_force = force;
    }


//-----------------------------------------------------------------------
    void Entity::SetTorque(const Vector3 &torque)
    //-------------------------------------------------------------------
    {
        if (m_pBody)
            m_pBody->m_torque = torque;
    }


//-----------------------------------------------------------------------
    void Entity::SetMass(float mass)
    //-------------------------------------------------------------------
    {
        if (m_pBody)
            m_pBody->m_mass = mass;
    }


//-----------------------------------------------------------------------
    void Entity::SetRedraw(DWORD r)
    //-------------------------------------------------------------------
    {
        m_redraw = r;
    }


//-----------------------------------------------------------------------
    void Entity::SetUpdate(DWORD u)
    //-------------------------------------------------------------------
    {
        m_update = u;
    }


//-----------------------------------------------------------------------
    void Entity::SetVisualID(int id)
    //-------------------------------------------------------------------
    {
        m_visualId = id;
    }


//-----------------------------------------------------------------------
    const Vector3 &Entity::GetPosition() const
    //-------------------------------------------------------------------
    {
        return m_position;
    }


//-----------------------------------------------------------------------
    const Vector3 &Entity::GetRotation() const
    //-------------------------------------------------------------------
    {
        return m_rotation;
    }


//-----------------------------------------------------------------------
    Vector3 Entity::GetVelocity() const
    //-------------------------------------------------------------------
    {
        if (!m_pBody)
            return Vector3(0, 0, 0);

        return m_pBody->m_velocity;
    }


//-----------------------------------------------------------------------
    Vector3 Entity::GetAngularVelocity() const
    //-------------------------------------------------------------------
    {
        if (!m_pBody)
            return Vector3(0, 0, 0);

        return m_pBody->m_rotVelocity;
    }


//-----------------------------------------------------------------------
    Vector3 Entity::GetForce() const
    //-------------------------------------------------------------------
    {
        if (!m_pBody)
            return Vector3(0, 0, 0);

        return m_pBody->m_force;
    }


//-----------------------------------------------------------------------
    Vector3 Entity::GetTorque() const
    //-------------------------------------------------------------------
    {
        if (!m_pBody)
            return Vector3(0, 0, 0);

        return m_pBody->m_torque;
    }


//-----------------------------------------------------------------------
    float Entity::GetMass() const
    //-------------------------------------------------------------------
    {
        if (!m_pBody)
            return 0;

        return m_pBody->m_mass;
    }


//-----------------------------------------------------------------------
    DWORD Entity::GetRedraw() const
    //-------------------------------------------------------------------
    {
        return m_redraw;
    }


//-----------------------------------------------------------------------
    DWORD Entity::GetUpdate() const
    //-------------------------------------------------------------------
    {
        return m_update;
    }


//-----------------------------------------------------------------------
    int Entity::GetVisualID() const
    //-------------------------------------------------------------------
    {
        return m_visualId;
    }


//-----------------------------------------------------------------------
    RigidBody *Entity::GetRigidBody() const
    //-------------------------------------------------------------------
    {
        return m_pBody;
    }


//-----------------------------------------------------------------------
    void Entity::CalculateLocalTransformMatrix()
    //-------------------------------------------------------------------
    {
        if (!m_localTransformRequiresUpdate)
            return;

        Matrix world, rot;

        D3DXMatrixTranslation(&world, m_position.x, m_position.y, m_position.z);

        D3DXMatrixRotationYawPitchRoll(&rot, D3DXToRadian(m_rotation.y),
                                             D3DXToRadian(m_rotation.x),
                                             D3DXToRadian(m_rotation.z));

        if (m_scale != Vector3(1, 1, 1))
        {
            Matrix scale;
            D3DXMatrixScaling(&scale, m_scale.x, m_scale.y, m_scale.z);

            m_localTransform = scale * rot * world;
        }
        else
        {
            m_localTransform = rot * world;
        }

        m_localTransformRequiresUpdate = false;
    }


//-----------------------------------------------------------------------
    void Entity::CalculateAbsoluteTransformMatrix()
    //-------------------------------------------------------------------
    {
        CalculateLocalTransformMatrix();

        if (!m_pParent)
        {
            m_absoluteTransform = m_localTransform;

            return;
        }

        const Matrix &parentTransform = m_pParent->GetAbsoluteTransformMatrix();

        m_absoluteTransform = m_localTransform * parentTransform;
    }


//-----------------------------------------------------------------------
    const Matrix &Entity::GetLocalTransformMatrix()
    //-------------------------------------------------------------------
    {
        if (m_localTransformRequiresUpdate)
            CalculateLocalTransformMatrix();

        return m_localTransform;
    }


//-----------------------------------------------------------------------
    const Matrix &Entity::GetAbsoluteTransformMatrix()
    //-------------------------------------------------------------------
    {
        CalculateAbsoluteTransformMatrix();

        return m_absoluteTransform;
    }

// EOF