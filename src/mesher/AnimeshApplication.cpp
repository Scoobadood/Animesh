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
                         std::vector<std::vector<nanogui::Vector3f>> &tangents,
                         std::vector<float> &adjustments,
                         std::vector<float> &errors
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
        adjustments.push_back(surfel.last_correction);
        errors.push_back(surfel.error);
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
    vector<float> adjustments;
    vector<float> errors;

    extract_coordinates(m_optimiser->get_surfel_data(), m_cameras, depth_map_dimensions, points, normals, tangents, adjustments, errors);
    mCanvas->set_data(points, normals, tangents, adjustments, errors);
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

    auto colouring_panel = new Widget(tools);
    colouring_panel->setLayout(new BoxLayout(Orientation::Vertical,
                                          Alignment::Middle, 0, 5));
    new Label(colouring_panel, "Colouring");
    auto * normal_colouring_button = new Button(colouring_panel, "Normal");
    normal_colouring_button->setFlags(Button::Flags::RadioButton);
    normal_colouring_button->setCallback([this](){
        mCanvas->set_colouring_mode(CrossFieldGLCanvas::NATURAL);
    });
    auto * adj_colouring_button = new Button(colouring_panel, "Adjustment");
    adj_colouring_button->setFlags(Button::Flags::RadioButton);
    adj_colouring_button->setCallback([this](){
        mCanvas->set_colouring_mode(CrossFieldGLCanvas::ADJUSTMENT);
    });
    auto * error_colouring_button = new Button(colouring_panel, "Error");
    error_colouring_button->setFlags(Button::Flags::RadioButton);
    error_colouring_button->setCallback([this](){
        mCanvas->set_colouring_mode(CrossFieldGLCanvas::ERROR);
    });
    std::vector<Button *> button_group{adj_colouring_button, normal_colouring_button, error_colouring_button};
    normal_colouring_button->setButtonGroup(button_group);
    adj_colouring_button->setButtonGroup(button_group);
    error_colouring_button->setButtonGroup(button_group);

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
