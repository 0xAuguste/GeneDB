//GeneDB.cpp
//Prompts for file name, performs requested functions pertaining to genes
//written by Auguste Brown

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

const string QUIT = "q";
const string PRINT = "p";
const string CAN_MUTATE = "cm";
const string CAN_EVOLVE = "ce";
const string COMMON_SEQUENCE = "fcs";
const string PRINT_PATHS = "pa";
const string CYCLICALLY_MUTATE = "ccm";

struct Gene;

//the Mutation struct holds a pointer to the gene it is mutating to, and
//the cost of the mutation
struct Mutation{
    Gene *gene;
    int cost;
};

//the Gene struct holds the code of the gene, an array of its possible mutations,
//and the number of mutations it can perform
struct Gene{
    string code;
    Mutation mutations[5];
    int num_mutations;
};

//////////////////////////////Function Declarations////////////////////////////
void make_genes(int *num_genes, Gene all_genes[], string filename);
void make_mutations(int *num_genes, Gene all_genes[], string filename);
Gene *find_gene(string code, Gene genes[], int num_genes);
string to_lower(string str);
int get_num_genes(string filename);
void operations(int num_genes, Gene genes[]);
void print_database(int num_genes, Gene genes[]);
void find_common_sequence(int num_genes, Gene genes[]);
void cm(Gene source, Gene target, int max_steps);
bool can_mutate(Gene source, Gene target, int max_steps, Gene checked_genes[],
                int num_checked);
void ce(Gene source, Gene target, int max_points);
bool can_evolve(Gene source, Gene target, int max_steps, Gene checked_genes[],
                 int num_checked);
void ccm(Gene source, int max_steps);
void pa(Gene source, Gene target, int max_steps);
void print_paths(Gene source, Gene target, int max_steps,
                  string path[], int index);
///////////////////////////////////////////////////////////////////////////////

int main(){
    int *num_genes = new int;
    string filename;

    cout << "Enter data file name: ";
    cin >> filename;

    *num_genes = get_num_genes(filename); //store the number of genes
    Gene *all_genes = new Gene[*num_genes]; //main data storage to hold Genes

    make_genes(num_genes, all_genes, filename); //create genes
    make_mutations(num_genes, all_genes, filename); //create mutations

    operations(*num_genes, all_genes); //take user input and perform functions

    //recycle memory
    delete num_genes;
    num_genes = nullptr;
    delete [] all_genes;
    all_genes = nullptr;
    return 0;
}

// void make_genes(int *num_genes, Gene all_genes[], string filename)
// Purpose: read in the gene data from the chosen file
// Parameters: int *num_genes, Gene all_genes[], string filename
// Returns: None
// Effects: Adds Genes to the all_genes[] array
void make_genes(int *num_genes, Gene all_genes[], string filename){
    ifstream infile;
    infile.open(filename);

    if (not infile.is_open()) {
        cout << "Error opening" << endl;
        return;
    }

    string temp;
    string line;
    infile >> temp; //skip first line of file
    getline(infile, temp); //skip newline character

    for (int row = 0; row < *num_genes; row++){
        getline(infile, line); //each line has a unique gene
        Gene new_gene;
        new_gene.code = line.substr(0,4); //extract the code of the gene
        all_genes[row] = new_gene; //add gene to database array
    }

    infile.close();
}

