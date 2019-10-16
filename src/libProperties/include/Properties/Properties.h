//
// Created by Dave Durbin on 2019-08-11.
//

#ifndef ANIMESH_PROPERTIES_H
#define ANIMESH_PROPERTIES_H

#include <map>

class Properties {
public:
    explicit Properties(const std::string& file_name );

    const std::string& getProperty(const std::string& key) const;
    int getIntProperty(const std::string& key) const;
    bool getBooleanProperty(const std::string& key) const;
    float getFloatProperty(const std::string& key) const;

//private:
    std::map<std::string, std::string> property_map;
};

#endif //ANIMESH_PROPERTIES_H
