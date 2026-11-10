#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>

#include "NN_standalone/NeuralNet.hpp"

inline sf::Texture texture;
inline sf::Texture texture_damaged;

inline sf::Vector2f spawn_point = {300,300};

inline void loadtexture() {
	if(!texture.loadFromFile("img/Entity.png") ||
		!texture_damaged.loadFromFile("img/Entity_damaged.png")) {

		std::cerr << "AIEntity.hpp loadtexture() was unable to load textures!";
	}
}

class AIEntity {
public:
	AIEntity() {
		network = NeuralNet(5, 12, 10, 5);

		sprite.setTexture(texture);

		sprite.setOrigin(sf::Vector2f(16,16));
		sprite.setScale(sf::Vector2f(0.2, 0.2));

		resetEntity();
	}

	void resetEntity() {
		hp = 10;
		reachedGoal = false;
		sprite.setPosition(spawn_point);
		
		frameTimer = 0;
	}

	void createRand() {
		network.initRand();
	}

	void createBased(const NeuralNet ne) {
		// only 0.02% get fully randomized 
		if(randomInt(50) == 0) {
			network.initRand();
		} else {
			network.inherit_neural_network(ne, randomInt(5));
		}
	}

	void update(const double timeScale) {
		prev_pos = sprite.getPosition();

		frameTimer += !reachedGoal;

		sprite.setTexture(texture);

		network.update();

		const float maxSpeed = 5.f;

		float moveX = network.getOutputAsVelocity().x / 3.f;
		float moveY = network.getOutputAsVelocity().y / 3.f;

		moveX = std::clamp(moveX, -maxSpeed, maxSpeed);
		moveY = std::clamp(moveY, -maxSpeed, maxSpeed);

		sprite.move(sf::Vector2f(moveX / timeScale, moveY / timeScale));

		sprite.setRotation(viewArea.getRotation());

		const float diffX = sprite.getPosition().x - prev_pos.x;
		const float diffY = sprite.getPosition().y - prev_pos.y;

		if(diffX > diffY) {
			if(diffX > 0) 
				sprite.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(32, 32)));
			if(diffX < 0) 
				sprite.setTextureRect(sf::IntRect(sf::Vector2i(32, 0), sf::Vector2i(32, 32)));
		} else {
			if(diffY < 0) 
				sprite.setTextureRect(sf::IntRect(sf::Vector2i(64, 0), sf::Vector2i(32, 32)));
			if(diffY > 0) 
				sprite.setTextureRect(sf::IntRect(sf::Vector2i(96, 0), sf::Vector2i(32, 32)));
		}
	}

	
	std::map<std::string, int> getSize() {
		return {{"con", network.getConnectionsCount()}, {"input", neurons_input}, {"hidden", neurons_hidden}, {"hidden2", neurons_hidden2}, {"output", neurons_output}};
	}

	NeuralNet* getNeuralNetwork() {
		return &network;
	}

	int get_hp() {
		return hp;
	}

	const sf::Vector2f& getLastPosition() {
		return prev_pos;
	} 

	bool is_eliminateable() {
		return false;
	}

	void flagExcellent() {
		reachedGoal = true;
	}

	bool is_excellent() {
		return reachedGoal;
	}

	int getFramesAlive() {
		return frameTimer;
	}

	auto getHitbox() {
		return sprite.getGlobalBounds();
	}

	void regenerate() {
		reachedGoal = false;
		hp = 10;
	}

	void harm() {
		if (getFramesAlive() < 10) return;

		hp -= 5;
		sprite.setPosition(getLastPosition());
	}

	bool is_alive() {
		return hp > 0;
	}

	void draw(sf::RenderTarget &window) {
		window.draw(sprite);
	}

private:
	int neurons_input = 0;
	int neurons_hidden = 0;
	int neurons_hidden2 = 0;
	int neurons_output = 0;

	int64_t frameTimer = 0;

	int64_t hp = 10;
	sf::RectangleShape viewArea;
	sf::Sprite sprite = sf::Sprite(texture);

	NeuralNet network;

	sf::Vector2f prev_pos = {0.f, 0.f};

	bool reachedGoal = false;
};