// Whitespace.cpp: Provides the main system for the Whitespace programming language, which is completely illegible.

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <string.h>
#include <strings.h>
#include <vector>
#include <time.h>
#include <sstream>
#include <signal.h>
using namespace std; // Really shouldn't do this...but it had to be done for the *$*#*@! code to compile, so...

//#define DEBUG

#ifdef DEBUG
#define log printf
#define DBG_ONLY
#define START_DBG_ONLY
#define END_DBG_ONLY
#else
#define log /\
/
#define DBG_ONLY log // Since the non-debug version of log just comments out the rest of the line its on, and this happens to be the intended behavior of DBG_ONLY, just use that. It's more concise.
#define START_DBG_ONLY /\
*
#define END_DBG_ONLY */
#endif

#ifndef WIN32
//#define stricmp(a,b) strcasecmp ((a), (b))
#endif

//#define \n \r\n // For Windows compatibility on Cygwin.

void usage (const char* name) throw ()
{
	printf ("Usage: %s filename [-r|--remove]\nIf the -r or --remove flags, are specified, then the interpreter will remove\n    the logfile upon successful program completion.\n", name);
	exit (1);
}

enum level {err, warn, info, debug};

namespace hidden
{
	struct lvconv
	{
		lvconv (const level& lvl) throw ():
			m_lvl (lvl)
		{
		}
		~lvconv () throw ()	{}
		operator string () throw ()
		{
			switch (m_lvl)
			{
			case err:
				return (string)"ERR";
			case warn:
				return (string)"WARN";
			case info:
				return (string)"info";
			case debug:
				return (string)"debug";
			default: // Assume info.
				return (string)"info";
			}
		}
		level m_lvl;
	};
}

std::vector<unsigned char> progmem; // Holds the program's memory.
size_t idx (0); // Holds the index of the current cell in the program's memory.
std::vector<unsigned char> code; // Holds the program's code.
size_t currpos (0); // Holds the index of the instruction that's currently being executed.
std::ofstream logger; // Handles the logfile for the interpreted program.
const char* progfilename; // Holds the name of the interpreter program.

void applog (const char* msg, const level lvl) throw ()
{
	logger<<'['<<time(NULL)<<"]: <"<<string((hidden::lvconv)lvl)<<"> "<<msg<<'\n';
	logger.flush ();
}

void logmemory () throw ()
{
	stringstream converter;
	converter<<"Program memory:";
	for (size_t i=0; i<progmem.size(); ++i)
		converter<<"\n\t\t\tmem["<<i<<"]: "<<(unsigned)progmem[i]<<'.';
	converter<<"\n\t\t\tCurrent index location: "<<idx<<'.';
	applog (converter.str().c_str(), debug);
}

void logcode () throw ()
{
	stringstream converter;
	converter<<"Read instructions:";
	for (size_t i=0; i<code.size(); ++i)
		converter<<"\n\t\t\tcode["<<i<<"]: "<<(unsigned)code[i]<<'.';
	applog (converter.str().c_str(), debug);
}

void reloadCode () throw ()
{
	log ("Reloading code from disk...\n");
	applog ("Reloading code database...", info);
	applog ("Replacing: ", debug);
	logcode ();
	ifstream program (progfilename);
	string command;
	std::streampos start (program.tellg());
	program.seekg (0, ios::end);
	std::streampos end (program.tellg());
	program.seekg (0);
	unsigned char count;
	code.reserve ((end-start)/10); // Reserve enough space for what (I think) should be a good average length for a program, given it's filesize.
	char* str (new char [101]); // Plenty for one number and a short informational string.
	sprintf (str, "Input filesize: %u.", unsigned(end-start));
	applog (str, info);
	delete[] str;
	getline (program, command);
	code.clear ();
	applog ("Rebuilding program code database...", info);
	while (!program.eof()) // Load the program code into the code vector.
	{
		unsigned char count=0;
		for (unsigned i=0; i<command.size() && command[i]==' '; ++i)
		{
			++count;
			if (count==9)
			{
				code.push_back (8);
				count=1;
			}
		}
		if (count)
			code.push_back (count);
		getline (program, command);
	}
	applog ("Done.", info);
	logcode ();
	applog ("Closing input file...", info);
	program.close();
	applog ("Done.", info);
}

const bool exec (unsigned char) throw ();

