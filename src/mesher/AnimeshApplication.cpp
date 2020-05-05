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

const nanogui::Vector3f HIGHLIGHTED_SURFEL_COLOUR{0.8f, 0.8f, 0.0f};
const nanogui::Vector3f HIGHLIGHTED_NEIGHBOUR_COLOUR{0.0f, 0.6f, 0.8f};


template<class T>
std::string format_with_commas(T value) {
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << value;
    return ss.str();
}


/**
 * Convert from surfel id to index
 */
unsigned int
AnimeshApplication::surfel_id_to_index(const std::string &id) {
    const auto &it = m_surfel_id_to_index.find(id);
    assert(it != m_surfel_id_to_index.end());
    return it->second;
}

/**
 * Convert from surfel index to id
 */
std::string
AnimeshApplication::surfel_index_to_id(unsigned int index) {
    assert(index < m_surfel_index_to_id.size());
    return m_surfel_index_to_id.at(index);
}


/**
 * Obtain new frame data and pass to canvas for redraw.
 */
void AnimeshApplication::update_canvas() {
    using namespace nanogui;
    using namespace std;

    auto temp_camera = m_cameras.at(m_frame_idx);
    const auto dims = m_optimiser->get_dimensions();
    temp_camera.set_image_size(dims);


    const auto &surfel_data = m_optimiser->get_surfel_data();

    m_surfel_data.clear();
    m_surfel_id_to_index.clear();
    m_surfel_index_to_id.clear();

    for (const auto &s : surfel_data) {
        for (const auto &fd : s.frame_data) {
            if (fd.pixel_in_frame.frame == m_frame_idx) {
                const auto point_in_space = temp_camera.to_world_coordinates(
                        fd.pixel_in_frame.pixel.x, fd.pixel_in_frame.pixel.y, fd.depth);
                m_surfel_data.emplace_back(point_in_space,
                                           fd.normal,
                                           fd.transform * s.tangent,
                                           s.last_correction,
                                           s.error
                );

                m_surfel_index_to_id.push_back(s.id);
                m_surfel_id_to_index.emplace(s.id, m_surfel_data.size() - 1);
            }
        }
    }

    m_canvas->set_data(m_surfel_data);
    maybe_highlight_surfel_and_neighbours();

    m_txt_num_surfels->setValue(format_with_commas(m_surfel_data.size()));
}

void AnimeshApplication::load_all_the_things() {
    using namespace std;
    using namespace spdlog;

    m_optimiser = new Optimiser(*m_properties);

    info("Loading depth maps");
    const auto depth_maps = load_depth_maps(*m_properties);
    const auto num_frames = depth_maps.size();

    info("Loading cameras");
    m_cameras = load_cameras(num_frames);

    m_optimiser->set_data(depth_maps, m_cameras);
    update_canvas();
}

void AnimeshApplication::surfel_selected(int surfel_idx) {
    // Lookup the ID
    m_selected_surfel_id = surfel_index_to_id(surfel_idx);

    spdlog::info("Surfel {:d} {:s} selected", surfel_idx, m_selected_surfel_id);

    maybe_highlight_surfel_and_neighbours();
}

void AnimeshApplication::update_selected_surfel_data(bool clear) {
    if( m_selected_surfel_id.empty() || clear ) {
        m_txt_selected_surfel_id->setValue("");
        m_txt_selected_surfel_idx->setValue("");
        m_txt_selected_surfel_err->setValue("");
        m_txt_selected_surfel_adj->setValue("");
        return;
    }

    m_txt_selected_surfel_id->setValue(m_selected_surfel_id);
    unsigned int surfel_idx = surfel_id_to_index(m_selected_surfel_id);
    m_txt_selected_surfel_idx->setValue(std::to_string(surfel_idx));
    m_txt_selected_surfel_err->setValue(format_with_commas(m_surfel_data.at(surfel_idx).error));
    m_txt_selected_surfel_adj->setValue(format_with_commas(m_surfel_data.at(surfel_idx).adjustment));
}

nanogui::TextBox *
make_label_textbox_pair( nanogui::Widget * container, int row, const std::string& label_text, const std::string& default_value = "") {
    using namespace nanogui;

    auto label = new Label(container, label_text);
    auto * agl = dynamic_cast<AdvancedGridLayout *>(container->layout());
    agl->setAnchor(label, AdvancedGridLayout::Anchor{0, row, 1, 1, Alignment::Fill,Alignment::Middle});
    auto tb = new TextBox(container, default_value);
    tb->setEditable(false);
    tb->setAlignment(TextBox::Alignment::Left);
    agl->setAnchor(tb,AdvancedGridLayout::Anchor{1, row, 1, 1, Alignment::Fill,Alignment::Middle});
    return tb;
}

