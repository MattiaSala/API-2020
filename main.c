#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define DELIMITER ","
#define MAX_LENGTH 1024
#define MAX_READ 20

/*============================ GLOBAL VARIABLES ===================================*/
unsigned int address1 = 0;
unsigned int address2 = 0;
unsigned int number = 0;
unsigned int num_of_undo = 0;
unsigned int num_of_redo = 0;
char cmdLine[MAX_READ] = "";
char maxLine[MAX_LENGTH] = "";
char *in_line = NULL;
/*===============================================================================*/

/*========================== STACK FOR COMMANDS AND UNDO STACK ===================================*/
struct cmd_node{
    unsigned int addr1;
    unsigned int addr2;
    char cmd;
    unsigned int num_of_el;
    unsigned int old_ht_size;
    char **lines;

    struct cmd_node *prev;
    struct cmd_node *next;
};

/*main stack for input commands*/
struct cmd_node *undo_top;
struct cmd_node *undo_bottom;
unsigned int undo_stack_size = 0;

/*stack for nodes undone by 'u' commands*/
struct cmd_node *redo_top;
struct cmd_node *redo_bottom;
unsigned int redo_stack_size = 0;
/*===============================================================================*/

/*======================== HASH TABLE FOR CURRENT CONTENT =========================*/
typedef struct{
    char *curr_line;
}key;
key *hashtable;
unsigned int hashtable_size = 0;
/*===============================================================================*/


/*==================================== FUNCTIONS ===================================*/
char findcmd(const char* s);
void resizeHashTable(size_t new_dimension);
void print(unsigned int addr_start, unsigned int addr_end);
void printHashTable();
void insertLineInHash(char *str, int index);

struct cmd_node* create_d_node(unsigned int a1, unsigned int a2);
struct cmd_node* create_c_node(unsigned int a1, unsigned int a2);

void push_undo_stack(struct cmd_node *node);
void deleteElements(unsigned int a1, unsigned int a2);

void undo_redo(unsigned int flag);
struct cmd_node *pop_undo_top();

//void pop_undo_restore_prev_state(struct cmd_node* popped_node);
void restore_undo();
void restore_redo();

struct cmd_node *create_redo_node(int address1, int address2, char cmd);
struct cmd_node *create_undo_node(int address1, int address2, char cmd);
void free_redo_stack();

/*===============================================================================*/

int main(int argc, char const *argv[]) {
    /*read line and analyze it*/
    hashtable = NULL;
    while ((in_line = fgets(cmdLine,MAX_READ,stdin)) != NULL) { //reads the entire line from stdin use

            char cmnd = findcmd(in_line);   //take the last char of the string to identify what the command is  fputs(),

            if(cmnd == 'q'){ //q: quit
                //printStack(bottom);
                //freeAll();
                //freeAllUndoStack(); //NEW
                return 0;
            }

            else if(cmnd == 'c'){    //c: changes the text between [addr1 ; addr2] with given lines from stdin
                //printHashTable();
                //printStack(bottom);
                //int old_size = hashtable_size;
                if((num_of_undo > 0) || (num_of_redo > 0)){//do all the remaining undo to do
                    undo_redo(1);
                    num_of_undo = 0;
                    num_of_redo = 0;
                }
                //free_redo_stack();
                address1 = atoi(strtok(in_line,DELIMITER));
                address2 = atoi(strtok(NULL,"c"));

                //printf("COMANDO IN INPUT: %d,%d%c\n",address1,address2,cmnd);

                

                struct cmd_node *new_node = create_c_node(address1,address2); //createnode(address1,address2,cmnd,strings);
                push_undo_stack(new_node);

                if(hashtable_size < address2){
                    resizeHashTable((size_t)address2);
                }

                for(int i=0; i <= (address2-address1);i++){
                    in_line = fgets(maxLine,MAX_LENGTH,stdin);
                    size_t size = strlen(in_line)+1;
                    char *string = NULL;
                    string = (char *) malloc((size)*sizeof(char));
                    //memcpy(string,in_line,(size));
                    strcpy(string,in_line);
                    insertLineInHash(string, (address1-1+i));
                }

            }
            else if(cmnd == 'd'){    //d: delete the text between [addr1 ; addr2]
                if((num_of_undo > 0) || (num_of_redo > 0)){//do all the remaining undo to do
                    undo_redo(1);
                    num_of_undo = 0;
                    num_of_redo = 0;
                }
                //free_redo_stack();
                address1 = atoi(strtok(in_line,DELIMITER));
                address2 = atoi(strtok(NULL,"d"));
                if(address1 == 0 && address2 > 0){
                    address1 = 1;
                }

                //printf("COMANDO IN INPUT: %d,%d%c\n",address1,address2,cmnd);

                struct cmd_node *new_node = create_d_node(address1,address2); //createnode(address1,address2,cmnd,NULL);
                //printHashTable();
                push_undo_stack(new_node);
                deleteElements(address1,address2);    //---------*******-------
                //printHashTable();
                //printf("MAIN STACK SIZE: %d  ---------  UNDO STACK SIZE: %d\n\n", stack_size, undo_stack_size);
            }
            else if(cmnd == 'p'){    //p: print lines from data structure into an output file

                if((num_of_undo > 0) || (num_of_redo > 0)){    //do all the remaining undo to do
                    undo_redo(0);
                    num_of_undo = 0;
                    num_of_redo = 0;
                }
                

                address1 = atoi(strtok(in_line,DELIMITER));
                address2 = atoi(strtok(NULL,"p"));

                //printf("COMANDO IN INPUT: %d,%d%c\n",address1,address2,cmnd);
                //printf("MAIN STACK SIZE: %d  ---------  UNDO STACK SIZE: %d\n\n", stack_size, undo_stack_size);
                //printHashTable();
                print(address1,address2);

            }
            else if(cmnd == 'u'){    //u: undo of num cmds
                number = atoi(strtok(in_line,"u"));

                if(undo_stack_size == 0){
                    num_of_undo = 0;
                }
                else if((num_of_undo + number) > undo_stack_size && (undo_stack_size > 0)){    //if the number of undos is bigger than the current size of stack of commands
                    num_of_undo = undo_stack_size;
                }else{
                    num_of_undo += number;
                }    
            }
            else if(cmnd == 'r'){    //r: redo of num cmds
                number = atoi(strtok(in_line,"r"));

                if(redo_stack_size == 0){
                    num_of_redo = 0;
                }
                else if((num_of_redo + number) > redo_stack_size && (redo_stack_size > 0) ){
                    num_of_redo = redo_stack_size;
                }else{
                    num_of_redo += number;
                }
                
            }

    }

}//end main

