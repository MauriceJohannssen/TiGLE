#pragma once
#include "Transform.h"
#include <tuple>

enum class ProjectionType {
	Orthographic,
	Perspective
};

class Camera : public Transform {
public:
	Camera(ProjectionType pProjection);
	glm::mat4 GetProjectionMatrix() const;
	glm::vec3 GetMovementVector() const;
	void SetMovementVector(const glm::vec3 vector);
	std::tuple<float, float> GetNearFarPlanes();

private:
	glm::vec3 movementVector;
	glm::mat4 projectionMatrix;
	std::tuple<float, float> nearFarPlane;
};
