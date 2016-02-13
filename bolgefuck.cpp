/*
 * Bolgefuck implementation by 1f604 (Since I also wrote the bolgefuck spec, this can be thought of as the "official" bolgefuck interpreter). 
 * Date published: 13 February 2016
 * https://github.com/1f604/bolgefuck
 *
 * Based on migimunz's brainfuck interpreter (2011): https://gist.github.com/migimunz/964338 
 * Also uses jww's code for reading a binary file into a vector (2014): http://stackoverflow.com/a/21802936
 * See sha256.cpp for license on the sha256 code. 
 * The following files are entirely my own work: examples, in, sub
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <vector>
#include <string>
#include <iterator>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <regex>
#include "sha256.h"
using namespace std;
#define TAPE_INITIAL_SIZE 300
#define TAPE_MAX_SIZE 30000 
#define BLANK_SYMBOL 0

//typedefs
typedef uint8_t byte;
typedef int_fast64_t infinite; 
typedef vector<byte> tape_t; 

//structs
struct environment
{
    tape_t tape;
    infinite CP; 
    infinite DP;
    infinite EP; 
    string cryptor;

    environment() : tape(TAPE_INITIAL_SIZE)
    {
        CP = DP = EP = 0;
        cryptor = "no tricks up my sleeve :^)";
        string s(TAPE_MAX_SIZE, 'a');
        cryptor.append(s);
    }
};

//prototypes
void cba2n(environment &env, infinite &p, infinite &X, infinite &Y); 
void read_file(environment &env, char *filename);
void interpret(environment &env);
void read_jump(environment &env);
void encrypt(environment &env);
void print_byte(byte b);

//global variables
int wimpmode(0);

int main(int argc, char** argv)
{
    environment env = environment(); 

    if ( argc < 2 ) {
        cerr << "Usage: " << argv[0] << " <file_name>" << endl;
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
    (b=='\n') ? cout << '\n' : (isprint(b)) ? cout << b : cout << "0x" << hex << (int)b << dec;
}
/*
 * Converts 2 byte arrays to 2 numbers. 
 */
void cba2n(environment &env, infinite &p, infinite &X, infinite &Y){  
    if (env.tape[p] != 'b' && env.tape[p] != 'h'){ 
        (isprint(env.tape[p])) ? cerr << "Invalid number format: " << env.tape[p] << endl : cerr << "Invalid number format: " << "0x" << hex << (int)env.tape[p] << dec << endl;
        exit(1);
    } 
    string s(env.tape.begin()+p,env.tape.end());
    regex r((env.tape[p] == 'b') ? "(b)([+-][01]+)[*]([+-][01]+)[*]" : "(h)([+-][0-9a-fA-F]+)[*]([+-][0-9a-fA-F]+)[*]" , regex_constants::ECMAScript | regex_constants::icase);
    smatch m;
    if (!regex_search(s, m, r)){
        cerr << "Incorrect J syntax" << endl;
        exit(1);
    }

    int base = (m[1] == 'b') ? 2 : 16;

    try {
        X = stoll(m[2], NULL, base);  
        Y = stoll(m[3], NULL, base);  
        if (llabs(X) > TAPE_MAX_SIZE || llabs(Y) > TAPE_MAX_SIZE){
            cerr << "Error: Tried to jump more distance than max tape length\n";
            exit(1);
        }
    }
    catch (const out_of_range& oor) {
        cerr << "Error: Jump parameters larger than max value of long long\n";
        exit(1);
    }
}

void read_file(environment &env, char *filename){

    ifstream file = ifstream(filename, ios::binary);

    if(!file.is_open())
    {
        cerr << "File not found : " << filename << endl;
        exit(1);
    }
    file.unsetf(ios::skipws);
    streampos fileSize;
    file.seekg(0, ios::end);
    fileSize = file.tellg();
    file.seekg(0, ios::beg);
    env.tape.insert(env.tape.begin(),
               istream_iterator<byte>(file),
               istream_iterator<byte>());
}

void read_jump(environment &env){ 
    infinite X, Y;
    infinite p = env.CP + 2;
    cba2n(env, p, X, Y);
    env.CP += (env.tape[env.DP] == env.tape[env.CP+1]) ? X : Y;
}

void encrypt(environment &env){ 
    string s(env.tape.begin(),env.tape.end()); 
    s.append(env.cryptor);
    char hash = sha256(s);
    env.EP++;
    if (env.EP == env.cryptor.size()){ 
        env.EP = 0;
    }
    env.cryptor[env.EP] = hash;
    s.append(env.cryptor); 
    hash = sha256(s);
    env.tape[env.CP] = hash;
}

void interpret(environment &env)
{
    string s(env.tape.begin(),env.tape.begin()+8);
    if (s=="wimpmode"){
        cout << "EXPORT GRADE ENABLED!\n";
        wimpmode = 1;
        env.CP+=8;
        env.DP+=8;
    }
    
    while( true )
    {   

        if (!wimpmode){
            encrypt(env);
        }

        if(env.DP >= TAPE_MAX_SIZE || env.CP >= TAPE_MAX_SIZE) { 
            cerr << "Reached tape limit"<<endl;
            exit(1);
        }
        if(env.DP < 0){
            env.DP = 0;
        }
        if(env.CP < 0){
            env.CP = 0;
        }
        while(env.CP >= env.tape.size()-2 || env.DP >= env.tape.size()-2) { 
            env.tape.push_back(BLANK_SYMBOL);
        }

        switch(env.tape[env.CP])
        {
        /** Single character instructions */ 
        case '>':
            env.DP++;
            env.CP++;
            break;
        case '<':
            env.DP--;
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
            cout << endl; 
            exit(0);
        /** Double character instructions */ 
        case 's': 
            env.tape[env.DP] = env.tape[env.CP+1]; 
            env.CP+=2;
            break;
        /** Variable length instructions */ 
        case 'J': 
            read_jump(env);
            break;
        default:
            if(wimpmode){
                cerr << "Invalid instruction: "; print_byte(env.tape[env.CP]); cerr << endl;
                exit(1);
            }
            encrypt(env);
            break;
        }
    }

}