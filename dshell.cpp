#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <pwd.h>
#include <string>
#include <signal.h>
#include <list>
#include <algorithm>
#include <stdlib.h>

using namespace std;

int signal_value = 0;

string replaceHome(string cwd);
bool commandAnalyse(string command);
void signal_handler(int sig);
list<string> tokenize(string str);
string getCommandName(string str);
bool knownCommand(string command);

string eval_word(string word);
string eval_path(string path);

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
        
        signal(SIGINT, signal_handler);
        // activation of control+c
        if (signal_value == 2) 
        {
            signal_value = 0;
            continue;
        }
        string comm;
        getline(cin, comm);
        if (comm.empty()) comm = "";
        if (!commandAnalyse(comm))
        {
            return 0;
        }

    }
}

// Evaluate a single word and eliminate environment variables. 
// Send only one word each use!!
string eval_word(string word)
{
    string to_return = "";
    string env_var = "";
    for (unsigned int i = 0; i < word.size(); i++)
    {
        if (word[i] == '$')
        {
            while (i + 1 < word.size() && word[i + 1] == '$') i++;
            while (++i < word.size())
            {
                if (word[i] == '$')
                {
                    const char *cast = env_var.c_str();
                    if (env_var != "") to_return += getenv(cast);
                    env_var = "";
                    break;
                }
                env_var += word[i++];
            }
        }
        else 
        {
            to_return += word[i];
        }
    }
    const char *cast = env_var.c_str();
    if (env_var != "") to_return += getenv(cast);
    return to_return;
}

// Signal handler. Should be called whenever we want to address a user signal
void signal_handler(int sig)
{
   signal_value = sig;
   if (sig == SIGINT)
   {
       cout << endl;
   }
}

// Analysing every command string. This function should be called recursively on every new command string
bool commandAnalyse(string str)
{
    string cmd = getCommandName(str);
    cout << eval_word(cmd) << endl;
    if (!knownCommand(cmd))
    {
        cout << str << ": No such command" << endl;
    }
    else if (str == "exit")
    {
        cout << "You chose to quit shell, good day" << endl;
        return false;
    }
    else if (cmd=="") { }
    else if (cmd == "cd")
    {
	    list<string> tokens = tokenize(str);
        list<string>::iterator it = tokens.begin();
        if(tokens.size()==2)
        {
            string newDir;
            newDir = getcwd(NULL, 0);
            advance(it,1);
            if((*it)[0]=='~')
            {
                string tempStr = (getpwuid(getuid()))->pw_dir;
                tempStr += '/';
                for (unsigned int i = 2; i < (*it).size(); i++)
                {
                    tempStr += (*it)[i];
                }
                newDir = tempStr;
            }
            else
                newDir+="/" + *it;
            if(chdir(newDir.c_str())!=0)
            {
                cout<<"Directory '/"+*it+"' does not exist"<<endl;
            }
        }
        else if(tokens.size()==1){}
        else
            cout<<"Wrong input"<<endl;
    }
    else 
    {
        list<string> tokens = tokenize(str);
        list<string>::iterator it = tokens.begin();
        for (unsigned int i=0; i < tokens.size(); i++)
        {
            cout << *it << endl;
            advance(it,1);
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
    return *it;
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
































