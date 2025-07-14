#pragma once
#include "flag.hpp"
#include "log.hpp"
#include "math.hpp"
#include "rapidxml.hpp"

#include <string>

class Image;

// integral
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const long long& payload,
                                const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const long& payload, const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const int& payload, const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const short& payload, const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const char& payload, const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const unsigned long long& payload,
                                const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const unsigned long& payload,
                                const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const unsigned int& payload,
                                const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const unsigned short& payload,
                                const std::string& name);
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const unsigned char& payload,
                                const std::string& name);
void Deserialize(rapidxml::xml_node<>& node, long long& payload);
void Deserialize(rapidxml::xml_node<>& node, long& payload);
void Deserialize(rapidxml::xml_node<>& node, int& payload);
void Deserialize(rapidxml::xml_node<>& node, short& payload);
void Deserialize(rapidxml::xml_node<>& node, char& payload);
void Deserialize(rapidxml::xml_node<>& node, unsigned long long& payload);
void Deserialize(rapidxml::xml_node<>& node, unsigned long& payload);
void Deserialize(rapidxml::xml_node<>& node, unsigned int& payload);
void Deserialize(rapidxml::xml_node<>& node, unsigned short& payload);
void Deserialize(rapidxml::xml_node<>& node, unsigned char& payload);

// bool
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const bool& payload, const std::string& name);
void Deserialize(rapidxml::xml_node<>& node, bool& payload);

// floating
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const double& payload, const std::string& name);
void Deserialize(rapidxml::xml_node<>& node, double& payload);

// vec2
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Vec2& payload, const std::string& name);
void Deserialize(rapidxml::xml_node<>& node, Vec2& payload);

// region
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Region& payload, const std::string& name);
void Deserialize(rapidxml::xml_node<>& node, Region& payload);

// Degrees
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Degrees& payload,
                                const std::string& name);
void Deserialize(rapidxml::xml_node<>& node, Degrees& payload);

// Radians
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Radians& payload,
                                const std::string& name);
void Deserialize(rapidxml::xml_node<>& node, Radians& payload);

// pose
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Pose& payload, const std::string& name);
void Deserialize(rapidxml::xml_node<>& node, Pose& payload);

// image
rapidxml::xml_node<>* Serialize(rapidxml::xml_document<>& doc,
                                const Image* payload, const std::string& name);
void Deserialize(rapidxml::xml_node<>& node, Image*& payload);

