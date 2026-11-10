#pragma once

#include <thread>
#include <iostream>
#include <math.h>

#include <SFML/Graphics.hpp>
#include <SFML/Main.hpp>

#include "../build/_deps/imgui-src/imgui.h"
#include "../build/_deps/imgui-sfml-src/imgui-SFML.h"

#include "global.hpp"
#include "AIEntity.hpp"
#include "NN_standalone/Math.hpp"

class AISimulation {
    public: 
        void init();

        void handleInput();

        void updateGui();

        void start();

        void purgeEntities() {
            entities.erase(
                std::remove_if(entities.begin(), entities.end(),
                    [](const std::unique_ptr<AIEntity>& o) { 
                        return o->get_hp() == 0 || o == nullptr;
                    }),
                entities.end());
        }

        void purgeInactiveEntities() {
            if(current_frame < 100) return;

            entities.erase(
                std::remove_if(entities.begin(), entities.end(),
                    [](const std::unique_ptr<AIEntity>& o) { 
                        return distance(o->getLastPosition().x, o->getLastPosition().y, spawn_point.x, spawn_point.y) < 50;
                    }),
                entities.end());
        }
 
        void entityReachGoal(auto &e, auto &w) {
            if (e->getFramesAlive() != 0) {
                e->flagExcellent();

                SIM_LOG("1+");

                // Add marker for used waypoint
                sf::RectangleShape marker;
                marker.setSize(sf::Vector2f(8, 8));
                marker.setOrigin(sf::Vector2f(4, 4));
                marker.setPosition(e->getLastPosition());
                marker.setRotation(sf::degrees(45.f));

                marker.setFillColor(sf::Color::Green);
                marker.setOutlineColor(sf::Color::White);
                marker.setOutlineThickness(2);

                usedWayPoints.push_back(marker);

                // TODO: Move it out of the view
                // e->sprite.setPosition(sf::Vector2f(-20000.f, -20000.f));
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
                SIM_LOG("\nNew generation, lets make a new one");

                entities.erase(
                    std::remove_if(entities.begin(), entities.end(),
                        [&](const std::unique_ptr<AIEntity>& o) { return o.get() != bestEntity; }),
                    entities.end());

                for (int i = 0;i < generation_entity_count;i++) {
                    std::unique_ptr<AIEntity> ai = std::make_unique<AIEntity>();
                    ai->createBased(*bestEntity->getNeuralNetwork());
                    ai->regenerate();
                    entities.push_back(std::move(ai));
                }
                
                float bestEntityReachedTime = bestEntity->getFramesAlive();

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
                    if (e->is_excellent() == false && e->getHitbox().findIntersection(w.getGlobalBounds())) {
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
                    const float val = e->getFramesAlive();

                    if (best > val && e->is_excellent() == true) {
                        best = val;
                        bestEntity = e.get();  // get raw pointer from unique_ptr
                    }
                }

                newGeneration(bestEntity);
            }
        }

        void checkEntityWallCollision(auto &e, auto &wall) {
            const float dist = distance(wall.getPosition().x, wall.getPosition().y, e->getLastPosition().x, e->getLastPosition().y);

            if(dist < 50.f) {
                float dx = wall.getPosition().x - e->getLastPosition().x;
                float dy = wall.getPosition().y - e->getLastPosition().y;

                // TODO
                e->getNeuralNetwork()->setInput(0, dx / 5.f);
                e->getNeuralNetwork()->setInput(1, dy / 5.f);
            }	

            if(dist < 10.f && e->is_excellent() == false) {
                e->harm();
            }
        }

        void updateEntities() {
            for(auto& e : entities) {
                e->getNeuralNetwork()->setInput(2, e->getLastPosition().x / 100.f);
                e->getNeuralNetwork()->setInput(3, e->getLastPosition().y / 100.f);

                e->getNeuralNetwork()->setInput(4, (float)current_frame / (float)frames_per_generation);
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