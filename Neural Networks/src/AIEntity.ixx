#pragma once

#include <SFML/Graphics.hpp>

#include "NeuralNet.ixx"

sf::Texture texture;
sf::Texture texture_damaged;

sf::Vector2f spawn_point = {300,300};

void loadtexture() {
	texture.loadFromFile("img/Entity.png");
	texture_damaged.loadFromFile("img/Entity_damaged.png");
}

struct AIEntity {
	AIEntity() {
		sprite.setTexture(texture);

		sprite.setOrigin(sf::Vector2f(16,16));

		sprite.setPosition(spawn_point);

		hp = 10;

		reachedGoal = false;
	}

	void createRand() {
		network.initRand();
	}

	void createBased(const NeuralNet &ne) {
		// 50% either get randomized by some amount, and 49.92 dont get any effect
		if(rand() % 2) 
			network.initBased(ne, rand() % 30);

		// only 0.02% get fully randomized 
		if((rand() % 50) == 0) {
			network.initRand();
		}

	}

	void update(const double timeScale) {
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


		sprite.move(moveX / timeScale, moveY / timeScale);

		sprite.setRotation(viewArea.getRotation());

		const float diffX = sprite.getPosition().x - prev_pos.x;
		const float diffY = sprite.getPosition().y - prev_pos.y;

		if(diffX > diffY) {
			if(diffX > 0) 
				sprite.setTextureRect(sf::IntRect(0, 0, 32, 32));
			if(diffX < 0) 
				sprite.setTextureRect(sf::IntRect(32, 0, 32, 32));
		} else {
			if(diffY < 0) 
				sprite.setTextureRect(sf::IntRect(64, 0, 32, 32));
			if(diffY > 0) 
				sprite.setTextureRect(sf::IntRect(96, 0, 32, 32));
		}

		prev_pos = sprite.getPosition();
	}

	sf::Time reachedTime = sf::Time::Zero;
	int64_t hp = 10;
	sf::RectangleShape viewArea;
	sf::Sprite sprite;
	sf::Clock timer;
	NeuralNet network;

	sf::Vector2f prev_pos = {0.f, 0.f};

	bool reachedGoal = false;
};