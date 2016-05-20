#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "SENNA_utils.h"
#include "SENNA_Hash.h"
#include "SENNA_Tokenizer.h"

#include "SENNA_POS.h"
#include "SENNA_CHK.h"
#include "SENNA_NER.h"
#include "SENNA_VBS.h"
#include "SENNA_PT0.h"
#include "SENNA_SRL.h"
#include "SENNA_PSG.h"

#include "fann.h"

/* fgets max sizes */
#define MAX_SENTENCE_SIZE 1024
#define MAX_TARGET_VB_SIZE 256

void data_callback(unsigned int num, unsigned int num_input, unsigned int num_output, fann_type* input, fann_type* output) {
}

int main_tr4(int argc, char *argv[])
{
    int i, j; 
 
   /**************************************************
      SENNA setup
   **************************************************/
   /* options */
    char *opt_path = NULL;
    int opt_usrtokens = 0;
    int vbs_hash_novb_idx = 22;

    /* inputs */
    SENNA_Hash *word_hash = SENNA_Hash_new(opt_path, "hash/words.lst");
    SENNA_Hash *caps_hash = SENNA_Hash_new(opt_path, "hash/caps.lst");
    SENNA_Hash *suff_hash = SENNA_Hash_new(opt_path, "hash/suffix.lst");
    SENNA_Hash *gazt_hash = SENNA_Hash_new(opt_path, "hash/gazetteer.lst");

    SENNA_Hash *gazl_hash = SENNA_Hash_new_with_admissible_keys(opt_path, "hash/ner.loc.lst", "data/ner.loc.dat");
    SENNA_Hash *gazm_hash = SENNA_Hash_new_with_admissible_keys(opt_path, "hash/ner.msc.lst", "data/ner.msc.dat");
    SENNA_Hash *gazo_hash = SENNA_Hash_new_with_admissible_keys(opt_path, "hash/ner.org.lst", "data/ner.org.dat");
    SENNA_Hash *gazp_hash = SENNA_Hash_new_with_admissible_keys(opt_path, "hash/ner.per.lst", "data/ner.per.dat");

    SENNA_Tokenizer *tokenizer = SENNA_Tokenizer_new(word_hash, caps_hash, suff_hash, gazt_hash, gazl_hash, gazm_hash, gazo_hash, gazp_hash, opt_usrtokens);
    SENNA_SRL *srl = SENNA_SRL_new(opt_path, "data/srl.dat");

    /* labels */
    SENNA_Hash *pos_hash = SENNA_Hash_new(opt_path, "hash/pos.lst");
    SENNA_Hash *srl_hash = SENNA_Hash_new(opt_path, "hash/srl.lst");

    SENNA_POS *pos = SENNA_POS_new(opt_path, "data/pos.dat");
    SENNA_VBS *vbs = SENNA_VBS_new(opt_path, "data/vbs.dat");
    SENNA_PT0 *pt0 = SENNA_PT0_new(opt_path, "data/pt0.dat");

    /* FANN  setup */
    const unsigned int num_input = 2;
    const unsigned int num_output = 1;
    const unsigned int num_layers = 3;
    const unsigned int num_neurons_hidden = 3;
    const float desired_error = (const float) 0.00001;
    const unsigned int max_epochs = 500000;
    const unsigned int epochs_between_reports = 100000;

    struct fann *ann = fann_create_standard(num_layers, num_input, num_neurons_hidden, num_output);

    fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
    fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);

    struct fann_train_data *train_data;
    train_data = fann_create_train_from_callback(4, 2, 1, &data_callback); 

   /**************************************************
      main program
   **************************************************/

    /* Read the training file line by line*/
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("training.dat", "r");

    int targets[] = {1, 1, 1, -1};
    int id=0;
    while ((read = getline(&line, &len, fp)) != -1) {

        SENNA_Tokens* tokens = SENNA_Tokenizer_tokenize(tokenizer, line);
        int *pos_labels = SENNA_POS_forward(pos, tokens->word_idx, tokens->caps_idx, tokens->suff_idx, tokens->n);
        int *pt0_labels = SENNA_PT0_forward(pt0, tokens->word_idx, tokens->caps_idx, pos_labels, tokens->n);
        int n_verbs = 0;
        char target_vb[MAX_TARGET_VB_SIZE];
        int *vbs_labels = SENNA_VBS_forward(vbs, tokens->word_idx, tokens->caps_idx, pos_labels, tokens->n);
        for(i = 0; i < tokens->n; i++){
           vbs_labels[i] = (vbs_labels[i] != vbs_hash_novb_idx);
           n_verbs += vbs_labels[i];
        }
        int **srl_labels = SENNA_SRL_forward(srl, tokens->word_idx, tokens->caps_idx, pt0_labels, vbs_labels, tokens->n);

        /* Logic */ 
        train_data->input[id][0] = -1;
        train_data->input[id][1] = -1;

        for(i = 0; i < tokens->n; i++){
          printf("%s %s ",tokens->words[i], SENNA_Hash_key(pos_hash, pos_labels[i]));
          printf("%s", (vbs_labels[i] ? tokens->words[i] : "-"));
          for(j = 0; j < n_verbs; j++){
            printf(" '%s'", SENNA_Hash_key(srl_hash, srl_labels[j][i]));

            //printf("%s %s %i %i", tokens->words[i], SENNA_Hash_key(srl_hash, srl_labels[j][i]), strcmp(tokens->words[i],"want"));
            if(strcmp(tokens->words[i],"want") == 0 && strcmp(SENNA_Hash_key(srl_hash, srl_labels[j][i]),"S-V") == 0){
              train_data->input[id][0] = 1;
            }
            else if(strcmp(tokens->words[i],"pizza") == 0 && strcmp(SENNA_Hash_key(srl_hash, srl_labels[j][i]),"E-A1") == 0){
              train_data->input[id][1] = 1;
            }          
          }
          printf("\n");
        }

        train_data->output[id][0] = targets[id];
        
        printf("%s", line);
        printf("Input: %f %f, Output: %d\n\n", train_data->input[id][0], train_data->input[id][1], targets[id]);
        id++;

    }

    /* Train a classifier */
    fann_train_on_data(ann, train_data, max_epochs, epochs_between_reports, desired_error);
    fann_save(ann, "asds.net");

    fann_destroy(ann);
    fclose(fp);


 return 0;
}


