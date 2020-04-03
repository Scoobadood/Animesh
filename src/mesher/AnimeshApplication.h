//
// Created by Dave Durbin on 28/3/20.
//

#ifndef ANIMESH_ANIMESHAPPLICATION_H
#define ANIMESH_ANIMESHAPPLICATION_H

#include <Properties/Properties.h>
#include <DepthMap/DepthMap.h>
#include "optimise.h"

#include "CrossFieldGLCanvas.h"
#include <nanogui/nanogui.h>

class AnimeshApplication : public nanogui::Screen {
public:
    AnimeshApplication(int argc, char * argv[]);
    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers);
    virtual void draw(NVGcontext *ctx);

private:


    void load_all_the_things(int argc, char * argv[]);

    CrossFieldGLCanvas *mCanvas;
    Properties *m_properties;
    std::vector<DepthMap> m_depth_maps;
    size_t m_num_frames;
    std::vector<Camera> m_cameras;
    std::vector<std::vector<DepthMap>> m_depth_map_hierarchy;
    int m_num_levels;
    size_t m_surfels_per_step;
    float m_convergence_threshold;
    std::vector<Surfel> m_current_level_surfels;
    Optimiser *m_optimiser;

};


#endif //ANIMESH_ANIMESHAPPLICATION_H
