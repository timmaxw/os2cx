#include "gui_scene_result.hpp"

namespace os2cx {

GuiSceneResultDisplacement::GuiSceneResultDisplacement(
    QWidget *parent, const Project *project, const std::string &result_name_
) :
    GuiSceneMesh(parent, project), result_name(result_name_)
{ }

void GuiSceneResultDisplacement::calculate_attributes(
    ElementId element_id,
    int face_index,
    NodeId node_id,
    QColor *color_out,
    PureVector *displacement_out
) {
    (void)element_id;
    (void)face_index;
    *color_out = QColor(0x80, 0x80, 0x80);
    const ContiguousMap<NodeId, PureVector> &result =
        project->results->node_vectors.at(result_name);
    *displacement_out = result[node_id];
}

} /* namespace os2cx */
