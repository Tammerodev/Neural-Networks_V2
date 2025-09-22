/*
	Example of artificial intelliegence
*/

#include <SFML/Graphics.hpp>
#include <thread>
#include <iostream>
#include <math.h>

#include "AIEntity.ixx"

sf::Texture walls_texture;

std::vector<sf::Sprite> walls;
std::vector<sf::RectangleShape> wayPoints;
std::vector<sf::RectangleShape> usedWayPoints;


std::vector<std::unique_ptr<AIEntity>> entities;

sf::Font regFont;
sf::Text text;


unsigned long long generation = 0;
unsigned long long absBestTime = 999999;
unsigned int generation_entity_count = 100;

bool renderEntities = true;

double timeScale = 1.f;

const int INVINCIBLE_HEALTH_VALUE = -1;


float distance(int x1, int y1, int x2, int y2)
{
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) * 1.0);
}

sf::RenderWindow window(sf::VideoMode(800, 800), "Neural networks");
sf::View view;

int frames_per_generation = 800;
int current_frame = 0;

void checkGen() {
	for (auto& e : entities) {
		if(!e) return;

		e->update(timeScale);
		for (auto& w : wayPoints) {
			if (e->reachedGoal == false && e->sprite.getGlobalBounds().intersects(w.getGlobalBounds())) {
				if (e->reachedTime == sf::Time::Zero) {
					e->reachedTime = e->timer.getElapsedTime();
					e->reachedGoal = true;

					// now, lets protect them from death before gen end
					// by giving an magic number amount of hp!
					e->hp = INVINCIBLE_HEALTH_VALUE;

					std::cout << "1+";
					std::cout.flush();

					// Add marker for used waypoint
					sf::RectangleShape marker;
					marker.setSize(sf::Vector2f(8, 8));
					marker.setOrigin(sf::Vector2f(4, 4));
					marker.setPosition(e->sprite.getPosition());
					marker.setRotation(45.f);

					marker.setFillColor(sf::Color::Green);
					marker.setOutlineColor(sf::Color::White);
					marker.setOutlineThickness(2);

					usedWayPoints.push_back(marker);

					// Move it out of the view
					e->sprite.setPosition(-20000.f, -20000.f);
				}
			}
		}
	}

	current_frame++;

	text.setString("Generaatio : " + std::to_string(generation)
	+"\nTeko�lyjen m��r� : " + std::to_string(entities.size())+
	"\nParas Aika : " + std::to_string(absBestTime) + "ms");
		if (current_frame >= frames_per_generation * timeScale) {
			current_frame = 0;
			AIEntity* bestEntity = nullptr;
			float best = 9999.f;

			for (auto& e : entities) {
				const float val = e->reachedTime.asSeconds() * e->hp;

				if (best > val && e->reachedGoal == true) {
					best = val;
					bestEntity = e.get();  // get raw pointer from unique_ptr

					std::cout << "bestt chose\n" << bestEntity->reachedGoal;
				}
			}

			if(bestEntity == nullptr) {
				std::cerr << "Nullptr best entity! \n";
				goto null_catch_init_entities;
			}
			else bestEntity->reachedGoal = false;


			if(bestEntity == nullptr) {
null_catch_init_entities:
				bestEntity = entities.emplace_back(std::make_unique<AIEntity>()).get();

				bestEntity->reachedGoal = false;

				entities.clear();
				for (int i = 0;i < generation_entity_count;i++) {
					entities.push_back(std::make_unique<AIEntity>());
					entities[i]->createRand();
				}

				std::cout << "\n\nNew generation, NO entity/entities reached goal. there are "<< entities.size() << " entities\n";

			} else {
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

				if (bestEntity->reachedTime != sf::Time::Zero) {
					if (bestEntity->reachedTime.asMilliseconds() < absBestTime) {
						absBestTime = bestEntity->reachedTime.asMilliseconds();
					}
				}

				bestEntity->reachedGoal = false;

				std::cout << "\n\nNew generation, entity/entities reached goal. there are "<< entities.size() << " entities\n";
							bestEntity->sprite.setPosition(spawn_point);
			}
			generation++;

			for(auto& e: entities) {
				if(e) e->reachedGoal = false;
			}
		}


}
                                                                                                    

