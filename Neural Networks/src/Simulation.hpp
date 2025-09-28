#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Main.hpp>

#include "../../build/_deps/imgui-src/imgui.h"
#include "../../build/_deps/imgui-sfml-src/imgui-SFML.h"

#include <thread>
#include <iostream>
#include <math.h>

#include "AIEntity.hpp"
#include "Math.hpp"


class AISimulation {
    public: 
        void init() {
            ImGui::SFML::Init(window);
            window.setFramerateLimit(75);

            int _map = 0;
            walls_texture.loadFromFile("img/wall.png");

            sf::Image map;
            map.loadFromFile(("img/"+std::to_string(_map)+".png"));

            loadtexture();

            // Load map
            for (int y = 0;y < map.getSize().y;y++) {
                for (int x = 0;x < map.getSize().x;x++) {
                    if (map.getPixel(sf::Vector2u(x, y)).r >= 250) {
                        sf::Sprite sprite = sf::Sprite(walls_texture);
                        sprite.setTexture(walls_texture);
                        sprite.setScale(sf::Vector2f(4.f, 4.f));
                        sprite.setPosition(sf::Vector2f(x * 16, y * 16));
                        walls.push_back(sprite);
                    }

                    if (map.getPixel(sf::Vector2u(x, y)).b >= 250) {
                        sf::RectangleShape shape;
                        shape.setFillColor(sf::Color::Blue);
                        shape.setPosition(sf::Vector2f(x * 16, y * 16));
                        shape.setSize(sf::Vector2f(16.f, 16.f));
                        wayPoints.push_back(shape);
                    }
                }
            }

            for (int i = 0;i < generation_entity_count;i++) {
                entities.push_back(std::make_unique<AIEntity>());
                entities[i]->createRand();
            }
        }

