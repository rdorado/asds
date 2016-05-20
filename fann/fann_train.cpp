#include "fann.h"
#include "fann.h"

void data_callback(unsigned int num, unsigned int num_input, unsigned int num_output, fann_type* input, fann_type* output) {

}

int main()
{
    /* Setup the ANN */
    const unsigned int num_input = 2;
    const unsigned int num_output = 1;
    const unsigned int num_layers = 3;
    const unsigned int num_neurons_hidden = 3;
    const float desired_error = (const float) 0.00001;
    const unsigned int max_epochs = 500000;
    const unsigned int epochs_between_reports = 1000;

    struct fann *ann = fann_create_standard(num_layers, num_input, num_neurons_hidden, num_output);

    fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
    fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);
   
    
    fann_train_data *train_data;
    train_data = fann_create_train_from_callback(4, 2, 1, &data_callback); 

    train_data->input[0][0] = -1;
    train_data->input[0][1] = -1;
    train_data->output[0][0] = -1;

    train_data->input[1][0] = -1;
    train_data->input[1][1] = 1;
    train_data->output[1][0] = 1;

    train_data->input[2][0] = 1;
    train_data->input[2][1] = -1;
    train_data->output[2][0] = 1;

    train_data->input[3][0] = 1;
    train_data->input[3][1] = 1;
    train_data->output[3][0] = -1;

    fann_train_on_data(ann, train_data, max_epochs, epochs_between_reports, desired_error);
    

//    fann_train_on_file(ann, "xor.data", max_epochs, epochs_between_reports, desired_error);

    fann_save(ann, "xor_float.net");

    fann_destroy(ann);

    return 0;
}
