/*************************************************************************************
*	irrlamb - https://github.com/jazztickets/irrlamb
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
#include <objects/sphere.h>
#include <engine/globals.h>
#include <engine/physics.h>
#include <engine/config.h>
#include <engine/namespace.h>
#include <objects/template.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <IAnimatedMesh.h>
#include <IAnimatedMeshSceneNode.h>
#include <ISceneManager.h>

// Constructor
_Sphere::_Sphere(const SpawnStruct &Object)
:	_Object() {
	TemplateStruct *Template = Object.Template;

	// Check for mesh file
	if(Template->Mesh != "") {
		
		// Get file path
		std::string MeshPath = std::string("meshes/") + Template->Mesh;

		// Add mesh
		IAnimatedMesh *AnimatedMesh = irrScene->getMesh(MeshPath.c_str());
		Node = irrScene->addAnimatedMeshSceneNode(AnimatedMesh);
		if(Node) {
			Node->setScale(vector3df(Template->Scale, Template->Scale, Template->Scale));
			if(Template->Textures[0] != "")
				Node->setMaterialTexture(0, irrDriver->getTexture(Template->Textures[0].c_str()));
			if(Template->CustomMaterial != -1)
				Node->setMaterialType((E_MATERIAL_TYPE)Template->CustomMaterial);

			// Add shadows
			if(Config.Shadows) {
				((IAnimatedMeshSceneNode *)Node)->addShadowVolumeSceneNode();
			}
		}
	}
	
	// Set up physics
	if(Physics.IsEnabled()) {

		// Create shape
		btVector3 HalfExtents = Template->Shape * 0.5f;
		btSphereShape *Shape = new btSphereShape(Template->Radius);

		// Set up physics
		CreateRigidBody(Object, Shape);
	}

	// Set common properties
	SetProperties(Object);
}
