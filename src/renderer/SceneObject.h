#pragma once

#include "Material.h"
#include "Mesh.h"

class SceneObject {
 public:
  Mesh* target;
  Material* material;

  SceneObject() = default;
  SceneObject(Mesh* m, Material* mat) : target(m), material(mat) {}
};

class LightObject : public SceneObject {
 public:
  Mesh* mesh;

  LightObject(Mesh* m, Emission* mat) : SceneObject(m, mat), mesh(m) {}
};
