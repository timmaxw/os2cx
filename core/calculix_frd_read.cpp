#include "calculix_frd_read.hpp"

#include <sstream>

namespace os2cx {

class CalculixFrdReader {
public:
    CalculixFrdReader(std::istream *s) : stream(s), line_no(1) { }

    template<int size>
    void read_text(char *buf) {
        int remaining = size;
        while (remaining > 0 && stream->peek() == ' ') {
            stream->ignore();
            --remaining;
        }
        char *insert_nul_at = buf;
        while (remaining > 0) {
            int c = stream->peek();
            if (c == '\n') {
                break;
            }
            if (c == -1) {
                fail("unexpected EOF");
            }
            if (c == '\0') {
                fail("unexpected NUL byte");
            }
            *buf = stream->get();
            ++buf;
            if (c != ' ') {
                /* don't update insert_nul_at unless we found a non-space
                character; this will ensure that we strip trailing spaces */
                insert_nul_at = buf;
            }
            --remaining;
        }
        *insert_nul_at = '\0';
    }

    template<int size>
    int read_text_int() {
        char buffer[size + 1];
        read_text<size>(buffer);
        char *endptr;
        int value = strtol(buffer, &endptr, 10);
        if (*buffer == '\0' || *endptr != '\0') {
            fail(std::string("invalid integer: '") + buffer + "'");
        }
        return value;
    }

    template<int size>
    double read_text_double() {
        char buffer[size + 1];
        read_text<size>(buffer);
        char *endptr;
        double value = strtod(buffer, &endptr);
        if (*buffer == '\0' || *endptr != '\0') {
            fail(std::string("invalid double: '") + buffer + "'");
        }
        return value;
    }

    template<int size>
    void read_bin(char *buf) {
        stream->read(buf, size);
        if (stream->eof()) {
            fail("unexpected EOF");
        }
        for (int i = 0; i < size; ++i) {
            if (buf[i] == '\n') {
                ++line_no;
            }
        }
    }

    int read_bin_int() {
        union {
            int value;
            char buf[sizeof(int)];
        };
        read_bin<sizeof(int)>(buf);
        return value;
    }

    float read_bin_float() {
        union {
            float value;
            char buf[sizeof(float)];
        };
        read_bin<sizeof(float)>(buf);
        return value;
    }

    double read_bin_double() {
        union {
            double value;
            char buf[sizeof(double)];
        };
        read_bin<sizeof(double)>(buf);
        return value;
    }

    void read_indent(int indent) {
        while (indent > 0) {
            int c = stream->get();
            if (c == -1) {
                fail("unexpected EOF");
            }
            if (c != ' ') {
                fail("expected space, got something else");
            }
            --indent;
        }
    }

    void read_eol() {
        while (true) {
            int c = stream->get();
            if (c == -1) {
                fail("expected EOL, got EOF");
            } else if (c == '\n') {
                break;
            } else if (c != ' ') {
                fail("expected EOL, got '" + std::string(1, c) + "'");
            }
        }
        ++line_no;
    }

    void read_eof() {
        if (stream->peek() != -1) {
            fail("expected EOF");
        }
    }

    void fail(const std::string &message) {
        std::stringstream ss;
        ss << message << " (at line " << line_no << ")";
        throw CalculixFrdFileReadError(ss.str());
    }