// void make_mutations(int *num_genes, Gene all_genes[], string filename)
// Purpose: read in the mutation data from the chosen file
// Parameters: int *num_genes, Gene all_genes[], string filename
// Returns: None
// Effects: Adds appropriate Mutations to the mutations[] array in each Gene
void make_mutations(int *num_genes, Gene all_genes[], string filename){
    ifstream infile;
    infile.open(filename);
    if (not infile.is_open()) {
        return;
    }

    string temp;
    string line;
    infile >> temp; //skip first line of file
    getline(infile, temp); //skip newline character

    for (int i = 0; i < *num_genes; i++){
        getline(infile, line); //Go to gene
        string mutations = line.substr(4, line.length()); //access mutations
        stringstream ss = stringstream(mutations); //convert to stringstream

        all_genes[i].num_mutations = 0;
        for (int m = 0; m < int(mutations.length()) / 6; m++){
            //each mutation is roughly 6 characters long
            string code;
            int cost;
            ss >> code >> cost;
            
            Mutation new_mutation; //create the mutation and assign attributes
            new_mutation.gene = find_gene(code, all_genes, *num_genes);
            new_mutation.cost = cost;

            all_genes[i].mutations[m] = new_mutation;//put mutation in database
            all_genes[i].num_mutations = m + 1;
        }
    }
}

// int get_num_genes(string filename)
// Purpose: reads in the first line of the input file and returns the value
// Parameters: string filename
// Returns: integer number of genes in the input file
int get_num_genes(string filename){
    ifstream infile;
    infile.open(filename);
    if (not infile.is_open()) {;
        return 0;
    }
    int num_genes;
    infile >> num_genes; //first line of file is the number of genes

    return num_genes;
}

// void operations(int num_genes, Gene genes[])
// Purpose: prompts user for input and performs the requested function, repeats
// Parameters: int num_genes, Gene genes[]
// Returns: None
void operations(int num_genes, Gene genes[]){
    string command, source, target;
    int max;

    cout << "Enter a command: ";
    cin >> command;
    command = to_lower(command); //adjust for case inclusivity

    if (command == QUIT) //exit recursion
        return;
    else if (command == PRINT) //print database
        print_database(num_genes, genes); //run print operation
    else if (command == CAN_MUTATE){
        cin >> source >> target >> max; //assign command variables
        cm(*find_gene(source, genes, num_genes), //run can_mutate operation
           *find_gene(target, genes, num_genes), max);
    }else if (command == COMMON_SEQUENCE){
        find_common_sequence(num_genes, genes); //run common sequence operation
    }else if (command == PRINT_PATHS){
        cin >> source >> target >> max; //assign command variables
        pa(*find_gene(source, genes, num_genes), //run print_paths operation
           *find_gene(target, genes, num_genes), max);
    }else if (command == CAN_EVOLVE){
        cin >> source >> target >> max; //assign command variables
        ce(*find_gene(source, genes, num_genes), //run can_evolve operation
           *find_gene(target, genes, num_genes), max);
    }else if (command == CYCLICALLY_MUTATE){
        cin >> source >> max; //assign command variables
        ccm(*find_gene(source, genes, num_genes), max);
    }
    operations(num_genes, genes); //run function again to get another command
}

// Gene *find_gene(string code, Gene genes[], int num_genes)
// Purpose: returns Gene whose code matches the input
// Parameters: string code, Gene genes[], int num_genes
// Returns: appropriate Gene
Gene *find_gene(string code, Gene genes[], int num_genes){
    for (int i = 0; i < num_genes; i++){
        if (genes[i].code == code) //if the gene has the correct code
            return &genes[i];
    }

    return nullptr;
}

// string to_lower(string str)
// Purpose: returns a lowercase version of input string str
// Parameters: string str
// Returns: lowercase string
string to_lower(string str){
    string temp = "";

    for (int i = 0; i < int(str.length()); i++){
        temp += tolower(str[i]);
    }

    return temp;
}

// void print_database(int num_genes, Gene genes[])
// Purpose: loops through all_genes array. Prints the genes and
//          their respective mutations
// Parameters: int num_genes, Gene genes[]
// Returns: None
void print_database(int num_genes, Gene genes[]){
    for (int i = 0; i < num_genes; i++){
        cout << "== " << genes[i].code << " ==" << endl
             << "Mutations: " << endl;
        for (int m = 0; m < genes[i].num_mutations; m++){
            cout << genes[i].mutations[m].gene->code << " - "
                 << "Cost: " << genes[i].mutations[m].cost << endl;
        }

        if (genes[i].num_mutations == 0)
            cout << "None" << endl;
    }

    cout << endl;
}

