#include "base.h"
#include "memobject.h"
#include "settings.h"
#include "camera.h"
#include "geometry.h"
#include "visual.h"
#include "resource.h"
#include "entity.h"
#include "font.h"
#include "viewport.h"
#include "patch.h"

#include <LeakWatcher.h>

#ifdef _DO_MEMORY_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------
	SceneGraph::SceneGraph(LPDIRECT3DDEVICE9 pDevice, LPRESOURCEMANAGER pResourceManager)
	//-------------------------------------------------------------------
	{
		AddUsedMemory(sizeof(SceneGraph), "SceneGraph::SceneGraph()");

		D3DDevice = pDevice;
		resourceManager = pResourceManager;

		
	}


//-----------------------------------------------------------------------
	void SceneGraph::SetCollisionEfficiency(float tolerance, float precision)
	//-------------------------------------------------------------------
	{
		collisionTolerance = tolerance;
		collisionPrecision = 1.0f / pow(10, precision);

		SetCollisionTolerance(collisionTolerance);
		SetCollisionPrecision(collisionPrecision);
	}


//-----------------------------------------------------------------------
	HRESULT SceneGraph::Render(LPKSRVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		leafsDrawn.resize(leafs.size());
		leafsDrawn.clear();

		facesDrawn.resize(geometry->numChunks);
		facesDrawn.clear();

		//if (patches.size())
			patchGeometry->Init(sizeof(VERTEX_NORMAL_TEX1), FVF_NORMAL_TEX1, IT_32);

		D3DXMATRIX identity;
		D3DXMatrixIdentity(&identity);
		D3DDevice->SetTransform(D3DTS_WORLD, &identity);

		D3DDevice->GetRenderState(D3DRS_DEPTHBIAS, (DWORD*)&currentDepthBias);

		if (pViewport->GetSettings()->drawSpacePartition)
		{
			if (outline->GetNumIndices())
			{
				D3DDevice->SetTexture(0, NULL);
				D3DDevice->SetTexture(1, NULL);
				D3DDevice->SetStreamSource(0, outline->GetVertexBuffer(), 0, outline->vertexLength);
				D3DDevice->SetIndices(outline->GetIndexBuffer());
				D3DDevice->SetFVF(outline->FVF);
				D3DDevice->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, numOutlineVerts, 0, numOutlineVerts / 2);
			}
			else
				Logf("Error: Space Partition can not be drawn.");
		}

		D3DDevice->SetStreamSource(0, geometry->GetVertexBuffer(), 0, geometry->vertexLength);
		D3DDevice->SetIndices(geometry->GetIndexBuffer());
		D3DDevice->SetFVF(geometry->FVF);

		if (visibilityType == SS_ENABLED)
		{
			for (int i = 0; i < leafs.size(); i++)
			{
				Vector3 min = leafs[i]->min;
				Vector3 max = leafs[i]->max;
				Vector3 c = pViewport->GetCamera()->GetPosition();

				// If camera is in leaf
				if (c.x >= min.x && c.x < max.x &&
					c.y >= min.y && c.y < max.y &&
					c.z >= min.z && c.z < max.z)
				{
					for (int n = 0; n < leafs.size(); n++)
					{
						//if (!(PVS[i * bytesPerCluster + (n/8)] & (1 << (n & 7))))
						if (PVS[i * bytesPerCluster + (n/8)] & (1 << (n & 7)))
						{
							Vector3 cMin = leafs[n]->min;
							Vector3 cMax = leafs[n]->max;

							if (!pViewport->GetCamera()->CheckBoxFrustum(cMin, cMax))
								continue;

							RenderLeaf(leafs[n], pViewport);
						}
					}

					// Camera can only be in one leaf
					break;
				}
			}
		}
		else
			RenderNode(root, pViewport);

		// Render patchGeometry
			if (patchGeometry->numChunks)
			{
				D3DDevice->SetStreamSource(0, patchGeometry->GetVertexBuffer(), 0, patchGeometry->vertexLength);
				D3DDevice->SetIndices(patchGeometry->GetIndexBuffer());
				D3DDevice->SetFVF(patchGeometry->FVF);

				for (int i = 0; i < patchGeometry->numChunks; i++)
					RenderChunk(&patchGeometry->chunks[i]);
			}

		// Render entities
		std::list<KSREntity *>::iterator e = entities.begin();
		for (; e != entities.end(); e++)
		{
			if ((*e)->GetRedraw())
			{
				if ((*e)->GetRedraw() == RT_ONCE)
					(*e)->SetRedraw(RT_NONE);

				int visualId = (*e)->GetVisualID();

				if (visualId > -1)
				{
					KSRVisual *visual = resourceManager->GetVisual(visualId);

					Vector3 position = (*e)->GetPosition();

					D3DXMATRIX world, rot;
					D3DXMatrixTranslation(&world, (*e)->GetPosition().x, (*e)->GetPosition().y, (*e)->GetPosition().z);
					D3DXMatrixRotationYawPitchRoll(&rot, D3DXToRadian((*e)->GetRotation().y), 
														 D3DXToRadian((*e)->GetRotation().x), 
														 D3DXToRadian((*e)->GetRotation().z));
					world = rot * world;
					D3DDevice->SetTransform(D3DTS_WORLD, &world);

					// Render sprites
					if (visual->sprites.size())
					{
						D3DXMATRIX billboard;
						D3DXMatrixTranspose(&billboard, &pViewport->GetCamera()->GetViewMatrix());
						billboard._14 = 0;
						billboard._24 = 0;
						billboard._34 = 0;
						billboard._41 = 0;
						billboard._42 = 0;
						billboard._43 = 0;
						billboard._44 = 1;

						std::vector<VisualSprite *>::iterator s = visual->sprites.begin();
						for (; s != visual->sprites.end(); s++)
						{
							D3DXMATRIX sprite;
							sprite = billboard * world;

							if ((*s)->fixedX)
							{
								sprite._11 = world._11;
								sprite._12 = world._12;
								sprite._13 = world._13;
							}

							if ((*s)->fixedY)
							{
								sprite._21 = world._21;
								sprite._22 = world._22;
								sprite._23 = world._23;
							}

							if ((*s)->fixedZ)
							{
								sprite._31 = world._31;
								sprite._32 = world._32;
								sprite._33 = world._33;
							}

							D3DDevice->SetTransform(D3DTS_WORLD, &sprite);

							D3DDevice->SetStreamSource(0, (*s)->vertexBuffer, 0, sizeof(VERTEX_DIFFUSE_TEX1));
							D3DDevice->SetFVF(FVF_DIFFUSE_TEX1);
							D3DDevice->SetTexture(0, resourceManager->GetTexture((*s)->textureId));
							D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
						}
					}

					// Render geometry
					D3DDevice->SetStreamSource(0, visual->geometry->GetVertexBuffer(), 0, visual->geometry->vertexLength);
					D3DDevice->SetIndices(visual->geometry->GetIndexBuffer());
					D3DDevice->SetFVF(visual->geometry->FVF);

					for (int i = 0; i < visual->geometry->numChunks; i++)
						RenderChunk(&visual->geometry->chunks[i]);
				}
			}
		}

		//if (patches.size())
			patchGeometry->Clear();

		return S_OK;
	}


