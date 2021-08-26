//
// Created by Dave Durbin on 19/5/20.
//

#pragma once

#include <map>
#include <string>
#include <vector>
#include <random>
#include <Eigen/Core>
#include "FrameData.h"
#include <memory>

class Surfel {
public:
    inline const Eigen::Vector3f &tangent() const { return m_tangent; }

    inline void setTangent(const Eigen::Vector3f &tangent) { m_tangent = tangent; }

    inline const std::string &id() const { return m_id; }

    inline const std::vector<FrameData> &frame_data() const { return m_frame_data; };

    inline std::vector<FrameData> &frame_data() { return m_frame_data; };

    inline const Eigen::Vector2f &reference_lattice_offset() const { return m_reference_lattice_offset; }

    inline void
    set_reference_lattice_offset(
            const Eigen::Vector2f &reference_offset) {
        m_last_posy_correction = reference_offset - m_reference_lattice_offset;
        m_reference_lattice_offset = reference_offset;
    }

    inline void set_rosy_smoothness(float smoothness) { m_rosy_smoothness = smoothness; }

    inline float rosy_smoothness() const { return m_rosy_smoothness; }

    inline void set_posy_smoothness(float smoothness) { m_posy_smoothness = smoothness; }

    inline float posy_smoothness() const { return m_posy_smoothness; }

    inline void set_rosy_correction(float correction) { m_last_rosy_correction = correction; }

    inline float rosy_correction() const { return m_last_rosy_correction; }

    inline void set_posy_correction(const Eigen::Vector2f &correction) { m_last_posy_correction = correction; }

    inline const Eigen::Vector2f &posy_correction() const { return m_last_posy_correction; }

    inline const std::vector<unsigned int> &frames() const { return m_frames; }

    inline size_t num_frames() const { return m_frames.size(); }

    friend std::ostream& operator<<( std::ostream& output, const Surfel& surfel ) {
      output << surfel.m_id;
      return output;
    }

    bool is_in_frame(unsigned int frame) const;

    void get_vertex_tangent_normal_for_frame(unsigned int frame_idx,
                                             Eigen::Vector3f &vertex,
                                             Eigen::Vector3f &tangent,
                                             Eigen::Vector3f &normal) const;

    void get_all_data_for_surfel_in_frame(unsigned int frame_idx,
                                          Eigen::Vector3f &vertex,
                                          Eigen::Vector3f &tangent,
                                          Eigen::Vector3f &orth_tangent,
                                          Eigen::Vector3f &normal,
                                          Eigen::Vector3f &closest_mesh_vertex) const;

    void get_all_data_for_surfel_in_frame(unsigned int frame_idx,
                                          Eigen::Vector3f &vertex,
                                          Eigen::Vector3f &tangent,
                                          Eigen::Vector3f &orth_tangent,
                                          Eigen::Vector3f &normal,
                                          Eigen::Vector3f &closest_mesh_vertex,
                                          unsigned short k_ij) const;

    static std::map<std::string, std::shared_ptr<Surfel>> m_surfel_by_id;

    void transform_surfel_via_frame(const std::shared_ptr<Surfel> &that_surfel_ptr,
                                    unsigned int frame_index,
                                    Eigen::Vector3f &transformed_other_norm,
                                    Eigen::Vector3f &transformed_other_tan) const;

private:

//    static std::shared_ptr<Surfel> surfel_for_id(const std::string &id);

    std::string m_id;
    std::vector<FrameData> m_frame_data;
    std::vector<unsigned int> m_frames;
    Eigen::Vector3f m_tangent;
    Eigen::Vector2f m_reference_lattice_offset;
    float m_rosy_smoothness;
    float m_last_rosy_correction;
    float m_posy_smoothness;
    Eigen::Vector2f m_last_posy_correction;

    const FrameData &frame_data_for_frame(unsigned int frame) const;

    // Use SurfelBuilder instances to construct this.
    Surfel(std::string id,
           const std::vector<FrameData> &frames,
           Eigen::Vector3f tangent = {1, 0, 0},
           Eigen::Vector2f reference_lattice_offset = {0, 0}
    );

    friend class SurfelBuilder;
};
