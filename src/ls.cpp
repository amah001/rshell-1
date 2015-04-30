#include<iostream>
#include<string>
#include<unistd.h>	//fork(), execvp(), etc
#include<sys/wait.h>	//wait calls
#include<stdio.h>	//perror
#include<errno.h>
#include<dirent.h>	//Directory Syscalls
#include<sys/stat.h> 
#include<sys/types.h>
#include<signal.h>	//signal(),kill(), etc
#include<pwd.h>		//pwd()
#include<sys/ioctl.h>	//ioctl()
#include<stdlib.h>
#include<cstring>
#include<string.h>
#include<vector>
#include<queue>
#include<fcntl.h>
#include<errno.h>
#include<algorithm>
#include<iomanip>
#include<pwd.h>
#include<grp.h>
using namespace std;


void printPerm(const string & file)
{
	struct group  *group;
	struct passwd *passwd;
	struct stat s;
	cerr << file << endl;
	if( ( stat(file.c_str(), &s ) )==-1 )
	{
		perror("stat()");
		exit(1);
	}	

	if(S_ISREG(s.st_mode)) cout << "-";
	else if(S_ISDIR(s.st_mode)) cout << "d";
	else if(S_ISCHR(s.st_mode)) cout << "c";
	else if(S_ISBLK(s.st_mode)) cout << "b";
	else if(S_ISLNK(s.st_mode)) cout << "l";
	else if(S_ISSOCK(s.st_mode)) cout << "s";

	cout << ((s.st_mode & S_IRUSR) ? "r" : "-");					
	cout << ((s.st_mode & S_IWUSR) ? "w" : "-");
	cout << ((s.st_mode & S_IXUSR) ? "x" : "-");
	cout << ((s.st_mode & S_IRGRP) ? "r" : "-");					
	cout << ((s.st_mode & S_IWGRP) ? "w" : "-");
	cout << ((s.st_mode & S_IXGRP) ? "x" : "-");
	cout << ((s.st_mode & S_IROTH) ? "r" : "-");					
	cout << ((s.st_mode & S_IWOTH) ? "w" : "-");
	cout << ((s.st_mode & S_IXOTH) ? "x" : "-");
	//===Number of Hard Links==============
	cout << setw(2) << s.st_nlink << " ";

	group = getgrgid(s.st_gid); 
	if(group==NULL) perror("getgrid()");
	passwd = getpwuid(s.st_uid);
	if(passwd==NULL) perror("getpwuid()");
	//===TIME===============================
	//string time;
	//time = ctime(&s.st_mtime);
	time_t t = s.st_mtime;
	struct tm lt;
	localtime_r(&t, &lt);
	char time[80];
	strftime(time,sizeof(time), "%c", &lt);
	//===OUTPUT=============================
	cout << setw(8) << passwd->pw_name;
	cout << setw(8) << group->gr_name;
	cout << setw(8) << s.st_size << " ";
	cout << setw(20) << time << " ";
	cout << file << endl;
}

void printFiles(const char* directory, bool dashA, bool dashL, bool dashR)
{
	vector<string>files;
	vector<string>dir;

	if(!dashR)// -a or no flags
	{
		DIR *dirp;
		if(NULL == (dirp = opendir(directory)))
		{
			perror("There was an error with opendir(). ");
			exit(1);
		}
				
		struct dirent *filespecs;
		errno = 0;
		while(NULL != (filespecs = readdir(dirp)))
		{
			if(!dashA){
				if(filespecs->d_name[0]!='.') files.push_back(filespecs->d_name);
				else continue;
			}
			else files.push_back(filespecs->d_name);
		}
		if(errno != 0)
		{
			perror("There was an error with readdir(). ");
			exit(1);
		}
		sort(files.begin(), files.end());
		for(unsigned int i = 0; i < files.size(); i++)
		{
			if(!dashL) cout << files.at(i) << "  ";
			else if(dashL) printPerm(files.at(i));//cout << "THIS IS -l:" << files.at(i) << endl; //printPerm(files.at(i));
		}
		if(-1 == closedir(dirp))
		{
			perror("There was an error with closedir(). ");
			exit(1);
		}
		cout << endl;
	}
	else
	{
		cout << "Flag Not Supported Yet" << endl;
	}

}

