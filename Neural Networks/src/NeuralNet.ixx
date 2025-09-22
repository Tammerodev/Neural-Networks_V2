#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <iostream>

#include "Neuron.ixx"
struct NeuralNet
{
	NeuralNet() {
		init();
	}


	void randomConnection() {
		cons.push_back(Connection(randFloat() - 0.5f,
				NeuronIndex(Input, rand() % neurons_input),
				NeuronIndex(Hidden, rand() % neurons_hidden)));

		cons.push_back(Connection(randFloat() - 0.5f,
			NeuronIndex(Hidden, rand() % neurons_hidden),
			NeuronIndex(Hidden2, rand() % neurons_hidden2)));

		cons.push_back(Connection(randFloat() - 0.5f,
			NeuronIndex(Hidden2, rand() % neurons_hidden2),
			NeuronIndex(Output, rand() % neurons_output)));
	}

	void initRand() {
		destory();
		init();

		for (int i = 0;i < randomInt(5000); i++) {
			randomConnection();
		}
	}

	Neuron* neuronAtNIndex(const NeuronIndex &i) {
		if(neurons.at(i.layer).size() <= i.index) return &neurons.at(Input).at(0);

		return &neurons.at(i.layer)[i.index];
	}

	const int neurons_input = 5;
	const int neurons_hidden = 12;
	const int neurons_hidden2 = 12;
	const int neurons_output = 5;

	const int margin = 10;

	void init() {
		// Input
		neurons.insert({ Input, std::vector<Neuron>(neurons_input + margin)});
		// Hidden layer
		neurons.insert({ Hidden, std::vector<Neuron>(neurons_hidden + margin)});
		neurons.insert({ Hidden2, std::vector<Neuron>(neurons_hidden2 + margin)});
		// Output layer
		neurons.insert({ Output, std::vector<Neuron>(neurons_output + margin)});
	}

	float randFloat() {
		return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	}

	int randomInt(int x) {
		return std::rand() % (x);
	}

	void initBased(const NeuralNet cpy, int randomness) {  // pass by const ref to avoid copies
		destory();
		init();

		if(cpy.neurons.size() <= 0 || cpy.cons.size() <= 0) {
			initRand(); 
			return;
		}

		neurons = cpy.neurons;
		cons = cpy.cons;

		for (int i = 0; i < randomness; i++) {
			randomConnection();
		}

		if (!cons.empty()) {
			for (int i = 0; i < randomness; i++) {
				cons[rand() % cons.size()].weight += randFloat();
			}
		}
	}


	void destory() {
		cons.clear();
		neurons.clear();
	}

	void update() {
    	try {
			// Make sure Output and Hidden layers exist
			if (neurons.size() > 0) {
				for (auto& n : neurons.at(Output)) {
					n.value = 0.f;
				}
				for (auto& n : neurons.at(Hidden)) {
					n.value = 0.f;
				}
				for (auto& n : neurons.at(Hidden2)) {
					n.value = 0.f;
				}
			}


			// Input => Hidden
			for (auto& con : cons) {
				if(con.connectedFromIndex.layer == Input)
					neuronAtNIndex(con.connectedToIndex)->value += neuronAtNIndex(con.connectedFromIndex)->value * con.weight;
			}

			// Hidden => Hidden2
			for (auto& con : cons) {
				if(con.connectedFromIndex.layer == Hidden)
					neuronAtNIndex(con.connectedToIndex)->value += neuronAtNIndex(con.connectedFromIndex)->value * con.weight;
			}

			// Hidden2 => Output
			for (auto& con : cons) {
				if(con.connectedFromIndex.layer == Hidden2)
					neuronAtNIndex(con.connectedToIndex)->value += neuronAtNIndex(con.connectedFromIndex)->value * con.weight;
			}
		}
		catch (std::exception& e) {
			std::cout << e.what() << '\n';
		}
	}


	void setInput(int index, float value) {
		if(index >= neurons_input) return;

		neurons.at(Input)[index].value = value;
	}

	sf::Vector2f getOutputAsVelocity() {
		return sf::Vector2f(getOutputFrom(2) - getOutputFrom(0), getOutputFrom(3) - getOutputFrom(1));
	}

	float getOutputFrom(int index) {
		if(index >= neurons_output) return 0.f;

		try {
			return neurons.at(Output).at(index).value;
		} catch(const std::exception& e) {
			init();
			initRand();
			std::cerr << "error getOutputFrom: " << e.what() << "\n"; 
		}

		return 0.f;
	}
	

private:
	std::map<Layer, std::vector<Neuron>> neurons;
	std::vector<Connection> cons;
};