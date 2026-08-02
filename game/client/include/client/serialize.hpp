#pragma once

#include "client/context.hpp"
#include "common/animation_player.hpp"

// AnimationPlayer
rapidxml::xml_node<>* Serialize(ClientContext& ctx,rapidxml::xml_document<>& doc,
                                const AnimationPlayer& payload,
                                const std::string& name);
void Deserialize(ClientContext& ctx, const rapidxml::xml_node<>& node, AnimationPlayer& payload);