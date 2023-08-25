#pragma once

#include <glm/glm.hpp>
#include <entt/entt.hpp>

namespace vk_engine
{
    /**
     * \breif R * scale * x + T
    */
    struct Transform
    {
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;
    };

    struct BoundingBox
    {
        glm::vec3 min;
        glm::vec3 max;
    };

    struct Relationship
    {
        entt::entity first_child;
        entt::entity next;
    };
}