//-----------------------------------------------------------------------
	void SceneGraph::RenderNode(TreeNode *node, LPKSRVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		// Frustum Culling
		if (!pViewport->GetCamera()->CheckBoxFrustum(node->min, node->max))
			return;

		// Draw leaf
		if (node->leafIndex > -1 && !leafsDrawn[node->leafIndex])
			RenderLeaf(leafs[node->leafIndex], pViewport);

		for (int i = 0; i < node->numChildren; i++)
			RenderNode(node->children[i], pViewport);
	}


//-----------------------------------------------------------------------
	void SceneGraph::RenderLeaf(TreeLeaf *leaf, LPKSRVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		if (leaf->numFaces == 0)
			return;

		GenerateLeafPatches(leaf, pViewport);

		for (int i = 0; i < leaf->numFaces; i++)
		{
			int f = leafFaceIndices[leaf->startFaceIndex + i];

			if (facesDrawn[f])
				continue;

			facesDrawn[f] = 1;

			RenderChunk(&geometry->chunks[f]);
		}
	}


//-----------------------------------------------------------------------
	void SceneGraph::RenderChunk(KSRChunk *chunk)
	//-------------------------------------------------------------------
	{
		if (!chunk)
			return;

		if (!chunk->numVerts)
		{
			Logf("Error: Invalid number of vertex indices in chunk.");
			return;
		}

		if (chunk->idTexture0 > -1)
			D3DDevice->SetTexture(0, resourceManager->GetTexture(chunk->idTexture0));
		else
			D3DDevice->SetTexture(0, NULL);

		if (chunk->idTexture1 > -1)
			D3DDevice->SetTexture(1, resourceManager->GetTexture(chunk->idTexture1));
		else
			D3DDevice->SetTexture(1, NULL);

		if (chunk->depthBias != currentDepthBias)
		{
			currentDepthBias = chunk->depthBias;
			D3DDevice->SetRenderState(D3DRS_DEPTHBIAS, KSRFloatToDW(currentDepthBias));
		}

		switch (chunk->type)
		{
		case FT_POLYGON:
			D3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, chunk->numVerts, chunk->startIndex, chunk->numVerts - 2);
		break;

		case FT_TRIANGLELIST:
			D3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, chunk->numVerts, chunk->startIndex, chunk->numVerts / 3);
		break;

		case FT_TRIANGLESTRIP:
			D3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, chunk->numVerts, chunk->startIndex, chunk->numVerts - 2);
		break;

		case FT_LINELIST:
			D3DDevice->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, chunk->numVerts, chunk->startIndex, chunk->numVerts / 2);
		break;

		case FT_LINESTRIP:
			D3DDevice->DrawIndexedPrimitive(D3DPT_LINESTRIP, 0, 0, chunk->numVerts, chunk->startIndex, chunk->numVerts - 1);
		break;

		case FT_POINTLIST:
			D3DDevice->DrawPrimitive(D3DPT_POINTLIST, chunk->startIndex, chunk->numVerts);
		break;
		}
	}


