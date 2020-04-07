//
// Created by Dave Durbin on 28/3/20.
//

#include <vector>
#include <map>
#include <memory>
#include <Properties/Properties.h>
#include <DepthMap/DepthMap.h>
#include <omp.h>
#include "correspondences_io.h"
#include "surfel_compute.h"
#include "optimise.h"
#include "types.h"
#include "surfel_io.h"
#include "utilities.h"
#include "depth_map_io.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "hierarchical_mesher_utilities.h"
#include <nanogui/nanogui.h>

#include "AnimeshApplication.h"

const char pre_smooth_filename_template[] = "presmooth_%02d.bin";
const char post_smooth_filename_template[] = "smoothed_%02d.bin";

void extract_coordinates(const std::vector<Surfel> &surfels,
                         const std::vector<Camera> &cameras,
                         const nanogui::Vector2i& depth_map_coordinates,
                         std::vector<std::vector<nanogui::Vector3f>> &points,
                         std::vector<std::vector<nanogui::Vector3f>> &normals,
                         std::vector<std::vector<nanogui::Vector3f>> &tangents
                         ) {
    using namespace std;
    using namespace nanogui;

    vector<Camera> temp_cameras;
    for(const auto & camera : cameras) {
        points.emplace_back(vector<Vector3f>{});
        normals.emplace_back(vector<Vector3f>{});
        tangents.emplace_back(vector<Vector3f>{});
        temp_cameras.push_back(camera);
        temp_cameras.back().set_image_size(depth_map_coordinates.x(), depth_map_coordinates.y());
    }

    for (const auto &surfel : surfels) {
        for (const auto &fd : surfel.frame_data) {
            const auto pif = fd.pixel_in_frame;
            const auto point_in_space = temp_cameras.at(pif.frame).to_world_coordinates(pif.pixel.x, pif.pixel.y,
                                                                                   fd.depth);
            points.at(pif.frame).push_back(point_in_space);
            normals.at(pif.frame).push_back(fd.normal);
            tangents.at(pif.frame).push_back(fd.transform * surfel.tangent);
        }
    }
}

void AnimeshApplication::load_all_the_things(int argc, char * argv[]) {
    using namespace std;
    using namespace spdlog;

    info("Loading properties");
    string property_file_name = (argc == 2) ? argv[1] : "animesh.properties";
    m_properties = new Properties(property_file_name);

    info("Loading depth maps");
    m_depth_maps = load_depth_maps(*m_properties);
    m_num_frames = m_depth_maps.size();

    info("Loading cameras");
    m_cameras = load_cameras(m_num_frames);

    info("Generating depth map hierarchy");
    m_depth_map_hierarchy = create_depth_map_hierarchy(*m_properties, m_depth_maps, m_cameras);
    maybe_save_depth_and_normal_maps(*m_properties, m_depth_map_hierarchy);
    m_num_levels = m_depth_map_hierarchy.size();

    // +-----------------------------------------------------------------------------------------------
    // | Construct Surfels for each level
    // +-----------------------------------------------------------------------------------------------
    m_surfels_per_step = m_properties->getIntProperty("surfels-per-step");
    m_convergence_threshold = m_properties->getFloatProperty("convergence-threshold");
    int current_level_index = m_num_levels - 1;
    info( "Generating surfels for level : {:d}", current_level_index);
    info( "   Getting correspondences");
    vector<vector<PixelInFrame>> correspondences = get_correspondences(*m_properties, current_level_index,
                                                                       m_depth_map_hierarchy.at(current_level_index),
                                                                       m_cameras);

    info( "   Generating Surfels");
    m_current_level_surfels = generate_surfels(m_depth_map_hierarchy.at(current_level_index),
                                                            correspondences, *m_properties);
    nanogui::Vector2i depth_map_dimensions{
            m_depth_map_hierarchy.at(current_level_index).at(0).width(),
            m_depth_map_hierarchy.at(current_level_index).at(0).height()
    };
    vector<vector<nanogui::Vector3f>> points;
    vector<vector<nanogui::Vector3f>> normals;
    vector<vector<nanogui::Vector3f>> tangents;

    extract_coordinates(m_current_level_surfels, m_cameras, depth_map_dimensions, points, normals, tangents);

    mCanvas->set_data(points, normals, tangents );
}




