#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>

#include "NeuralNet.hpp"

sf::Texture texture;
sf::Texture texture_damaged;

sf::Vector2f spawn_point = {300,300};

void loadtexture() {
	if(!texture.loadFromFile("img/Entity.png") ||
		!texture_damaged.loadFromFile("img/Entity_damaged.png")) {

		std::cerr << "AIEntity.hpp loadtexture() was unable to load textures!";
	}
}

struct AIEntity {
	AIEntity() {
		sprite.setTexture(texture);

		sprite.setOrigin(sf::Vector2f(16,16));

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

	void createBased(const NeuralNet &ne) {
		// 50% either get randomized by some amount, and 49.92 dont get any effect
		isDirectCopy = true;

		if(rand() % 2) {
			network.initBased(ne, rand() % 30);
			isDirectCopy = false;
		}

		// only 0.02% get fully randomized 
		if((rand() % 50) == 0) {
			network.initRand();
			isDirectCopy = false;
		}

	}

	void update(const double timeScale) {
		frameTimer += !reachedGoal;

		sprite.setTexture(texture);

		try
		{
			network.update();
		}
		catch (const std::exception&e)
		{
			printf("%c\n",e.what());
		}

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
				sprite.setTextureRect(sf::IntRect(sf::Vector2i(0, isDirectCopy * 32), sf::Vector2i(32, 32)));
			if(diffX < 0) 
				sprite.setTextureRect(sf::IntRect(sf::Vector2i(32, isDirectCopy * 32), sf::Vector2i(32, 32)));
		} else {
			if(diffY < 0) 
				sprite.setTextureRect(sf::IntRect(sf::Vector2i(64, isDirectCopy * 32), sf::Vector2i(32, 32)));
			if(diffY > 0) 
				sprite.setTextureRect(sf::IntRect(sf::Vector2i(96, isDirectCopy * 32), sf::Vector2i(32, 32)));
		}

		prev_pos = sprite.getPosition();
	}

	int64_t frameTimer = 0;

	int64_t hp = 10;
	sf::RectangleShape viewArea;
	sf::Sprite sprite = sf::Sprite(texture);

	NeuralNet network;

	sf::Vector2f prev_pos = {0.f, 0.f};

	bool reachedGoal = false;
	bool isDirectCopy = false;
};