// void cm(Gene source, Gene target, int max_steps)
// Purpose: Runs output functionality for the can_mutate operation.
//          Allocates memory for checked_genes array
// Parameters: Gene source, Gene target, int max_steps
// Returns: None
void cm(Gene source, Gene target, int max_steps){
    //this array will hold the genes that can_muatate has already checked
    //in order to increase efficiency
    Gene *checked_genes = new Gene[100];

    if (can_mutate(source, target, max_steps, checked_genes, 0)){
        cout << source.code << " can mutate into " << target.code << " in "
             << max_steps << " steps or less." << endl;
    }
    else{
        cout << source.code << " cannot mutate into " << target.code << " in "
             << max_steps << " steps or less." << endl;
    }

    cout << endl;

    delete [] checked_genes;
    checked_genes = nullptr;
}

// bool can_mutate(Gene source, Gene target, int max_steps, Gene checked_genes[],
//                 int num_checked)
// Purpose: Runs logic for can_mutate operation
// Parameters: Gene source, Gene target, int max_steps, Gene checked_genes[],
//             int num_checked
// Returns: true if source can mutate into target in max steps, false otherwise
// Notes: recursive
bool can_mutate(Gene source, Gene target, int max_steps, Gene checked_genes[],
                 int num_checked){
    if (max_steps <= 0) //base case
        return false;

    for (int i = 0; i < source.num_mutations; i++){
        if (source.mutations[i].gene->code == target.code){
            //source gene has been found
            return true;
        }
    }

    for (int i = 0; i < num_checked; i++){
        if (checked_genes[i].code == source.code)
            //source gene has already been checked
            return false;
    }

    checked_genes[num_checked] = source; //mark currnet gene as checked
    num_checked++;

    //recursive case
    bool possible_path = false;
    for (int i = 0; i < source.num_mutations; i++){
        if (can_mutate(*(source.mutations[i].gene), target, max_steps-1,
            checked_genes, num_checked))
            possible_path = true;
    }
    
    return possible_path;
}

// void ce(Gene source, Gene target, int max_points)
// Purpose: Runs output functionality for the can_evolve operation.
//          Allocates memory for checked_genes array
// Parameters: Gene source, Gene target, int max_steps
// Returns: None
void ce(Gene source, Gene target, int max_points){
    Gene *checked_genes = new Gene[100]; //same as cm()

    if (can_mutate(source, target, 25, checked_genes, 0)){
        
        if (can_evolve(source, target, max_points, checked_genes, 0)){
            cout << source.code << " can mutate into " << target.code
                 << " with <= " << max_points << " evolutionary cost." << endl;
        }
        else{
            cout << source.code << " cannot mutate into " << target.code
                 << " with only " << max_points
                 << " evolutionary cost." << endl;
        }
    }

    else{
        cout << "It is not possible for " << source.code << " to mutate into "
             << target.code << "." << endl;
    }

    cout << endl;

    delete [] checked_genes;
    checked_genes = nullptr;
}

// bool can_evolve(Gene source, Gene target, int max_points, Gene checked_genes[],
//                 int num_checked)
// Purpose: Runs logic for can_evolve operation
// Parameters: Gene source, Gene target, int max_points, Gene checked_genes[],
//             int num_checked
// Returns: true if source can mutate into target in max points, false otherwise
// Notes: recursive
bool can_evolve(Gene source, Gene target, int max_points, Gene checked_genes[],
                 int num_checked){

    for (int i = 0; i < source.num_mutations; i++){
        if (source.mutations[i].gene->code == target.code){
            if (source.mutations[i].cost > max_points)
                return false;
            return true;
        }
    }

    for (int i = 0; i < num_checked; i++){
        if (checked_genes[i].code == source.code)
            return false;
    }

    checked_genes[num_checked] = source;
    num_checked++;

    bool possible_path = false;
    for (int i = 0; i < source.num_mutations; i++){
        if (can_evolve(*(source.mutations[i].gene), target,
            max_points - source.mutations[i].cost, checked_genes, num_checked))
            possible_path = true;
    }
    
    return possible_path;
}