/*
*Finds last char of the string
*@param const char* s --> string to get last char
@return --> last char of the string
*/char findcmd(const char* s){
    char c = '0';

    for(;*s != '\n' && *s != '\0';s++){ //AGGIUNTO: && *s != '\0' (controllare se giusto effettivamente!)
        c = *s;
    }
    return c;
}

/*
*   reallocates and grows the hashtable dimension given the param.
*   @param int new_dimension --> new dimension of the hash table
*/
void resizeHashTable(size_t new_dimension){
    if(new_dimension == 0){
        if(hashtable != NULL){
            free(hashtable);    //va anche senza
            hashtable = NULL;
            hashtable_size = 0;
        }
    }else{
        if(hashtable_size == 0 && hashtable == NULL){
            hashtable = (key *)calloc(new_dimension,sizeof(key));        
        }else{
            hashtable = (key *)realloc(hashtable,(new_dimension)*sizeof(key));
            for(int i=hashtable_size; i<new_dimension; i++){    //meh non mi piace ma fa funzionare
                hashtable[i].curr_line = NULL;
            }
        }
        hashtable_size = new_dimension;
    }
}

/*
* Print command 'p'
* @param unsigned int addr_start --> starting address
* @param unsigned int addr_end --> ending address
*/
void print(unsigned int addr_start, unsigned int addr_end){
    if(addr_start == 0 && addr_end ==0){    //0,0p
        fputs(".\n",stdout);
    }else if(hashtable == NULL || (addr_start > hashtable_size && addr_end > hashtable_size) ){
        for(int i=addr_start; i<=addr_end; i++){
            fputs(".\n",stdout);
        }
    }    
    /*else if(addr_start > hashtable_size && addr_end > hashtable_size && hashtable != NULL){ //se stampo indirizzi oltre la dimensione hashtable.
        //for(int i=0; i< (addr_end-addr_start);i++){
        //    fputs(".\n",stdout);
        //}
        for(int i=addr_start; i<= addr_end;i++){
            fputs(".\n",stdout);
        }

    }*/
    else if(addr_start <= hashtable_size && addr_end > hashtable_size && hashtable != NULL){ //se l'indirizzo iniziale è contenuto nella hashtable, ma non quello  finale.
        for(int i=addr_start-1; i<= hashtable_size-1;i++){ 
            if(hashtable[i].curr_line != NULL){
                fputs(hashtable[i].curr_line ,stdout);
            }else{
                fputs("YOU READING A NULL LINE!!! \n",stdout);
            }
        }

        int k = hashtable_size;
        while(k < addr_end){
            fputs(".\n",stdout);
            k++;
        }

    }else if( addr_start <= hashtable_size && addr_end <= hashtable_size && hashtable != NULL ){  //Se sia addr_start che addr_end sono dimensioni interne alla hashtable_size
        for(int i=addr_start-1; i< addr_end;i++){
            //if(hashtable[i].curr_line == NULL){
            //    fputs("STAI A LEGGE NULL FRATM\n",stdout);
            //}else{
                fputs(hashtable[i].curr_line ,stdout);
            //}
            
        }
    }
}

/*
* prints the hash table content
*/
void printHashTable(){
    fputs("=================================================\n",stdout);
    if(hashtable_size == 0){
        fputs("Hash Table is EMPTY\n",stdout);
    }
    else{
        printf("Hash Table size: %d\n",hashtable_size);
        for(int i=0; i< hashtable_size;i++){
            printf("H[%d]: %s\n",i,hashtable[i].curr_line);
        }
    }

    fputs("=================================================\n",stdout);
}

