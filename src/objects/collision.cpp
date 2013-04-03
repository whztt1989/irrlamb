/*************************************************************************************
*	irrlamb - http://irrlamb.googlecode.com
*	Copyright (C) 2013  Alan Witkowski
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************************/
#include "collision.h"
#include "../engine/physics.h"
#include "../engine/globals.h"
#include "../engine/filestream.h"
#include "template.h"
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>

static bool CustomMaterialCallback(btManifoldPoint &ManifoldPoint, const btCollisionObjectWrapper *Object0, int PartID0, int Index0, const btCollisionObjectWrapper *Object1, int PartID1, int Index1) {

	if(1) {
		btAdjustInternalEdgeContacts(ManifoldPoint, Object1, Object0, PartID1, Index1);
	}

	return false;
}


// Constructor
CollisionClass::CollisionClass(const SpawnStruct &Object)
:	ObjectClass(),
	TriangleIndexVertexArray(NULL),
	VertexList(NULL),
	FaceList(NULL) {
	
	gContactAddedCallback = CustomMaterialCallback;

	// Load collision mesh file
	FileClass MeshFile;
	if(MeshFile.OpenForRead(Object.Template->CollisionFile.c_str())) {

		// Read header
		int VertCount = MeshFile.ReadInt();
		int FaceCount = MeshFile.ReadInt();

		// Allocate memory for lists
		VertexList = new float[VertCount * 3];
		FaceList = new int[FaceCount * 3];

		// Read vertices
		int VertexIndex = 0;
		for(int i = 0; i < VertCount; i++) {
			VertexList[VertexIndex++] = MeshFile.ReadFloat();
			VertexList[VertexIndex++] = MeshFile.ReadFloat();
			VertexList[VertexIndex++] = -MeshFile.ReadFloat();
		}

		// Read faces
		int FaceIndex = 0;
		for(int i = 0; i < FaceCount; i++) {
			FaceList[FaceIndex++] = MeshFile.ReadInt();
			FaceList[FaceIndex++] = MeshFile.ReadInt();
			FaceList[FaceIndex++] = MeshFile.ReadInt();
		}

		// Create triangle array
		TriangleIndexVertexArray = new btTriangleIndexVertexArray(FaceCount, FaceList, 3 * sizeof(int), VertCount * 3, VertexList, 3 * sizeof(float));

		// Create bvh shape
		btBvhTriangleMeshShape *Shape = new btBvhTriangleMeshShape(TriangleIndexVertexArray, true);
		TriangleInfoMap = new btTriangleInfoMap();
		btGenerateInternalEdgeInfo(Shape, TriangleInfoMap);

		// Create physics body
		CreateRigidBody(Object, Shape);
		SetProperties(Object);
		
		RigidBody->setCollisionFlags(RigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

		MeshFile.Close();
	}
}

// Destructor
CollisionClass::~CollisionClass() {

	delete TriangleInfoMap;
	delete TriangleIndexVertexArray;
	delete[] VertexList;
	delete[] FaceList;
}
