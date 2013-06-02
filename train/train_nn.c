#include "fann.h"

int main()
{
    const unsigned int num_input = 13;
    const unsigned int num_output = 4;
    const unsigned int num_layers = 4;
    const unsigned int num_neurons_hidden1 = 8;
	const unsigned int num_neurons_hidden2 = 5;
    const float desired_error = (const float) 0.001;
    const unsigned int max_epochs = 500000;
    const unsigned int epochs_between_reports = 1000;

    struct fann *ann = fann_create_standard(num_layers, num_input, num_neurons_hidden1, num_neurons_hidden2,num_output);

    fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
    fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);
	fann_set_training_algorithm(ann,FANN_TRAIN_RPROP);
	fann_randomize_weights(ann,0.0,0.1);
    fann_train_on_file(ann, "new_feature_train.set", max_epochs, epochs_between_reports, desired_error);

    fann_save(ann, "digital.net");

    fann_destroy(ann);

    return 0;
}