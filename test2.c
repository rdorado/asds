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

int main(int argc, char *argv[])
{
   char str[100];
   int i, j; 
   int x0=-1, x1=-1; 

   /**************************************************
      SENNA setup
   **************************************************/
   /* options */
    char *opt_path = NULL;
    int opt_usrtokens = 0;
    int vbs_hash_novb_idx = 22;
    int state=0;
 
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
 
    /* FANN setup*/
    fann_type *calc_out;
    fann_type input[2];

    struct fann *ann = fann_create_from_file("asds.net");

    while(strcmp(str,"exit")!=0){

      /* Make question and read user's input */
      printf("A> Hi. What do you want?\nY> ");

      fgets(str, 100, stdin);
      i = strlen(str)-1;
      if( str[ i ] == '\n') str[i] = '\0';

      /* String to features */
      SENNA_Tokens* tokens = SENNA_Tokenizer_tokenize(tokenizer, str);
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
      input[0]=-1;
      input[1]=-1;

      for(i = 0; i < tokens->n; i++){
        printf("%s %s ",tokens->words[i], SENNA_Hash_key(pos_hash, pos_labels[i]));
        printf("%s", (vbs_labels[i] ? tokens->words[i] : "-"));
        for(j = 0; j < n_verbs; j++){
          printf(" '%s'", SENNA_Hash_key(srl_hash, srl_labels[j][i]));

            //printf("%s %s %i %i", tokens->words[i], SENNA_Hash_key(srl_hash, srl_labels[j][i]), strcmp(tokens->words[i],"want"));
          if(strcmp(tokens->words[i],"want") == 0 && strcmp(SENNA_Hash_key(srl_hash, srl_labels[j][i]),"S-V") == 0){
            input[0] = 1;
          }
          else if(strcmp(tokens->words[i],"pizza") == 0 && strcmp(SENNA_Hash_key(srl_hash, srl_labels[j][i]),"E-A1") == 0){
            input[1] = 1;
          }          
        }
        printf("\n");
      }

      calc_out = fann_run(ann, input);

      printf("Inputs: x0= %f, x1= %f, Output: %f\n", input[0], input[1], calc_out[0]);

      /* Decision Logic */
      if(calc_out[0] < 0){
         printf("A> I don't know what you are talking about!\n\n");
      }
      else{
         printf("A> Sure!\n\n");
      }

    }

  
  return 0;
}

