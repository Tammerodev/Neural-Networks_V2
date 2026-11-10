#pragma once

#include <map>
#include <iostream>

#include "Neuron.hpp"
#include "Math.hpp"

class NeuralNet
{
public:
	NeuralNet() {

	}

	NeuralNet(int n_input, int n_hidden, int n_hidden2, int n_output) {
		neurons_input = n_input;
		neurons_hidden = n_hidden;
		neurons_hidden2 = n_hidden2;
		neurons_output = n_output;

		create_neural_network_map();
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
		delete_neural_network_map();
		create_neural_network_map();

		for (int i = 0;i < randomInt(5000); i++) {
			randomConnection();
		}
	}

	Neuron* neuronAtNIndex(const NeuronIndex &i) {
		if(neurons.at(i.layer).size() <= i.index) return &neurons.at(Input).at(0);

		return &neurons.at(i.layer)[i.index];
	}

	void create_neural_network_map() {
		// Input
		neurons.insert({ Input, std::vector<Neuron>(neurons_input)});
		// Hidden layer
		neurons.insert({ Hidden, std::vector<Neuron>(neurons_hidden)});
		neurons.insert({ Hidden2, std::vector<Neuron>(neurons_hidden2)});
		// Output layer
		neurons.insert({ Output, std::vector<Neuron>(neurons_output)});
	}

	void inherit_neural_network(const NeuralNet cpy, int randomness) {  // pass by const ref to avoid copies
		delete_neural_network_map();
		create_neural_network_map();

		if(cpy.neurons.size() <= 0 || cpy.cons.size() <= 0) {
			initRand(); 
			return;
		}

		neurons = cpy.neurons;
		cons = cpy.cons;

		for (int i = 0; i < randomInt(randomness); i++) {
			randomConnection();
		}

		if (!cons.empty()) {
			for (int i = 0; i < randomInt(randomness); i++) {
				cons[rand() % cons.size()].weight += randFloat() * randFloat();
			}
		}
	}


	void delete_neural_network_map() {
		cons.clear();
		neurons.clear();
	}

	void update() {
		for (auto& con : cons) {
			if(con.connectedFromIndex.layer == Input) continue;

			neurons.at(con.connectedFromIndex.layer).at(con.connectedFromIndex.index) = 0.f;
			neurons.at(con.connectedToIndex.layer).at(con.connectedToIndex.index) = 0.f;
		}

		for (auto& con : cons) {
			if(con.connectedFromIndex.layer == Input)
				neuronAtNIndex(con.connectedToIndex)->value += neuronAtNIndex(con.connectedFromIndex)->value * con.weight;
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

	void setInput(int index, float value) {
		if(index >= neurons_input) return;

		neurons.at(Input)[index].value = value;
	}

	sf::Vector2f getOutputAsVelocity() {
		return sf::Vector2f(getOutputFrom(0), getOutputFrom(1));
	}

	float getOutputFrom(int index) {
		if(index >= neurons_output) return 0.f;

		return neurons.at(Output).at(index).value;
	}

	uint64_t getConnectionsCount() {
		return cons.size();
	}
	
private:
	std::map<Layer, std::vector<Neuron>> neurons;
	std::vector<Connection> cons;

	int neurons_input = 0;
	int neurons_hidden = 0;
	int neurons_hidden2 = 0;
	int neurons_output = 0;
};