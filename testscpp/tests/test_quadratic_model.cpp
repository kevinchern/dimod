// Copyright 2021 D-Wave Systems Inc.
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.

#include <iostream>

#include "../Catch2/single_include/catch2/catch.hpp"
#include "dimod/quadratic_model.h"

namespace dimod {

TEMPLATE_TEST_CASE_SIG("Scenario: BinaryQuadraticModel tests", "[qmbase][bqm]",
                       ((typename Bias, Vartype vartype), Bias, vartype),
                       (double, Vartype::BINARY), (double, Vartype::SPIN),
                       (float, Vartype::BINARY), (float, Vartype::SPIN)) {
    GIVEN("an empty BQM") {
        auto bqm = BinaryQuadraticModel<Bias>(vartype);

        WHEN("the bqm is resized") {
            bqm.resize(10);

            THEN("it will have the correct number of variables with 0 bias") {
                REQUIRE(bqm.num_variables() == 10);
                REQUIRE(bqm.num_interactions() == 0);
                for (auto v = 0u; v < bqm.num_variables(); ++v) {
                    REQUIRE(bqm.linear(v) == 0);
                }
            }
        }

        AND_GIVEN("some COO-formatted arrays") {
            int irow[4] = {0, 2, 0, 1};
            int icol[4] = {0, 2, 1, 2};
            float bias[4] = {.5, -2, 2, -3};
            std::size_t length = 4;

            WHEN("we add the biases with add_quadratic") {
                bqm.add_quadratic(&irow[0], &icol[0], &bias[0], length);

                THEN("it takes its values from the arrays") {
                    REQUIRE(bqm.num_variables() == 3);

                    if (bqm.vartype() == Vartype::SPIN) {
                        REQUIRE(bqm.linear(0) == 0);
                        REQUIRE(bqm.linear(1) == 0);
                        REQUIRE(bqm.linear(2) == 0);
                        REQUIRE(bqm.offset() == -1.5);
                    } else {
                        REQUIRE(bqm.vartype() == Vartype::BINARY);
                        REQUIRE(bqm.linear(0) == .5);
                        REQUIRE(bqm.linear(1) == 0);
                        REQUIRE(bqm.linear(2) == -2);
                        REQUIRE(bqm.offset() == 0);
                    }

                    REQUIRE(bqm.num_interactions() == 2);
                    REQUIRE(bqm.quadratic(0, 1) == 2);
                    REQUIRE(bqm.quadratic(2, 1) == -3);
                    REQUIRE_THROWS_AS(bqm.quadratic_at(0, 2),
                                      std::out_of_range);
                }
            }
        }

        AND_GIVEN("some COO-formatted arrays with duplicates") {
            int irow[6] = {0, 2, 0, 1, 0, 0};
            int icol[6] = {0, 2, 1, 2, 1, 0};
            float bias[6] = {.5, -2, 2, -3, 4, 1};
            std::size_t length = 6;

            WHEN("we add the biases with add_quadratic") {
                bqm.add_quadratic(&irow[0], &icol[0], &bias[0], length);

                THEN("it combines duplicate values") {
                    REQUIRE(bqm.num_variables() == 3);

                    if (bqm.vartype() == Vartype::SPIN) {
                        REQUIRE(bqm.linear(0) == 0);
                        REQUIRE(bqm.linear(1) == 0);
                        REQUIRE(bqm.linear(2) == 0);
                        REQUIRE(bqm.offset() == -.5);
                    } else {
                        REQUIRE(bqm.vartype() == Vartype::BINARY);
                        REQUIRE(bqm.linear(0) == 1.5);
                        REQUIRE(bqm.linear(1) == 0.);
                        REQUIRE(bqm.linear(2) == -2);
                        REQUIRE(bqm.offset() == 0);
                    }

                    REQUIRE(bqm.num_interactions() == 2);
                    REQUIRE(bqm.quadratic(0, 1) == 6);
                    REQUIRE(bqm.quadratic(2, 1) == -3);
                    REQUIRE_THROWS_AS(bqm.quadratic_at(0, 2),
                                      std::out_of_range);
                }
            }
        }

        AND_GIVEN("some COO-formatted arrays with multiple duplicates") {
            int irow[4] = {0, 1, 0, 1};
            int icol[4] = {1, 2, 1, 0};
            float bias[4] = {-1, 1, -2, -3};
            std::size_t length = 4;

            WHEN("we add the biases with add_quadratic") {
                bqm.add_quadratic(&irow[0], &icol[0], &bias[0], length);

                THEN("it combines duplicate values") {
                    REQUIRE(bqm.num_variables() == 3);
                    REQUIRE(bqm.linear(0) == 0);
                    REQUIRE(bqm.linear(1) == 0);
                    REQUIRE(bqm.linear(2) == 0);

                    REQUIRE(bqm.num_interactions() == 2);
                    REQUIRE(bqm.quadratic(0, 1) == -6);
                    REQUIRE(bqm.quadratic(1, 0) == -6);
                    REQUIRE(bqm.quadratic(2, 1) == 1);
                    REQUIRE(bqm.quadratic(1, 2) == 1);
                    REQUIRE_THROWS_AS(bqm.quadratic_at(0, 2),
                                      std::out_of_range);
                    REQUIRE_THROWS_AS(bqm.quadratic_at(2, 0),
                                      std::out_of_range);
                }
            }
        }
    }

    GIVEN("a BQM constructed from a dense array") {
        float Q[9] = {1, 0, 3, 2, 1, 0, 1, 0, 0};
        int num_variables = 3;

        auto bqm = BinaryQuadraticModel<Bias>(Q, num_variables, vartype);

        THEN("it handles the diagonal according to its vartype") {
            REQUIRE(bqm.num_variables() == 3);

            if (bqm.vartype() == Vartype::SPIN) {
                REQUIRE(bqm.linear(0) == 0);
                REQUIRE(bqm.linear(1) == 0);
                REQUIRE(bqm.linear(2) == 0);
                REQUIRE(bqm.offset() == 2);
            } else {
                REQUIRE(bqm.vartype() == Vartype::BINARY);
                REQUIRE(bqm.linear(0) == 1);
                REQUIRE(bqm.linear(1) == 1);
                REQUIRE(bqm.linear(2) == 0);
                REQUIRE(bqm.offset() == 0);
            }
        }

        THEN("it gets its quadratic from the off-diagonal") {
            REQUIRE(bqm.num_interactions() == 2);

            // test both forward and backward
            REQUIRE(bqm.quadratic(0, 1) == 2);
            REQUIRE(bqm.quadratic(1, 0) == 2);
            REQUIRE(bqm.quadratic(0, 2) == 4);
            REQUIRE(bqm.quadratic(2, 0) == 4);
            REQUIRE(bqm.quadratic(1, 2) == 0);
            REQUIRE(bqm.quadratic(2, 1) == 0);

            // ignores 0s
            REQUIRE_THROWS_AS(bqm.quadratic_at(1, 2), std::out_of_range);
            REQUIRE_THROWS_AS(bqm.quadratic_at(2, 1), std::out_of_range);
        }

        THEN("we can iterate over the neighborhood") {
            auto span = bqm.neighborhood(0);
            auto pairs = std::vector<std::pair<std::size_t, Bias>>(span.first,
                                                                   span.second);

            REQUIRE(pairs[0].first == 1);
            REQUIRE(pairs[0].second == 2);
            REQUIRE(pairs[1].first == 2);
            REQUIRE(pairs[1].second == 4);
            REQUIRE(pairs.size() == 2);
        }
    }

    GIVEN("a BQM with five variables, two interactions and an offset") {
        auto bqm = BinaryQuadraticModel<Bias>(5, vartype);
        bqm.linear(0) = 1;
        bqm.linear(1) = -3.25;
        bqm.linear(2) = 0;
        bqm.linear(3) = 3;
        bqm.linear(4) = -4.5;
        bqm.set_quadratic(0, 3, -1);
        bqm.set_quadratic(3, 1, 5.6);
        bqm.set_quadratic(0, 1, 1.6);
        bqm.offset() = -3.8;

        AND_GIVEN("the set of all possible five variable samples") {
            // there are smarter ways to do this but it's simple
            std::vector<std::vector<int>> spn_samples;
            std::vector<std::vector<int>> bin_samples;
            for (auto i = 0; i < 1 << bqm.num_variables(); ++i) {
                std::vector<int> bin_sample;
                std::vector<int> spn_sample;
                for (size_t v = 0; v < bqm.num_variables(); ++v) {
                    bin_sample.push_back((i >> v) & 1);
                    spn_sample.push_back(2 * ((i >> v) & 1) - 1);
                }

                bin_samples.push_back(bin_sample);
                spn_samples.push_back(spn_sample);
            }

            std::vector<double> energies;
            if (vartype == Vartype::SPIN) {
                for (auto& sample : spn_samples) {
                    energies.push_back(bqm.energy(sample));
                }
            } else {
                for (auto& sample : bin_samples) {
                    energies.push_back(bqm.energy(sample));
                }
            }

            WHEN("we change the vartype to spin") {
                bqm.change_vartype(Vartype::SPIN);

                THEN("the energies will match") {
                    for (size_t si = 0; si < energies.size(); ++si) {
                        REQUIRE(energies[si] ==
                                Approx(bqm.energy(spn_samples[si])));
                    }
                }
            }

            WHEN("we change the vartype to binary") {
                bqm.change_vartype(Vartype::BINARY);
                THEN("the energies will match") {
                    for (size_t si = 0; si < energies.size(); ++si) {
                        REQUIRE(energies[si] ==
                                Approx(bqm.energy(bin_samples[si])));
                    }
                }
            }
        }
    }
}

TEMPLATE_TEST_CASE_SIG(
        "Scenario: BQMs can be combined", "[bqm]",
        ((typename B0, typename B1, Vartype vartype), B0, B1, vartype),
        (float, float, Vartype::BINARY), (double, float, Vartype::BINARY),
        (float, double, Vartype::BINARY), (double, double, Vartype::BINARY),
        (float, float, Vartype::SPIN), (double, float, Vartype::SPIN),
        (float, double, Vartype::SPIN), (double, double, Vartype::SPIN)) {
    GIVEN("a BQM with 3 variables") {
        auto bqm0 = BinaryQuadraticModel<B0>(3, vartype);
        bqm0.linear(2) = -1;
        bqm0.set_quadratic(0, 1, 1.5);
        bqm0.set_quadratic(0, 2, -2);
        bqm0.set_quadratic(1, 2, 7);
        bqm0.offset() = -4;

        AND_GIVEN("a BQM with 5 variables and the same vartype") {
            auto bqm1 = BinaryQuadraticModel<B1>(5, vartype);
            bqm1.linear(0) = 1;
            bqm1.linear(1) = -3.25;
            bqm1.linear(2) = 2;
            bqm1.linear(3) = 3;
            bqm1.linear(4) = -4.5;
            bqm1.set_quadratic(0, 1, 5.6);
            bqm1.set_quadratic(0, 3, -1);
            bqm1.set_quadratic(1, 2, 1.6);
            bqm1.set_quadratic(3, 4, -25);
            bqm1.offset() = -3.8;

            WHEN("the first is updated with the second") {
                bqm0.add_bqm(bqm1);

                THEN("the biases are added") {
                    REQUIRE(bqm0.num_variables() == 5);
                    REQUIRE(bqm0.num_interactions() == 5);

                    CHECK(bqm0.offset() == Approx(-7.8));

                    CHECK(bqm0.linear(0) == Approx(1));
                    CHECK(bqm0.linear(1) == Approx(-3.25));
                    CHECK(bqm0.linear(2) == Approx(1));
                    CHECK(bqm0.linear(3) == Approx(3));
                    CHECK(bqm0.linear(4) == Approx(-4.5));

                    CHECK(bqm0.quadratic(0, 1) == Approx(7.1));
                    CHECK(bqm0.quadratic(0, 2) == Approx(-2));
                    CHECK(bqm0.quadratic(0, 3) == Approx(-1));
                    CHECK(bqm0.quadratic(1, 2) == Approx(8.6));
                    CHECK(bqm0.quadratic(3, 4) == Approx(-25));
                }
            }

            WHEN("the second is updated with the first") {
                bqm1.add_bqm(bqm0);

                THEN("the biases are added") {
                    REQUIRE(bqm1.num_variables() == 5);
                    REQUIRE(bqm1.num_interactions() == 5);

                    CHECK(bqm1.offset() == Approx(-7.8));

                    CHECK(bqm1.linear(0) == Approx(1));
                    CHECK(bqm1.linear(1) == Approx(-3.25));
                    CHECK(bqm1.linear(2) == Approx(1));
                    CHECK(bqm1.linear(3) == Approx(3));
                    CHECK(bqm1.linear(4) == Approx(-4.5));

                    CHECK(bqm1.quadratic(0, 1) == Approx(7.1));
                    CHECK(bqm1.quadratic(0, 2) == Approx(-2));
                    CHECK(bqm1.quadratic(0, 3) == Approx(-1));
                    CHECK(bqm1.quadratic(1, 2) == Approx(8.6));
                    CHECK(bqm1.quadratic(3, 4) == Approx(-25));
                }
            }

            WHEN("the update involves a mapping") {
                std::vector<int> mapping = {7, 2, 0};
                bqm1.add_bqm(bqm0, mapping);

                THEN("the biases are mapped appropriately") {
                    REQUIRE(bqm1.num_variables() == 8);
                    REQUIRE(bqm1.num_interactions() == 6);

                    CHECK(bqm1.offset() == Approx(-7.8));

                    CHECK(bqm1.linear(0) == Approx(0));
                    CHECK(bqm1.linear(1) == Approx(-3.25));
                    CHECK(bqm1.linear(2) == Approx(2));
                    CHECK(bqm1.linear(3) == Approx(3));
                    CHECK(bqm1.linear(4) == Approx(-4.5));
                    CHECK(bqm1.linear(5) == Approx(0));
                    CHECK(bqm1.linear(6) == Approx(0));
                    CHECK(bqm1.linear(7) == Approx(0));

                    CHECK(bqm1.quadratic(0, 1) == Approx(5.6));
                    CHECK(bqm1.quadratic(0, 2) == Approx(1.5));
                    CHECK(bqm1.quadratic(0, 3) == Approx(-1));
                    CHECK(bqm1.quadratic(1, 2) == Approx(1.6));
                    CHECK(bqm1.quadratic(3, 4) == Approx(-25));
                }
            }
        }

        AND_GIVEN("a BQM with 5 variables and a different vartype") {
            Vartype vt;
            if (vartype == Vartype::SPIN) {
                vt = Vartype::BINARY;
            } else {
                vt = Vartype::SPIN;
            }

            auto bqm1 = BinaryQuadraticModel<B1>(5, vt);
            bqm1.linear(0) = 1;
            bqm1.linear(1) = -3.25;
            bqm1.linear(2) = 2;
            bqm1.linear(3) = 3;
            bqm1.linear(4) = -4.5;
            bqm1.set_quadratic(0, 1, 5.6);
            bqm1.set_quadratic(0, 3, -1);
            bqm1.set_quadratic(1, 2, 1.6);
            bqm1.set_quadratic(3, 4, -25);
            bqm1.offset() = -3.8;

            WHEN("the first is updated with the second") {
                auto bqm0_cp = BinaryQuadraticModel<B0>(bqm0);
                auto bqm1_cp = BinaryQuadraticModel<B1>(bqm1);

                bqm0.add_bqm(bqm1);

                THEN("it was as if the vartype was changed first") {
                    bqm1_cp.change_vartype(vartype);
                    bqm0_cp.add_bqm(bqm1_cp);

                    REQUIRE(bqm0.num_variables() == bqm0_cp.num_variables());
                    REQUIRE(bqm0.num_interactions() ==
                            bqm0_cp.num_interactions());
                    REQUIRE(bqm0.offset() == Approx(bqm0_cp.offset()));
                    for (auto u = 0u; u < bqm0.num_variables(); ++u) {
                        REQUIRE(bqm0.linear(u) == Approx(bqm0_cp.linear(u)));

                        auto span = bqm0.neighborhood(u);
                        for (auto it = span.first; it != span.second; ++it) {
                            REQUIRE((*it).second == Approx(bqm0_cp.quadratic_at(
                                                            u, (*it).first)));
                        }
                    }
                }
            }
        }
    }
}

SCENARIO("Neighborhood can be manipulated") {
    GIVEN("An empty Neighborhood") {
        auto neighborhood = Neighborhood<float, size_t>();

        WHEN("some variables/biases are emplaced") {
            neighborhood.emplace_back(0, .5);
            neighborhood.emplace_back(1, 1.5);
            neighborhood.emplace_back(3, -3);

            THEN("we can retrieve the biases with .at()") {
                REQUIRE(neighborhood.size() == 3);
                REQUIRE(neighborhood.at(0) == .5);
                REQUIRE(neighborhood.at(1) == 1.5);
                REQUIRE(neighborhood.at(3) == -3);

                // should throw an error
                REQUIRE_THROWS_AS(neighborhood.at(2), std::out_of_range);
                REQUIRE(neighborhood.size() == 3);
            }

            THEN("we can retrieve the biases with []") {
                REQUIRE(neighborhood.size() == 3);
                REQUIRE(neighborhood[0] == .5);
                REQUIRE(neighborhood[1] == 1.5);
                REQUIRE(neighborhood[2] == 0);  // created
                REQUIRE(neighborhood[3] == -3);
                REQUIRE(neighborhood.size() == 4);  // since 2 was inserted
            }

            THEN("we can retrieve the biases with .get()") {
                REQUIRE(neighborhood.size() == 3);
                REQUIRE(neighborhood.get(0) == .5);
                REQUIRE(neighborhood.get(1) == 1.5);
                REQUIRE(neighborhood.get(1, 2) == 1.5);  // use real value
                REQUIRE(neighborhood.get(2) == 0);
                REQUIRE(neighborhood.get(2, 1.5) == 1.5);  // use default
                REQUIRE(neighborhood.at(3) == -3);
                REQUIRE(neighborhood.size() == 3);  // should not change
            }

            THEN("we can modify the biases with []") {
                neighborhood[0] += 7;
                neighborhood[2] -= 3;

                REQUIRE(neighborhood.at(0) == 7.5);
                REQUIRE(neighborhood.at(2) == -3);
            }

            THEN("we can create a vector from the neighborhood") {
                std::vector<std::pair<size_t, float>> pairs(
                        neighborhood.begin(), neighborhood.end());

                REQUIRE(pairs[0].first == 0);
                REQUIRE(pairs[0].second == .5);
                REQUIRE(pairs[1].first == 1);
                REQUIRE(pairs[1].second == 1.5);
                REQUIRE(pairs[2].first == 3);
                REQUIRE(pairs[2].second == -3);
            }

            THEN("we can create a vector from the const neighborhood") {
                std::vector<std::pair<size_t, float>> pairs(
                        neighborhood.cbegin(), neighborhood.cend());

                REQUIRE(pairs[0].first == 0);
                REQUIRE(pairs[0].second == .5);
                REQUIRE(pairs[1].first == 1);
                REQUIRE(pairs[1].second == 1.5);
                REQUIRE(pairs[2].first == 3);
                REQUIRE(pairs[2].second == -3);
            }

            THEN("we can modify the biases via the iterator") {
                auto it = neighborhood.begin();

                (*it).second = 18;
                REQUIRE(neighborhood.at(0) == 18);

                it++;
                (*it).second = -48;
                REQUIRE(neighborhood.at(1) == -48);

                // it++;
                // it->second += 1;
            }
        }
    }
}
}  // namespace dimod