/*
* Insert of string pointer into hashtable
*/
void insertLineInHash(char *str, int index){
    //printf("INDEX: %d   HT_SIZE: %d\n",index,hashtable_size);
    if(hashtable != NULL && hashtable_size>0){
        if(hashtable[index].curr_line == NULL){
            hashtable[index].curr_line = str;
        }else{
            if( strcmp(hashtable[index].curr_line, str) != 0){  //se linea in input è diversa da quella che ho già in hashtable, non la inserisco (altrimenti duplico e occupo memoria)
                hashtable[index].curr_line = str; //stringa diversa, cella i-esima hashtable punta alla stringa in mmemoria
            }else{  //se riga già dentro, allora non la duplico
                free(str);
                str = NULL;
            }
        }

        
    }else{
        hashtable[index].curr_line = str;
    }
    
    
}

/*
* creates a 'c' node
*/
struct cmd_node* create_c_node(unsigned int a1, unsigned int a2){
    struct cmd_node *node = (struct cmd_node*)malloc(sizeof(struct cmd_node));
    node->addr1 = a1;
    node->addr2 = a2;
    node->cmd = 'c';
    node->old_ht_size = hashtable_size;
        
    if(hashtable == NULL && hashtable_size == 0){   //hashtable vuota: non salvo i puntatori delle vecchie stringhe
        node->lines = NULL;
        node->num_of_el = 0;

    }else if(hashtable != NULL && hashtable_size>0){    //se hashtable non è vuota
        if(a1 > hashtable_size){
            node->lines = NULL;
            node->num_of_el = 0;
        }
        else if( (a1 <= hashtable_size) && (a2 > hashtable_size) ){ //if starting address is <= than the actual hash table size but the ending address is bigger
            node->num_of_el = (hashtable_size-a1+1);//(hashtable_size-a1+1);
            node->lines = (char **)malloc((node->num_of_el)*sizeof(char*));

            for(int i=0; i <= (node->num_of_el - 1);i++){
                node->lines[i] = hashtable[a1-1+i].curr_line;
            }   

        }else {
            node->num_of_el = (a2-a1+1);
            node->lines = (char **)malloc((node->num_of_el)*sizeof(char*));

            for(int i=0; i <= (node->num_of_el-1); i++){
                node->lines[i] = hashtable[a1-1+i].curr_line;
            }
            
        }
    }

    node->prev = NULL;
    node->next = NULL;

    return node;
}

struct cmd_node* create_d_node(unsigned int a1, unsigned int a2){
    struct cmd_node *node = (struct cmd_node*)malloc(sizeof(struct cmd_node));
    node->addr1 = a1;
    node->addr2 = a2;
    node->cmd = 'd';
    node->old_ht_size = hashtable_size;

    if(hashtable != NULL &&  hashtable_size >0){  //if the hash table exists
        //int n_cycles = 0;
        if(a1 > hashtable_size){    //se bisogna fare la delete di righe che non ci sono
            node->lines = NULL;
            node->num_of_el = 0;
        }
        else if( (a1 <= hashtable_size) && (a2 > hashtable_size) ){ //if starting address is <= than the actual hash table size but the ending address is bigger
            node->num_of_el = (hashtable_size-a1+1);
            node->lines = (char **)malloc((hashtable_size-a1+1)*sizeof(char*));
            for(int i=0; i <= (hashtable_size-a1);i++){
                node->lines[i] = hashtable[a1-1+i].curr_line;
            }
            
        }else {
            node->num_of_el = (a2-a1+1);
            node->lines = (char **)malloc((a2-a1+1)*sizeof(char*));
            for(int i=0; i <= (a2-a1);i++){
                node->lines[i] = hashtable[a1-1+i].curr_line;
            }
        }
            
    }else{  //if the hash table doesn't exists
        node->lines = NULL;
        node->num_of_el = 0;
    }

    node->prev = NULL;
    node->next = NULL;

    return node;
}

/*
*Push of a command node into the stack
*@param a1 --> addr1
*@param a2 --> addr2
*@param c  --> cmd
*@param str_array --> array of strings to put into the text
*/
void push_undo_stack(struct cmd_node *node){
    if(undo_stack_size == 0 && undo_top == NULL){    //provare anche con  top == NULL
        undo_bottom = node;
        undo_top = undo_bottom;
        undo_top->next = NULL; //---------new
        undo_stack_size++;

    }else{
 
        if(node != NULL){
            undo_top->next = node;
            node->prev = undo_top;
            undo_top = node;
            undo_top->next = NULL; //---------new

            undo_stack_size++;
        }
    }
}

