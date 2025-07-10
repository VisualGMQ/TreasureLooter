#include "rapidxml.hpp"
#include "lyra/lyra.hpp"

int main(int argc, char** argv) {
    rapidxml::xml_document<> doc;
    std::string data = "<welcome>Hello RapidXML</welcome>";
    doc.parse<0>(data.data());
    rapidxml::xml_node<>* node = doc.first_node("welcome");
    std::cout << node->value() << std::endl;
    return 0;
}