nanogui::Widget *
make_label_value_panel( nanogui::Widget * container, int rows ) {
    using namespace nanogui;

    std::vector<int> row_heights;
    for( int i=0; i<rows;++i) {
        row_heights.push_back(0);
    }
    auto panel_layout = new AdvancedGridLayout({0, 0}, row_heights, 5);
    panel_layout->setColStretch(0, 0.2);
    panel_layout->setColStretch(1, 0.8);
    auto panel = new Widget(container);
    panel->setLayout(panel_layout);
    return panel;
}

void AnimeshApplication::make_global_data_panel(nanogui::Widget *window) {
    using namespace nanogui;

    auto data_panel = make_label_value_panel(window, 3);
    m_txt_num_surfels = make_label_textbox_pair(data_panel, 0, "# Surfels");
    m_txt_mean_error = make_label_textbox_pair(data_panel, 1, "Mean Smoothness");
    m_txt_global_error = make_label_textbox_pair(data_panel, 2, "Global Smoothness");
}

void AnimeshApplication::make_surfel_data_panel(nanogui::Widget *window) {
    using namespace nanogui;

    auto stat_panel = make_label_value_panel(window, 4);
    m_txt_selected_surfel_idx = make_label_textbox_pair(stat_panel, 0, "Surfel Idx" );
    m_txt_selected_surfel_id = make_label_textbox_pair(stat_panel,1, "Surfel ID");
    m_txt_selected_surfel_err = make_label_textbox_pair(stat_panel,2, "Smoothness");
    m_txt_selected_surfel_adj = make_label_textbox_pair(stat_panel,3, "Last Adj");
}

void AnimeshApplication::maybe_highlight_surfel_and_neighbours() {
    m_canvas->remove_highlights();
    if (m_optimiser->surfel_is_in_frame(m_selected_surfel_id, m_frame_idx)) {
        auto surfel_idx = surfel_id_to_index(m_selected_surfel_id);
        m_canvas->highlight_surfel(surfel_idx, HIGHLIGHTED_SURFEL_COLOUR);
        spdlog::debug("Highlighting surfel {:s}", m_selected_surfel_id);

        // And it's neighbours in frame
        const auto neighbours = m_optimiser->get_neighbours_of_surfel_in_frame(m_selected_surfel_id, m_frame_idx);
        for (const auto &n : neighbours) {
            spdlog::debug("Highlighting neighbour {:s}", n);
            auto n_idx = surfel_id_to_index(n);
            m_canvas->highlight_surfel(n_idx, HIGHLIGHTED_NEIGHBOUR_COLOUR);
        }
        update_selected_surfel_data();
    } else {
        update_selected_surfel_data(true);
    }
}

/**
 * Set the frame being rendered.
 * @param frame_idx
 */
void AnimeshApplication::set_frame(unsigned int frame_idx) {
    if (m_frame_idx != frame_idx) {
        m_frame_idx = frame_idx;
        update_canvas();
    }
}

void AnimeshApplication::make_colour_panel(nanogui::Widget *container) {
    using namespace nanogui;

    auto colouring_panel = new Widget(container);
    colouring_panel->setLayout(new BoxLayout(Orientation::Vertical,
                                             Alignment::Fill, 0, 5));

    new Label(colouring_panel, "Colouring");
    auto *normal_colouring_button = new Button(colouring_panel, "Normal");
    normal_colouring_button->setFlags(Button::Flags::RadioButton);
    normal_colouring_button->setCallback([this]() {
        m_canvas->set_colouring_mode(CrossFieldGLCanvas::NATURAL);
    });
    auto *adj_colouring_button = new Button(colouring_panel, "Adjustment");
    adj_colouring_button->setFlags(Button::Flags::RadioButton);
    adj_colouring_button->setCallback([this]() {
        m_canvas->set_colouring_mode(CrossFieldGLCanvas::ADJUSTMENT);
    });
    auto *error_colouring_button = new Button(colouring_panel, "Abs. Smoothness");
    error_colouring_button->setFlags(Button::Flags::RadioButton);
    error_colouring_button->setCallback([this]() {
        m_canvas->set_colouring_mode(CrossFieldGLCanvas::ERROR);
    });
    auto *error_rel_colouring_button = new Button(colouring_panel, "Rel. Smoothness");
    error_rel_colouring_button->setFlags(Button::Flags::RadioButton);
    error_rel_colouring_button->setCallback([this]() {
        m_canvas->set_colouring_mode(CrossFieldGLCanvas::ERROR_REL);
    });
    std::vector<Button *> button_group{adj_colouring_button, normal_colouring_button, error_colouring_button,
                                       error_rel_colouring_button};
    normal_colouring_button->setButtonGroup(button_group);
    adj_colouring_button->setButtonGroup(button_group);
    error_colouring_button->setButtonGroup(button_group);
    error_rel_colouring_button->setButtonGroup(button_group);
}