/*
*Push of a command node into the redo stack
*@param node --> node to push into redo stack
*/
void push_redo_stack(struct cmd_node *node){
    if(redo_stack_size == 0 && redo_top == NULL){    //provare anche con  top == NULL
        redo_bottom = node;
        redo_top = redo_bottom;
        redo_top->next = NULL; //---------new
        redo_stack_size++;

    }else{
 
        if(node != NULL){
            redo_top->next = node;
            redo_top->prev = redo_top;
            redo_top = node;
            redo_top->next = NULL; //---------new

            redo_stack_size++;
        }
    }
}


/*
* Delete elements from hash table given the boundaries
* @param unsigned int a1 --> starting node to eliminate
* @param unsigned int a2 --> ending node to eliminate
*/
void deleteElements(unsigned int a1, unsigned int a2){
    if(hashtable != NULL && (a1 > 0 && a2 > 0)){    //HASH TABLE non nulla, indirizzi da eliminare positivi > 0
        if(a1 == hashtable_size && a2 >= hashtable_size){    //se elimino dall'ultima riga in poi
            resizeHashTable(hashtable_size-1);
        }
        else if ( (a1 == 1) && (a2 >= hashtable_size) ){
            hashtable_size = 0;
            free(hashtable);
            hashtable = NULL;   //<---AGGIUNTO QUESTO, PERCHÈ DOPO CHE SI FA LA FREE, È BUONA PRATICA METTERE A NULL CIÒ DI CUI SI È FATTA LA FREE.
        }
        else if( (a1<hashtable_size) && (a2>hashtable_size) ){ //se 'a1' è dentro nella hashtable, ma a2 no.
            int n_del = hashtable_size-a1+1;
            resizeHashTable(hashtable_size - n_del);
        }
        else if( (a1<hashtable_size) && (a2<hashtable_size) ){ //caso generico se entrambi indirizzi contenuti
            int n_del = (a2-a1+1);
            int i=0;

            while((a1-1+i+n_del)<= (hashtable_size-1)){
                hashtable[a1-1+i].curr_line = hashtable[a1-1+i+n_del].curr_line;
                i++;
            }
            resizeHashTable(hashtable_size - n_del);

        }
        else if ( (a1<hashtable_size) && (a2==hashtable_size) )
        {
            int n_del = a2-a1+1;
            resizeHashTable(hashtable_size - n_del);           
        }
        
    }
    else if(hashtable != NULL && (a1 == 0 && a2 > 0)){
        if (a2 >= hashtable_size){
            hashtable_size = 0;
            free(hashtable);
            hashtable = NULL;  //<---AGGIUNTO QUESTO, PERCHÈ DOPO CHE SI FA LA FREE, È BUONA PRATICA METTERE A NULL CIÒ DI CUI SI È FATTA LA FREE.
        }
        else{
            int ad1 = (a1+1);
            int n_del = (a2-a1);//+1
            for(int i = 0; i <= hashtable_size-n_del-1;i++){
                hashtable[ad1-1+i].curr_line = hashtable[ad1-1+i+n_del].curr_line;
            }
             
            resizeHashTable(hashtable_size - n_del+1);  
        }
        
    }
    
}

