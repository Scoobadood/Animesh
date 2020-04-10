//
// Created by Dave Durbin on 28/3/20.
//

#include <vector>
#include <memory>
#include <Properties/Properties.h>
#include <DepthMap/DepthMap.h>
#include "optimise.h"
#include "types.h"
#include "utilities.h"
#include "spdlog/spdlog.h"
#include <nanogui/nanogui.h>

#include "AnimeshApplication.h"

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
void AnimeshApplication::update_canvas_data( ) {
    using namespace nanogui;
    using namespace std;
    // TODO: Give nanogui something to render
    auto dims = m_optimiser->get_dimensions();
    Vector2i depth_map_dimensions{ dims.first,  dims.second };
    vector<vector<Vector3f>> points;
    vector<vector<Vector3f>> normals;
    vector<vector<Vector3f>> tangents;

    extract_coordinates(m_optimiser->get_surfel_data(), m_cameras, depth_map_dimensions, points, normals, tangents);
    mCanvas->set_data(points, normals, tangents );
}

void AnimeshApplication::load_all_the_things(int argc, char * argv[]) {
    using namespace std;
    using namespace spdlog;

    info("Loading properties");
    string property_file_name = (argc == 2) ? argv[1] : "animesh.properties";
    m_properties = new Properties(property_file_name);

    m_optimiser = new Optimiser(*m_properties);

    info("Loading depth maps");
    const auto depth_maps = load_depth_maps(*m_properties);
    const auto num_frames = depth_maps.size();

    info("Loading cameras");
    m_cameras = load_cameras(num_frames);

    m_optimiser->set_data( depth_maps, m_cameras);
    update_canvas_data();
}

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

    auto step_panel = new Widget(tools);
    step_panel->setLayout(new BoxLayout(Orientation::Vertical,
                                          Alignment::Middle, 0, 5));
    auto *step_button = new Button(step_panel, "Step");
    step_button->setCallback([this]() {
        m_optimiser->optimise_do_one_step();
        update_canvas_data();
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