//    while (current_level_index >= 0) {
//        info( "Generating surfels for level : {:d}", current_level_index);
//
//        info( "   Getting correspondences");
//        vector<vector<PixelInFrame>> correspondences = get_correspondences(properties, current_level_index,
//                                                                           depth_map_hierarchy.at(current_level_index),
//                                                                           cameras);
//
//        info( "   Generating Surfels");
//        vector<Surfel> current_level_surfels = generate_surfels(depth_map_hierarchy.at(current_level_index),
//                                                                correspondences, properties);
//
//        if (!previous_level_surfels.empty()) {
//            initialise_tangents_from_previous_level(current_level_surfels, previous_level_surfels, properties);
//        }
//
//        info( "   Saving presmoothed Surfels");
//        save_surfels_to_file(file_name_from_template_and_level(pre_smooth_filename_template,
//                                                               current_level_index), current_level_surfels);
//
//        info( "   Optimising");
//        m_optimiser = new Optimiser(m_convergence_threshold, num_frames, surfels_per_step);
//        int cycles = o.optimise(current_level_surfels);
//        info("Optimisation completed in {:d} cycles", cycles);
//
//        info( "   Saving smoothed Surfels");
//        save_surfels_to_file(file_name_from_template_and_level(post_smooth_filename_template,
//                                                               current_level_index), current_level_surfels);
//
//        // +-----------------------------------------------------------------------------------------------
//        // | Remap surfels
//        // +-----------------------------------------------------------------------------------------------
//        previous_level_surfels = current_level_surfels;
//        Surfel::surfel_by_id.clear();
//        for (auto &s : previous_level_surfels) {
//            Surfel::surfel_by_id.emplace(s.id, ref(s));
//        }
//        --current_level_index;
//    }

AnimeshApplication::AnimeshApplication(int argc, char * argv[]) : nanogui::Screen(Eigen::Vector2i(800, 600), "NanoGUI Test", false) {
    using namespace nanogui;

    auto *window = new Window(this, "GLCanvas Demo");
    window->setPosition(Vector2i(15, 15));
    window->setLayout(new GroupLayout());

    mCanvas = new CrossFieldGLCanvas(window);
    mCanvas->setBackgroundColor({100, 100, 100, 255});
    mCanvas->setSize({400, 400});

    auto *tools = new Widget(window);
    tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                   Alignment::Middle, 0, 20));

    auto frame_panel = new Widget(tools);
    frame_panel->setLayout(new BoxLayout(Orientation::Vertical,
                                         Alignment::Middle, 0, 5));
    new Label(frame_panel, "Frame");
    auto frame_slider = new Slider(frame_panel);
    frame_slider->setRange(std::make_pair(0, 8));
    frame_slider->setValue(0.0f);
    frame_slider->setCallback([this](float value) {
        mCanvas->setFrame((int)(floor(value)));
        mCanvas ->drawGL();
    });

    auto rotx_panel = new Widget(tools);
    rotx_panel->setLayout(new BoxLayout(Orientation::Vertical,
                                        Alignment::Middle, 0, 5));
    new Label(rotx_panel, "Rot X");
    auto *rotx_slider = new Slider(rotx_panel);
    rotx_slider->setRange(std::make_pair(-3.14159265f, 3.14159265f));
    rotx_slider->setValue(0.0f);
    rotx_slider->setCallback([this](float value) {
        mCanvas->setRotX(value);
        mCanvas ->drawGL();
    });

    auto roty_panel = new Widget(tools);
    roty_panel->setLayout(new BoxLayout(Orientation::Vertical,
                                        Alignment::Middle, 0, 5));
    new Label(roty_panel, "Rot Y");
    auto *roty_slider = new Slider(roty_panel);
    roty_slider->setRange(std::make_pair(-3.14159265f, 3.14159265f));
    roty_slider->setValue(0.0f);
    roty_slider->setCallback([this](float value) {
        mCanvas->setRotY(value);
        mCanvas ->drawGL();
    });

    auto rotz_panel = new Widget(tools);
    rotz_panel->setLayout(new BoxLayout(Orientation::Vertical,
                                        Alignment::Middle, 0, 5));
    new Label(rotz_panel, "Rot Z");
    auto *rotz_slider = new Slider(rotz_panel);
    rotz_slider->setRange(std::make_pair(-3.14159265f, 3.14159265f));
    rotz_slider->setValue(0.0f);
    rotz_slider->setCallback([this](float value) {
        mCanvas->setRotZ(value);
        mCanvas ->drawGL();
    });

    auto radius_panel = new Widget(tools);
    radius_panel->setLayout(new BoxLayout(Orientation::Vertical,
                                          Alignment::Middle, 0, 5));
    new Label(radius_panel, "Radius");
    auto *radius_slider = new Slider(radius_panel);
    radius_slider->setRange(std::make_pair(0.0, 10));
    radius_slider->setValue(1.0f);
    radius_slider->setCallback([this](float value) {
        mCanvas->setRadius(value);
        mCanvas ->drawGL();
    });


    performLayout();

    load_all_the_things(argc, argv);
}

bool AnimeshApplication::keyboardEvent(int key, int scancode, int action, int modifiers) {
    if (Screen::keyboardEvent(key, scancode, action, modifiers))
        return true;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        setVisible(false);
        return true;
    }
    return false;
}

void AnimeshApplication::draw(NVGcontext *ctx) {
    /* Draw the user interface */
    Screen::draw(ctx);
}
