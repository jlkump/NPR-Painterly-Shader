#ifndef CONTROLS_HPP
#define CONTROLS_HPP
void computeMatricesFromInputs();

void computeMatricesFromRotationOverTime(glm::vec3 center_of_rotation);

glm::mat4 getProjectionMatrix();

glm::mat4 getViewMatrix();
#endif