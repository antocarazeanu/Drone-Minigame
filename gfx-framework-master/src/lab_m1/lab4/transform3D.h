#pragma once

#include "utils/glm_utils.h"


namespace transform3D
{
    // Translate matrix
    inline glm::mat4 Translate(float translateX, float translateY, float translateZ)
    {
        // TODO(student): Implement the translation matrix
		glm::mat4 matrix = glm::mat4(1);
		matrix[3][0] = translateX;
		matrix[3][1] = translateY;
		matrix[3][2] = translateZ;
		return matrix;

    }

    // Scale matrix
    inline glm::mat4 Scale(float scaleX, float scaleY, float scaleZ)
    {
        // TODO(student): Implement the scaling matrix
		glm::mat4 matrix = glm::mat4(1);
		matrix[0][0] = scaleX;
		matrix[1][1] = scaleY;
		matrix[2][2] = scaleZ;
		return matrix;

    }

    // Rotate matrix relative to the OZ axis
    inline glm::mat4 RotateOZ(float radians)
    {
        // TODO(student): Implement the rotation matrix
		glm::mat4 matrix = glm::mat4(1);
		matrix[0][0] = cos(radians);
		matrix[0][1] = -sin(radians);
		matrix[1][0] = sin(radians);
		matrix[1][1] = cos(radians);
		return matrix;

    }

    // Rotate matrix relative to the OY axis
    inline glm::mat4 RotateOY(float radians)
    {
        // TODO(student): Implement the rotation matrix
		glm::mat4 matrix = glm::mat4(1);
		matrix[0][0] = cos(radians);
		matrix[0][2] = sin(radians);
		matrix[2][0] = -sin(radians);
		matrix[2][2] = cos(radians);
		return matrix;

    }

    // Rotate matrix relative to the OX axis
    inline glm::mat4 RotateOX(float radians)
    {
        // TODO(student): Implement the rotation matrix
		glm::mat4 matrix = glm::mat4(1);
		matrix[1][1] = cos(radians);
		matrix[1][2] = -sin(radians);
		matrix[2][1] = sin(radians);
		matrix[2][2] = cos(radians);
		return matrix;

    }
}   // namespace transform3D