int main(int argc, char** argv)
{
	vector<string>files;
	vector<string>directory;
	vector<string>userIN;

	bool dashA = false;
	bool dashL = false;
	bool dashR = false;

	if(argc < 1)
	{
		cout << "No Arguments Passed" << endl;
		exit(1);
	}
	else
	{
		//Takes all user input, but drops all flags 
		for(int i = 1; i < argc; i++)
		{
			if(argv[i][0]!=' ' && argv[i][0]!='-') userIN.push_back(argv[i]); //Non flag
			else if(argv[i][0]!=' ' && argv[i][0]=='-') //Possible Flags
			{
				for(int j = 1; argv[i][j]!='\0'; j++){
					//cout << "ARG: " << argv[i][j] << endl;
					if(argv[i][j]=='a') dashA = true;
					else if(argv[i][j]=='l') dashL = true;
					else if(argv[i][j]=='R') dashR = true;
					else{
						cerr << "Invalid Flag" << endl;
						exit(1);
					}
				}
			}
			else{}
		}
		if(userIN.size() > 0){	
			for(unsigned int i = 0; i < userIN.size(); i++)
			{
				struct stat s;
				if( ( stat(userIN.at(i).c_str(), &s ) )==-1 )
				{
					perror("stat()");
					exit(1);
				}	
				if(S_ISREG(s.st_mode)) files.push_back(userIN.at(i));//printPerm(userIN.at(i).c_str());
				//else if(S_ISREG(s.st_mode) && !dashL) cout << userIN.at(i) << " ";
				else if(S_ISDIR(s.st_mode))
				{
					//cerr << "HERE";
					if(userIN.at(i).at(userIN.at(i).size()-1)!='/')
					{	
						//cerr << "GOT HERE";
						userIN.at(i) = userIN.at(i) + '/'; 
						//directory.push_back(userIN.at(i));
					}
					directory.push_back(userIN.at(i));
				}
			}
			//For Files
			if(files.size()>0)
			{	
				int maxWidth = 72;;
				int currWidth = 0;
				for(unsigned int i = 0; i < files.size(); i++)
				{
					//if(files.size()==0 || files.empty()) break;
					if(currWidth==maxWidth)
					{
						currWidth = 0;
						cout << endl;
					}
					currWidth += files.at(i).size() + 1;
					cout << files.at(i) << " ";
				}
				cout << endl;
			}
			if(directory.size()>0)
			{	
				//For Directories;
				for(unsigned int i = 0; i < directory.size(); i++)
				{
					cout << directory.at(i) << endl;
					printFiles(directory.at(i).c_str(), dashA, dashL, dashR);
				}
			}

			
		}
		else{//Ouput files in the current directory
			printFiles(".", dashA, dashL, dashR);
		}
		//cout << endl;	
	}

	return 0;
}


/*
	else if(dashL && !dashR)//-l -a or -l
	{
		DIR *dirp;
		if(NULL == (dirp = opendir(directory)))
		{
			perror("There was an error with opendir(). ");
			exit(1);
		}
				
		struct dirent *filespecs;
		errno = 0;
		while(NULL != (filespecs = readdir(dirp)))
		{
			if(!dashA){
				if(filespecs->d_name[0]!='.') files.push_back(filespecs->d_name);
				else continue;
			}
			else files.push_back(filespecs->d_name);
		}
		if(errno != 0)
		{
			perror("There was an error with readdir(). ");
			exit(1);
		}
		if(-1 == closedir(dirp))
		{
			perror("There was an error with closedir(). ");
			exit(1);
		}
		sort(files.begin(), files.end());
		for(unsigned int i = 0; i < files.size(); i++)
		{
			//cerr << files.at(i);
			printPerm(files.at(i));
			cout << endl;
		}
	}
	
	else if(!dashA && !dashL && !dashR)//No Flag
	{
		DIR *dirp;
		if(NULL == (dirp = opendir(directory)))
		{
			perror("There was an error with opendir(). ");
			exit(1);
		}
				
		struct dirent *filespecs;
		errno = 0;
		while(NULL != (filespecs = readdir(dirp)))
		{
			if(filespecs->d_name[0]!='.') files.push_back(filespecs->d_name);
			else continue;
		}
		if(errno != 0)
		{
			perror("There was an error with readdir(). ");
			exit(1);
		}
		if(-1 == closedir(dirp))
		{
			perror("There was an error with closedir(). ");
			exit(1);
		}
		sort(files.begin(), files.end());
		for(unsigned int i = 0; i < files.size(); i++)
		{
			cout << files.at(i) << " ";
		}
		cout << endl;

	}*/