void AnimeshApplication::make_frame_selector_panel(nanogui::Widget *container, unsigned int num_frames) {
    using namespace nanogui;

    auto frame_panel = new Widget(container);
    frame_panel->setLayout(new BoxLayout(Orientation::Horizontal,
                                         Alignment::Middle, 0, 5));

    new Label(frame_panel, "Frame ");

    auto frame_slider = new Slider(frame_panel);
    frame_slider->setRange(std::make_pair(0, num_frames - 1));
    frame_slider->setValue(0.0f);
    frame_slider->setCallback([this](float value) {
        set_frame((unsigned int) (floor(value)));
    });
}

void AnimeshApplication::make_buttons_panel(nanogui::Widget *container) {
    using namespace nanogui;

    auto step_panel = new Widget(container);
    step_panel->setLayout(new BoxLayout(Orientation::Vertical,
                                        Alignment::Fill, 0, 5));
    auto *step_button = new Button(step_panel, "Step");
    step_button->setCallback([this]() {
        m_optimiser->optimise_do_one_step();
        m_txt_mean_error->setValue(format_with_commas(m_optimiser->get_mean_error()));
        m_txt_global_error->setValue(format_with_commas(m_optimiser->get_mean_error() * m_optimiser->get_surfel_data().size()));
        update_canvas();
        m_canvas->drawGL();
    });
    auto *reset_button = new Button(step_panel, "Reset");
    reset_button->setCallback([this]() {
        m_canvas->centre();
        m_canvas->drawGL();
    });
}

void AnimeshApplication::build_ui() {
    using namespace nanogui;

    auto layout =new AdvancedGridLayout({0,0}, {0});
    layout->setColStretch(0, 0.2f);
    layout->setColStretch(1, 0.8f);
    layout->setRowStretch(0, 1.0f);
    setLayout(layout);

    auto tool_window = new Widget(this);
    tool_window->setLayout(new GridLayout(Orientation::Vertical, 8, Alignment::Fill));
    make_frame_selector_panel(tool_window, 10);
    make_colour_panel(tool_window);
    make_buttons_panel(tool_window);
    make_surfel_data_panel(tool_window);
    make_global_data_panel(tool_window);
    layout->setAnchor(tool_window, AdvancedGridLayout::Anchor{0, 0, Alignment::Fill, Alignment::Fill});

    m_canvas = new CrossFieldGLCanvas(this);
    m_canvas->setBackgroundColor({100, 100, 100, 255});
    m_canvas->set_click_callback([=](int surfel_idx) {
        this->surfel_selected(surfel_idx);
    });
    layout->setAnchor(m_canvas, AdvancedGridLayout::Anchor{1, 0, Alignment::Fill, Alignment::Fill});

    setResizeCallback([=](nanogui::Vector2i size){
        spdlog::info( "Resize event {:d}, {:d}", size.x(), size.y());
        performLayout();
    });
    performLayout();
}

AnimeshApplication::AnimeshApplication(int argc, char *argv[]) :
        nanogui::Screen(Eigen::Vector2i(800, 600), "Animesh", true),
        m_frame_idx{0},
        m_txt_selected_surfel_id{nullptr},
        m_txt_selected_surfel_idx{nullptr},
        m_txt_selected_surfel_err{nullptr},
        m_txt_selected_surfel_adj{nullptr} {
    using namespace spdlog;
    using namespace std;

    info("Loading properties");
    string property_file_name = (argc == 2) ? argv[1] : "animesh.properties";
    m_properties = new Properties(property_file_name);

    build_ui();

    load_all_the_things();
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
