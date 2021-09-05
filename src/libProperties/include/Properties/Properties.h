//
// Created by Dave Durbin on 2019-08-11.
//

#pragma once

#include <map>
#include <string>

class Properties {
public:
    Properties()= default;
    explicit Properties(const std::string& file_name );
    explicit Properties(const std::map<std::string, std::string>& values);

    const std::string& getProperty(const std::string& key) const;
    int getIntProperty(const std::string& key) const;
    bool getBooleanProperty(const std::string& key) const;
    float getFloatProperty(const std::string& key) const;
    std::vector<float> getListOfFloatProperty(const std::string& key) const;
    bool hasProperty(const std::string& key) const;

private:
    std::map<std::string, std::string> property_map;
};