int main(){
	regFont.loadFromFile("Font.ttf");
	text.setFont(regFont);
	int _map = 0;
	walls_texture.loadFromFile("img/wall.png");

	sf::Image map;


	{
		using std::cout;
		int seed = 0;

		system("CLS");


		cout << "***************************************************\n";
		cout << "				HL Research team\n";
		cout << "Code by Lauri Etelamaki\n";
		cout << "W,A,S,D to move neural networks\n";
		cout << "SPACE to activate all input neurons\n";
		cout << "***************************************************\n";



		std::wcout << "Sy�t� tason numero : ";
		std::cin >> _map;
		if (_map > 3) {
			std::cout << "Maailma ei olemassa ; palautetaan arvo 0";
			_map = 0;
		}

		if(_map == 2) {
			spawn_point = {45*16, 40*16};
			frames_per_generation = 25000;
		}

		std::wcout << "Kuinka monta teko�ly� joka generaatiolla luodaan? : ";
		std::cin >> generation_entity_count;
		std::wcout << "Sy�t� satunnaisgeneraattorin siemen : ";
		std::cin >> seed;
		srand(seed);
	}

	map.loadFromFile(("img/"+std::to_string(_map)+".png"));
	// Load map
	for (int y = 0;y < map.getSize().y;y++) {
		for (int x = 0;x < map.getSize().x;x++) {
			if (map.getPixel(x, y).r >= 250) {
				sf::Sprite sprite;
				sprite.setTexture(walls_texture);
				sprite.setScale(4.f, 4.f);
				sprite.setPosition(x * 16, y * 16);
				walls.push_back(sprite);
			}

			if (map.getPixel(x, y).b >= 250) {
				sf::RectangleShape shape;
				shape.setFillColor(sf::Color::Blue);
				shape.setPosition(x * 16, y * 16);
				shape.setSize(sf::Vector2f(16.f, 16.f));
				wayPoints.push_back(shape);
			}
		}
	}
	sf::Event ev {};

	loadtexture();

	for (int i = 0;i < generation_entity_count;i++) {
		entities.push_back(std::make_unique<AIEntity>());
		entities[i]->createRand();
	}

	window.setFramerateLimit(240);
	// Game loop
	while (window.isOpen()) {
		checkGen();

		window.clear();
		window.setView(view);
		while (window.pollEvent(ev)) {
			if (ev.type == sf::Event::Closed) {
				window.close();
			}

			// first neuron is distance to goal!
			
			for(auto& e : entities) {
				e->network.setInput(2, e->sprite.getPosition().x / 100.f);
				e->network.setInput(3, e->sprite.getPosition().y / 100.f);

				e->network.setInput(4, (float)current_frame / (float)frames_per_generation);
			}

			timeScale = 1.0f;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
				timeScale = 10.f;
			} else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
				timeScale = 0.3f;
			}

			if (ev.type == sf::Event::MouseWheelMoved)
			{
				if (ev.mouseWheel.delta >= 1.f) {
					view.zoom(1.05f);
				} else if (ev.mouseWheel.delta <= -1.f) {
					view.zoom(.95f);
				}
			}
		}

		if (renderEntities) {
			for (auto& e : entities) {
				if(e == nullptr) continue;

				window.draw(e->sprite);
			}
		}

		for (auto& wall : walls) {
			window.draw(wall);

			for (auto& e : entities) {
				if(e == nullptr) continue;

				// Set neural networks inout neuron value to distance between player and the wall
				const float dist = distance(wall.getPosition().x, wall.getPosition().y, e->sprite.getPosition().x, e->sprite.getPosition().y);

				if(dist < 50.f) {
					float dx = wall.getPosition().x - e->sprite.getPosition().x;
					float dy = wall.getPosition().y - e->sprite.getPosition().y;

					e->network.setInput(0, dx / 20.f);
					e->network.setInput(1, dy / 20.f);
				}	

				if(dist < 10.f && e->hp != INVINCIBLE_HEALTH_VALUE) {
					e->hp = 0;
				}

			}
		}

		for (auto& w : wayPoints) {
			window.draw(w);
		}

		for (auto& uw : usedWayPoints) {
			window.draw(uw);
		}


		entities.erase(
			std::remove_if(entities.begin(), entities.end(),
				[](const std::unique_ptr<AIEntity>& o) { return o->hp == 0 || o == nullptr; }),
			entities.end());

		window.draw(text);
		window.display();
	}
	return 0;
}