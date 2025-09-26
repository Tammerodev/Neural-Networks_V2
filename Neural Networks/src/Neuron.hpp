#pragma once 


enum Layer {
	Input, Hidden, Hidden2, Output
};

struct Neuron {
	Neuron() {

	}
	Neuron(float v) : value(v) {

	}
	float value = 0.f;
};

struct NeuronIndex {
	NeuronIndex() {
		layer = Input;
		index = 0;
	}

	NeuronIndex(Layer l, int i) : layer(l), index(i) {
		layer = l;
		index = i;
	}

	Layer layer = Input;
	int index = 0;
};

struct Connection {
	Connection(float w, NeuronIndex from, NeuronIndex to) {
		weight = w;
		connectedFromIndex = from;
		connectedToIndex = to;
	}
	float weight = 0.f;
	NeuronIndex connectedFromIndex;
	NeuronIndex connectedToIndex;

	int connectedFromLayer = 0;
};