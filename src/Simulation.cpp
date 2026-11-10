#include "Simulation.hpp"

void AISimulation::init()
{
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

void AISimulation::handleInput()
{
    while (auto ev = window.pollEvent()) {

        if (ev->is<sf::Event::Closed>()) {
            window.close();
        }

        // Resizing window
        if (const auto* resized = ev->getIf<sf::Event::Resized>())
            view.setSize(sf::Vector2f(resized->size));

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

void AISimulation::updateGui()
{
    ImGui::SFML::Update(window, deltaClock.restart());

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoBackground;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoInputs;

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

    // Display info about an entity when hovering over it
    if(currentlySelectedEntityIndex != -1 && !(currentlySelectedEntityIndex < 0 || currentlySelectedEntityIndex >= entities.size())) {
        const auto& e = entities.at(currentlySelectedEntityIndex);
        auto esize_params = e->getSize();

        ImGui::Begin("Selected entity");

        ImGui::SetWindowSize(ImVec2(600, 500));
        ImGui::SetWindowPos(ImVec2(mousePixelPos + sf::Vector2i(32, 32)));
        ImGui::Text("Entity #%i", currentlySelectedEntityIndex);

        ImGui::SeparatorText("Neural network");
        ImGui::Text("Total connections: %i", esize_params.at("con"));

        for(int output_neuron_i = 0; output_neuron_i < esize_params.at("output"); output_neuron_i++) {

            float progbar_neuron_x = e->getNeuralNetwork()->getOutputFrom(output_neuron_i);

            float progress_saturated = (progbar_neuron_x + 2.5f) / 5.0f;

            progress_saturated = std::clamp(progress_saturated, 0.0f, 1.0f);

            char buf[32];
            snprintf(buf, sizeof(buf), "%.3f", progbar_neuron_x);

            ImGui::ProgressBar(progress_saturated, ImVec2(0.f, 0.f), buf);

            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::Text("neuron [output][%i]", output_neuron_i);
        }

        ImGui::Text("Composite velocity: %f, %f", e->getNeuralNetwork()->getOutputAsVelocity().x, e->getNeuralNetwork()->getOutputAsVelocity().y);

        ImGui::End();
    }

    ImGui::SFML::Render(window);
}

void AISimulation::start()
{
    while (window.isOpen()) {
        window.clear();
        window.setView(view);

        purgeEntities();
        purgeInactiveEntities();

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

            // Check if hovering mouse over an entity
            if(currentlySelectedEntityIndex == -1 && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                if(e->getHitbox().findIntersection(sf::FloatRect(sf::Vector2f(worldPos.x - 5.f, worldPos.y - 5.f), sf::Vector2f(20.f, 20.f)))) {
                    sf::RectangleShape indicator;
                    indicator.setOrigin(sf::Vector2f(16.f, 16.f));
                    indicator.setSize(sf::Vector2f(32, 32));
                    indicator.setPosition(e->getLastPosition());
                    indicator.setOutlineThickness(3);
                    indicator.setOutlineColor(sf::Color::Green);

                    window.draw(indicator);

                    currentlySelectedEntityIndex = entityIndex;
                }
            }

            // In case entity is destroyed while inspected via GUI
            if(e->is_alive() == false && entityIndex == currentlySelectedEntityIndex) {
                currentlySelectedEntityIndex = -1;
            }

            e->draw(window);
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