/*
*Undo of a n_undo number of commands
*@param unsigned int flag --> flag to tell if undo function is called at a 'p' command (0) or 'c'/'d' command (1)
*/
void undo_redo(unsigned int flag){
    
    if (flag == 1){ //'c'/'d' command
        if(redo_top != NULL){
            if( (num_of_undo > 0) || (num_of_redo > 0) ){
                //printf("NUM_UNDO && NUM_REDO >0 !\n");
                undo_redo(0);
            }

            free_redo_stack();//free nodes of undo stack
        }
        
    }else{ //'p' command
        if( (num_of_undo == 0) && (num_of_redo > 0) ){//SOLO REDO
            if( num_of_redo >= redo_stack_size ){
                //riporta tutto il contenuto dell'undo_stack nello stack principale
                /*int cycles = undo_stack_size;
                for(int i=1; i<=(cycles);i++){
                    struct cmd_node *new_redo_node = create_redo_node(undo_top->addr1, undo_top->addr2, undo_top->cmd);
                    push_redo_stack(new_redo_node);
                    //printf("REDO STACK SIZE: %d\n",redo_stack_size);
                    restore_undo();

                    //----PERÒ È SBAGLIATA DA FARE COSÌ-----*
                    //free(hashtable);
                    //hashtable = NULL;
                    //hashtable_size = 0;
                    //---------//
                    
                    struct cmd_node *old_undo_node = undo_top;
                    undo_top = old_undo_node->prev;

                    free(old_undo_node->lines);
                    free(old_undo_node);

                    undo_stack_size--;
                }*/
                while (redo_stack_size != 0 && redo_top != NULL){
                    struct cmd_node *new_undo_node = create_undo_node(redo_top->addr1, redo_top->addr2, redo_top->cmd);
                    push_undo_stack(new_undo_node);
                    //printf("REDO STACK SIZE: %d\n",redo_stack_size);
                    restore_redo();

                    struct cmd_node *old_redo_node = redo_top;
                    redo_top = old_redo_node->prev;

                    free(old_redo_node->lines);
                    free(old_redo_node);

                    redo_stack_size--;
                }

            }else{
                //riporta num_of_redo nodi nello stack pricipale
                for(int i=1; i<=(num_of_redo);i++){
                    struct cmd_node *new_undo_node = create_undo_node(redo_top->addr1, redo_top->addr2, redo_top->cmd);
                    push_undo_stack(new_undo_node);
                    //printf("REDO STACK SIZE: %d\n",redo_stack_size);
                    restore_redo();

                    struct cmd_node *old_redo_node = redo_top;
                    redo_top = old_redo_node->prev;
                    free(old_redo_node->lines);
                    free(old_redo_node);

                    redo_stack_size--;
                }
            }

        }else if( (num_of_undo > 0) && (num_of_redo == 0) ){    //SOLO UNDO
            if(num_of_undo >= undo_stack_size){
                //porta il restante dello stack principale, dentro l'undo_stack
                //printf("REDO STACK SIZE: %d\n",redo_stack_size);
                //printf("UNDO STACK SIZE: %d\n",undo_stack_size);
                while (undo_stack_size != 0 && undo_top != NULL)
                {
                    //pop_undo_restore_prev_state(undo_top);
                    //struct cmd_node *node_popped = undo_top;//pop_undo_top();
                    //undo_top = node_popped->prev;
                    //node_popped->prev = NULL;

                    //
                    //push_redo_stack(node_popped);
                    //if(node_popped != NULL){
                    //    pop_restorePrevState(node_popped);
                    //    push_undo(node_popped);
                    //}

                    struct cmd_node *new_redo_node = create_redo_node(undo_top->addr1, undo_top->addr2, undo_top->cmd);
                    push_redo_stack(new_redo_node);
                    //printf("REDO STACK SIZE: %d\n",redo_stack_size);
                    restore_undo();

                    /*----PERÒ È SBAGLIATA DA FARE COSÌ-----*/
                    //free(hashtable);
                    //hashtable = NULL;
                    //hashtable_size = 0;
                    /*---------*/
                    
                    struct cmd_node *old_undo_node = undo_top;
                    undo_top = old_undo_node->prev;

                    free(old_undo_node->lines);
                    free(old_undo_node);

                    undo_stack_size--;
                }

                //printf("UNDO STACK SIZE: %d\n",undo_stack_size);
                
            }else{
                //porta num_of_undo nodi da stack principale in undo_stack 
                for(int i=1; i<=(num_of_undo);i++){
                    struct cmd_node *new_redo_node = create_redo_node(undo_top->addr1, undo_top->addr2, undo_top->cmd);
                    push_redo_stack(new_redo_node);
                    restore_undo();
                    struct cmd_node *old_undo_node = undo_top;
                    undo_top = old_undo_node->prev;

                    free(old_undo_node->lines);
                    free(old_undo_node);

                    undo_stack_size--;
                }
            }
        }else if( (num_of_undo > 0) && (num_of_redo > 0) ){     //CASO MEDIO
            if(num_of_undo >= num_of_redo){
                num_of_undo = num_of_undo - num_of_redo;

                if( num_of_undo >= undo_stack_size ){
                    //riporta tutto il contenuto dell'undo_stack nello stack principale
                    while (undo_stack_size != 0 && undo_top != NULL){
                        struct cmd_node *new_redo_node = create_redo_node(undo_top->addr1, undo_top->addr2, undo_top->cmd);
                        push_redo_stack(new_redo_node);
                        restore_undo();

                        struct cmd_node *old_undo_node = undo_top;
                        undo_top = old_undo_node->prev;

                        free(old_undo_node->lines);
                        free(old_undo_node);

                        undo_stack_size--;
                    }
                }else{
                    //fa num_of_undo undo, date dalla somma di undo e redo
                    for(int i=1; i<=(num_of_undo);i++){
                        struct cmd_node *new_redo_node = create_redo_node(undo_top->addr1, undo_top->addr2, undo_top->cmd);
                        push_redo_stack(new_redo_node);
                        restore_undo();
                        struct cmd_node *old_undo_node = undo_top;
                        undo_top = old_undo_node->prev;

                        free(old_undo_node->lines);
                        free(old_undo_node);

                        undo_stack_size--;
                    }
                }//end caso medio solo undo

            }else if(num_of_undo < num_of_redo){
                num_of_redo = num_of_redo - num_of_undo;

                if(num_of_redo >= redo_stack_size){
                    //azzera redo_stack
                    while (redo_stack_size != 0 && redo_top != NULL){
                        struct cmd_node *new_undo_node = create_undo_node(redo_top->addr1, redo_top->addr2, redo_top->cmd);
                        push_undo_stack(new_undo_node);
                        //printf("REDO STACK SIZE: %d\n",redo_stack_size);
                        restore_redo();

                        struct cmd_node *old_redo_node = redo_top;
                        redo_top = old_redo_node->prev;

                        free(old_redo_node->lines);
                        free(old_redo_node);

                        redo_stack_size--;
                    }
                }else{
                    //rimette num_of_redo-nodi nello stack principale
                    for(int i=1; i<=(num_of_redo);i++){
                        struct cmd_node *new_undo_node = create_undo_node(redo_top->addr1, redo_top->addr2, redo_top->cmd);
                        push_undo_stack(new_undo_node);
                        //printf("REDO STACK SIZE: %d\n",redo_stack_size);
                        restore_redo();

                        struct cmd_node *old_redo_node = redo_top;
                        redo_top = old_redo_node->prev;
                        free(old_redo_node->lines);
                        free(old_redo_node);

                        redo_stack_size--;
                    }
                }

            }
        }//FINE CASO MEDIO
        
    }
    
    
}