        void handleInput() {
            while (const std::optional ev = window.pollEvent()) {
                if (ev->is<sf::Event::Closed>()) {
                    window.close();
                }

                // Resizing window
                if (const auto* resized = ev->getIf<sf::Event::Resized>())
                {
                    view.setSize(sf::Vector2f(resized->size));
                }

                // ImGui
                ImGui::SFML::ProcessEvent(window, *ev);

                timeScale = 1.0f;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
                    timeScale = 10.f;
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
                    timeScale = 0.3f;
                }

                if (const auto* mouseWheelScrolled = ev->getIf<sf::Event::MouseWheelScrolled>())
                {
                    if (mouseWheelScrolled->delta >= 1.f) {
                        view.zoom(1.05f);
                    } else if (mouseWheelScrolled->delta <= -1.f) {
                        view.zoom(.95f);
                    }
                }
            }
        }

        void updateGui() {
            ImGui::SFML::Update(window, deltaClock.restart());

            ImGuiWindowFlags window_flags = 0;
            window_flags |= ImGuiWindowFlags_NoBackground;
            window_flags |= ImGuiWindowFlags_NoTitleBar;
            window_flags |= ImGuiWindowFlags_NoInputs;


            // etc.
            bool open_ptr = true;
            ImGui::Begin("global", &open_ptr, window_flags);
            ImGui::SetWindowSize(ImVec2(1000, 1000));

            

            ImGui::SetWindowFontScale(2.f);
            ImGui::Text("Generation: %i", generation);
            if(usedWayPoints.size())
                ImGui::Text("Absolute best time: %i frames", absBestTime);

            ImGui::End();

            ImGui::Begin("Simulation");
            ImGui::Text("Current frame %i / %i", current_frame, frames_per_generation);
            ImGui::Text("Entities currently %i / %i", entities.size(), generation_entity_count);

            ImGui::InputInt("Frames per generation: ", (int*)&frames_per_generation);
            ImGui::InputInt("Entities per generation: ", (int*)&generation_entity_count);
            ImGui::End();

            if(currentlySelectedEntityIndex != -1 && !(currentlySelectedEntityIndex < 0 || currentlySelectedEntityIndex >= entities.size())) {
                const auto& e = entities.at(currentlySelectedEntityIndex);

                ImGui::Begin("Selected entity");

                ImGui::SetWindowSize(ImVec2(600, 500));
                ImGui::SetWindowPos(ImVec2(mousePixelPos + sf::Vector2i(32, 32)));
                ImGui::Text("Entity #%i", currentlySelectedEntityIndex);

                ImGui::SeparatorText("Neural network");
                ImGui::Text("Total connections: %i", e->network.getConnectionsCount());

                for(int output_neuron_i = 0; output_neuron_i < e->network.neurons_output; ++output_neuron_i) {
                    const float progbar_neuron_x = e->network.getOutputFrom(output_neuron_i);

                    float progress_saturated = (progbar_neuron_x + 2.5f) / 5.0f;

                    progress_saturated = std::clamp(progress_saturated, 0.0f, 1.0f);

                    char buf[32];
                    snprintf(buf, sizeof(buf), "%.3f", progbar_neuron_x);

                    ImGui::ProgressBar(progress_saturated, ImVec2(0.f, 0.f), buf);

                    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                    ImGui::Text("neuron [output][%i]", output_neuron_i);
                }

                ImGui::Text("Composite velocity: %f, %f", e->network.getOutputAsVelocity().x, e->network.getOutputAsVelocity().y);

                ImGui::End();
            }

            ImGui::SFML::Render(window);
        }

        void start() {
            init();

            // Game loop
            while (window.isOpen()) {
                window.clear();
                window.setView(view);

                purgeEntities();
                handleInput();
                updateEntities();
                checkGen();

                currentlySelectedEntityIndex = -1;

                mousePixelPos = sf::Mouse::getPosition(window);
                // convert it to world coordinates
                sf::Vector2f worldPos = window.mapPixelToCoords(mousePixelPos);

                for (int entityIndex = 0; entityIndex < entities.size(); entityIndex++) {
                    auto& e = entities.at(entityIndex);

                    if(e == nullptr) continue;

                    if(currentlySelectedEntityIndex == -1 && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                        if(e->sprite.getGlobalBounds().findIntersection(sf::FloatRect(sf::Vector2f(worldPos.x - 5.f, worldPos.y - 5.f), sf::Vector2f(20.f, 20.f)))) {
                            sf::RectangleShape indicator;
                            indicator.setOrigin(sf::Vector2f(16.f, 16.f));
                            indicator.setSize(e->sprite.getGlobalBounds().size);
                            indicator.setPosition(e->sprite.getPosition());
                            indicator.setOutlineThickness(3);
                            indicator.setOutlineColor(sf::Color::Green);

                            window.draw(indicator);

                            currentlySelectedEntityIndex = entityIndex;
                        }
                    }

                    // In case entity is destroyed while inspected via GUI
                    if(e->hp == 0 && entityIndex == currentlySelectedEntityIndex) {
                        currentlySelectedEntityIndex = -1;
                    }

                    window.draw(e->sprite);
                }

                for (auto& w : wayPoints) {
                    window.draw(w);
                }

                for (auto& uw : usedWayPoints) {
                    window.draw(uw);
                }

                updateGui();

                window.display();
            }
        }

        void purgeEntities() {
            entities.erase(
                std::remove_if(entities.begin(), entities.end(),
                    [](const std::unique_ptr<AIEntity>& o) { 
                        return o->hp == 0 || o == nullptr;
                    }),
                entities.end());
        }
 
        void entityReachGoal(auto &e, auto &w) {
            if (e->frameTimer != 0) {
                e->reachedGoal = true;

                // now, lets protect them from death before gen end
                // by giving an magic number amount of hp!
                e->hp = INVINCIBLE_HEALTH_VALUE;

                SIM_LOG("1+");

                // Add marker for used waypoint
                sf::RectangleShape marker;
                marker.setSize(sf::Vector2f(8, 8));
                marker.setOrigin(sf::Vector2f(4, 4));
                marker.setPosition(e->sprite.getPosition());
                marker.setRotation(sf::degrees(45.f));

                marker.setFillColor(sf::Color::Green);
                marker.setOutlineColor(sf::Color::White);
                marker.setOutlineThickness(2);

                usedWayPoints.push_back(marker);

                // Move it out of the view
                e->sprite.setPosition(sf::Vector2f(-20000.f, -20000.f));
            }
        }
        
        void newGeneration(auto bestEntity) {
            if(bestEntity == nullptr) {
                std::cerr << "Nullptr best entity! \n";
                bestEntity = entities.emplace_back(std::make_unique<AIEntity>()).get();

                entities.clear();
                for (int i = 0;i < generation_entity_count;i++) {
                    entities.push_back(std::make_unique<AIEntity>());
                    entities[i]->createRand();
                }

                SIM_LOG("\nNew generation, NO entity/entities reached goal. there are %i entities\n", entities.size());
            } else {
                // Remove everything but the best entity
                entities.erase(
                    std::remove_if(entities.begin(), entities.end(),
                        [&](const std::unique_ptr<AIEntity>& o) { return o.get() != bestEntity; }),
                    entities.end());

                for (int i = 0;i < generation_entity_count;i++) {
                    std::unique_ptr<AIEntity> ai = std::make_unique<AIEntity>();
                    ai->createBased(bestEntity->network);
                    ai->reachedGoal = false;
                    entities.push_back(std::move(ai));
                }
                
                float bestEntityReachedTime = bestEntity->frameTimer;

                if (bestEntityReachedTime < absBestTime) {
                    absBestTime = bestEntityReachedTime;
                }

                SIM_LOG("\nNew generation, entity/entities reached goal. there are %i entities\n", entities.size());
            }

            generation++;

            // Reset fields of all entities
            for(auto& e: entities) {
                if(e) {
                    e->resetEntity();
                }
            }
        }

        void checkGen() {
            for (auto& e : entities) {
                if(!e) return;

                e->update(timeScale);
                for (auto& w : wayPoints) {
                    if (e->reachedGoal == false && e->sprite.getGlobalBounds().findIntersection(w.getGlobalBounds())) {
                        entityReachGoal(e, w);
                    }
                }
            }

            current_frame++;

            if (current_frame >= frames_per_generation) {
                current_frame = 0;
                AIEntity* bestEntity = nullptr;

                float best = FLT_MAX;

                for (auto& e : entities) {
                    const float val = e->frameTimer;

                    if (best > val && e->reachedGoal == true) {
                        best = val;
                        bestEntity = e.get();  // get raw pointer from unique_ptr
                    }
                }

                newGeneration(bestEntity);
            }
        }

        void checkEntityWallCollision(auto &e, auto &wall) {
            const float dist = distance(wall.getPosition().x, wall.getPosition().y, e->sprite.getPosition().x, e->sprite.getPosition().y);

            if(dist < 50.f) {
                float dx = wall.getPosition().x - e->sprite.getPosition().x;
                float dy = wall.getPosition().y - e->sprite.getPosition().y;

                e->network.setInput(0, dx / 5.f);
                e->network.setInput(1, dy / 5.f);
            }	

            if(dist < 10.f && e->hp != INVINCIBLE_HEALTH_VALUE) {
                e->hp = 0;
            }
        }

        void updateEntities() {
            for(auto& e : entities) {
                e->network.setInput(2, e->sprite.getPosition().x / 100.f);
                e->network.setInput(3, e->sprite.getPosition().y / 100.f);

                e->network.setInput(4, (float)current_frame / (float)frames_per_generation);
            }

            for (auto& wall : walls) {
                window.draw(wall);

                for (auto& e : entities) {
                    if(e == nullptr) continue;

                    checkEntityWallCollision(e, wall);
                }
            }
        }

        sf::Clock deltaClock;                                                                                                 


    private:
        sf::Texture walls_texture;

        std::vector<sf::Sprite> walls;
        std::vector<sf::RectangleShape> wayPoints;
        std::vector<sf::RectangleShape> usedWayPoints;

        sf::Vector2i mousePixelPos {0, 0};

        std::vector<std::unique_ptr<AIEntity>> entities;

        unsigned long long generation = 0;
        uint64_t absBestTime = UINT64_MAX;
        unsigned int generation_entity_count = 400;

        bool renderEntities = true;

        double timeScale = 1.f;

        const int INVINCIBLE_HEALTH_VALUE = -1;

        sf::RenderWindow window = sf::RenderWindow(sf::VideoMode({800, 800}), "Neural networks");
        sf::View view;

        int frames_per_generation = 800;
        int current_frame = 0;

        // -1 = No entity selected
        int currentlySelectedEntityIndex = -1;
};