//-----------------------------------------------------------------------
	void SceneGraph::GenerateLeafPatches(TreeLeaf *leaf, LPKSRVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		if (!leaf)
			return;

		if (!leaf->numFaces)
			return;

		for (int i = 0; i < leaf->numFaces; i++)
		{
			int f = leafFaceIndices[leaf->startFaceIndex + i];

			KSRChunk chunk = geometry->chunks[f];

			if (chunk.type != FT_PATCH)
				continue;

			GenerateLeafPatch(&chunk, pViewport);
		}
	}


//-----------------------------------------------------------------------
	HRESULT SceneGraph::GenerateLeafPatch(KSRChunk *source, LPKSRVIEWPORT pViewport)
	//-------------------------------------------------------------------
	{
		if (!source)
			return E_FAIL;

		VERTEX_NORMAL_TEX1 *pVerts = NULL;
		int *pIndices = NULL;

		geometry->Lock(NULL, (LPVOID*)&pVerts, (LPVOID*)&pIndices);

		std::vector<LPKSRPATCH>::iterator i = patches.begin();
		for (; i != patches.end(); i++)
		{
			if (FAILED((*i)->Render(source, pVerts, pIndices, pViewport)))
				return E_FAIL;
		}

		if (patches.size())
			geometry->Unlock();

		return S_OK;
	}