/*
*returns the top node from the undo stack 
*/
struct cmd_node *pop_undo_top(){
    //if(undo_top != NULL){
        struct cmd_node *node_to_pop = undo_top;
        undo_top = undo_top->prev;
        undo_top->next = NULL;

        node_to_pop->next = NULL;
        node_to_pop->prev = NULL;

        return node_to_pop;
    //}
    //return NULL;
}


/*void pop_undo_restore_prev_state(struct cmd_node* popped_node){
    //struct cmd_node* node = popped_node;
    if(popped_node->cmd == 'c'){
        int added_lines = popped_node->addr2-popped_node->addr1+1-popped_node->num_of_el; //calcolo la differenza tra righe effettivamente modificate e quelle aggiunte
        if( added_lines == 0 ){   //ho solo cambiato e non aggiunto

            for(int i=0; i < (popped_node->addr2 - popped_node->addr1); i++){
                char *temp = hashtable[popped_node->addr1-1+i].curr_line;
                hashtable[popped_node->addr1-1+i].curr_line = popped_node->lines[i];
                popped_node->lines[i] = temp;
            }
            
        }else if(added_lines > 0){
            if(popped_node->num_of_el == 0 && popped_node->lines == NULL){  //prima o l'hashtable era vuota o ho solo aggiunto nuove righe
                
                if(popped_node->addr1 > 1){ //avevo solo aggiunto

                    popped_node->lines = (char **)realloc(popped_node->lines,(added_lines)*sizeof(char*));

                    for(int i=0; i < (popped_node->addr2 - popped_node->addr1); i++){
                        popped_node->lines[i] = hashtable[popped_node->addr1-1+i].curr_line;
                    }
                    resizeHashTable(hashtable_size-added_lines);
                    popped_node->num_of_el = added_lines;

                }else{  //hashtable era vuota
                    //popped_node->lines = (char **)realloc(popped_node->lines,(added_lines)*sizeof(char*));
                    
                    for(int i=0; i < (popped_node->addr2 - popped_node->addr1); i++){
                        printf("INDEX: %d\n",i);
                        popped_node->lines[i] = hashtable[popped_node->addr1-1+i].curr_line;
                    }
                    resizeHashTable(0)//hashtable_size-added_lines); //(provare a controlloare cosa succedere se si fda realloc di 0)
                    if(hashtable_size != 0){
                        //hashtable = NULL;
                        hashtable_size = 0;
                    }

                    //popped_node->num_of_el = added_lines;
                }
            }else{  //ho modificato delle righe e aggiunto delle nuove
                
                for(int i=0; i < (popped_node->num_of_el); i++){
                    char *temp = hashtable[popped_node->addr1-1+i].curr_line;
                    hashtable[popped_node->addr1-1+i].curr_line = popped_node->lines[i];
                    popped_node->lines[i] = temp;
                }

                popped_node->lines = (char **)realloc(popped_node->lines,(added_lines)*sizeof(char*));
                
                for(int i=0; i < (popped_node->addr2 - popped_node->num_of_el); i++){
                        popped_node->lines[i] = hashtable[popped_node->addr1-1+i].curr_line;
                }

                resizeHashTable(hashtable_size-added_lines);
                 popped_node->num_of_el = added_lines;


            }
        }
    }else{  //it's a 'd' command
        //TODO
    }

    //forse dovrei fare che ritorna il nodo poppato
}*/


/*
* Frees the entire redo stack
*/
void free_redo_stack(){
    while(redo_top != NULL){
        struct cmd_node* temp = redo_top;
        redo_top = redo_top->prev;

        if(temp->lines != NULL){
            free(temp->lines);
        }
        
        free(temp);
        redo_stack_size--;
    }

    redo_top = NULL;
    redo_bottom = redo_top;
}


/*
*
*/
struct cmd_node *create_redo_node(int address1, int address2, char cmd){
    //NON CANCELLARE NULLA DI QUELLO CHE È COMMENTATO
    /*struct cmd_node *node = (struct cmd_node*)malloc(sizeof(struct cmd_node));
    node->addr1 = address1;
    node->addr2 = address2;
    node->cmd = cmd;
    node->old_ht_size = hashtable_size;

    if(hashtable_size==0 && hashtable ==NULL){
        node->num_of_el = 0;
        node->lines = NULL;
    }else{
        node->num_of_el = (address2-address1+1);
        node->lines = (char **)malloc((node->num_of_el)*sizeof(char*));

        if(hashtable_size >= node->num_of_el){  //controllare bene se serve
            for(int i=0; i<=node->num_of_el-1; i++){
                //printf("READING FROM HASHTABLE[%d]: %s\n",(address1-1+i),hashtable[address1-1+i].curr_line);
                node->lines[i] = hashtable[address1-1+i].curr_line;
            }
        }
    }    

    node->prev = NULL;
    node->next = NULL;

    return node;*/
    if(cmd == 'c'){
        return create_c_node(address1,address2);
    }
    return create_d_node(address1,address2);
}

