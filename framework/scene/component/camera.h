#pragma once

#include <Eigen/Dense>

namespace vk_engine {
class Camera {
public:
    
    Camera() = default;

    ~Camera() = default;

    void setPosition(const Eigen::Vector3f &position)
    {
        view_mat_(0, 3) = -position.x();
        view_mat_(1, 3) = -position.y();
        view_mat_(2, 3) = -position.z();
    }
    
    // default: z: front, x: right, y: up
    void setRotationEuraXYZ(const Eigen::Vector3f &eura_xyz)
    {
        Eigen::Matrix3f rotation_mat;
        rotation_mat = Eigen::AngleAxisf(eura_xyz.x(), Eigen::Vector3f::UnitX())
                     * Eigen::AngleAxisf(eura_xyz.y(), Eigen::Vector3f::UnitY())
                     * Eigen::AngleAxisf(eura_xyz.z(), Eigen::Vector3f::UnitZ());
        view_mat_.block<3, 3>(0, 0) = rotation_mat.transpose();
    }

    void setRotationQuat(const Eigen::Quaternionf &quat)
    {
        Eigen::Matrix3f rotation_mat = quat.toRotationMatrix();
        view_mat_.block<3, 3>(0, 0) = rotation_mat.transpose();
    }

    void setClipPlanes(float near, float far)
    {
        near_ = near;
        far_ = far;
        dirty_proj_ = true;
    }

    void setFovHorizontal(float fov) // width / focalLength
    {
        fov_horizontal_ = fov;
        dirty_proj_ = true;
    }

    void setAspect(float aspect) // width / height
    {
        aspect_ = aspect;
        dirty_proj_ = true;
    }

    const Eigen::Matrix4f &getViewMatrix() const noexcept { return view_mat_; }
    
    const Eigen::Matrix4f &getProjectionMatrix() { 
        if (!dirty_proj_) return proj_mat_;
        proj_mat_ << 1.0f / tan(fov_horizontal_ / 2.0f), 0.0f, 0.0f, 0.0f,
                     0.0f, aspect_ / tan(fov_horizontal_ / 2.0f), 0.0f, 0.0f,
                     0.0f, 0.0f, far_/(far_-near_), far_*near_/(far_-near_),
                     0.0f, 0.0f, -1.0f, 0.0f;
        dirty_proj_ = false;
    }

private:
  bool dirty_proj_{true};
  float near_{0.1f};
  float far_{1000.0f};
  float fov_horizontal_{60.0f};
  float aspect_{1.0f};
  Eigen::Matrix4f proj_mat_;

  Eigen::Matrix4f view_mat_{Eigen::Matrix4f::Identity()};
};
} // namespace vk_engine