void loop () throw ()
{
	log ("Looping...\n");
	unsigned num (1);
	log ("Setting up data for range of execution...\n");
	size_t start (currpos+1);
	size_t end (start);
	bool justonce (false); // Use if we run out of instructions before ending the loop.
	for (; num; ++end)
	{
		if (end==code.size())
		{
			justonce=true;
			--end; // So that we can be naive later on.
			break;
		}
		if (code[end]==7)
			++num;
		else if (code[end]==8)
		--num;
	}
	log ("Done.\n");
	if (progmem[idx])
	{	
		log ("Executing loop...\n");
		if (justonce)
		{
			log ("WARNING: No loop end detected. Loop will only execute once.\n");
			applog ("No loop end was detected. Loop shall act as an if conditional for the remaining code in the program.", warn);
		}
		while (progmem [idx])
		{
			log ("Condition: %u\n", progmem [idx]);
			log ("Executing code...\n");
			for (size_t i=start; i<end; ++i)
			{
				char str [101]; // 101 should be plenty.
				sprintf (str, "Executing instruction at location %u: %u.", i, code[i]);
				applog (str, info);
				{
				if (exec (code[i]))
					char str [151]; // 151 should be enough to cover our needs.
					sprintf (str, "Syntax error executing instruction %u, instruction number %u of %u. Program execution: %.2F%% complete.", (unsigned)code[i], (unsigned)i, (unsigned)code.size(), float(100*i)/code.size());
					printf ("%s\n", str);
					applog (str, err);
					exit (100);
				}
				logmemory ();
				currpos=i;
			}
			log ("Done.\n");
			if (idx>=progmem.size())
			{
				applog ("Detected an attempt to loop with a condition beyond the last allocated cell. While this does not affect behavior, the unnecessary burst of null allocations may reduce program performance, especially if this occurs often.", warn);
				while (idx>=progmem.size()) // Sanity check...
					progmem.push_back (0);
			}
			if (justonce)
				exit (0);
		}
		log ("Done.\nLoop condition: %u.\n", progmem[idx]);
	}
	else
		log ("Loop condition null on first attempt.\n");
}

const bool exec (unsigned char cmd) throw ()
{
	log ("Executing command %u...\n", cmd);
	switch (cmd)
	{
	case 1:
		++idx;
		break;
	case 2:
		if (idx)
			--idx;
		else
		{
			applog ("Detected an attempt to leftshift when at the left bound of the array. Attempting to wrap pointer around array...", warn);
			if (progmem.size()) // Ensure that we don't try setting idx to -1 (it is unsigned, after all...that would be bad...). Instead, leave it as zero. (This may not be expected behavior: Left-shifting one byte may leave the cell at the current position (!). At some point, we may want to add an {else idx=1} to prevent this.
				idx=progmem.size()-1;
		}
		break;
	case 3:
		if (idx>=progmem.size())
		{
			if (idx>progmem.size())
			{
				applog ("Detected an attempt to increment a cell more than one cell beyond the last allocated cell. While this does not affect behavior, it may reflect a memory inefficiency.", warn);
				while (idx>progmem.size()) // Sanity check: Ensure that the pointer stays in sync with our memory vector.
					progmem.push_back (0);
			}
			progmem.push_back (1);
		}
		else
			++progmem[idx];
		break;
	case 4:
		if (idx>=progmem.size())
		{
			if (idx>progmem.size())
			{
				applog ("Detected an attempt to decrement a cell more than one cell beyond the last allocated cell. While this does not affect behavior, it may reflect a memory inefficiency.", warn);
				while (idx>progmem.size()) // Sanity check: Ensure that the pointer stays in sync with our memory vector.
					progmem.push_back (0);
				progmem.push_back (255); // Simulate an integer rollaround for a two's complement unsigned char.
			}
		}
		else
			--progmem[idx];
		break;
	case 5:
		putchar (idx>=progmem.size() ? 0 : progmem[idx]); // The ternary operator ensures that, if an access would be outside the bounds of our memory, we pretend that we're outputting a null block.
		if (idx>=progmem.size())
			applog ("Detected an attempt to output a cell beyond the boundary of the allocated memory. Outputting a null byte.", warn);
		fflush (stdout);
		break;
	case 6:
		getchar(); // Pull the \n out of the stream. I know, this confused me too, that's why I added this comment.
		if (idx>=progmem.size())
		{
			if (idx>progmem.size())
			{
				applog ("Detected an attempt to store input to a cell more than one cell beyond the last allocated cell. While this does not affect behavior, it may reflect a memory inefficiency.", warn);
				while (idx>progmem.size()) // Sanity check: Ensure that the pointer stays in sync with our memory vector.
					progmem.push_back (0);
			}
			progmem.push_back (getchar());
		}
		else
			progmem[idx]=getchar();
		break;
	case 7:
		if (idx>=progmem.size())
		{
			applog ("Detected an attempt to loop with a condition beyond the last allocated cell. While this does not affect behavior, the unnecessary burst of null allocations may reduce program performance, especially if this occurs often.", warn);
			while (idx>=progmem.size()) // Sanity check: Ensure that the pointer stays in sync with our memory vector.
				progmem.push_back (0);
		}
		loop ();
	case 8:
		break;
	default:
		return true;
	}
	return false;
}

