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


int main_tr1(int argc, char *argv[])
{
    int i; 
 
   /**************************************************
      SENNA setup
   **************************************************/
   /* options */
    char *opt_path = NULL;
    int opt_usrtokens = 0;

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


   /**************************************************
      main program
   **************************************************/

    /* Read the training file line by line*/
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("training.dat", "r");

    while ((read = getline(&line, &len, fp)) != -1) {
        SENNA_Tokens* tokens = SENNA_Tokenizer_tokenize(tokenizer, line);
        
        for(i = 0; i < tokens->n; i++){
           printf("%s\n", tokens->words[i]);
        }
        printf("%s\n", line);
    }
    fclose(fp);


 return 0;
}