//-----------------------------------------------------------------------
	HRESULT SceneGraph::Update(float deltaTime)
	//-------------------------------------------------------------------
	{
		bodyCollisions.clear();
		worldCollisions.clear();

		std::vector<KSRCollisionInfo> collisions;
		collisions.clear();

		//DebugPrintf("Frame %d at %f\n\n", frameNumber, deltaTime);

		// Update dynamic states: Pre Collision Response
		std::list<KSREntity *>::iterator i = entities.begin();
		for (; i != entities.end(); i++)
		{
			if ((*i)->GetUpdate())
			{
				//DebugPrintf("Pre Response\n\n");

				KSRRigidBody *body = (*i)->GetRigidBody();

				//body->force = Vector3(0, -800, 0);

				body->acceleration = body->force / body->mass;
				body->rotAcceleration = body->torque / body->mass;

				body->velocity += body->acceleration * deltaTime;
				body->rotVelocity += body->rotAcceleration * deltaTime;

				body->lastPosition = body->position;
				body->lastRotation = body->rotation;

				body->position += body->velocity * deltaTime;
				body->rotation += body->rotVelocity * deltaTime;
			}
		}

		std::vector<TreeLeaf *>::iterator leaf = leafs.begin();
		for (; leaf != leafs.end(); leaf++)
		{
			// Make a list of entites in this leaf
				std::list<KSREntity *> leafEntities;
				std::vector<int> numEntityCollisions;

				for (i = entities.begin(); i != entities.end(); i++)
				{
					KSRRigidBody *body = (*i)->GetRigidBody();

					if (AABBSphereCollision(KSRAABB((*leaf)->min, (*leaf)->max), 
											KSRSphere(body->lastPosition, body->totalBoundingRadius), NULL) ||
						AABBSphereCollision(KSRAABB((*leaf)->min, (*leaf)->max), 
											KSRSphere(body->position,	  body->totalBoundingRadius), NULL))
						leafEntities.push_back(*i);
				}

				LPVOID pVertsLocked = NULL;
				int *pIndicesLocked = NULL;

				if (leafEntities.size() && (*leaf)->numFaces)
				{
					if (FAILED(geometry->Lock(NULL, &pVertsLocked, (LPVOID*)&pIndicesLocked)))
						return E_FAIL;
				}

				numEntityCollisions.resize(leafEntities.size());

			// Entity-world collision detection
				int iEntity = 0;
				for (i = leafEntities.begin(); i != leafEntities.end(); i++, iEntity++)
				{
					KSRRigidBody *body = (*i)->GetRigidBody();

					for (int f = (*leaf)->startFaceIndex; f < (*leaf)->startFaceIndex + (*leaf)->numFaces; f++)
					{
						KSRChunk face = geometry->chunks[leafFaceIndices[f]];

						Vector3 *faceVerts = new Vector3[face.numVerts];
						for (int v = 0; v < face.numVerts; v++)
							faceVerts[v] = geometry->GetVertexPosition(pVertsLocked, pIndicesLocked[face.startIndex + v]);

						Vector3 faceNormal = Normalise(Cross(faceVerts[1] - faceVerts[0], faceVerts[2] - faceVerts[0]));

						// If it doesn't have any bounding volumes, body is a rigid particle
						std::vector<KSRBoundingVolume>::iterator b = body->boundingVolumes.begin();
						for (; b != body->boundingVolumes.end(); b++)
						{
							KSRCollisionInfo *result = NULL;

							if (b->sphereRadius)
							{
								Vector3 point, pos;
								float time;
								if (SweepSphereStaticPolygon(KSRSphere(body->lastPosition, b->sphereRadius),
															 KSRSphere(body->position, b->sphereRadius),
															 KSRPolygon(faceNormal, faceVerts, face.numVerts), 
															 &time, &pos, &point))
								{
									body->position = pos;
									result = new KSRCollisionInfo;
									result->collisionNormal = faceNormal;
									result->contactPointWorld = point;
								}
							}
							else if (b->boundingBox)
							{
	
							}
							else if (b->cylinderHeight && b->cylinderRadius)
							{

							}

							if (result)
							{
								result->type = CT_BODYWORLD;
								result->body[0] = (*i);
								result->body[1] = NULL;
								result->bodyID[0] = iEntity;
								result->bodyID[1] = -1;
								result->contactPointBody[0] = result->contactPointWorld - body->position;
								result->relativeVelocity = body->velocity;
								result->relativeNormalVelocity = Dot(result->relativeVelocity, result->collisionNormal);

								worldCollisions.push_back(*result);
								collisions.push_back(*result);
								numEntityCollisions[iEntity]++;
								delete result;
							}
						}

						delete[] faceVerts;
					}
				}

				if (leafEntities.size() && (*leaf)->numFaces)
				{
					geometry->Unlock();
				}

			// Collision Response - Body/World
				std::vector<KSRCollisionInfo>::iterator n = collisions.begin();
				for (; n != collisions.end(); n++)
				{
					if (n->body[0] && n->bodyID[0] > -1)
					{
						KSRRigidBody *body = NULL;
						if (n->body[0]->GetRigidBody())
							body = n->body[0]->GetRigidBody();
						else
							continue;

						float impulse = (-(1.0f + 0.25f) * n->relativeNormalVelocity) / (1/body->mass);

						body->velocity += ((impulse * n->collisionNormal) / body->mass) / numEntityCollisions[n->bodyID[0]];
						body->rotVelocity += Cross(n->contactPointBody[0], (impulse * n->collisionNormal));
					}
				}
				collisions.clear();

			// Entity-Entity collision detection
				for (i = leafEntities.begin(), iEntity = 0; i != leafEntities.end(); i++, iEntity++)
				{
					int jEntity = iEntity;
					std::list<KSREntity *>::iterator j;
					for (j = i; j != leafEntities.end(); j++, jEntity++)
					{
						if (j == i)
							continue;

						KSRRigidBody *body = (*i)->GetRigidBody();
						KSRRigidBody *body2 = (*j)->GetRigidBody();

						if (!body2)
							continue;

						if (AABBSphereCollision(KSRAABB((*leaf)->min, (*leaf)->max), 
							KSRSphere(body->position, body->totalBoundingRadius), NULL))
						{
							if (SweepSphereSphere(KSRSphere(body->lastPosition, body->totalBoundingRadius),
												  KSRSphere(body->position,		body->totalBoundingRadius),
												  KSRSphere(body2->lastPosition, body2->totalBoundingRadius),
												  KSRSphere(body2->position,	 body2->totalBoundingRadius),
												  NULL, NULL, NULL, NULL))
							{
								std::vector<KSRBoundingVolume>::iterator b = body->boundingVolumes.begin();
								for (; b != body->boundingVolumes.end(); b++)
								{
									std::vector<KSRBoundingVolume>::iterator c = body2->boundingVolumes.begin();
									for (; c != body2->boundingVolumes.end(); c++)
									{
										KSRCollisionInfo *result = NULL;

										if (b->sphereRadius && c->sphereRadius)
										{
											Vector3 point, pos1, pos2;
											float time;

											if (SweepSphereSphere(KSRSphere(body->lastPosition, b->sphereRadius),
																  KSRSphere(body->position,		b->sphereRadius),
																  KSRSphere(body2->lastPosition, c->sphereRadius),
																  KSRSphere(body2->position,	 c->sphereRadius),
																  &time, &pos1, &pos2, &point))
											{
												/*
												DebugPrintf("Collision Detected: Body %d and Body %d at %f\n", iEntity, jEntity, time);
												DebugPrintf("Body %d Position: %f, %f, %f\n", iEntity, body->position.x,
																									   body->position.y,
																									   body->position.z);
												DebugPrintf("Body %d Position: %f, %f, %f\n\n", jEntity, body2->position.x,
																										 body2->position.y,
																										 body2->position.z);*/

												body->position = pos1;
												body2->position = pos2;

												result = new KSRCollisionInfo;
												result->contactPointWorld = point;
												result->contactPointBody[0] = point - body->position;
												result->contactPointBody[1] = point - body2->position;
												result->collisionNormal = Normalise(body->position - body2->position);
											}
										}
										else if (b->boundingBox && c->boundingBox)
										{
											Vector3 point, pos1, pos2;
											//float time;

											/*if (AABBAABBCollision(KSRAABB(body->position + b->offset - b->boundingBox * 0.5f,
																 		  body->position + b->offset + b->boundingBox * 0.5f),
																  KSRAABB(body2->position + c->offset - c->boundingBox * 0.5f,
																		  body2->position + c->offset + c->boundingBox * 0.5f),
																		  &point))
											{
											if (SweepAABBAABB(KSRAABB(body->lastPosition + b->offset - b->boundingBox * 0.5f,
																	  body->lastPosition + b->offset + b->boundingBox * 0.5f),
															  KSRAABB(body->position + b->offset - b->boundingBox * 0.5f,
																	  body->position + b->offset + b->boundingBox * 0.5f),
															  KSRAABB(body2->lastPosition + c->offset - c->boundingBox * 0.5f,
																	  body2->lastPosition + c->offset + c->boundingBox * 0.5f),
															  KSRAABB(body2->position + c->offset - c->boundingBox * 0.5f,
																	  body2->position + c->offset + c->boundingBox * 0.5f),
																	  &time, &pos1, &pos2))
											{
												body->position = pos1;
												body2->position = pos2;
												point = body->position + (body2->position - body->position) * time;

												result = new KSRCollisionInfo;
												result->contactPointWorld = point;
												result->contactPointBody[0] = point - body->position;
												result->contactPointBody[1] = point - body2->position;
												result->collisionNormal = Normalise(body->position - body2->position);
											}*/
											if (OBBOBBCollision(KSROBB(body->lastPosition + b->offset - b->boundingBox * 0.5f,
																	   body->lastPosition + b->offset + b->boundingBox * 0.5f,
																	   body->rotation),
																KSROBB(body2->lastPosition + c->offset - c->boundingBox * 0.5f,
																	   body2->lastPosition + c->offset + c->boundingBox * 0.5f,
																	   body2->rotation), NULL))
											{
												body->velocity = Vector3(0, 0, 0);
												body2->velocity = Vector3(0, 0, 0);
											}
										}
										else if (b->cylinderHeight && b->cylinderRadius)
										{

										}

										if (result)
										{
											result->type = CT_BODYBODY;
											result->body[0] = (*i);
											result->body[1] = (*j);
											result->bodyID[0] = iEntity;
											result->bodyID[1] = jEntity;

											result->relativeVelocity = body->velocity - body2->velocity;
											result->relativeNormalVelocity = Dot(result->relativeVelocity, result->collisionNormal);

											bodyCollisions.push_back(*result);
											collisions.push_back(*result);
											delete result;
										}
									}
								}
							}
						}
					}
				}

			// Collision Response - Body
			/*
				n = collisions.begin();
				for (; n != collisions.end(); n++)
				{
					if (n->body[0] && n->bodyID[0] > -1 && n->body[1] && n->bodyID[1] > -1)
					{
						KSRRigidBody *body1 = NULL;
						KSRRigidBody *body2 = NULL;

						if (n->body[0]->GetRigidBody())
							body1 = n->body[0]->GetRigidBody();
						else
							continue;

						if (n->body[1]->GetRigidBody())
							body2 = n->body[1]->GetRigidBody();
						else
							continue;

						float impulse = (-(1.0f + 0.5f) * n->relativeNormalVelocity) /
										((1/body1->mass + 1/body2->mass));

						body1->velocity += (impulse * n->collisionNormal) / body1->mass;
						//body1->rotVelocity += Cross(n->contactPointBody[0], (impulse * n->collisionNormal));

						body2->velocity -= (impulse * n->collisionNormal) / body2->mass;
						//body2->rotVelocity -= Cross(n->contactPointBody[1], (impulse * n->collisionNormal));

						DebugPrintf("Collision Resolved: Body %d and Body %d\n", n->bodyID[0], n->bodyID[1]);
						DebugPrintf("Body %d Position: %f, %f, %f\n", n->bodyID[0], n->body[0]->GetPosition().x,
																				  n->body[0]->GetPosition().y,
																				  n->body[0]->GetPosition().z);
						DebugPrintf("Body %d Position: %f, %f, %f\n\n", n->bodyID[1], n->body[1]->GetPosition().x,
																				  n->body[1]->GetPosition().y,
																				  n->body[1]->GetPosition().z);
					}
				}
				collisions.clear();*/
		}

		// Update dynamic states: Post Collision Response
		/*i = entities.begin();
		int bodyNum = 0;
		for (; i != entities.end(); i++, bodyNum++)
		{
			if ((*i)->GetUpdate())
			{
				if ((*i)->GetUpdate() == RT_ONCE)
					(*i)->SetUpdate(RT_ONCE);

				KSRRigidBody *body = (*i)->GetRigidBody();

				DebugPrintf("Post Response\n");
				DebugPrintf("Body %d\n", bodyNum);
				DebugPrintf("Velocity: %f, %f, %f\n", body->velocity.x, body->velocity.y, body->velocity.z);
				DebugPrintf("Position: %f, %f, %f\n\n", body->position.x, body->position.y, body->position.z);
			}
		}

		DebugPrintf("-------------------------------------------------\n\n");
*/
		frameNumber++;

		return S_OK;
	}