void catch_ctrl_break (int) throw () // This encapsulator effectively allows reloadCode() to be placed in a signal() call.
{
	reloadCode ();
}

int main (int argc, char* argv[]) throw ()
{
	DBG_ONLY setvbuf (stdout, NULL, _IONBF, 500);
	#ifdef SIGQUIT
	signal (SIGQUIT, catch_ctrl_break);
	#endif
	signal (SIGTERM, catch_ctrl_break);
	if (!(argc==2 || (argc==3 && !(stricmp (argv[1], "-r") && stricmp (argv[1], "--remove") && stricmp (argv[2], "-r") && stricmp (argv[2], "--remove")))))
		usage (argv[0]);
	progfilename=argv[argc==3 ? ((stricmp (argv[1], "-r") && stricmp (argv[1], "--remove")) ? 1 : 2) : 1]; // To support the remove flag.
	logger.open (string(string(progfilename)+".log").c_str(),ios::out|ios::trunc);
	if (!ifstream (progfilename))
	{
		printf ("ERROR: File %s could not be opened for input.\n", progfilename);
		applog (string(string("File ")+progfilename+" not found.").c_str(), err);
		logger.close();
		return 127;
	}
	progmem.reserve (30000); // Reserve the standard 30,000 byte-wide memory space for brainfuck programs. I like that standard (For purposes of memory-efficiency, this could be lowered if so wanted).
	ifstream program (progfilename);
	string command;
	std::streampos start (program.tellg());
	program.seekg (0, ios::end);
	std::streampos end (program.tellg());
	program.seekg (0);
	unsigned char count;
	code.reserve ((end-start)/10); // Reserve enough space for what (I think) should be a good average length for a program, given it's filesize.
	std::stringstream converter;
	converter<<unsigned(end-start);
	char* str (new char [18+converter.str().size()]); // 18 is the number of spaces necessary for our string.
	converter.str ("");
	sprintf (str, "Input filesize: %u.", unsigned(end-start));
	applog (str, info);
	delete[] str;
	getline (program, command);
	applog ("Building program code database...", info);
	while (!program.eof()) // Load the program code into the code vector.
	{
		count=0;
		for (unsigned i=0; i<command.size() && command[i]==' '; ++i)
		{
			++count;
			if (count==9)
			{
				code.push_back (8);
				count=1;
			}
		}
		if (count)
			code.push_back (count);
		getline (program, command);
	}
	logcode ();
	applog ("Done.", info);
	applog ("Closing input file...", info);
	program.close();
	applog ("Done.", info);
	converter<<code.size();
	str=new char [converter.str().size()+39]; // 39 is the number of spaces necessary for our message storage.
	converter.str ("");
	for (; currpos<code.size(); ++currpos)
	{
		sprintf (str, "Executing instruction at location %u: %u.", currpos, code[currpos]);
		applog (str, info);
		if (exec (code[currpos]))
		{
			char str [151]; // 151 should be enough to cover our needs.
			sprintf (str, "Syntax error executing instruction %u, instruction number %u of %u. Program execution: %.2F%% complete.", (unsigned)code[currpos], (unsigned)currpos, (unsigned)code.size(), float(100*currpos)/code.size());
			printf ("%s\n", str);
			applog (str, err);
			exit (100);
		}
		logmemory ();
	}
	if (argc==3) // Easy check for if the remove flag was specified. We just remove the log, so if the program crashes, we still have a debugging logfile.
	{
		logger.close ();
		logger.open (string(string(progfilename)+".log").c_str(), ios::trunc); // Truncate the file, just in case we can't remove it, for some ungodly reason.
		logger.close ();
		remove (string(string(progfilename)+".log").c_str());
	}
	return 0;
}