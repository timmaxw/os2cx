#include <iostream>

#include "project_run.hpp"

namespace os2cx {

void print_measurements(const Project &project) {
    int i = 1;
    for (const auto &result : project.results->results) {
        std::cout << "Result " << i;
        ++i;

        if (result.type == Results::Result::Type::Static) {
            std::cout << " (static)";
        } else if (result.type == Results::Result::Type::Eigenmode) {
            std::cout << " (eigenmodes)";
        } else if (result.type == Results::Result::Type::ModalDynamic) {
            std::cout << " (modal dynamic)";
        } else {
            assert(false);
        }
        std::cout << std::endl;

        for (const Results::Result::Step &step : result.steps) {
            if (result.type == Results::Result::Type::Eigenmode ||
                    result.type == Results::Result::Type::ModalDynamic) {
                double period = 1 / step.frequency;
                Unit unit_s("s", UnitType::Time, 1.0, Unit::Style::Metric);
                WithUnit<double> period_in_s =
                    project.unit_system.system_to_unit(unit_s, period);
                double frequency_in_Hz = 1 / period_in_s.value_in_unit;
                std::cout << "Frequency = " << frequency_in_Hz << "Hz" << std::endl;
            }

            for (const auto &measure_pair : project.measure_objects) {
                std::cout << measure_pair.first << " = ";

                double max_datum = measure_pair.second.measure(project, step);

                if (isnan(max_datum)) {
                    std::cout << "N/A";
                } else {
                    Unit unit = project.unit_system.suggest_unit(
                        guess_unit_type_for_dataset(measure_pair.second.dataset),
                        max_datum);
                    WithUnit<double> max_datum_2 =
                        project.unit_system.system_to_unit(unit, max_datum);
                    std::cout << max_datum_2.value_in_unit;
                    std::cout << unit.name;
                }
                std::cout << std::endl;
            }
        }
    }
}

}  /* namespace os2cx */

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: os2cx path/to/file.scad" << std::endl;
        return 1;
    }
    std::string scad_path = argv[1];

    os2cx::Project project(scad_path);
    os2cx::ProjectRunCallbacks callbacks;

    os2cx::project_run(&project, &callbacks);

    os2cx::print_measurements(project);

    return 0;
}
