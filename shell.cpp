#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <pwd.h>
#include <string>
#include <string.h>
#include <signal.h>
#include <list>
#include <algorithm>
#include <stdlib.h>
#include <sys/wait.h>
#include <vector>

using namespace std;

int exit_status = 0;

string replaceHome(string cwd);
bool commandAnalyse(string command);
void signal_handler(int sig);
list<string> tokenize(string str);
string getCommandName(list<string>);
bool knownCommand(string command);

string eval_word(string word);
string eval_dollar(string, unsigned int&);

int main()
{
    cout << "Welcome to OS Shell!" << endl;
    while(true)
    {
        
        if (cin.eof())
        {
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
string eval_word(string str)
{
    string word = "";
    string to_return = "";

    if (str[0] == '~')
    {
        word += (getpwuid(getuid()))->pw_dir;
        word += str.substr(1, str.size());
    }
    else word = str;

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
        if (word[i] == '$' || word[i] == '/')
        {
            if (env_var[0] == '\0') return string(1, word[i]);
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

// Analysing every command string. This function should be called recursively on every new command string
bool commandAnalyse(string str)
{
    list<string> tokens = tokenize(str);
    string cmd = getCommandName(tokens);
    
    if (cmd == "exit")
    {
        cout << "You chose to quit shell, good day" << endl;
        return false;
    }
    else if (cmd=="") { }
    else if (cmd == "cd")
    {
        list<string>::iterator it = tokens.begin();
        if(tokens.size() == 1) 
        {
            string homedir = (getpwuid(getuid()))->pw_dir;
            exit_status = chdir(homedir.c_str());
        }
        else if (tokens.size() == 2)
        {
            advance(it,1);
            //string path = eval_path(*it);
            string path = eval_word(*it);
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
        list<string>::iterator it = tokens.begin();
        char** args = new char*[tokens.size() + 1];
        string word;
        for (unsigned int i = 0; i < tokens.size(); i++)
        {
            word = eval_word((string)*it);
            args[i]=new char[word.size()+1];
            strcpy(args[i],(char*)word.c_str());
            advance(it,1);
        }
        args[tokens.size()] = NULL;
        
        int i=tokens.size();
        int flag=0;
        if (strcmp(args[i-1],"&")==0) {
            flag = 1;
            args[i-1] = NULL;
        }
        int pid = fork(); //forking the process
        if (pid<0) {
            perror("fork");
            exit(1);
        }
        //child process
        if (pid==0)
        { 
            if(execvp(cmd.c_str(), args) < 0)//executing, if there is an error exiting with killing the child process
            {
                    perror("executing error");
                    exit_status=127;
                    cout<<"exit_status= "<<exit_status<<endl;
            }      
        }
        //parent process  
        else 
        {
            exit_status=0;
            if (flag==0) // if not zombie
            { 
                if (waitpid(pid, &exit_status,0)==-1){perror("waitError");} //reaping zombies, wait for the child to end
                //zpid = waitpid(-1, &exit_status,0);
            }
            else 
            { //if zoombie
                cout << "[" << pid << "]" << endl;
            }
        }
        pid = waitpid(-1, &exit_status, WNOHANG);

		if (pid > 0)
        {
            exit_status = WTERMSIG(exit_status) + 128;
			cout << '[' << pid << "] : exited, status=" << exit_status << endl;
        }
		
    }
    return true;
}

// Receives a string of user input and returns it's first value which is the command name
string getCommandName(list<string> tokens)
{
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
    if (newstr != "") tokens.push_back(newstr);

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






























