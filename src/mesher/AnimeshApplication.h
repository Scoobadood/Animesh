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
    Optimiser *m_optimiser;

};


#endif //ANIMESH_ANIMESHAPPLICATION_H
