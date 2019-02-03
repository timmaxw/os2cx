#include "calculix_inp_read.hpp"

namespace os2cx {

class CalculixInpReader {
public:
    CalculixInpReader(std::istream *s, Mesh3 *m) : stream(s), mesh(m) { }

    void read_entire_file() {
        next_line();
        while (!line.empty()) {
            if (line[0] == "*HEADING") {
                next_group();
            } else if (line[0] == "*NODE") {
                read_node_group();
            } else if (line[0] == "*ELEMENT") {
                read_element_group();
            } else if (line[0] == "**") {
                next_group();
            } else {
                fail("Unknown control word");
            }
        }
    }

    void next_line() {
        line.clear();
        std::string word;
        while (isspace(stream->peek())) {
            stream->ignore();
        }
        while (true) {
            int c = stream->get();
            if (c == ',') {
                line.push_back(word);
                word.clear();
                while (isspace(stream->peek())) {
                    stream->ignore();
                }
            } else if (isspace(c) || c == -1) {
                if (!word.empty()) {
                    line.push_back(word);
                }
                break;
            } else {
                word += std::toupper(c);
            }
        }
    }

    void next_group() {
        do {
            next_line();
        } while (!line.empty() && line[0][0] != '*');
    }

    void read_node_group() {
        while (true) {
            next_line();
            if (line.empty() || line[0][0] == '*') {
                break;
            }
            if (line.size() != 4) {
                fail("Expect node line to be four numbers");
            }
            NodeId node_id = NodeId::from_int(safe_atoi(line[0]));
            if (node_id != mesh->nodes.key_end()) {
                fail("Expect node numbers to be consecutive");
            }
            Node3 node;
            node.point = Point(
                safe_atod(line[1]),
                safe_atod(line[2]),
                safe_atod(line[3]));
            mesh->nodes.push_back(node);
        }
    }

    void read_element_group() {
        bool found_type = false;
        ElementType type;
        for (size_t i = 1; i < line.size(); ++i) {
            if (line[i].substr(0, 5) == "TYPE=") {
                try {
                    type = element_type_from_string(line[i].substr(5));
                } catch (const std::domain_error &) {
                    fail("Unrecognized element type: " + line[i].substr(5));
                }
                found_type = true;
                break;
            }
        }
        if (!found_type) {
            fail("Element group has no type specified");
        }
        int type_cardinality = element_type_shape(type).vertices.size();

        while (true) {
            next_line();
            if (line.empty() || line[0][0] == '*') {
                break;
            }
            if (static_cast<int>(line.size()) != 1 + type_cardinality) {
                fail("Expect element line to be 1+N numbers");
            }
            Element3 element;
            element.type = type;
            ElementId element_id = ElementId::from_int(safe_atoi(line[0]));
            if (element_id != mesh->elements.key_end()) {
                fail("Expect element numbers to be consecutive");
            }
            for (int i = 0; i < type_cardinality; ++i) {
                NodeId node_id = NodeId::from_int(safe_atoi(line[i+1]));
                if (!mesh->nodes.key_in_range(node_id)) {
                    fail("Element refers to a node that isn't in the mesh");
                }
                element.nodes[i] = node_id;
            }
            mesh->elements.push_back(element);
        }
    }

    int safe_atoi(const std::string &digits) {
        const char *c_str = digits.c_str();
        char *endptr;
        int value = strtol(c_str, &endptr, 10);
        if (digits.size() == 0 || *endptr != '\0') {
            fail("Invalid integer");
        }
        return value;
    }

    double safe_atod(const std::string &digits) {
        const char *c_str = digits.c_str();
        char *endptr;
        double value = strtod(c_str, &endptr);
        if (digits.size() == 0 || *endptr != '\0') {
            fail("Invalid floating-point number");
        }
        return value;
    }

    void fail(const std::string &message) {
        std::string line2 = line[0];
        for (size_t i = 1; i < line.size(); ++i) {
            line2 += ", " + line[i];
        }
        throw CalculixFileReadError(message + "\nOn line:\n" + line2);
    }

    std::istream *stream;
    std::vector<std::string> line;
    Mesh3 *mesh;
};

void read_calculix_nodes_and_elements(std::istream &stream, Mesh3 *mesh_out) {
    CalculixInpReader reader(&stream, mesh_out);
    reader.read_entire_file();
}


} /* namespace os2cx */

