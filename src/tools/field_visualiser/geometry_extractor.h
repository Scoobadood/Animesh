//
// Created by Dave Durbin (Old) on 2/8/21.
//

#ifndef ANIMESH_TOOLS_FIELD_VISUALISER_GEOMETRY_EXTRACTOR_H
#define ANIMESH_TOOLS_FIELD_VISUALISER_GEOMETRY_EXTRACTOR_H

class geometry_extractor {
public:
  inline void set_frame(int frame) {
    if(m_frame != frame) {
      m_frame = frame;
    }
  }

  inline int get_frame() const {
    return m_frame;
  }

private:
  int m_frame;
};

#endif //ANIMESH_TOOLS_FIELD_VISUALISER_GEOMETRY_EXTRACTOR_H