    std::istream *stream;
    int line_no;
};

void read_user_header_record(CalculixFrdReader &r) {
    char buf[66 + 1];
    r.read_text<66>(buf);
    r.read_eol();
}

enum class FrdFormat {
    TextShort = 0,
    TextLong = 1,
    BinaryFloat = 2,
    BinaryDouble = 3
};

void read_nodal_point_coordinate_block(CalculixFrdReader &r) {
    r.read_indent(18);
    int numnod = r.read_text_int<12>();
    r.read_indent(37);
    FrdFormat format = static_cast<FrdFormat>(r.read_text_int<1>());
    r.read_eol();

    if (format == FrdFormat::TextShort || format == FrdFormat::TextLong) {
        while (true) {
            r.read_indent(1);
            int code = r.read_text_int<2>();
            if (code == -1) {
                if (format == FrdFormat::TextShort) {
                    r.read_text_int<5>();
                } else {
                    r.read_text_int<10>();
                }
                r.read_text_double<12>();
                r.read_text_double<12>();
                r.read_text_double<12>();
                r.read_eol();
            } else if (code == -3) {
                r.read_eol();
                break;
            } else {
                r.fail("bad code in nodal point coordinate block");
            }
        }
    } else if (format == FrdFormat::BinaryFloat ||
            format == FrdFormat::BinaryDouble) {
        for (int i = 0; i < numnod; ++i) {
            r.read_bin_int();
            for (int i = 0; i < 3; ++i) {
                if (format == FrdFormat::BinaryFloat) {
                    r.read_bin_float();
                } else {
                    r.read_bin_double();
                }
            }
        }
    } else {
        r.fail("bad format in nodal point coordinate block");
    }
}

enum CalculixFrdElementType {
    Beam2 = 11,
    Beam3 = 12,
    Shell3 = 7,
    Shell6 = 8,
    Shell4 = 9,
    Shell8 = 10,
    Tet4 = 3,
    Tet10 = 6,
    Brick8 = 1,
    Brick20 = 4,
    Penta6 = 2,
    Penta15 = 5
};

bool valid_frd_element_type(int type) {
    return (type >= 1 && type <= 12);
}

int nodes_for_frd_element_type(CalculixFrdElementType type) {
    switch (type) {
    case Beam2: return 2;
    case Beam3: return 3;
    case Shell3: return 3;
    case Shell6: return 6;
    case Shell4: return 4;
    case Shell8: return 8;
    case Tet4: return 4;
    case Tet10: return 10;
    case Brick8: return 8;
    case Brick20: return 20;
    case Penta6: return 6;
    case Penta15: return 15;
    default: assert(false);
    }
}

void read_element_definition_block(CalculixFrdReader &r) {
    r.read_indent(18);
    int numelem = r.read_text_int<12>();
    r.read_indent(37);
    FrdFormat format = static_cast<FrdFormat>(r.read_text_int<1>());
    r.read_eol();

    if (format == FrdFormat::TextShort || format == FrdFormat::TextLong) {
        while (true) {
            r.read_indent(1);
            int code = r.read_text_int<2>();
            if (code == -1) {
                if (format == FrdFormat::TextShort) {
                    r.read_text_int<5>();
                } else {
                    r.read_text_int<10>();
                }
                int frd_element_type = r.read_text_int<5>();
                if (!valid_frd_element_type(frd_element_type)) {
                    r.fail("unrecognized element type");
                }
                r.read_text_int<5>();
                r.read_text_int<5>();
                r.read_eol();

                int num_nodes = nodes_for_frd_element_type(
                    static_cast<CalculixFrdElementType>(frd_element_type));
                while (num_nodes > 0) {
                    r.read_indent(1);
                    code = r.read_text_int<2>();
                    if (code != -2) {
                        r.fail("expected nodes for element");
                    }
                    int num_nodes_this_line;
                    if (format == FrdFormat::TextShort) {
                        num_nodes_this_line = std::min(num_nodes, 15);
                    } else {
                        num_nodes_this_line = std::min(num_nodes, 10);
                    }
                    num_nodes -= num_nodes_this_line;
                    for (int i = 0; i < num_nodes_this_line; ++i) {
                        if (format == FrdFormat::TextShort) {
                            r.read_text_int<5>();
                        } else {
                            r.read_text_int<10>();
                        }
                    }
                    r.read_eol();
                }
            } else if (code == -3) {
                r.read_eol();
                break;
            } else {
                r.fail("bad code in element definition block");
            }
        }
    } else if (format == FrdFormat::BinaryFloat) {
        for (int i = 0; i < numelem; ++i) {
            r.read_bin_int();
            int frd_element_type = r.read_bin_int();
            r.read_bin_int();
            r.read_bin_int();
            int num_nodes = nodes_for_frd_element_type(
                static_cast<CalculixFrdElementType>(frd_element_type));
            for (int i = 0; i < num_nodes; ++i) {
                r.read_bin_int();
            }
        }
    } else {
        r.fail("bad format in element definition block");
    }
}

void read_parameter_header_record(CalculixFrdReader &r) {
    char buf[66 + 1];
    r.read_text<66>(buf);
    r.read_eol();
}

void read_nodal_results_block_entity(
    CalculixFrdReader &r,
    FrdEntity *entity_out
) {
    r.read_indent(1);
    int key = r.read_text_int<2>();
    if (key != -5) {
        r.fail("expected key=-5 in nodal results block");
    }
    r.read_indent(2);

    char name[8 + 1];
    r.read_text<8>(name);
    entity_out->name = std::string(name);

    entity_out->menu = r.read_text_int<5>();
    if (entity_out->menu != 1) {
        r.fail("expected menu=1");
    }

    entity_out->type = static_cast<FrdEntity::Type>(r.read_text_int<5>());
    entity_out->ind1 = r.read_text_int<5>();
    entity_out->ind2 = r.read_text_int<5>();

    try {
        entity_out->exist = static_cast<FrdEntity::Exist>(r.read_text_int<5>());
    } catch (const CalculixFrdFileReadError &) {
        entity_out->exist = FrdEntity::Exist::Provided;
    }

    if (entity_out->exist != FrdEntity::Exist::Provided) {
        char calculation_name[8 + 1];
        r.read_text<8>(calculation_name);
        entity_out->calculation_name = std::string(calculation_name);
    }

    r.read_eol();
}

void read_nodal_results_block_data(
    CalculixFrdReader &r,
    FrdFormat format,
    int num_values,
    NodeId *node_id_out,
    double *values_out
) {
    if (format == FrdFormat::TextShort || format == FrdFormat::TextLong) {
        r.read_indent(1);
        int key = r.read_text_int<2>();
        if (key != -1) {
            r.fail("expected key=-1 in nodal results block");
        }
        if (format == FrdFormat::TextShort) {
            *node_id_out = NodeId::from_int(r.read_text_int<5>());
        } else {
            *node_id_out = NodeId::from_int(r.read_text_int<10>());
        }
        int i = 0;
        for (; i < num_values && i < 6; ++i) {
            values_out[i] = r.read_text_double<12>();
        }
        r.read_eol();

        while (i < num_values) {
            r.read_indent(1);
            int key = r.read_text_int<2>();
            if (key != -2) {
                r.fail("expected key=-2 in nodal results block");
            }
            for (int j = 0; i < num_values && j < 6; ++i, ++j) {
                values_out[i] = r.read_text_double<12>();
            }
            r.read_eol();
        }
    } else if (format == FrdFormat::BinaryFloat ||
            format == FrdFormat::BinaryDouble) {
        *node_id_out = NodeId::from_int(r.read_bin_int());
        for (int i = 0; i < num_values; ++i) {
            if (format == FrdFormat::BinaryFloat) {
                values_out[i] = r.read_bin_float();
            } else {
                values_out[i] = r.read_bin_double();
            }
        }
    } else {
        r.fail("bad format in nodal results block");
    }
}

void read_nodal_results_block(
    CalculixFrdReader &r,
    NodeId node_id_begin,
    NodeId node_id_end,
    FrdAnalysis *analysis_out
) {
    char setname[6 + 1];
    r.read_text<6>(setname);
    analysis_out->setname = std::string(setname);

    analysis_out->value = r.read_text_double<12>();

    int numnod = r.read_text_int<12>();

    char text[20 + 1];
    r.read_text<20>(text);
    analysis_out->text = std::string(text);

    analysis_out->ctype = static_cast<FrdAnalysis::CType>(r.read_text_int<2>());

    analysis_out->numstp = r.read_text_int<5>();

    char analys[10 + 1];
    r.read_text<10>(analys);
    analysis_out->analys = analys;

    FrdFormat format = static_cast<FrdFormat>(r.read_text_int<2>());
    r.read_eol();

    r.read_indent(1);
    int key = r.read_text_int<2>();
    if (key != -4) {
        r.fail("expected key=-4 in nodal results block");
    }
    r.read_indent(2);

    char name[8 + 1];
    r.read_text<8>(name);
    analysis_out->name = std::string(name);

    int ncomps = r.read_text_int<5>();

    analysis_out->rtype = static_cast<FrdAnalysis::RType>(r.read_text_int<5>());

    r.read_eol();

    analysis_out->entities.resize(ncomps);
    int ncomps_present = 0;
    for (int i = 0; i < ncomps; ++i) {
        read_nodal_results_block_entity(r, &analysis_out->entities[i]);

        if (analysis_out->entities[i].exist == FrdEntity::Exist::Provided) {
            analysis_out->entities[i].data =
                ContiguousMap<NodeId, double>(node_id_begin, node_id_end, NAN);
            ++ncomps_present;
        }
    }

    std::vector<double> values(ncomps_present);
    for (int i = 0; i < numnod; ++i) {
        NodeId node_id;
        read_nodal_results_block_data(
            r, format, ncomps_present, &node_id, values.data());
        if (node_id < node_id_begin || !(node_id < node_id_end)) {
            r.fail("node id out of range");
        }

        int i_present = 0;
        for (int i = 0; i < ncomps; ++i) {
            if (analysis_out->entities[i].exist == FrdEntity::Exist::Provided) {
                analysis_out->entities[i].data[node_id] = values[i_present];
                ++i_present;
            }
        }
    }

    if (format == FrdFormat::TextShort || format == FrdFormat::TextLong) {
        r.read_indent(1);
        int key = r.read_text_int<2>();
        if (key != -3) {
            r.fail("expected key=-3 in nodal results block");
        }
        r.read_eol();
    }
}

void read_calculix_frd(
    std::istream &stream,
    NodeId node_id_begin,
    NodeId node_id_end,
    std::vector<FrdAnalysis> *analyses_out
) {
    CalculixFrdReader r(&stream);

    r.read_indent(1);
    int key = r.read_text_int<4>();
    char code[1 + 1];
    r.read_text<1>(code);
    r.read_eol();
    if (key != 1 || code[0] != 'C') {
        r.fail("expected header");
    }

    while (true) {
        r.read_indent(1);
        int key = r.read_text_int<4>();
        if (key == 9999) {
            r.read_eol();
            r.read_eof();
            break;
        }
        char code[2];
        r.read_text<1>(code);
        if (key == 1 && code[0] == 'U') {
            read_user_header_record(r);
        } else if (key == 2 && code[0] == 'C') {
            read_nodal_point_coordinate_block(r);
        } else if (key == 3 && code[0] == 'C') {
            read_element_definition_block(r);
        } else if (key == 1 && code[0] == 'P') {
            read_parameter_header_record(r);
        } else if (key == 100 && code[0] == 'C') {
            FrdAnalysis analysis;
            read_nodal_results_block(r, node_id_begin, node_id_end, &analysis);
            analyses_out->push_back(std::move(analysis));
        } else {
            r.fail("unrecognized block code");
        }
    }
}

} /* namespace os2cx */