//-----------------------------------------------------------------------
	void SceneGraph::RecurseDrawPartition(TreeNode *&node, std::vector<Vector3> &verts)
	//-------------------------------------------------------------------
	{
		if (!node)
			return;

		Vector3 a = node->min;
		Vector3 b = node->max;

		// Bottom face
		verts.push_back(a);
		verts.push_back(Vector3(b.x, a.y, a.z));

		verts.push_back(Vector3(b.x, a.y, a.z));
		verts.push_back(Vector3(b.x, a.y, b.z));

		verts.push_back(Vector3(b.x, a.y, b.z));
		verts.push_back(Vector3(a.x, a.y, b.z));

		verts.push_back(Vector3(a.x, a.y, b.z));
		verts.push_back(a);

		// Top face
		verts.push_back(Vector3(a.x, b.y, a.z));
		verts.push_back(Vector3(b.x, b.y, a.z));

		verts.push_back(Vector3(b.x, b.y, a.z));
		verts.push_back(Vector3(b.x, b.y, b.z));

		verts.push_back(Vector3(b.x, b.y, b.z));
		verts.push_back(Vector3(a.x, b.y, b.z));

		verts.push_back(Vector3(a.x, b.y, b.z));
		verts.push_back(Vector3(a.x, b.y, a.z));

		// side edges
		verts.push_back(Vector3(a.x, a.y, a.z));
		verts.push_back(Vector3(a.x, b.y, a.z));

		verts.push_back(Vector3(b.x, a.y, a.z));
		verts.push_back(Vector3(b.x, b.y, a.z));

		verts.push_back(Vector3(b.x, a.y, b.z));
		verts.push_back(Vector3(b.x, b.y, b.z));

		verts.push_back(Vector3(a.x, a.y, b.z));
		verts.push_back(Vector3(a.x, b.y, b.z));

		for (int i = 0; i < node->numChildren; i++)
			RecurseDrawPartition(node->children[i], verts);
	}