/*
*
*/
struct cmd_node *create_undo_node(int address1, int address2, char cmd){
    //NON CANCELLARE NULLA DI QUELLO CHE È COMMENTATO
    /*struct cmd_node *node = (struct cmd_node*)malloc(sizeof(struct cmd_node));
    node->addr1 = address1;
    node->addr2 = address2;
    node->cmd = cmd;
    node->old_ht_size = hashtable_size;

    if(hashtable_size==0 && hashtable ==NULL){
        node->num_of_el = 0;
        node->lines = NULL;
    }else{
        node->num_of_el = (address2-address1+1);
        node->lines = (char **)malloc((node->num_of_el)*sizeof(char*));

        if(hashtable_size >= node->num_of_el){  //controllare bene se serve
            
            for(int i=0; i<=node->num_of_el-1; i++){
                //printf("READING FROM HASHTABLE[%d]: %s\n",(address1-1+i),hashtable[address1-1+i].curr_line);
                node->lines[i] = hashtable[address1-1+i].curr_line;
            }
        }
        //else{
            //CONTROLLARE COSA E FINO A DOVE LEGGERE NEL CASO hashtable_size < node->num_of_el
        //}

        
        //printf("--------------------------------------------\n");
        //for(int i=0; i<=node->num_of_el-1; i++){
            //printf("READING FROM HASHTABLE[%d]: %s\n",(address1-1+i),hashtable[address1-1+i].curr_line);
        //    node->lines[i] = hashtable[address1-1+i].curr_line;
        //}
        //printf("--------------------------------------------\n");
        
    }    

    node->prev = NULL;
    node->next = NULL;

    return node;*/

    if(cmd == 'c'){
        return create_c_node(address1,address2);
    }
    return create_d_node(address1,address2);
}

/*
*
*/
void restore_undo(){     
    if(undo_top->cmd == 'c'){
        if(undo_top->num_of_el == 0 && undo_top->lines == NULL){    //Non si aveva solo aggiunto e la hashtable era vuota
            //fputs("SEI IN 1\n",stdout);
            if(undo_top->old_ht_size > 0){//hashtable esisteva già e ho solo aggiunto righe
                resizeHashTable(undo_top->old_ht_size);
            }else{  //hashtable era vuota
                free(hashtable);
                hashtable = NULL;
                hashtable_size = 0;
            }
            

        }else if(undo_top->num_of_el > 0 && undo_top->lines != NULL){
            if(undo_top->num_of_el == (undo_top->addr2-undo_top->addr1+1) ){    //Avevo solo cambiato righe
                for(int i=0; i<= ((undo_top->num_of_el)-1);i++){
                    hashtable[undo_top->addr1-1+i].curr_line = undo_top->lines[i];
                }

            }else if(undo_top->num_of_el < (undo_top->addr2-undo_top->addr1+1) ){   //Avevo cambiato alcune righe e aggiunto n°righe pari a: ((addr2-addr1+1)-num_of_el)
                //fputs("SEI IN 3\n",stdout);
                resizeHashTable(undo_top->old_ht_size);
                for(int i=0; i<= ((undo_top->num_of_el)-1);i++){
                    hashtable[undo_top->addr1-1+i].curr_line = undo_top->lines[i];
                }
            }
        }
        //VERIFICARE SE CI SONO TUTTI I CASI
    }else if(undo_top->cmd == 'd'){
        if(undo_top->num_of_el != 0 && undo_top->lines != NULL){    //ho eliminato qualcosa, se lines fosse nullo non faccio nulla perchè non avevo cancellato nulla
            int n_of_deletes = (undo_top->addr2-undo_top->addr1+1);
            int prev_ht_size = hashtable_size;

            if(n_of_deletes == undo_top->num_of_el){  
                //printf("UNDO_TOP->OLD_HT_SIZE: %d\n",undo_top->old_ht_size);              
                resizeHashTable(undo_top->old_ht_size);

                if(prev_ht_size > 0){//if(undo_top->old_ht_size > 0){
                    //shift of current elements in hashtable
                    int i = 0;
                    int n_cycles = (hashtable_size - undo_top->addr2);//(hashtable_size - undo_top->addr1);//(undo_top->addr2 - undo_top->addr1); 
                    while(i < n_cycles){   // <= (?)
                        //printf("HT_SIZE: %d\n", hashtable_size);
                        if(hashtable[prev_ht_size-1-i].curr_line == NULL){printf("STAI LEGGENDO NULL !!!!\n");}
                        hashtable[hashtable_size-1-i].curr_line = hashtable[prev_ht_size-1-i].curr_line;
                        i++;
                    }
                }
                //shift of current elements in hashtable
                /*int i = 0;
                int n_cycles = (hashtable_size - popped_node->addr1);
                while(i < n_cycles){   // <= (?)
                    if(hashtable[old_hash_table_size-1-i].curr_line == NULL){printf("STAI LEGGENDO NULL !!!!\n");}
                    hashtable[hashtable_size-1-i].curr_line = hashtable[old_hash_table_size-1-i].curr_line;
                    i++;
                }*/

                //insert of old lines
                for(int k = 0; k <= (undo_top->num_of_el-1); k++){ //(undo_top->addr2-1) -->mettere addr2 è sbagliato
                    hashtable[undo_top->addr1-1+k].curr_line = undo_top->lines[k];
                }
                
            }else if(n_of_deletes > undo_top->num_of_el){
                resizeHashTable(undo_top->old_ht_size);

                //rimetto le vecchie linee in fondo come erano prima della delete
                int i = 0;
                while(i < undo_top->num_of_el){   // <= (?)
                    hashtable[undo_top->addr1-1+i].curr_line = undo_top->lines[i];
                    i++;
                }
            }
        }
    }//end restore 'd'
}

