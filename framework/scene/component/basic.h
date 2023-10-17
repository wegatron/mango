#pragma once

#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <Eigen/Dense>

namespace vk_engine
{
    struct BoundingBox
    {
        glm::vec3 min;
        glm::vec3 max;
    };

    struct TransformRelationship
    {
        std::shared_ptr<TransformRelationship> parent;
        std::shared_ptr<TransformRelationship> child;
        std::shared_ptr<TransformRelationship> sibling;                
        Eigen::Matrix4f ltransform{Eigen::Matrix4f::Identity()}; // local transformation
        Eigen::Matrix4f gtransform{Eigen::Matrix4f::Identity()}; // global transformation
    };

    // entt renderable entity
    /**
     * BoundingBox bounding_box; // bounding box
     * uint32_t transform_index; // id std::shared_ptr<TransformRelationship> tr_re; // transform group it belongs to     
     * uint32_t material_index; // std::shared_ptr<Material> material; // material
    */
}