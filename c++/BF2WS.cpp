// BF2WS.cpp: Provides the main function for the automated translation of brainfuck sourcecode into Whitespace.

#include <fstream>
#include <stdio.h>
#include <string>
using namespace std; // See similar declaration in whitespace.cpp.

void usage (char* name) throw ()
{
	fprintf (stderr,"Usage: %s [--reverse | -r] input output\nNote: input must always come before output", name);
}

const string WS2BF (const char index) throw ()
{
	string out;
	switch (index)
	{
	case 1:
		out=">";
		break;
	case 2:
		out="<";
		break;
	case 3:
		out="+";
		break;
	case 4:
		out="-";
		break;
	case 5:
		out=".";
		break;
	case 6:
		out=",";
		break;
	case 7:
		out="[";
		break;
	case 8:
		out="]";
		break;
	default:
		out=string ("\0\0");
		out[0]=index;
		break;
	}
	return out;
}

const string BF2WS (const char inst) throw ()
{
	const string conv ("><+-.,[]");
	size_t i (conv.find (inst)+1);
	if (i==string::npos)
		return string("")+inst;
	string out ("");
	for (; i; --i)
		out+=" ";
	return out+'\n';
}

int main (int argc, char* argv[]) throw ()
{
	fprintf (stderr, "Checking parameter validity...\n");
	if (argc<3 && argc>4 && (argc==4 && (!(stricmp(argv[1],"-r") || stricmp(argv[1],"--reverse") || stricmp(argv[2],"-r") || stricmp(argv[2],"--reverse") || stricmp(argv[3],"-r") || stricmp(argv[3],"--reverse")))))
	{
		fprintf (stderr, "Fail.\n");
		usage (argv[0]);
		return 1;
	}
	fprintf (stderr, "Parameters are valid.\n");
	bool reverse (false);
	size_t in (1),out (2);
	fprintf (stderr, "Checking for reverse flag...\n");
	if (!(stricmp(argv[1],"-r") && stricmp(argv[1],"--reverse")))
	{
		fprintf (stderr, "Found at position 1.\n");
		reverse=true;
		in=2;
		out=3;
	}
	else if (!(stricmp(argv[2],"-r") && stricmp(argv[2],"--reverse")))
	{
		fprintf (stderr, "Found at position 2.\n");
		reverse=true;
		in=1;
		out=3;
	}
	else if (argc==4 && !(stricmp(argv[3],"-r") && stricmp(argv[3],"--reverse")))
	{
		fprintf (stderr, "Found at position 3.\n");
		reverse=true;
		in=1;
		out=2;
	}
	fprintf (stderr, "Done.\n");
	fprintf (stderr, "Access argv[%u].\n",in);
	string inname (argv[in]);
	fprintf (stderr, "Checking input filename %s...\n", inname.c_str());
	if (!ifstream (inname.c_str()))
	{
		fprintf (stderr, "Fail.\n");
		fprintf (stderr, "ERROR: Input file %s not found.", inname.c_str());
		return 127;
	}
	fprintf (stderr, "Pass.\n");
	fprintf (stderr, "Access argv[%u].\n",out);
	string outname (argv[out]);
	fprintf (stderr, "Setting up data structures...\n");
	const string (*convert) (const char)=reverse ? &WS2BF : &BF2WS;
	ifstream input (inname.c_str());
	inname.clear();
	ofstream output (outname.c_str(), ios::out | ios::trunc);
	outname.clear();
	fprintf (stderr, "Done.\n");
	fprintf (stderr, "Starting conversion.\n");
	if (reverse)
	{
		fprintf (stderr, "Detected reverse mode.\n");
		string line;
		while (!input.eof())
		{
			getline (input, line);
			char count (0);
			for (unsigned i=0; i<line.size() && line[i]==' '; ++i)
			{
				count++;
				if (count==9)
				{
					output<<convert (8);
					count=1;
				}
			}
			output<<convert (count);
		}
	}
	else
	{
		fprintf (stderr, "Detected normal mode.\n");
		while (!input.eof())
			output<<convert (input.get());
	}
	fprintf (stderr, "Done.\n");
	fprintf (stderr, "Cleaning up...\n");
	input.close();
	output.close();
	fprintf (stderr, "Exiting...Goodbye!\n");
	return 0;
}