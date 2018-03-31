#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <pwd.h>
#include <string>
#include <signal.h>
#include <list>
#include <algorithm>
#include <stdlib.h>
#include <sys/wait.h>

using namespace std;

int exit_status = 0;

string replaceHome(string cwd);
bool commandAnalyse(string command);
void signal_handler(int sig);
list<string> tokenize(string str);
string getCommandName(string str);
bool knownCommand(string command);

string eval_word(string word);
string eval_dollar(string, unsigned int&);
string eval_path(string);

int main()
{
    cout << "Welcome to OS Shell!" << endl;
    while(true)
    {
        
        if (cin.eof())
        {
            cout << "Reached End of File. Quitting" << endl;
            return 0;
        }
        string cwd = getcwd(NULL, 0);
        
        cwd = replaceHome(cwd);

        cout << "OS Shell: " << cwd << " >> " << flush;
        
        string comm;
        getline(cin, comm);
        if (comm.empty()) comm = "";
        if (!commandAnalyse(comm))
        {
            return 0;
        }

    }
}

// Evaluate a single word (and path) and eliminate environment variables. 
// Send only one word each use!!
string eval_word(string word)
{
    string to_return = "";

    for (unsigned int i = 0; i < word.size(); i++)
    {
        if (word[i] == '$')
            to_return += eval_dollar(word.substr(i + 1, word.size()), i);
        else to_return += word[i];
    }

    return to_return;
}

// Utility function. Do not use it
string eval_dollar(string word, unsigned int &index)
{
    string env_var = "";
    
    if (word[0] == '?')
    {
        index++;
        return to_string(exit_status);
    }

    for (unsigned int i = 0; i < word.size(); i++)
    {
        if (word[i] == '$')
        {
            if (env_var[0] == '\0') return "$";
            else
            {
                index += env_var.size();
                const char *cast = env_var.c_str();
                const char *to_return = getenv(cast);
                if (!to_return) return "";
                return string(to_return);
            }
        }
        else env_var += word[i];
    }
    if (env_var[0] == '\0') return "$";
    else
    {
        index += env_var.size();
        const char *cast = env_var.c_str();
        const char *to_return = getenv(cast);
        if (!to_return) return "";
        return string(to_return);
    }
}

// Send every path here to evaluate it and replace all variables (if exist)
// and replace home directory with ~ if necessary
string eval_path(string path)
{
    string to_return = "";
    if (path[0] == '~') 
    {
        to_return += (getpwuid(getuid()))->pw_dir;
        to_return += eval_word(path.substr(1, path.size()));
    }
    else to_return += eval_word(path);

    return to_return;
}

// Analysing every command string. This function should be called recursively on every new command string
bool commandAnalyse(string str)
{
    string cmd = getCommandName(str);
   
    /*if (!knownCommand(cmd))
    {
        exit_status = 127;
        cout << cmd << ": No such command" << endl;
    }*/
    if (cmd == "exit")
    {
        cout << "You chose to quit shell, good day" << endl;
        return false;
    }
    else if (cmd=="") { }
    else if (cmd == "cd")
    {
        list<string> tokens = tokenize(str);
        list<string>::iterator it = tokens.begin();
        if(tokens.size() == 1) 
        {
            string homedir = (getpwuid(getuid()))->pw_dir;
            exit_status = chdir(homedir.c_str());
        }
        else if (tokens.size() == 2)
        {
            advance(it,1);
            string path = eval_path(*it);
            exit_status = chdir(path.c_str());
            if (exit_status != 0)
            {
                exit_status = 1;
                cout << "Directory " + path + " could not be located" << endl;
            }
        }
        else 
        {
            exit_status = 1;
            cout << "cd: Too many arguments" << endl;
        }
    }
    else
    {
        pid_t kidpid = fork();
        if (kidpid == 0)
        {
            char *args[2];
            string arg = "-la";
            args[0] = (char*)arg.c_str();
            args[1] = NULL;
            execvp("ls", args);
        }
        else if (kidpid > 0)
        {
            if (waitpid(kidpid, 0, 0) < 0)
            {
            }
        }
    }
    return true;
}

// Receives a string of user input and returns it's first value which is the command name
string getCommandName(string str)
{
    list<string> tokens = tokenize(str);
    list<string>::iterator it = tokens.begin();
    if (tokens.size() == 0) return "";
    return eval_word(string(*it));
}

// Returns true if a command is known and can be used, else returns false
bool knownCommand(string command)
{
    list<string> comm_list( {"", "cd", "exit"} );
    return find(comm_list.begin(), comm_list.end(), command) != comm_list.end();
}

// Receives a user input string and returns a list of strings containing every word with split ' '
list<string> tokenize(string str)
{
    list<string> tokens;
    string newstr = "";

    for (unsigned int i=0; i < str.size(); i++)
    {
        if (str[i] == ' ')
        {
            if (i != 0)
            {
                tokens.push_back(newstr);
                newstr = "";
            }
            while (str[i+1] == ' ') i++;
        }
        else newstr += str[i];
    }
    if (newstr != "" && newstr != " ") tokens.push_back(newstr);

    return tokens;
}

// Receives a current directory string and replaces home directory with ~ if necessary
string replaceHome(string cwd)
{
    string first;

    struct passwd *pw = getpwuid(getuid());
    const string homedir = pw->pw_dir;

    if (cwd.find(homedir) != string::npos)
    {
        first = cwd.substr(0, homedir.size());
        if (first == homedir)
        {
            string newstr = "~";
            for (unsigned int i = homedir.size(); i < cwd.size(); i++)
            {
                newstr += cwd[i];
            }
            cwd = newstr;
        }
    }
    return cwd;
}






























