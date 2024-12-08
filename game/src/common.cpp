#include "common.hpp"

namespace tl {

uint32_t ParseFloat(std::string_view text, float* values, uint32_t count) {
    size_t begin = 0, end = 0;
    while (count > 0 && begin < text.size() && end < text.size()) {
        if (text[begin] == ' ') {
            begin++;
        } else {
            end = begin;
            while (end < text.size() && text[end] != ' ') {
                end++;
            }

            *values = std::atof(text.data() + begin);
            values++;
            count--;

            begin = end;
        }
    }

    return count;
}

#define CASE(expr) case expr: return #expr;

const char* GetXMLErrStr(tinyxml2::XMLError err) {
    switch (err) {
        CASE(tinyxml2::XML_SUCCESS)
        CASE(tinyxml2::XML_NO_ATTRIBUTE)
        CASE(tinyxml2::XML_WRONG_ATTRIBUTE_TYPE)
        CASE(tinyxml2::XML_ERROR_FILE_NOT_FOUND)
        CASE(tinyxml2::XML_ERROR_FILE_COULD_NOT_BE_OPENED)
        CASE(tinyxml2::XML_ERROR_PARSING_ELEMENT)
        CASE(tinyxml2::XML_ERROR_PARSING_ATTRIBUTE)
        CASE(tinyxml2::XML_ERROR_PARSING_TEXT)
        CASE(tinyxml2::XML_ERROR_PARSING_CDATA)
        CASE(tinyxml2::XML_ERROR_PARSING_COMMENT)
        CASE(tinyxml2::XML_ERROR_PARSING_DECLARATION)
        CASE(tinyxml2::XML_ERROR_PARSING_UNKNOWN)
        CASE(tinyxml2::XML_ERROR_EMPTY_DOCUMENT)
        CASE(tinyxml2::XML_ERROR_MISMATCHED_ELEMENT)
        CASE(tinyxml2::XML_ERROR_PARSING)
        CASE(tinyxml2::XML_CAN_NOT_CONVERT_TEXT)
        CASE(tinyxml2::XML_NO_TEXT_NODE)
        CASE(tinyxml2::XML_ELEMENT_DEPTH_EXCEEDED)
        CASE(tinyxml2::XML_ERROR_FILE_READ_ERROR)
        default:
            return "unknown error";
    }
}

#undef CASE

}  // namespace tl