// void find_common_sequence(int num_genes, Gene genes[])
// Purpose: Finds all common sequences in database and prints their codes
// Parameters: int num_genes, Gene genes[]
// Returns: None
void find_common_sequence(int num_genes, Gene genes[]){
    Gene *checked_genes = new Gene[100]; //same as cm()

    for (int i = 0; i < num_genes; i++){
        bool common = true;
        for (int m = 0; m < num_genes; m++){
            if (m != i and !can_mutate(genes[i], genes[m], 10,checked_genes,0))
                common = false;
        }

        if (common)
            cout << genes[i].code << " is a common sequence.\n";
    }

    cout << endl;

    delete [] checked_genes;
    checked_genes = nullptr;
}

// void ccm(Gene source, int max_steps)
// Purpose: Runs output functionality for can cyclically mutate operation,
//          using can_mutate function
// Parameters: Gene source, int max_steps
// Returns: None
void ccm(Gene source, int max_steps){
    Gene *checked_genes = new Gene[100]; //same as cm()

    //source and target gene are the same
    if (can_mutate(source, source, max_steps, checked_genes, 0)){
        cout << source.code << " can perform cyclic mutation in "
             << max_steps << " steps or less." << endl;
    }
    else{
        cout << source.code << " cannot perform cyclic mutation in "
             << max_steps << " steps or less." << endl;
    }

    cout << endl;

    delete [] checked_genes;
    checked_genes = nullptr;
}

// void pa(Gene source, Gene target, int max_steps)
// Purpose: -Runs some output functionality for can print all paths operation.
//          -Checks if mutation is possible
//          -Allocates space for path[] array
// Parameters: Gene source, Gene target, int max_steps
// Returns: None
void pa(Gene source, Gene target, int max_steps){
    Gene *checked_genes = new Gene[100]; //same as cm()

    if (!can_mutate(source, target, max_steps, checked_genes, 0)){
        cout << "There are no mutation paths from " << source.code
             << " to " << target.code << "." << endl;
        
        delete [] checked_genes;
        checked_genes = nullptr;
    }

    else{
        string *path = new string[100];
        print_paths(source, target, max_steps, path, 0);

        delete [] path;
        path = nullptr;
    }

    cout << endl;
}

// void print_paths(Gene source, Gene target, int max_steps,
//                  string path[], int index)
// Purpose: Prints all possible paths from source to target
// Parameters: Gene source, Gene target, int max_steps,
//             string path[], int index
// Returns: None
void print_paths(Gene source, Gene target, int max_steps,
                  string path[], int index){
    for (int i = 0; i <= index; i++){
        if (source.code == path[i]){ //source gene has already been checked
            return;
        }
    }
    path[index] = source.code; //add current gene to path

    if (source.code == target.code){ //if target gene has been reached
        for (int i = 0; i < index; i++){
            cout << path[i] << " --> ";
        }
        cout << target.code << endl;

        path[index] = "";//make sure program doesn't see target gene is checked
        return;
    }

    for (int i = 0; i < source.num_mutations; i++){ //recursive case
        Gene *checked_genes = new Gene[100];
        if (can_mutate(*(source.mutations[i].gene), target, max_steps - 1,
            checked_genes, 0) or source.mutations[i].gene->code == target.code){
            
            print_paths(*(source.mutations[i].gene), target, max_steps - 1,
                        path, index + 1);
        }

        delete [] checked_genes;
        checked_genes = nullptr;
    }
}
/*
  O
 o-O
O---o
 O-o
  O
 o-O
o---O
O---o
 O-o
  O
 o-O
o---O
O---o
 O-o
  O 
*/
