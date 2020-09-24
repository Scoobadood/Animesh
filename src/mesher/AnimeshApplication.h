//
// Created by Dave Durbin on 28/3/20.
//

#ifndef ANIMESH_ANIMESHAPPLICATION_H
#define ANIMESH_ANIMESHAPPLICATION_H

#include <Properties/Properties.h>
#include <DepthMap/DepthMap.h>
#include "RoSy/RoSyOptimiser.h"

#include "CrossFieldGLCanvas.h"
#include <nanogui/nanogui.h>

class AnimeshApplication : public nanogui::Screen {
public:
    AnimeshApplication(int argc, char *argv[]);

    bool keyboardEvent(int key, int scancode, int action, int modifiers) override;

    void draw(NVGcontext *ctx) override;

    void surfel_selected(int surfel_id);

private:

    void update_canvas();
    void update_selected_surfel_data(bool clear = false);

    void load_all_the_things();

    void build_ui();

    CrossFieldGLCanvas *m_canvas;
    Properties *m_properties;
    RoSyOptimiser *m_optimiser;
    std::vector<Camera> m_cameras;

    void make_buttons_panel(nanogui::Widget *container);

    void make_surfel_data_panel(nanogui::Widget *window);

    void make_frame_selector_panel(nanogui::Widget *container, unsigned int num_frames);

    void make_colour_panel(nanogui::Widget *container);

    void make_global_data_panel(nanogui::Widget * window);

    void set_frame(unsigned int frame);

    void maybe_highlight_surfel_and_neighbours();

    unsigned int surfel_id_to_index(const std::string &id);

    std::string surfel_index_to_id(unsigned int index);

    unsigned int m_frame_idx;

    nanogui::TextBox *m_txt_selected_surfel_id;
    nanogui::TextBox *m_txt_selected_surfel_idx;
    nanogui::TextBox *m_txt_selected_surfel_err;
    nanogui::TextBox *m_txt_selected_surfel_adj;

    nanogui::TextBox *m_txt_num_surfels;
    nanogui::TextBox *m_txt_mean_error;
    nanogui::TextBox *m_txt_global_error;

    std::string m_selected_surfel_id;

    std::map<std::string, unsigned int> m_surfel_id_to_index;
    std::vector<std::string> m_surfel_index_to_id;
    std::vector<CrossFieldGLCanvas::SurfelData> m_surfel_data;
};


#endif //ANIMESH_ANIMESHAPPLICATION_H
