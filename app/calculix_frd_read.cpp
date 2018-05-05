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
            --remaining;
        }
        *buf = '\0';
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
        int c = stream->get();
        if (c == -1) {
            fail("expected EOL, got EOF");
        }
        if (c != '\n') {
            fail("expected EOL, got garbage");
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
                ElementType type =
                    static_cast<ElementType>(r.read_text_int<5>());
                if (!valid_element_type(type)) {
                    r.fail("unrecognized element type");
                }
                r.read_text_int<5>();
                r.read_text_int<5>();
                r.read_eol();

                int num_nodes =
                    ElementTypeInfo::get(type).shape->vertices.size();
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
            ElementType type = static_cast<ElementType>(r.read_bin_int());
            r.read_bin_int();
            r.read_bin_int();
            int num_nodes = ElementTypeInfo::get(type).shape->vertices.size();
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

enum class FrdAnalysis_ICType {
    Static = 0,
    TimeStep = 1,
    Frequency = 2,
    LoadStep = 3,
    UserNamed = 4
};

enum class Frd_IRType {
    NodalMaterialIndependent = 1,
    NodalMaterialDependent = 2,
    Element = 3
};

enum class FrdEntity_ICType {
    Scalar = 1,
    Vector_3 = 2,
    Matrix = 4,
    VectorAmplitudePhase_3 = 12
};

enum class Frd_IExist {
    Provided = 0,
    ShouldCalculate = 1,
    ProvidedEarmarked = 2
};

void read_nodal_results_block_entity(
    CalculixFrdReader &r,
    FrdEntity_ICType expected_ictype,
    int expected_icind1,
    int expected_icind2,
    Frd_IExist expected_iexist
) {
    r.read_indent(1);
    int key = r.read_text_int<2>();
    if (key != -5) {
        r.fail("expected key=-5 in nodal results block");
    }
    r.read_indent(2);
    char comp_name[8 + 1];
    r.read_text<8>(comp_name);
    int menu = r.read_text_int<5>();
    if (menu != 1) {
        r.fail("expected menu=1");
    }
    FrdEntity_ICType ictype =
        static_cast<FrdEntity_ICType>(r.read_text_int<5>());
    if (ictype != expected_ictype) {
        r.fail("unexpected value of ictype");
    }
    int icind1 = r.read_text_int<5>();
    if (icind1 != expected_icind1) {
        r.fail("unexpected value of icind1");
    }
    int icind2 = r.read_text_int<5>();
    if (icind2 != expected_icind2) {
        r.fail("unexpected value of icind2");
    }
    if (expected_iexist != Frd_IExist::Provided) {
        Frd_IExist iexist = static_cast<Frd_IExist>(r.read_text_int<5>());
        if (iexist != expected_iexist) {
            r.fail("unexpected value of iexist");
        }
        char icname[8 + 1];
        r.read_text<8>(icname);
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
    Results *results_out
) {
    char setname[6 + 1];
    r.read_text<6>(setname);
    double value = r.read_text_double<12>();
    if (value != 1.0) {
        r.fail("not implemented support for value!=1.0");
    }
    int numnod = r.read_text_int<12>();
    char text[20 + 1];
    r.read_text<20>(text);
    FrdAnalysis_ICType analysis_ictype =
        static_cast<FrdAnalysis_ICType>(r.read_text_int<2>());
    if (analysis_ictype != FrdAnalysis_ICType::Static) {
        r.fail("not implemented support for ictype!=static");
    }
    int numstp = r.read_text_int<5>();
    if (numstp != 1) {
        r.fail("not implemented support for numstp!=1");
    }
    char analys[10 + 1];
    r.read_text<10>(analys);
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
    int ncomps = r.read_text_int<5>();
    Frd_IRType irtype = static_cast<Frd_IRType>(r.read_text_int<5>());
    if (irtype != Frd_IRType::NodalMaterialIndependent) {
        r.fail("not implemented support for irtype!=1");
    }
    r.read_eol();

    if (ncomps == 4) {
        read_nodal_results_block_entity(r,
            FrdEntity_ICType::Vector_3, 1, 0, Frd_IExist::Provided);
        read_nodal_results_block_entity(r,
            FrdEntity_ICType::Vector_3, 2, 0, Frd_IExist::Provided);
        read_nodal_results_block_entity(r,
            FrdEntity_ICType::Vector_3, 3, 0, Frd_IExist::Provided);
        read_nodal_results_block_entity(r,
            FrdEntity_ICType::Vector_3, 0, 0, Frd_IExist::ShouldCalculate);

        ContiguousMap<NodeId, PureVector> result(
            node_id_begin, node_id_end,
            PureVector::raw(NAN, NAN, NAN));

        for (int i = 0; i < numnod; ++i) {
            NodeId node_id;
            double values[3];
            read_nodal_results_block_data(r, format, 3, &node_id, values);
            if (!result.key_in_range(node_id)) {
                r.fail("node id out of range");
            }
            result[node_id] = PureVector::raw(values[0], values[1], values[2]);
        }

        results_out->node_vectors[name] = std::move(result);

    } else {
        r.fail("not implemented ncomps!=4");
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
    Results *results_out
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
            read_nodal_results_block(
                r, node_id_begin, node_id_end, results_out);
        } else {
            r.fail("unrecognized block code");
        }
    }
}

} /* namespace os2cx */

