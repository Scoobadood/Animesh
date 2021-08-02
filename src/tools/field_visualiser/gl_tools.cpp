//
// Created by Dave Durbin (Old) on 2/8/21.
//

#include "gl_tools.h"
#include <Eigen/Core>
#include <Eigen/Geometry>

Eigen::Vector3f to_cartesian() {
  float x = m_radius * sinf(m_phi) * sinf(m_theta);
  float y = m_radius * cosf(m_phi);
  float z = m_radius * sinf(m_phi) * cosf(m_theta);
  return m_camera_target + nanogui::Vector3f{x, y, z};
}

/**
 * Compute a vector for the direction form the camera origin through a given pixel.
 * @param pixel_coord The pixel
 * @return The unit vector
 */
Eigen::Vector3f compute_ray_through_pixel(unsigned int x, unsigned int y) {
  using namespace Eigen;

  const auto camera_origin = to_cartesian();

  Vector3f N{camera_origin-m_camera_target};
  const auto n = N.normalized();

  // u is a vector that is perpendicular to the plane spanned by
  // N and view up vector (cam->up), ie in the image plane and horizontal
  Vector3f U = Vector3f{0, mup, 0}.cross(n);
  const auto u = U.normalized();

  // v is a vector perpendicular to N and U, i.e vertical in image palne
  const auto v = n.cross(u);

  const auto fov_rad = m_field_of_view * DEG_TO_RAD;
  double image_plane_height = 2 * tan(fov_rad.y() * 0.5f) * m_focal_length;
  double image_plane_width = 2 * tan(fov_rad.x() * 0.5f) * m_focal_length;

  const auto image_plane_centre = camera_origin - (n * m_focal_length);
  const auto image_plane_origin = image_plane_centre - (u * image_plane_width * 0.5f) - (v * image_plane_height * 0.5f);

  // Compute pixel dimensions in world units
  const auto pixel_width = image_plane_width / size().x();
  const auto pixel_height = image_plane_height / size().y();

  // Construct a ray from the camera origin to the world coordinate
  Vector3f pixel_in_world = image_plane_origin
      + ((pixel_coord.x() + 0.5) * pixel_width * u)
      + ((pixel_coord.y() + 0.5) * pixel_height * v);

  return  (pixel_in_world - camera_origin).normalized();
}