//-----------------------------------------------------------------------
	void SceneGraph::RecurseOutputPartition(TreeNode *node, std::vector<TreeNodeOutput> &outputNodes)
	//-------------------------------------------------------------------
	{
		TreeNodeOutput outputNode;
		outputNode.min = node->min;
		outputNode.max = node->max;
		outputNode.numChildren = node->numChildren;
		outputNode.leafIndex = node->leafIndex;

		if (node->parent)
			outputNode.parent = node->parent->id;
		else
			outputNode.parent = -1;

		for (int i = 0; i < node->numChildren; i++)
			outputNode.children[i] = node->children[i]->id;

		outputNodes.push_back(outputNode);

		for (i = 0; i < node->numChildren; i++)
			RecurseOutputPartition(node->children[i], outputNodes);
	}


//-----------------------------------------------------------------------
	void SceneGraph::RecurseInputPartition(TreeNode *node, TreeNodeOutput *outputNodes, int &numNodes)
	//-------------------------------------------------------------------
	{
		if (!node || !outputNodes)
			return;

		if (node->numChildren)
			node->children = new TreeNode *[node->numChildren];

		for (int i = 0; i < node->numChildren; i++)
		{
			int childID = outputNodes[node->id].children[i];

			node->children[i] = new TreeNode(outputNodes[childID].min, outputNodes[childID].max, node, numNodes);
			node->children[i]->leafIndex = outputNodes[childID].leafIndex;
			node->children[i]->numChildren = outputNodes[childID].numChildren;

			numNodes++;

			RecurseInputPartition(node->children[i], outputNodes, numNodes);
		}
	}


//-----------------------------------------------------------------------
	void SceneGraph::RecurseSphereCollisionPartition(TreeNode *node, KSRSphere sphere1, KSRSphere sphere2, std::vector<TreeLeaf *> &leafs, std::vector<TreeLeaf *> &collidingLeafs)
	//-------------------------------------------------------------------
	{
		if (!node)
			return;

		if (!AABBSphereCollision(KSRAABB(node->min, node->max), sphere1, NULL) &&
			!AABBSphereCollision(KSRAABB(node->min, node->max), sphere2, NULL))
			return;

		if (!node->numChildren)
		{
			collidingLeafs.push_back(leafs[node->leafIndex]);
			return;
		}

		for (int i = 0; i < node->numChildren; i++)
			RecurseSphereCollisionPartition(node->children[i], sphere1, sphere2, leafs, collidingLeafs);
	}

// EOF