void restore_redo(){
    if(redo_top->cmd == 'c'){
        if(hashtable == NULL && hashtable_size == 0){  //HT adesso è vuota
            resizeHashTable(redo_top->old_ht_size);
            for(int i=0; i<= ((redo_top->num_of_el)-1);i++){
                hashtable[redo_top->addr1-1+i].curr_line = redo_top->lines[i];
            }

        }else if(hashtable != NULL && hashtable_size > 0){  //HT adesso NON È vuota
            if(redo_top->old_ht_size == hashtable_size){    //Ho solo cambiato delle righe senza aggiungerne
                
                for(int i=0; i<= ((redo_top->num_of_el)-1);i++){
                    hashtable[redo_top->addr1-1+i].curr_line = redo_top->lines[i];
                }

            }else if(redo_top->old_ht_size > hashtable_size){

                resizeHashTable(redo_top->old_ht_size);
                for(int i=0; i<= ((redo_top->num_of_el)-1);i++){
                    hashtable[redo_top->addr1-1+i].curr_line = redo_top->lines[i];
                }

            }

        }

        //VERIFICARE SE CI SONO TUTTI I CASI

    }else if(redo_top->cmd == 'd'){
        deleteElements(redo_top->addr1,redo_top->addr2);
        /*if(redo_top->num_of_el != 0 && redo_top->lines != NULL){    //ho eliminato qualcosa, se lines fosse nullo non faccio nulla perchè non avevo cancellato nulla
            int n_of_deletes = (redo_top->addr2-redo_top->addr1+1);
            int prev_ht_size = hashtable_size;

            if(n_of_deletes == redo_top->num_of_el){  
                //printf("UNDO_TOP->OLD_HT_SIZE: %d\n",undo_top->old_ht_size);              
                resizeHashTable(redo_top->old_ht_size);

                if(prev_ht_size > 0){//if(undo_top->old_ht_size > 0){
                    //shift of current elements in hashtable
                    int i = 0;
                    int n_cycles = (hashtable_size - redo_top->addr2);//(hashtable_size - undo_top->addr1);//(undo_top->addr2 - undo_top->addr1); 
                    while(i < n_cycles){   // <= (?)
                        //printf("HT_SIZE: %d\n", hashtable_size);
                        if(hashtable[prev_ht_size-1-i].curr_line == NULL){printf("STAI LEGGENDO NULL !!!!\n");}
                        hashtable[hashtable_size-1-i].curr_line = hashtable[prev_ht_size-1-i].curr_line;
                        i++;
                    }
                }
                //shift of current elements in hashtable
                //int i = 0;
                //int n_cycles = (hashtable_size - popped_node->addr1);
                //while(i < n_cycles){   // <= (?)
                //    if(hashtable[old_hash_table_size-1-i].curr_line == NULL){printf("STAI LEGGENDO NULL !!!!\n");}
                //    hashtable[hashtable_size-1-i].curr_line = hashtable[old_hash_table_size-1-i].curr_line;
                //    i++;
                //}

                //insert of old lines
                for(int k = 0; k <= (redo_top->num_of_el-1); k++){ //(undo_top->addr2-1) -->mettere addr2 è sbagliato
                    hashtable[redo_top->addr1-1+k].curr_line = redo_top->lines[k];
                }
                
            }else if(n_of_deletes > redo_top->num_of_el){
                resizeHashTable(redo_top->old_ht_size);

                //rimetto le vecchie linee in fondo come erano prima della delete
                int i = 0;
                while(i < redo_top->num_of_el){   // <= (?)
                    hashtable[redo_top->addr1-1+i].curr_line = redo_top->lines[i];
                    i++;
                }
            }
        }*/
    } //end restore 'd'
}