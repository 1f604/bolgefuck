#include <stdio.h>
#include <vector>
#include <iterator>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <ctype.h>
using namespace std;
#define TAPE_INITIAL_SIZE 300
#define TAPE_MAX_SIZE 30000 //According to the spec this should be infinite, but we live in the real world. Please note vector size may be over double this number depending on C++ compiler implementation
#define BLANK_SYMBOL 0

//typedefs
typedef uint8_t byte;
typedef size_t infinite;
typedef vector<byte> tape_t; //use a vector so we don't have to manually resize the tape. 


//structs
struct environment
{
    tape_t tape;
    infinite CP; //we use index notation instead of iterators for ease of reading, even though iterators are "more" container independent. 
    infinite DP;

    environment() : tape(TAPE_INITIAL_SIZE)
    {
        CP = DP = 0;
    }
};

//prototypes
void read_file(environment &env, char *filename);
void interpret(environment &env);
void read_jump(environment &env);
void print_byte(byte b);

int main(int argc, char** argv)
{

    environment env = environment(); //creates env on the stack. Since the elements of a vector are stored on the heap, growing the tape doesn't use more stack space.  

    if ( argc < 2 ) {
        std::cerr << "Usage: " << argv[0] << " <file_name>" << std::endl;
        exit(1);
    }

    read_file(env,argv[1]);

    interpret(env);

    return 0;
}

/*
 * Prints a tape cell. If it's not printable, outputs hex representation of it.
 */
void print_byte(byte b)
{
    if(isprint(b))
        std::cout << b;
    else
        std::cout << "0x" << std::hex << (int)b << std::dec;
}

/*
 * Reads code from a stream into env.instructions.
 * '\0' are appended on either side, the interpreter just skips them.
 */
void read_file(environment &env, char *filename){

    std::ifstream file = std::ifstream(filename, std::ios::binary);

    if(!file.is_open())
    {
        std::cerr << "File not found : " << filename << std::endl;
        exit(1);
    }

    cout << "vector size is: "<<env.tape.capacity()<<endl;
    /** first read in the file */
    // Stop eating new lines in binary mode!!!
    file.unsetf(std::ios::skipws);
    // get its size:
    std::streampos fileSize;
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    // read the data:
    env.tape.insert(env.tape.begin(),
               std::istream_iterator<byte>(file),
               std::istream_iterator<byte>());
}


void read_jump(environment &env){ //J a X Y means if tape[DP]==a goto X else goto Y. Example: Jkb+1011*b-1* means if tape[DP]=='k' goto DP+11 else goto DP-1. The format is as follows: b or h signifies binary or hex. + or - signifies positive or negative. Followed by the number in binary or hex. Followed by a * to indicate the number has ended. 

    infinite X = 4;
    infinite Y = -1;
    
    env.CP += (env.tape[env.DP] == env.tape[env.CP+1]) ? X : Y;
}


void interpret(environment &env)
{


    /** now start interpreting */
    for (tape_t::iterator i = env.tape.begin(); i != env.tape.end(); ++i)
    std::cout << *i << ' ';

    env.CP = 0;
    cout << "the 1st char is:" << env.tape[env.CP]<<endl;
    cout << "vector size is: "<<env.tape.capacity()<<endl;
    
    while( true )
    {
        switch(env.tape[env.CP])
        {
        /** Single character instructions */ 
        case '>':
            if(env.DP >= TAPE_MAX_SIZE) {
                std::cout << "DP reached tape limit";
                exit(1);
            }
            if(env.DP == env.tape.size()) {
                env.tape.push_back(BLANK_SYMBOL);
            }
            else{
                env.DP++;
            }
            env.CP++;
            break;
        case '<':
            if(env.DP != 0){
                env.DP--;
            }
            env.CP++;
            break;
        case 'p':
            print_byte(env.tape[env.DP]);
            env.CP++;
            break;
        case 'i':
            env.tape[env.DP] = cin.get();
            env.CP++;
            break;
        case 'H':
            cout << endl; //flush the buffer when program finishes. 
            exit(0);
        /** Double character instructions */ 
        case 's':
            if (env.CP+1 == env.tape.size()){
                env.tape[env.DP] = BLANK_SYMBOL;
            }
            else{
                env.tape[env.DP] = env.tape[env.CP+1];
            }
            env.CP+=2;
            break;
        /** Variable length instructions */ 
        case 'J': 
            read_jump(env);
            break;
        }
    }

}